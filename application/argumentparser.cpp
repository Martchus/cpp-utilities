#include "argumentparser.h"
#include "failure.h"

#include "../conversion/stringconversion.h"
#include "../misc/random.h"

#include <algorithm>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;
using namespace std::placeholders;

/*!
    \namespace ApplicationUtilities
    \brief Contains currently only ArgumentParser and related classes.
*/
namespace ApplicationUtilities {

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
 * Constructs an Argument with the given \a name, \a abbreviation and \a description.
 *
 * The \a name and the abbreviation mustn't contain any whitespaces.
 * The \a name mustn't be empty. The \a abbreviation and the \a description might be empty.
 */
Argument::Argument(const std::string &name, const std::string abbreviation, const std::string &description) :
    m_required(false),
    m_combinable(false),
    m_implicit(false),
    m_denotesOperation(false),
    m_requiredValueCount(0),
    m_default(false),
    m_present(false),
    m_isMainArg(false)
{
    setName(name);
    setAbbreviation(abbreviation);
    setDescription(description);
}

/*!
 * Constructs an Argument with the given \a name, \a abbreviation and \a description.
 * The \a name and the abbreviation mustn't contain any whitespaces.
 * The \a name mustn't be empty. The \a abbreviation and the \a description might be empty.
 */
Argument::Argument(const char *name, const char *abbreviation, const char *description) :
    m_required(false),
    m_combinable(false),
    m_implicit(false),
    m_denotesOperation(false),
    m_requiredValueCount(0),
    m_default(false),
    m_present(false),
    m_isMainArg(false)
{
    setName(name);
    setAbbreviation(abbreviation);
    setDescription(description);
}

/*!
 * Destroys the Argument.
 */
Argument::~Argument()
{}

///*!
// * \brief Checks whether the argument is ambigious.
// * \returns Returns zero if the argument is not ambigious. Returns 1 if the name
// *          is ambiguous and 2 if the abbreviation is ambiguous.
// */
//unsigned char Argument::isAmbiguous(const ArgumentParser &parser) const
//{
//    function<unsigned char (const ArgumentVector &args, const Argument *toCheck)> check;
//    check = [&check] (const ArgumentVector &args, const Argument *toCheck) -> unsigned char {
//        for(const auto *arg : args) {
//            if(arg != toCheck) {
//                if(!toCheck->name().empty() && arg->name() == toCheck->name()) {
//                    return 1;
//                } else if(!toCheck->abbreviation().empty() && arg->abbreviation() == toCheck->abbreviation()) {
//                    return 2;
//                }
//            }
//            if(auto res = check(arg->parents(), toCheck)) {
//                return res;
//            }
//        }
//        return 0;
//    };
//    if(auto res = check(parser.mainArguments(), this)) {
//        return res;
//    }
//    return check(parents(), this);
//}

/*!
 * Appends the name, the abbreviation and the description of the Argument
 * to the give ostream.
 */
void Argument::printInfo(ostream &os, unsigned char indentionLevel) const
{
    for(unsigned char i = 0; i < indentionLevel; ++i) os << "  ";
    if(!name().empty()) {
        os << "--" << name();
    }
    if(!name().empty() && !abbreviation().empty()) {
        os << ", ";
    }
    if(!abbreviation().empty()) {
        os << "-" << abbreviation();
    }
    if(requiredValueCount() > 0) {
        int valueNamesPrint = 0;
        for(auto i = valueNames().cbegin(), end = valueNames().cend(); i != end && valueNamesPrint < requiredValueCount(); ++i) {
            os << " [" << *i << "]";
            ++valueNamesPrint;
        }
        for(; valueNamesPrint < requiredValueCount(); ++valueNamesPrint) {
            os << " [value " << (valueNamesPrint + 1) << "]";
        }
    } else if(requiredValueCount() < 0) {
        for(auto i = valueNames().cbegin(), end = valueNames().cend(); i != end; ++i) {
            os << " [" << *i << "]";
        }
        os << " ...";
    }
    ++indentionLevel;
    if(!description().empty()) {
        os << endl;
        for(unsigned char i = 0; i < indentionLevel; ++i) os << "  ";
        os << description();
    }
    if(isRequired()) {
        os << endl;
        for(unsigned char i = 0; i < indentionLevel; ++i) os << "  ";
        os << "This argument is required.";
    }
    os << endl;
    for(const Argument *arg : secondaryArguments()) {
        arg->printInfo(os, indentionLevel + 1);
    }
}

/*!
 * This function return the first present and uncombinable argument of the given list of arguments.
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
 * Sets the secondary arguments for this arguments. The given arguments will be considered as
 * secondary arguments of this argument by the argument parser. This means that the parser
 * will complain if these arguments are given, but not this argument. If secondary arguments are
 * labeled as mandatory their parent is also mandatory.
 *
 * The Argument does not take ownership. Do not destroy the given arguments as long as they are
 * used as secondary arguments.
 *
 * \sa secondaryArguments()
 * \sa addSecondaryArgument()
 * \sa hasSecondaryArguments()
 */
void Argument::setSecondaryArguments(const ArgumentInitializerList &secondaryArguments)
{
    // remove this argument from the parents list of the previous secondary arguments
    for(Argument *arg : m_secondaryArgs) {
        arg->m_parents.erase(remove(arg->m_parents.begin(), arg->m_parents.end(), this), arg->m_parents.end());
    }
    // assign secondary arguments
    m_secondaryArgs.assign(secondaryArguments);
    // add this argument to the parents list of the assigned secondary arguments
    // and set the parser
    for(Argument *arg : m_secondaryArgs) {
        if(find(arg->m_parents.cbegin(), arg->m_parents.cend(), this) == arg->m_parents.cend()) {
            arg->m_parents.push_back(this);
        }
    }
}

/*!
 * Adds \a arg as a secondary argument for this argument.
 *
 * \sa secondaryArguments()
 * \sa setSecondaryArguments()
 * \sa hasSecondaryArguments()
 */
void Argument::addSecondaryArgument(Argument *arg)
{
    if(find(m_secondaryArgs.cbegin(), m_secondaryArgs.cend(), arg) == m_secondaryArgs.cend()) {
        m_secondaryArgs.push_back(arg);
        if(find(arg->m_parents.cbegin(), arg->m_parents.cend(), this) == arg->m_parents.cend()) {
            arg->m_parents.push_back(this);
        }
    }
}

/*!
 * Returns the names of the parents in the form "parent1", "parent2, "parent3", ...
 * Returns an empty string if this Argument has no parents.
 * \sa parents()
 */
string Argument::parentNames() const
{
    string res;
    if(m_parents.size()) {
        vector<string> names;
        names.reserve(m_parents.size());
        for(const Argument *parent : m_parents) {
            names.push_back(parent->name());
        }
        res.assign(ConversionUtilities::joinStrings(names, ", "));
    }
    return res;
}

/*!
 * Returns true if at least one of the parents is present.
 * Returns false if this argument has no parents or none of its parents is present.
 */
bool Argument::isParentPresent() const
{
    for(Argument *parent : m_parents) {
        if(parent->isPresent()) {
            return true;
        }
    }
    return false;
}

/*!
 * Checks if this arguments conflicts with other arguments. If the argument is in conflict
 * with an other argument this argument will be returned. Otherwise nullptr will be returned.
 */
Argument *Argument::conflictsWithArgument() const
{
    if(!isCombinable() && isPresent()) {
        for(Argument *parent : m_parents) {
            for(Argument *sibling : parent->secondaryArguments()) {
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
 * Constructs a new ArgumentParser.
 */
ArgumentParser::ArgumentParser() :
    m_actualArgc(0),
    m_ignoreUnknownArgs(false)
{}

/*!
 * Sets the main arguments for the parser. The parser will use these argument definitions
 * to when parsing the command line arguments and when printing help information.
 *
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
 * Prints help information for all main arguments which have been set using setMainArguments().
 */
void ArgumentParser::printHelp(ostream &os) const
{
    if(!m_mainArgs.size()) {
        return;
    }
    os << "Available arguments:\n";
    for(const Argument *arg : m_mainArgs) {
        arg->printInfo(os);
    }
}

/*!
 * Returns the first argument definition which matches the predicate.
 * The search includes all assigned main argument definitions and their sub arguments.
 */
Argument *ArgumentParser::findArg(const ArgumentPredicate &predicate) const
{
    return findArg(m_mainArgs, predicate);
}

/*!
 * Returns the first argument definition which matches the predicate.
 * The search includes all provided \a arguments and their sub arguments.
 */
Argument *ArgumentParser::findArg(const ArgumentVector &arguments, const ArgumentPredicate &predicate)
{
    for(Argument *arg : arguments) {
        if(predicate(arg)) {
            return arg; // argument matches
        } else if(Argument *subarg = findArg(arg->secondaryArguments(), predicate)) {
            return subarg; // a secondary argument matches
        }
    }
    return nullptr; // no argument matches
}

/*!
 * This method is used to verify the setup of the command line parser before parsing.
 *
 * This function will throw std::invalid_argument when a mismatch is detected:
 *  - An argument is used as main argument and as sub argument at the same time.
 *  - An argument abbreviation is used more then once.
 *  - An argument name is used more then once.
 * If none of these conditions are met, this method will do nothing.
 *
 * \remarks Usually you don't need to call this function manually because it is called by
 *          parse() automatically befor the parsing is performed.
 */
void ArgumentParser::verifySetup() const
{
    vector<Argument *> verifiedArgs;
    vector<string> abbreviations;
    vector<string> names;
    const Argument *implicitArg = nullptr;
    function<void (const ArgumentVector &args)> checkArguments;
    checkArguments = [&verifiedArgs, &abbreviations, &names, &checkArguments, &implicitArg, this] (const ArgumentVector &args) {
        for(Argument *arg : args) {
            if(find(verifiedArgs.cbegin(), verifiedArgs.cend(), arg) != verifiedArgs.cend()) {
                continue; // do not verify the same argument twice
            }
            if(arg->isMainArgument() && arg->parents().size()) {
                throw invalid_argument("Argument \"" + arg->name() + "\" can not be used as main argument and sub argument at the same time.");
            }
            if(!arg->abbreviation().empty() && find(abbreviations.cbegin(), abbreviations.cend(), arg->abbreviation()) != abbreviations.cend()) {
                throw invalid_argument("Abbreviation \"" + arg->abbreviation() + "\" has been used more then once.");
            }
            if(find(names.cbegin(), names.cend(), arg->name()) != names.cend()) {
                throw invalid_argument("Name \"" + arg->name() + "\" has been used more then once.");
            }
            if(arg->isDefault() && arg->requiredValueCount() > 0 && arg->defaultValues().size() < static_cast<size_t>(arg->requiredValueCount())) {
                throw invalid_argument("Default argument \"" + arg->name() + "\" doesn't provide the required number of default values.");
            }
            if(arg->isImplicit()) {
                if(implicitArg) {
                    throw invalid_argument("The arguments \"" + implicitArg->name() + "\" and \"" + arg->name() + "\" can not be both implicit.");
                } else {
                    implicitArg = arg;
                }
            }
            abbreviations.push_back(arg->abbreviation());
            names.push_back(arg->name());
            verifiedArgs.push_back(arg);
            checkArguments(arg->secondaryArguments());
        }
    };
    checkArguments(m_mainArgs);
}

/*!
 * This method invokes verifySetup() before parsing. See its do documentation for more
 * information about execptions that might be thrown to indicate an invalid setup of the parser.
 *
 * If the parser is setup properly this method will parse the given command line arguments using
 * the previsously set argument definitions.
 * If one of the previsously defined arguments has been found, its present flag will be set to true
 * and the parser reads all values tied to this argument.
 * If an argument has been found which does not match any of the previous definitions it will be
 * considered as unknown.
 * If the given command line arguments are not valid according the defined arguments an
 * Failure will be thrown.
 */
void ArgumentParser::parseArgs(int argc, char *argv[])
{
    // initiate parser
    verifySetup();
    m_actualArgc = 0; // reset actual agument count
    unsigned int actualArgc = 0;
    int valuesToRead = 0;
    // read current directory
    if(argc >= 1) {
        m_currentDirectory = string(*argv);
    } else {
        m_currentDirectory.clear();
    }
    // function for iterating through all arguments
    function<void(Argument *, const ArgumentVector &, const function<void (Argument *, Argument *)> &)> foreachArg;
    foreachArg = [&foreachArg] (Argument *parent, const ArgumentVector &args, const function<void (Argument *, Argument *)> &proc) {
        for(Argument *arg : args) {
            proc(parent, arg);
            foreachArg(arg, arg->secondaryArguments(), proc);
        }
    };
    // parse given arguments
    if(argc >= 2) {
        Argument *currentArg = nullptr;
        // iterate through given arguments
        for(char **i = argv + 1, **end = argv + argc; i != end; ++i) {
            string givenArg(*i); // convert argument to string
            if(!givenArg.empty()) { // skip empty entries
                if(valuesToRead <= 0 && givenArg.size() > 1 && givenArg.front() == '-') {
                    // we have no values left to read and the given arguments starts with '-'
                    // -> the next "value" is a main argument or a sub argument
                    ArgumentPredicate pred;
                    string givenId;
                    size_t equationPos = givenArg.find('=');
                    if(givenArg.size() > 2 && givenArg[1] == '-') {
                        // the argument starts with '--'
                        // -> the full argument name has been provided
                        givenId = givenArg.substr(2, equationPos - 2);
                        pred = [&givenId, equationPos] (Argument *arg) {
                            return arg->name() == givenId;
                        };
                    } else {
                        // the argument starts with a single '-'
                        // -> the abbreviation has been provided
                        givenId = givenArg.substr(1, equationPos - 1);
                        pred = [&givenId, equationPos] (Argument *arg) {
                            return arg->abbreviation() == givenId;
                        };
                    }
                    // find the corresponding instance of the Argument class
                    currentArg = findArg(pred);
                    if(currentArg) {
                        // the corresponding instance of Argument class has been found
                        if(currentArg->m_present) {
                            // the argument has been provided more then once
                            throw Failure("The argument \"" + currentArg->name() + "\" has been specified more than one time.");
                        } else {
                            // set present flag of argument
                            currentArg->m_present = true;
                            ++actualArgc; // we actually found an argument
                            // now we might need to read values tied to that argument
                            valuesToRead = currentArg->requiredValueCount();
                            if(equationPos != string::npos) {
                                // a value has been specified using the --argument=value syntax
                                string value = givenArg.substr(equationPos + 1);
                                if(valuesToRead != 0) {
                                    currentArg->m_values.push_back(value);
                                    if(valuesToRead > 0) {
                                        --valuesToRead;
                                    }
                                } else {
                                    throw Failure("Invalid extra information \"" + value + "\" (specified using \"--argument=value\" syntax) for the argument \"" + currentArg->name() + "\" given.");
                                }
                            }
                        }
                    } else {
                        // the given argument seems to be unknown
                        if(valuesToRead < 0) {
                            // we have a variable number of values to expect -> treat "unknown argument" as value
                            goto readValue;
                        } else {
                            // we have no more values to expect so we need to complain about the unknown argument
                            goto invalidArg;
                        }
                    }
                } else {
                    readValue:
                    if(!currentArg) {
                        // we have not parsed an argument before
                        // -> check if an argument which denotes the operation is specified
                        if(i == argv + 1) {
                            for(Argument *arg : m_mainArgs) {
                                if(!arg->isPresent() && arg->denotesOperation()
                                        && (arg->name() == givenArg || arg->abbreviation() == givenArg)) {
                                    currentArg = arg;
                                    break;
                                }
                            }
                            if(currentArg) {
                                currentArg->m_present = true;
                                ++actualArgc; // we actually found an argument
                                // now we might need to read values tied to that argument
                                valuesToRead = currentArg->requiredValueCount();
                                continue;
                            }
                        }
                        // -> check if there's an implicit argument definition
                        try {
                            foreachArg(nullptr, m_mainArgs, [&actualArgc, &valuesToRead, &currentArg, this] (Argument *, Argument *arg) {
                                if(!arg->isPresent() && arg->isImplicit()) {
                                    throw arg;
                                }
                            });
                        } catch(Argument *arg) {
                            // set present flag of argument
                            arg->m_present = true;
                            ++actualArgc; // we actually found an argument
                            // now we might need to read values tied to that argument
                            valuesToRead = arg->requiredValueCount();
                            currentArg = arg;
                        }
                    }
                    if(currentArg) {
                        // check if the given value is tied to the most recently parsed argument
                        if(valuesToRead == 0) {
                            throw Failure("Invalid extra information \"" + givenArg + "\" for the argument \"" + currentArg->name() + "\" given.");
                        } else if(valuesToRead < 0) {
                            currentArg->m_values.push_back(givenArg);
                        } else {
                            currentArg->m_values.push_back(givenArg);
                            --valuesToRead; // one value less to be read
                        }
                    } else {
                        // there is no implicit argument definition -> the "value" has to be an argument
                        // but does not start with '-' or '--'
                        invalidArg:
                        string msg("The given argument \"" + givenArg + "\" is unknown.");
                        if(m_ignoreUnknownArgs) {
                            cout << msg << " It will be ignored." << endl;
                        } else {
                            throw Failure(msg);
                        }
                    }
                }
            }
        }
    }
    // iterate actually through all arguments using previously defined function to check gathered arguments
    foreachArg(nullptr, m_mainArgs, [this] (Argument *parent, Argument *arg) {
        if(!arg->isPresent()) {
            // the argument might be flagged as present if its a default argument
            if(arg->isDefault() && (arg->isMainArgument() || (parent && parent->isPresent()))) {
                arg->m_present = true;
                if(firstPresentUncombinableArg(arg->isMainArgument() ? m_mainArgs : parent->secondaryArguments(), arg)) {
                    arg->m_present = false;
                }
            }
            // throw an error if mandatory argument is not present
            if(!arg->isPresent() && (arg->isRequired() && (arg->isMainArgument() || (parent && parent->isPresent())))) {
                throw Failure("The argument \"" + arg->name() + "\" is required but not given.");
            }
        }
    });
    foreachArg(nullptr, m_mainArgs, [this] (Argument *, Argument *arg) {
        if(arg->isPresent()) {
            if(!arg->isMainArgument() && arg->parents().size() && !arg->isParentPresent()) {
                if(arg->parents().size() > 1) {
                    throw Failure("The argument \"" + arg->name() + "\" needs to be used together with one the following arguments: " + arg->parentNames());
                } else {
                    throw Failure("The argument \"" + arg->name() + "\" needs to be used together with the argument \"" + arg->parents().front()->name() + "\".");
                }
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
                throw Failure("The argument \"" + conflictingArgument->name() + "\" can not be combined with \"" + arg->name() + "\".");
            }
            if(!arg->allRequiredValuesPresent()) {
                stringstream ss(stringstream::in | stringstream::out);
                ss << "Not all required information for the given argument \"" << arg->name() << "\" provided. You have to give the following information:";
                int valueNamesPrint = 0;
                for(const auto &name : arg->m_valueNames) {
                    ss << "\n" << name;
                    ++valueNamesPrint;
                }
                while(valueNamesPrint < arg->m_requiredValueCount) {
                    ss << "\nvalue " << (++valueNamesPrint);
                }
                throw Failure(ss.str());
            }
        }
    });
    // set actual argument count
    m_actualArgc = actualArgc;
    // interate through all arguments again to invoke callback functions
    foreachArg(nullptr, m_mainArgs, [] (Argument *parent, Argument *arg) {
        if(arg->m_callbackFunction) {
            if(arg->isMainArgument() || (parent && parent->isPresent())) {
                // only invoke if its a main argument or the parent is present
                if(arg->isPresent()) {
                    if(arg->isDefault() && !arg->values().size()) {
                        arg->m_callbackFunction(arg->defaultValues());
                    } else {
                        arg->m_callbackFunction(arg->values());
                    }
                }
            }
        }
    });
}

/*!
 * \brief The HelpArgument class prints help information for an argument parser
 *        when present (--help, -h).
 */

/*!
 * \brief Constructs a new help argument for the specified parser.
 */
HelpArgument::HelpArgument(ArgumentParser &parser) :
    Argument("help", "h", "shows this information")
{
    setCallback([&parser] (const StringVector &) {parser.printHelp(cout);});
}

}
