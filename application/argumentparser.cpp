#include "./argumentparser.h"
#include "./commandlineutils.h"
#include "./failure.h"

#include "../conversion/stringconversion.h"
#include "../io/path.h"
#include "../io/ansiescapecodes.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdlib>
#ifdef LOGGING_ENABLED
# include <fstream>
#endif

using namespace std;
using namespace std::placeholders;
using namespace ConversionUtilities;
using namespace IoUtilities;

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
 * \brief The ArgumentDenotationType enum specifies the type of a given argument denotation.
 */
enum ArgumentDenotationType : unsigned char {
    Value = 0, /**< parameter value */
    Abbreviation = 1, /**< argument abbreviation */
    FullName = 2 /**< full argument name */
};

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
    m_environmentVar(nullptr),
    m_description(description),
    m_example(example),
    m_minOccurrences(0),
    m_maxOccurrences(1),
    m_combinable(false),
    m_denotesOperation(false),
    m_requiredValueCount(0),
    m_implicit(false),
    m_isMainArg(false),
    m_valueCompletionBehavior(ValueCompletionBehavior::PreDefinedValues | ValueCompletionBehavior::Files | ValueCompletionBehavior::Directories | ValueCompletionBehavior::FileSystemIfNoPreDefinedValues),
    m_preDefinedCompletionValues(nullptr)
{}

/*!
 * \brief Destroys the Argument.
 */
Argument::~Argument()
{}

/*!
 * \brief Returns the first parameter value of the first occurrence of the argument.
 * \remarks
 * - If the argument is not present and the an environment variable has been set
 *   using setEnvironmentVariable() the value of the specified variable will be returned.
 * - Returns nullptr if no value is available though.
 */
const char *Argument::firstValue() const
{
    if(!m_occurrences.empty() && !m_occurrences.front().values.empty()) {
        return m_occurrences.front().values.front();
    } else if(m_environmentVar) {
        return getenv(m_environmentVar);
    } else {
        return nullptr;
    }
}

/*!
 * \brief Writes the name, the abbreviation and other information about the Argument to the give ostream.
 */
void Argument::printInfo(ostream &os, unsigned char indentionLevel) const
{
    for(unsigned char i = 0; i < indentionLevel; ++i) os << ' ' << ' ';
    EscapeCodes::setStyle(os, EscapeCodes::TextAttribute::Bold);
    if(notEmpty(name())) {
        os << '-' << '-' << name();
    }
    if(notEmpty(name()) && abbreviation()) {
        os << ',' << ' ';
    }
    if(abbreviation()) {
        os << '-' << abbreviation();
    }
    EscapeCodes::setStyle(os);
    if(requiredValueCount()) {
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
    m_executable(nullptr),
    m_unknownArgBehavior(UnknownArgumentBehavior::Fail),
    m_defaultArg(nullptr)
{}

/*!
 * \brief Sets the main arguments for the parser. The parser will use these argument definitions
 *        to when parsing the command line arguments and when printing help information.
 * \remarks
 *  - The parser does not take ownership. Do not destroy the arguments as long as they are used as
 *    main arguments.
 *  - Sets the first specified argument as default argument if none has been assigned yet and the
 *    first argument does not require any values or has no mandatory sub arguments.
 */
void ArgumentParser::setMainArguments(const ArgumentInitializerList &mainArguments)
{
    if(mainArguments.size()) {
        for(Argument *arg : mainArguments) {
            arg->m_isMainArg = true;
        }
        m_mainArgs.assign(mainArguments);
        if(!m_defaultArg) {
            if(!(*mainArguments.begin())->requiredValueCount()) {
                bool subArgsRequired = false;
                for(const Argument *subArg : (*mainArguments.begin())->subArguments()) {
                    if(subArg->isRequired()) {
                        subArgsRequired = true;
                        break;
                    }
                }
                if(!subArgsRequired) {
                    m_defaultArg = *mainArguments.begin();
                }
            }
        }
    } else {
        m_mainArgs.clear();
    }
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
 * \brief Prints help text for all assigned arguments.
 */
void ArgumentParser::printHelp(ostream &os) const
{
    EscapeCodes::setStyle(os, EscapeCodes::TextAttribute::Bold);
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
    EscapeCodes::setStyle(os);
    if(!m_mainArgs.empty()) {
        os << "Available arguments:";
        for(const Argument *arg : m_mainArgs) {
            os << '\n';
            arg->printInfo(os);
        }
    }
    if(applicationUrl && *applicationUrl) {
        os << "\nProject website: " << applicationUrl << endl;
    }
}

/*!
 * \brief Parses the specified command line arguments.
 * \remarks
 *  - The results are stored in the Argument instances assigned as main arguments and sub arguments.
 *  - Calls the assigned callbacks if no constraints are violated.
 * \throws Throws Failure if the specified arguments violate the constraints defined
 *         by the Argument instances.
 */
void ArgumentParser::parseArgs(int argc, const char *const *argv)
{
#ifdef LOGGING_ENABLED
    {
        fstream logFile("/tmp/args.log", ios_base::out);
        for(const char *const *i = argv, *const *end = argv + argc; i != end; ++i) {
            logFile << *i << '\n';
        }
    }
#endif
    IF_DEBUG_BUILD(verifyArgs(m_mainArgs);)
            m_actualArgc = 0;
    if(argc) {
        // the first argument is the executable name
        m_executable = *argv;

        // check for further arguments
        if(--argc) {
            // if the first argument (after executable name) is "--bash-completion-for" bash completion for the following arguments is requested
            bool completionMode = !strcmp(*++argv, "--bash-completion-for");
            unsigned int currentWordIndex;
            if(completionMode) {
                // the first argument after "--bash-completion-for" is the index of the current word
                try {
                    currentWordIndex = (--argc ? stringToNumber<unsigned int, string>(*(++argv)) : 0);
                    ++argv, --argc;
                } catch(const ConversionException &) {
                    currentWordIndex = argc - 1;
                }
            }

            // those variables are modified by readSpecifiedArgs() and reflect the current reading position
            size_t index = 0;
            Argument *lastDetectedArgument = nullptr;

            // read specified arguments
            try {
                const char *const *argv2 = argv;
                readSpecifiedArgs(m_mainArgs, index, argv2, argv + (completionMode ? min(static_cast<unsigned int>(argc), currentWordIndex + 1) : static_cast<unsigned int>(argc)), lastDetectedArgument, completionMode);
            } catch(const Failure &) {
                if(!completionMode) {
                    throw;
                }
            }

            if(completionMode) {
                printBashCompletion(argc, argv, currentWordIndex, lastDetectedArgument);
                exit(0); // prevent the applicaton to continue with the regular execution
            }
        } else {
            // no arguments specified -> flag default argument as present if one is assigned
            if(m_defaultArg) {
                m_defaultArg->m_occurrences.emplace_back(0);
            }
        }
        checkConstraints(m_mainArgs);
        invokeCallbacks(m_mainArgs);
    } else {
        m_executable = nullptr;
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
    bool hasImplicit = false;
    for(const Argument *arg : args) {
        assert(find(verifiedArgs.cbegin(), verifiedArgs.cend(), arg) == verifiedArgs.cend());
        verifiedArgs.push_back(arg);
        assert(arg->isMainArgument() || !arg->denotesOperation());
        assert(!arg->isImplicit() || !hasImplicit);
        hasImplicit |= arg->isImplicit();
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
void ArgumentParser::readSpecifiedArgs(ArgumentVector &args, std::size_t &index, const char *const *&argv, const char *const *end, Argument *&lastArg, bool completionMode)
{
    Argument *const parentArg = lastArg;
    const vector<Argument *> &parentPath = parentArg ? parentArg->path(parentArg->occurrences() - 1) : vector<Argument *>();
    Argument *lastArgInLevel = nullptr;
    vector<const char *> *values = nullptr;
    while(argv != end) {
        if(values && lastArgInLevel->requiredValueCount() != static_cast<size_t>(-1) && values->size() < lastArgInLevel->requiredValueCount()) {
            // there are still values to read
            values->emplace_back(*argv);
            ++index, ++argv;
        } else {
            // determine denotation type
            const char *argDenotation = *argv;
            if(!*argDenotation && (!lastArgInLevel || values->size() >= lastArgInLevel->requiredValueCount())) {
                // skip empty arguments
                ++index, ++argv;
                continue;
            }
            bool abbreviationFound = false;
            unsigned char argDenotationType = Value;
            *argDenotation == '-' && (++argDenotation, ++argDenotationType)
                    && *argDenotation == '-' && (++argDenotation, ++argDenotationType);

            // try to find matching Argument instance
            Argument *matchingArg = nullptr;
            size_t argDenLen;
            if(argDenotationType != Value) {
                const char *const equationPos = strchr(argDenotation, '=');
                for(argDenLen = equationPos ? static_cast<size_t>(equationPos - argDenotation) : strlen(argDenotation); argDenLen; matchingArg = nullptr) {
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
                            if(arg->name() && !strncmp(arg->name(), argDenotation, argDenLen) && *(arg->name() + argDenLen) == '\0') {
                                matchingArg = arg;
                                break;
                            }
                        }
                    }

                    if(matchingArg) {
                        // an argument matched the specified denotation
                        matchingArg->m_occurrences.emplace_back(index, parentPath, parentArg);

                        // prepare reading parameter values
                        values = &matchingArg->m_occurrences.back().values;
                        if(equationPos) {
                            values->push_back(equationPos + 1);
                        }

                        // read sub arguments if no abbreviated argument follows
                        ++index, ++m_actualArgc, lastArg = lastArgInLevel = matchingArg;
                        if(argDenotationType != Abbreviation || (!*++argDenotation && argDenotation != equationPos)) {
                            readSpecifiedArgs(matchingArg->m_subArgs, index, ++argv, end, lastArg, completionMode);
                            break;
                        } // else: another abbreviated argument follows
                    } else {
                        break;
                    }
                }
            }

            if(!matchingArg) {
                if(argDenotationType != Value) {
                    // unknown argument might be a sibling of the parent element
                    for(auto parentArgument = parentPath.crbegin(), pathEnd = parentPath.crend(); ; ++parentArgument) {
                        for(Argument *sibling : (parentArgument != pathEnd ? (*parentArgument)->subArguments() : m_mainArgs)) {
                            if(sibling->occurrences() < sibling->maxOccurrences()) {
                                if((argDenotationType == Abbreviation && (sibling->abbreviation() && sibling->abbreviation() == *argDenotation))
                                        || (sibling->name() && !strncmp(sibling->name(), argDenotation, argDenLen))) {
                                    return;
                                }
                            }
                        }
                        if(parentArgument == pathEnd) {
                            break;
                        }
                    };
                }

                if(lastArgInLevel && values->size() < lastArgInLevel->requiredValueCount()) {
                    // unknown argument might just be a parameter of the last argument
                    values->emplace_back(abbreviationFound ? argDenotation : *argv);
                    ++index, ++argv;
                    continue;
                }

                // first value might denote "operation"
                if(!index) {
                    for(Argument *arg : args) {
                        if(arg->denotesOperation() && arg->name() && !strcmp(arg->name(), *argv)) {
                            (matchingArg = arg)->m_occurrences.emplace_back(index, parentPath, parentArg);
                            ++index, ++argv;
                            break;
                        }
                    }
                }

                if(!matchingArg && (!completionMode || (argv + 1 != end))) {
                    // use the first default argument which is not already present
                    for(Argument *arg : args) {
                        if(arg->isImplicit() && !arg->isPresent()) {
                            (matchingArg = arg)->m_occurrences.emplace_back(index, parentPath, parentArg);
                            break;
                        }
                    }
                }

                if(matchingArg) {
                    // an argument matched the specified denotation
                    if(lastArgInLevel == matchingArg) {
                        break; // TODO: why?
                    }

                    // prepare reading parameter values
                    values = &matchingArg->m_occurrences.back().values;

                    // read sub arguments
                    ++m_actualArgc, lastArg = lastArgInLevel = matchingArg;
                    readSpecifiedArgs(matchingArg->m_subArgs, index, argv, end, lastArg, completionMode);
                    continue;
                }

                if(!parentArg) {
                    if(completionMode) {
                        ++index, ++argv;
                    } else {
                        switch(m_unknownArgBehavior) {
                        case UnknownArgumentBehavior::Warn:
                            cerr << "The specified argument \"" << *argv << "\" is unknown and will be ignored." << endl;
                            FALLTHROUGH;
                        case UnknownArgumentBehavior::Ignore:
                            ++index, ++argv;
                            break;
                        case UnknownArgumentBehavior::Fail:
                            throw Failure("The specified argument \"" + string(*argv) + "\" is unknown and will be ignored.");
                        }
                    }
                } else {
                    return; // unknown argument name or abbreviation found -> continue with parent level
                }
            }
        }
    }
}
/*!
 * \brief Returns whether \a arg1 should be listed before \a arg2 when
 *        printing completion.
 *
 * Arguments are sorted by name (ascending order). However, all arguments
 * denoting an operation are listed before all other arguments.
 */
bool compareArgs(const Argument *arg1, const Argument *arg2)
{
    if(arg1->denotesOperation() && !arg2->denotesOperation()) {
        return true;
    } else if(!arg1->denotesOperation() && arg2->denotesOperation()) {
        return false;
    } else {
        return strcmp(arg1->name(), arg2->name()) < 0;
    }
}

/*!
 * \brief Inserts the specified \a siblings in the \a target list.
 * \remarks Only inserts siblings which could still occur at least once more.
 */
void insertSiblings(const ArgumentVector &siblings, list<const Argument *> &target)
{
    bool onlyCombinable = false;
    for(const Argument *sibling : siblings) {
        if(sibling->isPresent() && !sibling->isCombinable()) {
            onlyCombinable = true;
            break;
        }
    }
    for(const Argument *sibling : siblings) {
        if((!onlyCombinable || sibling->isCombinable()) && sibling->occurrences() < sibling->maxOccurrences()) {
            target.push_back(sibling);
        }
    }
}

/*!
 * \brief Prints the bash completion for the specified arguments and the specified \a lastPath.
 * \remarks Arguments must have been parsed before with readSpecifiedArgs(). When calling this method, completionMode must
 *          be set to true.
 */
void ArgumentParser::printBashCompletion(int argc, const char *const *argv, unsigned int currentWordIndex, const Argument *lastDetectedArg)
{
    // variables to store relevant completions (arguments, pre-defined values, files/dirs)
    list<const Argument *> relevantArgs, relevantPreDefinedValues;
    bool completeFiles = false, completeDirs = false, noWhitespace = false;

    // get the last argument the argument parser was able to detect successfully
    size_t lastDetectedArgIndex;
    vector<Argument *> lastDetectedArgPath;
    if(lastDetectedArg) {
        lastDetectedArgIndex = lastDetectedArg->index(lastDetectedArg->occurrences() - 1);
        lastDetectedArgPath = lastDetectedArg->path(lastDetectedArg->occurrences() - 1);
    }

    bool nextArgumentOrValue;
    const char *const *lastSpecifiedArg;
    unsigned int lastSpecifiedArgIndex;
    if(argc) {
        // determine last arg omitting trailing empty args
        lastSpecifiedArgIndex = static_cast<unsigned int>(argc) - 1;
        lastSpecifiedArg = argv + lastSpecifiedArgIndex;
        for(; lastSpecifiedArg >= argv && **lastSpecifiedArg == '\0'; --lastSpecifiedArg, --lastSpecifiedArgIndex);
    }

    if(lastDetectedArg && lastDetectedArg->isPresent()) {
        if((nextArgumentOrValue = currentWordIndex > lastDetectedArgIndex)) {
            // parameter values of the last arg are possible completions
            auto currentValueCount = lastDetectedArg->values(lastDetectedArg->occurrences() - 1).size();
            if(currentValueCount) {
                currentValueCount -= (currentWordIndex - lastDetectedArgIndex);
            }
            if(lastDetectedArg->requiredValueCount() == static_cast<size_t>(-1) || (currentValueCount < lastDetectedArg->requiredValueCount())) {
                if(lastDetectedArg->valueCompletionBehaviour() & ValueCompletionBehavior::PreDefinedValues) {
                    relevantPreDefinedValues.push_back(lastDetectedArg);
                }
                if(!(lastDetectedArg->valueCompletionBehaviour() & ValueCompletionBehavior::FileSystemIfNoPreDefinedValues) || !lastDetectedArg->preDefinedCompletionValues()) {
                    completeFiles = completeFiles || lastDetectedArg->valueCompletionBehaviour() & ValueCompletionBehavior::Files;
                    completeDirs = completeDirs || lastDetectedArg->valueCompletionBehaviour() & ValueCompletionBehavior::Directories;
                }
            }

            if(lastDetectedArg->requiredValueCount() == static_cast<size_t>(-1) || lastDetectedArg->values(lastDetectedArg->occurrences() - 1).size() >= lastDetectedArg->requiredValueCount()) {
                // sub arguments of the last arg are possible completions
                for(const Argument *subArg : lastDetectedArg->subArguments()) {
                    if(subArg->occurrences() < subArg->maxOccurrences()) {
                        relevantArgs.push_back(subArg);
                    }
                }

                // siblings of parents are possible completions as well
                for(auto parentArgument = lastDetectedArgPath.crbegin(), end = lastDetectedArgPath.crend(); ; ++parentArgument) {
                    insertSiblings(parentArgument != end ? (*parentArgument)->subArguments() : m_mainArgs, relevantArgs);
                    if(parentArgument == end) {
                        break;
                    }
                }
            }
        } else {
            // since the argument could be detected (hopefully unambiguously?) just return it for "final completion"
            relevantArgs.push_back(lastDetectedArg);
        }

    } else {
        nextArgumentOrValue = true;
        insertSiblings(m_mainArgs, relevantArgs);
    }

    // read the "opening" (started but not finished argument denotation)
    const char *opening = nullptr;
    string compoundOpening;
    size_t openingLen, compoundOpeningStartLen = 0;
    unsigned char openingDenotationType = Value;
    if(argc && nextArgumentOrValue) {
        if(currentWordIndex < argc) {
            opening = argv[currentWordIndex];
            // For some reasons completions for eg. "set --values disk=1 tag=a" are splitted so the
            // equation sign is an own argument ("set --values disk = 1 tag = a").
            // This is not how values are treated by the argument parser. Hence the opening
            // must be joined again. In this case only the part after the equation sign needs to be
            // provided for completion so compoundOpeningStartLen is set to number of characters to skip.
            size_t minCurrentWordIndex = (lastDetectedArg ? lastDetectedArgIndex : 0);
            if(currentWordIndex > minCurrentWordIndex && !strcmp(opening, "=")) {
                compoundOpening.reserve(compoundOpeningStartLen = strlen(argv[--currentWordIndex]) + 1);
                compoundOpening = argv[currentWordIndex];
                compoundOpening += '=';
            } else if(currentWordIndex > (minCurrentWordIndex + 1) && !strcmp(argv[currentWordIndex - 1], "=")) {
                compoundOpening.reserve((compoundOpeningStartLen = strlen(argv[currentWordIndex -= 2]) + 1) + strlen(opening));
                compoundOpening = argv[currentWordIndex];
                compoundOpening += '=';
                compoundOpening += opening;
            }
            if(!compoundOpening.empty()) {
                opening = compoundOpening.data();
            }
        } else {
            opening = *lastSpecifiedArg;
        }
        *opening == '-' && (++opening, ++openingDenotationType)
                && *opening == '-' && (++opening, ++openingDenotationType);
        openingLen = strlen(opening);
    }

    relevantArgs.sort(compareArgs);

    // print "COMPREPLY" bash array
    cout << "COMPREPLY=(";
    // -> completions for parameter values
    for(const Argument *arg : relevantPreDefinedValues) {
        if(arg->preDefinedCompletionValues()) {
            bool appendEquationSign = arg->valueCompletionBehaviour() & ValueCompletionBehavior::AppendEquationSign;
            if(argc && currentWordIndex <= lastSpecifiedArgIndex && opening) {
                if(openingDenotationType == Value) {
                    bool wordStart = true, ok = false, equationSignAlreadyPresent = false;
                    int wordIndex = 0;
                    for(const char *i = arg->preDefinedCompletionValues(), *end = opening + openingLen; *i;) {
                        if(wordStart) {
                            const char *i1 = i, *i2 = opening;
                            for(; *i1 && i2 != end && *i1 == *i2; ++i1, ++i2);
                            ok = (i2 == end);
                            wordStart = false;
                            wordIndex = 0;
                        } else if((wordStart = (*i == ' ') || (*i == '\n'))) {
                            equationSignAlreadyPresent = false;
                        } else if(*i == '=') {
                            equationSignAlreadyPresent = true;
                        }
                        if(ok) {
                            if(!compoundOpeningStartLen || wordIndex >= compoundOpeningStartLen) {
                                cout << *i;
                            }
                            ++i, ++wordIndex;
                            if(appendEquationSign && !equationSignAlreadyPresent) {
                                switch(*i) {
                                case ' ': case '\n': case '\0':
                                    cout << '=';
                                    noWhitespace = true;
                                    equationSignAlreadyPresent = false;
                                }
                            }
                        } else {
                            ++i;
                        }
                    }
                    cout << ' ';
                }
            } else if(appendEquationSign) {
                bool equationSignAlreadyPresent = false;
                for(const char *i = arg->preDefinedCompletionValues(); *i;) {
                    cout << *i;
                    switch(*(++i)) {
                    case '=':
                        equationSignAlreadyPresent = true;
                        break;
                    case ' ': case '\n': case '\0':
                        if(!equationSignAlreadyPresent) {
                            cout << '=';
                            equationSignAlreadyPresent = false;
                        }
                    }
                }
            } else {
                cout << arg->preDefinedCompletionValues() << ' ';
            }
        }
    }
    // -> completions for further arguments
    for(const Argument *arg : relevantArgs) {
        if(argc && currentWordIndex <= lastSpecifiedArgIndex && opening) {
            switch(openingDenotationType) {
            case Value:
                if(!arg->denotesOperation() || strncmp(arg->name(), opening, openingLen)) {
                    continue;
                }
                break;
            case Abbreviation:
                break;
            case FullName:
                if(strncmp(arg->name(), opening, openingLen)) {
                    continue;
                }
            }
        }

        if(openingDenotationType == Abbreviation && opening) {
            cout << '-' << opening << arg->abbreviation() << ' ';
        } else if(arg->denotesOperation() && (!actualArgumentCount() || (currentWordIndex == 0 && (!lastDetectedArg || (lastDetectedArg->isPresent() && lastDetectedArgIndex == 0))))) {
            cout << arg->name() << ' ';
        } else {
            cout << '-' << '-' << arg->name() << ' ';
        }
    }
    // -> completions for files and dirs
    // -> if there's already an "opening", determine the dir part and the file part
    string actualDir, actualFile;
    bool haveFileOrDirCompletions = false;
    if(argc && currentWordIndex == lastSpecifiedArgIndex && opening) {
        // the "opening" might contain escaped characters which need to be unescaped first
        string unescapedOpening(opening);
        findAndReplace<string>(unescapedOpening, "\\ ", " ");
        findAndReplace<string>(unescapedOpening, "\\,", ",");
        findAndReplace<string>(unescapedOpening, "\\[", "[");
        findAndReplace<string>(unescapedOpening, "\\]", "]");
        findAndReplace<string>(unescapedOpening, "\\!", "!");
        findAndReplace<string>(unescapedOpening, "\\#", "#");
        findAndReplace<string>(unescapedOpening, "\\$", "$");
        // determine the "directory" part
        string dir = directory(unescapedOpening);
        if(dir.empty()) {
            actualDir = ".";
        } else {
            if(dir[0] == '\"' || dir[0] == '\'') {
                dir.erase(0, 1);
            }
            if(dir.size() > 1 && (dir[dir.size() - 2] == '\"' || dir[dir.size() - 2] == '\'')) {
                dir.erase(dir.size() - 2, 1);
            }
            actualDir = move(dir);
        }
        // determine the "file" part
        string file = fileName(unescapedOpening);
        if(file[0] == '\"' || file[0] == '\'') {
            file.erase(0, 1);
        }
        if(file.size() > 1 && (file[dir.size() - 2] == '\"' || dir[file.size() - 2] == '\'')) {
            file.erase(file.size() - 2, 1);
        }
        actualFile = move(file);
    }
    // -> completion for files
    if(completeFiles) {
        if(argc && currentWordIndex <= lastSpecifiedArgIndex && opening) {
            for(const string &dirEntry : directoryEntries(actualDir.c_str(), DirectoryEntryType::File)) {
                if(startsWith(dirEntry, actualFile)) {
                    cout << '\'';
                    if(actualDir != ".") {
                        cout << actualDir;
                    }
                    cout << dirEntry << '\'' << ' ';
                    haveFileOrDirCompletions = true;
                }
            }
        } else {
            for(const string &dirEntry : directoryEntries(".", DirectoryEntryType::File)) {
                cout << dirEntry << ' ';
                haveFileOrDirCompletions = true;
            }
        }
    }
    // -> completion for dirs
    if(completeDirs) {
        if(argc && currentWordIndex <= lastSpecifiedArgIndex && opening) {
            for(const string &dirEntry : directoryEntries(actualDir.c_str(), DirectoryEntryType::Directory)) {
                if(startsWith(dirEntry, actualFile)) {
                    cout << '\'';
                    if(actualDir != ".") {
                        cout << actualDir;
                    }
                    cout << dirEntry << '\'' << ' ';
                    haveFileOrDirCompletions = true;
                }
            }
        } else {
            for(const string &dirEntry : directoryEntries(".", DirectoryEntryType::Directory)) {
                cout << '\'' << dirEntry << '/' << '\'' << ' ';
                haveFileOrDirCompletions = true;
            }
        }
    }
    cout << ')';

    // ensure file or dir completions are formatted appropriately
    if(haveFileOrDirCompletions) {
        cout << "; compopt -o filenames";
    }

    // ensure trailing whitespace is ommitted
    if(noWhitespace) {
        cout << "; compopt -o nospace";
    }

    cout << endl;
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
            if(!arg->allRequiredValuesPresent(i)) {
                stringstream ss(stringstream::in | stringstream::out);
                ss << "Not all parameter for argument \"" << arg->name() << "\" ";
                if(i) {
                    ss << " (" << (i + 1) << " occurrence) ";
                }
                ss << "provided. You have to provide the following parameter:";
                size_t valueNamesPrint = 0;
                for(const auto &name : arg->m_valueNames) {
                    ss << ' ' << name, ++valueNamesPrint;
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
 *  - Invokes the assigned callback methods for each occurrence of
 *    the argument.
 */
void ArgumentParser::invokeCallbacks(const ArgumentVector &args)
{
    for(const Argument *arg : args) {
        // invoke the callback for each occurrence of the argument
        if(arg->m_callbackFunction) {
            for(const auto &occurrence : arg->m_occurrences) {
                arg->m_callbackFunction(occurrence);
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
    setCallback([&parser] (const ArgumentOccurrence &) {
        CMD_UTILS_START_CONSOLE;
        parser.printHelp(cout);
    });
}

}
