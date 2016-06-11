#include "./argumentparser.h"
#include "./commandlineutils.h"
#include "./failure.h"

#include "../conversion/stringconversion.h"
#include "../misc/random.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>

using namespace std;
using namespace std::placeholders;
using namespace ConversionUtilities;

/*!
 *  \namespace ApplicationUtilities
 *  \brief Contains currently only ArgumentParser and related classes.
 */
namespace ApplicationUtilities {

/// \cond

const char *applicationName = nullptr;
const char *applicationAuthor = nullptr;
const char *applicationVersion = nullptr;
const char *applicationUrl = nullptr;

inline bool notEmpty(const char *str)
{
    return str && *str;
}

/// \endcond

/*!
 * \class ApplicationUtilities::Argument
 * \brief The Argument class is a wrapper for command line argument information.
 *
 * Instaces of the Argument class are used as definition when parsing command line
 * arguments. Arguments can be assigned to an ArgumentParser using
 * ArgumentParser::setMainArguments() and to another Argument instance using
 * Argument::setSecondaryArguments().
 */

/*!
 * \brief Constructs an Argument with the given \a name, \a abbreviation and \a description.
 *
 * The \a name and the abbreviation mustn't contain any whitespaces.
 * The \a name mustn't be empty. The \a abbreviation and the \a description might be empty.
 */
Argument::Argument(const char *name, char abbreviation, const char *description, const char *example) :
    m_name(name),
    m_abbreviation(abbreviation),
    m_description(description),
    m_example(example),
    m_minOccurrences(0),
    m_maxOccurrences(1),
    m_combinable(false),
    m_denotesOperation(false),
    m_requiredValueCount(0),
    m_default(false),
    m_isMainArg(false)
{}

/*!
 * \brief Destroys the Argument.
 */
Argument::~Argument()
{}

/*!
 * \brief Appends the name, the abbreviation and the description of the Argument to the give ostream.
 */
void Argument::printInfo(ostream &os, unsigned char indentionLevel) const
{
    for(unsigned char i = 0; i < indentionLevel; ++i) os << "  ";
    if(notEmpty(name())) {
        os << '-' << '-' << name();
    }
    if(notEmpty(name()) && abbreviation()) {
        os << ',' << ' ';
    }
    if(abbreviation()) {
        os << '-' << abbreviation();
    }
    if(requiredValueCount() != 0) {
        unsigned int valueNamesPrint = 0;
        for(auto i = valueNames().cbegin(), end = valueNames().cend(); i != end && valueNamesPrint < requiredValueCount(); ++i) {
            os << ' ' << '[' << *i << ']';
            ++valueNamesPrint;
        }
        if(requiredValueCount() == static_cast<size_t>(-1)) {
            os << " ...";
        } else {
            for(; valueNamesPrint < requiredValueCount(); ++valueNamesPrint) {
                os << " [value " << (valueNamesPrint + 1) << ']';
            }
        }
    }
    ++indentionLevel;
    if(notEmpty(description())) {
        os << endl;
        for(unsigned char i = 0; i < indentionLevel; ++i) os << ' ' << ' ';
        os << description();
    }
    if(isRequired()) {
        os << endl;
        for(unsigned char i = 0; i < indentionLevel; ++i) os << ' ' << ' ';
        os << "This argument is required.";
    }
    if(notEmpty(example())) {
        for(unsigned char i = 0; i < indentionLevel; ++i) os << ' ' << ' ';
        os << endl << "Usage: " << example();
    }
    os << endl;
    for(const auto *arg : subArguments()) {
        arg->printInfo(os, indentionLevel + 1);
    }
}

/*!
 * \brief This function return the first present and uncombinable argument of the given list of arguments.
 *
 * The Argument \a except will be ignored.
 */
Argument *firstPresentUncombinableArg(const ArgumentVector &args, const Argument *except)
{
    for(Argument *arg : args) {
        if(arg != except && arg->isPresent() && !arg->isCombinable()) {
            return arg;
        }
    }
    return nullptr;
}

/*!
 * \brief Sets the secondary arguments for this arguments.
 *
 * The given arguments will be considered as secondary arguments of this argument by the argument parser.
 * This means that the parser will complain if these arguments are given, but not this argument.
 * If secondary arguments are labeled as mandatory their parent is also mandatory.
 *
 * The Argument does not take ownership. Do not destroy the given arguments as long as they are
 * used as secondary arguments.
 *
 * \sa secondaryArguments()
 * \sa addSecondaryArgument()
 * \sa hasSecondaryArguments()
 */
void Argument::setSubArguments(const ArgumentInitializerList &secondaryArguments)
{
    // remove this argument from the parents list of the previous secondary arguments
    for(Argument *arg : m_subArgs) {
        arg->m_parents.erase(remove(arg->m_parents.begin(), arg->m_parents.end(), this), arg->m_parents.end());
    }
    // assign secondary arguments
    m_subArgs.assign(secondaryArguments);
    // add this argument to the parents list of the assigned secondary arguments
    // and set the parser
    for(Argument *arg : m_subArgs) {
        if(find(arg->m_parents.cbegin(), arg->m_parents.cend(), this) == arg->m_parents.cend()) {
            arg->m_parents.push_back(this);
        }
    }
}

/*!
 * \brief Adds \a arg as a secondary argument for this argument.
 *
 * \sa secondaryArguments()
 * \sa setSecondaryArguments()
 * \sa hasSecondaryArguments()
 */
void Argument::addSubArgument(Argument *arg)
{
    if(find(m_subArgs.cbegin(), m_subArgs.cend(), arg) == m_subArgs.cend()) {
        m_subArgs.push_back(arg);
        if(find(arg->m_parents.cbegin(), arg->m_parents.cend(), this) == arg->m_parents.cend()) {
            arg->m_parents.push_back(this);
        }
    }
}

/*!
 * \brief Returns whether at least one parent argument is present.
 * \remarks Returns always true for main arguments.
 */
bool Argument::isParentPresent() const
{
    if(isMainArgument()) {
        return true;
    }
    for(const Argument *parent : m_parents) {
        if(parent->isPresent()) {
            return true;
        }
    }
    return false;
}

/*!
 * \brief Checks if this arguments conflicts with other arguments.
 *
 * If the argument is in conflict with an other argument this argument will be returned.
 * Otherwise nullptr will be returned.
 */
Argument *Argument::conflictsWithArgument() const
{
    if(!isCombinable() && isPresent()) {
        for(Argument *parent : m_parents) {
            for(Argument *sibling : parent->subArguments()) {
                if(sibling != this && sibling->isPresent() && !sibling->isCombinable()) {
                    return sibling;
                }
            }
        }
    }
    return nullptr;
}

/*!
 * \brief Resets occurrences and values.
 */
void Argument::reset()
{
    m_indices.clear();
    m_values.clear();
}

/*!
 * \class ApplicationUtilities::ArgumentParser
 * \brief The ArgumentParser class provides a means for handling command line arguments.
 *
 * To setup the parser create instances of ApplicationUtilities::Argument to define a
 * set of known arguments and assign these to the parser using setMainArguments().
 *
 * To invoke parsing call parseArgs(). The parser will verify the previously
 * assigned definitions (and might throw std::invalid_argument) and then parse the
 * given command line arguments according the definitions (and might throw
 * ApplicationUtilities::Failure).
 */

/*!
 * \brief Constructs a new ArgumentParser.
 */
ArgumentParser::ArgumentParser() :
    m_actualArgc(0),
    m_currentDirectory(nullptr),
    m_ignoreUnknownArgs(false)
{}

/*!
 * \brief Sets the main arguments for the parser. The parser will use these argument definitions
 *        to when parsing the command line arguments and when printing help information.
 *
 * \remarks
 * The parser does not take ownership. Do not destroy the arguments as long as they are used as
 * main arguments.
 */
void ArgumentParser::setMainArguments(const ArgumentInitializerList &mainArguments)
{
    for(Argument *arg : mainArguments) {
        arg->m_isMainArg = true;
    }
    m_mainArgs.assign(mainArguments);
}

/*!
 * \brief Adds the specified \a argument to the main argument.
 * \remarks
 * The parser does not take ownership. Do not destroy the argument as long as it is used as
 * main argument.
 */
void ArgumentParser::addMainArgument(Argument *argument)
{
    argument->m_isMainArg = true;
    m_mainArgs.push_back(argument);
}

/*!
 * \brief Prints help information for all main arguments which have been set using setMainArguments().
 */
void ArgumentParser::printHelp(ostream &os) const
{
    if(applicationName && *applicationName) {
        os << applicationName;
        if(applicationVersion && *applicationVersion) {
            os << ',' << ' ';
        }
    }
    if(applicationVersion && *applicationVersion) {
        os << "version " << applicationVersion;
    }
    if((applicationName && *applicationName) || (applicationVersion && *applicationVersion)) {
        os << '\n' << '\n';
    }
    if(!m_mainArgs.empty()) {
        os << "Available arguments:\n";
        for(const auto *arg : m_mainArgs) {
            arg->printInfo(os);
        }
    }
    if(applicationUrl && *applicationUrl) {
        os << "\nProject website: " << applicationUrl << endl;
    }
}

/*!
 * \brief Returns the first argument definition which matches the predicate.
 *
 * The search includes all assigned main argument definitions and their sub arguments.
 */
Argument *ArgumentParser::findArg(const ArgumentPredicate &predicate) const
{
    return findArg(m_mainArgs, predicate);
}

/*!
 * \brief Returns the first argument definition which matches the predicate.
 *
 * The search includes all provided \a arguments and their sub arguments.
 */
Argument *ArgumentParser::findArg(const ArgumentVector &arguments, const ArgumentPredicate &predicate)
{
    for(Argument *arg : arguments) {
        if(predicate(arg)) {
            return arg; // argument matches
        } else if(Argument *subarg = findArg(arg->subArguments(), predicate)) {
            return subarg; // a secondary argument matches
        }
    }
    return nullptr; // no argument matches
}

/*!
 * \brief Parses the specified command line arguments.
 * \remarks
 *  - The results are stored in the Argument instances assigned as main arguments and sub arguments.
 *  - Calls the assigned callbacks if no constraints are violated.
 * \throws Throws Failure if the specified arguments violate the constraints defined
 *         by the Argument instances.
 */
void ArgumentParser::parseArgs(int argc, const char *argv[])
{
    IF_DEBUG_BUILD(verifyArgs(m_mainArgs);)
    m_actualArgc = 0;
    if(argc > 0) {
        m_currentDirectory = *argv;
        size_t index = 0;
        ++argv;
        readSpecifiedArgs(m_mainArgs, index, argv, argv + argc - 1);
        checkConstraints(m_mainArgs);
        invokeCallbacks(m_mainArgs);
    } else {
        m_currentDirectory = nullptr;
    }
}

#ifdef DEBUG_BUILD
/*!
 * \brief Verifies the specified \a argument definitions.
 *
 * Asserts that
 *  - The same argument has not been added twice to the same parent.
 *  - Only one argument within a parent is default or implicit.
 *  - Only main arguments denote operations.
 *  - Argument abbreviations are unique within one parent.
 *  - Argument names are unique within one parent.
 *
 * \remarks
 *  - Verifies the sub arguments, too.
 *  - For debugging purposes only; hence only available in debug builds.
 */
void ApplicationUtilities::ArgumentParser::verifyArgs(const ArgumentVector &args)
{
    vector<const Argument *> verifiedArgs;
    verifiedArgs.reserve(args.size());
    vector<char> abbreviations;
    abbreviations.reserve(args.size());
    vector<string> names;
    names.reserve(args.size());
    bool hasDefault = false;
    for(const Argument *arg : args) {
        assert(find(verifiedArgs.cbegin(), verifiedArgs.cend(), arg) == verifiedArgs.cend());
        verifiedArgs.push_back(arg);
        assert(arg->isMainArgument() || !arg->denotesOperation());
        assert(!arg->isDefault() || !hasDefault);
        hasDefault |= arg->isDefault();
        assert(!arg->abbreviation() || find(abbreviations.cbegin(), abbreviations.cend(), arg->abbreviation()) == abbreviations.cend());
        abbreviations.push_back(arg->abbreviation());
        assert(!arg->name() || find(names.cbegin(), names.cend(), arg->name()) == names.cend());
        assert(arg->requiredValueCount() == 0 || arg->subArguments().size() == 0);
        names.emplace_back(arg->name());
        verifyArgs(arg->subArguments());
    }
}
#endif

/*!
 * \brief Reads the specified commands line arguments.
 * \remarks Results are stored in Argument instances added as main arguments and sub arguments.
 */
void ArgumentParser::readSpecifiedArgs(ArgumentVector &args, std::size_t &index, const char **&argv, const char **end)
{
    enum ArgumentDenotationType : unsigned char {
        Value = 0, // parameter value
        Abbreviation = 1, // argument abbreviation
        FullName = 2 // full argument name
    };

    bool isTopLevel = index == 0;
    Argument *lastArg = nullptr;
    vector<const char *> *values = nullptr;
    while(argv != end) {
        if(values && lastArg->requiredValueCount() != static_cast<size_t>(-1) && values->size() < lastArg->requiredValueCount()) {
            // there are still values to read
            values->emplace_back(*argv);
            ++index, ++argv;
        } else {
            // determine denotation type
            const char *argDenotation = *argv;
            bool abbreviationFound = false;
            unsigned char argDenotationType = Value;
            *argDenotation == '-' && (++argDenotation, ++argDenotationType)
                    && *argDenotation == '-' && (++argDenotation, ++argDenotationType);

            // try to find matching Argument instance
            Argument *matchingArg = nullptr;
            if(argDenotationType != Value) {
                const char *const equationPos = strchr(argDenotation, '=');
                for(const auto argDenLen = equationPos ? static_cast<size_t>(equationPos - argDenotation) : strlen(argDenotation); ; matchingArg = nullptr) {
                    // search for arguments by abbreviation or name depending on the denotation type
                    if(argDenotationType == Abbreviation) {
                        for(Argument *arg : args) {
                            if(arg->abbreviation() && arg->abbreviation() == *argDenotation) {
                                matchingArg = arg;
                                abbreviationFound = true;
                                break;
                            }
                        }
                    } else {
                        for(Argument *arg : args) {
                            if(arg->name() && !strncmp(arg->name(), argDenotation, argDenLen)) {
                                matchingArg = arg;
                                break;
                            }
                        }
                    }

                    if(matchingArg) {
                        // an argument matched the specified denotation
                        matchingArg->m_indices.push_back(index);

                        // prepare reading parameter values
                        matchingArg->m_values.emplace_back();
                        values = &matchingArg->m_values.back();
                        if(equationPos) {
                            values->push_back(equationPos + 1);
                        }

                        // read sub arguments if no abbreviated argument follows
                        ++index, ++m_actualArgc, lastArg = matchingArg;
                        if(argDenotationType != Abbreviation || (!*++argDenotation && argDenotation != equationPos)) {
                            readSpecifiedArgs(matchingArg->m_subArgs, index, ++argv, end);
                            break;
                        } // else: another abbreviated argument follows
                    } else {
                        break;
                    }
                }
            }

            if(!matchingArg) {
                if(lastArg && values->size() < lastArg->requiredValueCount()) {
                    // treat unknown argument as parameter of last argument
                    values->emplace_back(abbreviationFound ? argDenotation : *argv);
                    ++index, ++argv;
                    continue;
                } else {
                    // first value might denote "operation"
                    if(isTopLevel) {
                        for(Argument *arg : args) {
                            if(arg->denotesOperation() && arg->name() && !strcmp(arg->name(), *argv)) {
                                (matchingArg = arg)->m_indices.push_back(index);
                                ++index, ++argv;
                                break;
                            }
                        }
                    }

                    if(!matchingArg) {
                        // use the first default argument
                        for(Argument *arg : args) {
                            if(arg->isDefault()) {
                                (matchingArg = arg)->m_indices.push_back(index);
                                break;
                            }
                        }
                    }
                    if(matchingArg) {
                        // an argument matched the specified denotation
                        if(lastArg == matchingArg) {
                            break;
                        }                        

                        // prepare reading parameter values
                        matchingArg->m_values.emplace_back();
                        values = &matchingArg->m_values.back();

                        // read sub arguments if no abbreviated argument follows
                        ++m_actualArgc, lastArg = matchingArg;
                        readSpecifiedArgs(matchingArg->m_subArgs, index, argv, end);
                        continue;
                    }
                }
                if(isTopLevel) {
                    if(m_ignoreUnknownArgs) {
                        cerr << "The specified argument \"" << *argv << "\" is unknown and will be ignored." << endl;
                        ++index, ++argv;
                    } else {
                        throw Failure("The specified argument \"" + string(*argv) + "\" is unknown and will be ignored.");
                    }
                } else {
                    return; // unknown argument name or abbreviation found -> continue with parent level
                }
            }
        }
    }
}

/*!
 * \brief Checks the constrains of the specified \a args.
 * \remarks Checks the contraints of sub arguments, too.
 */
void ArgumentParser::checkConstraints(const ArgumentVector &args)
{
    for(const Argument *arg : args) {
        const auto occurrences = arg->occurrences();
        if(arg->isParentPresent() && occurrences > arg->maxOccurrences()) {
            throw Failure("The argument \"" + string(arg->name()) + "\" mustn't be specified more than " + numberToString(arg->maxOccurrences()) + (arg->maxOccurrences() == 1 ? " time." : " times."));
        }
        if(arg->isParentPresent() && occurrences < arg->minOccurrences()) {
            throw Failure("The argument \"" + string(arg->name()) + "\" must be specified at least " + numberToString(arg->minOccurrences()) + (arg->minOccurrences() == 1 ? " time." : " times."));
        }
        Argument *conflictingArgument = nullptr;
        if(arg->isMainArgument()) {
            if(!arg->isCombinable() && arg->isPresent()) {
                conflictingArgument = firstPresentUncombinableArg(m_mainArgs, arg);
            }
        } else {
            conflictingArgument = arg->conflictsWithArgument();
        }
        if(conflictingArgument) {
            throw Failure("The argument \"" + string(conflictingArgument->name()) + "\" can not be combined with \"" + arg->name() + "\".");
        }
        for(size_t i = 0; i != occurrences; ++i) {
            if(!arg->allRequiredValuesPresent(occurrences)) {
                stringstream ss(stringstream::in | stringstream::out);
                ss << "Not all parameter for argument \"" << arg->name() << "\" ";
                if(i) {
                    ss << " (" << (i + 1) << " occurrence) ";
                }
                ss << "provided. You have to provide the following parameter:";
                size_t valueNamesPrint = 0;
                for(const auto &name : arg->m_valueNames) {
                    ss << "\n" << name;
                    ++valueNamesPrint;
                }
                if(arg->m_requiredValueCount != static_cast<size_t>(-1)) {
                    while(valueNamesPrint < arg->m_requiredValueCount) {
                        ss << "\nvalue " << (++valueNamesPrint);
                    }
                }
                throw Failure(ss.str());
            }
        }

        // check contraints of sub arguments recursively
        checkConstraints(arg->m_subArgs);
    }
}

/*!
 * \brief Invokes the callbacks for the specified \a args.
 * \remarks
 *  - Checks the callbacks for sub arguments, too.
 *  - Invokes the assigned callback methods for each occurance of
 *    the argument.
 */
void ArgumentParser::invokeCallbacks(const ArgumentVector &args)
{
    for(const Argument *arg : args) {
        // invoke the callback for each occurance of the argument
        if(arg->m_callbackFunction) {
            for(const auto &valuesOfOccurance : arg->m_values) {
                if(arg->isDefault() && valuesOfOccurance.empty()) {
                    arg->m_callbackFunction(arg->defaultValues());
                } else {
                    arg->m_callbackFunction(valuesOfOccurance);
                }
            }
        }
        // invoke the callbacks for sub arguments recursively
        invokeCallbacks(arg->m_subArgs);
    }
}

/*!
 * \class HelpArgument
 * \brief The HelpArgument class prints help information for an argument parser
 *        when present (--help, -h).
 */

/*!
 * \brief Constructs a new help argument for the specified parser.
 */
HelpArgument::HelpArgument(ArgumentParser &parser) :
    Argument("help", 'h', "shows this information")
{
    setCallback([&parser] (const std::vector<const char *> &) {
        CMD_UTILS_START_CONSOLE;
        parser.printHelp(cout);
    });
}

}
