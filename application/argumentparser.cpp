#include "./argumentparser.h"
#include "./argumentparserprivate.h"
#include "./commandlineutils.h"
#include "./failure.h"

#include "../conversion/stringbuilder.h"
#include "../conversion/stringconversion.h"
#include "../io/ansiescapecodes.h"
#include "../io/path.h"
#include "../misc/levenshtein.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <set>
#include <sstream>
#include <string>

using namespace std;
using namespace std::placeholders;
using namespace ConversionUtilities;
using namespace EscapeCodes;
using namespace IoUtilities;

/*!
 *  \namespace ApplicationUtilities
 *  \brief Contains currently only ArgumentParser and related classes.
 */
namespace ApplicationUtilities {

/*!
 * \brief The ArgumentDenotationType enum specifies the type of a given argument denotation.
 */
enum ArgumentDenotationType : unsigned char {
    Value = 0, /**< parameter value */
    Abbreviation = 1, /**< argument abbreviation */
    FullName = 2 /**< full argument name */
};

/*!
 * \brief The ArgumentCompletionInfo struct holds information internally used for shell completion and suggestions.
 */
struct ArgumentCompletionInfo {
    ArgumentCompletionInfo(const ArgumentReader &reader);

    const Argument *const lastDetectedArg;
    size_t lastDetectedArgIndex = 0;
    vector<Argument *> lastDetectedArgPath;
    list<const Argument *> relevantArgs;
    list<const Argument *> relevantPreDefinedValues;
    const char *const *lastSpecifiedArg = nullptr;
    unsigned int lastSpecifiedArgIndex = 0;
    bool nextArgumentOrValue = false;
    bool completeFiles = false, completeDirs = false;
};

/*!
 * \brief Constructs a new completion info for the specified \a reader.
 * \remarks Only assigns some defaults. Use ArgumentParser::determineCompletionInfo() to populate the struct with actual data.
 */
ArgumentCompletionInfo::ArgumentCompletionInfo(const ArgumentReader &reader)
    : lastDetectedArg(reader.lastArg)
{
}

struct ArgumentSuggestion {
    ArgumentSuggestion(const char *unknownArg, size_t unknownArgSize, const char *suggestion, bool hasDashPrefix);
    ArgumentSuggestion(const char *unknownArg, size_t unknownArgSize, const char *suggestion, size_t suggestionSize, bool hasDashPrefix);
    bool operator<(const ArgumentSuggestion &other) const;
    bool operator==(const ArgumentSuggestion &other) const;
    void addTo(multiset<ArgumentSuggestion> &suggestions, size_t limit) const;

    const char *const suggestion;
    const size_t suggestionSize;
    const size_t editingDistance;
    const bool hasDashPrefix;
};

ArgumentSuggestion::ArgumentSuggestion(const char *unknownArg, size_t unknownArgSize, const char *suggestion, size_t suggestionSize, bool isOperation)
    : suggestion(suggestion)
    , suggestionSize(suggestionSize)
    , editingDistance(MiscUtilities::computeDamerauLevenshteinDistance(unknownArg, unknownArgSize, suggestion, suggestionSize))
    , hasDashPrefix(isOperation)
{
}

ArgumentSuggestion::ArgumentSuggestion(const char *unknownArg, size_t unknownArgSize, const char *suggestion, bool isOperation)
    : ArgumentSuggestion(unknownArg, unknownArgSize, suggestion, strlen(suggestion), isOperation)
{
}

bool ArgumentSuggestion::operator<(const ArgumentSuggestion &other) const
{
    return editingDistance < other.editingDistance;
}

void ArgumentSuggestion::addTo(multiset<ArgumentSuggestion> &suggestions, size_t limit) const
{
    if (suggestions.size() >= limit && !(*this < *--suggestions.end())) {
        return;
    }
    suggestions.emplace(*this);
    while (suggestions.size() > limit) {
        suggestions.erase(--suggestions.end());
    }
}

/*!
 * \class ArgumentReader
 * \brief The ArgumentReader class internally encapsulates the process of reading command line arguments.
 * \remarks
 * - For meaning of parameter see documentation of corresponding member variables.
 * - Results are stored in specified \a args and assigned sub arguments.
 * - This class is explicitely *not* part of the public API.
 */

/*!
 * \brief Initializes the internal reader for the specified \a parser and arguments.
 */
ArgumentReader::ArgumentReader(ArgumentParser &parser, const char *const *argv, const char *const *end, bool completionMode)
    : parser(parser)
    , args(parser.m_mainArgs)
    , index(0)
    , argv(argv)
    , end(end)
    , lastArg(nullptr)
    , argDenotation(nullptr)
    , completionMode(completionMode)
{
}

/*!
 * \brief Resets the ArgumentReader to continue reading new \a argv.
 */
ArgumentReader &ArgumentReader::reset(const char *const *argv, const char *const *end)
{
    this->argv = argv;
    this->end = end;
    index = 0;
    lastArg = nullptr;
    argDenotation = nullptr;
    return *this;
}

/*!
 * \brief Reads the commands line arguments specified when constructing the object.
 * \remarks Reads on main-argument-level.
 */
bool ArgumentReader::read()
{
    return read(args);
}

/*!
 * \brief Returns whether the \a denotation with the specified \a denotationLength matches the argument's \a name.
 */
bool Argument::matchesDenotation(const char *denotation, size_t denotationLength) const
{
    return m_name && !strncmp(m_name, denotation, denotationLength) && *(m_name + denotationLength) == '\0';
}

/*!
 * \brief Reads the commands line arguments specified when constructing the object.
 * \remarks The argument definitions to look for are specified via \a args. The method calls itself recursively
 *          to check for nested arguments as well.
 * \returns Returns true if all arguments have been processed. Returns false on early exit because some argument
 *          is unknown and behavior for this case is set to UnknownArgumentBehavior::Fail.
 */
bool ArgumentReader::read(ArgumentVector &args)
{
    // method is called recursively for sub args to the last argument (which is nullptr in the initial call) is the current parent argument
    Argument *const parentArg = lastArg;
    // determine the current path
    const vector<Argument *> &parentPath = parentArg ? parentArg->path(parentArg->occurrences() - 1) : vector<Argument *>();

    Argument *lastArgInLevel = nullptr;
    vector<const char *> *values = nullptr;

    // iterate through all argument denotations; loop might exit earlier when an denotation is unknown
    while (argv != end) {
        // check whether there are still values to read
        if (values && lastArgInLevel->requiredValueCount() != Argument::varValueCount && values->size() < lastArgInLevel->requiredValueCount()) {
            // read arg as value and continue with next arg
            values->emplace_back(argDenotation ? argDenotation : *argv);
            ++index, ++argv, argDenotation = nullptr;
            continue;
        }

        // determine how denotation must be processed
        bool abbreviationFound = false;
        if (argDenotation) {
            // continue reading childs for abbreviation denotation already detected
            abbreviationFound = false;
            argDenotationType = Abbreviation;
        } else {
            // determine denotation type
            argDenotation = *argv;
            if (!*argDenotation && (!lastArgInLevel || values->size() >= lastArgInLevel->requiredValueCount())) {
                // skip empty arguments
                ++index, ++argv, argDenotation = nullptr;
                continue;
            }
            abbreviationFound = false;
            argDenotationType = Value;
            *argDenotation == '-' && (++argDenotation, ++argDenotationType) && *argDenotation == '-' && (++argDenotation, ++argDenotationType);
        }

        // try to find matching Argument instance
        Argument *matchingArg = nullptr;
        if (argDenotationType != Value) {
            // determine actual denotation length (everything before equation sign)
            const char *const equationPos = strchr(argDenotation, '=');
            const auto argDenotationLength = equationPos ? static_cast<size_t>(equationPos - argDenotation) : strlen(argDenotation);

            // loop through each "part" of the denotation
            // names are read at once, but for abbreviations each character is considered individually
            for (; argDenotationLength; matchingArg = nullptr) {
                // search for arguments by abbreviation or name depending on the previously determined denotation type
                if (argDenotationType == Abbreviation) {
                    for (Argument *const arg : args) {
                        if (arg->abbreviation() && arg->abbreviation() == *argDenotation) {
                            matchingArg = arg;
                            abbreviationFound = true;
                            break;
                        }
                    }
                } else {
                    for (Argument *const arg : args) {
                        if (arg->matchesDenotation(argDenotation, argDenotationLength)) {
                            matchingArg = arg;
                            break;
                        }
                    }
                }
                if (!matchingArg) {
                    break;
                }

                // an argument matched the specified denotation so add an occurrence
                matchingArg->m_occurrences.emplace_back(index, parentPath, parentArg);

                // prepare reading parameter values
                values = &matchingArg->m_occurrences.back().values;

                // read value after equation sign
                if ((argDenotationType != Abbreviation && equationPos) || (++argDenotation == equationPos)) {
                    values->push_back(equationPos + 1);
                    argDenotation = nullptr;
                }

                // read sub arguments, distinguish whether further abbreviations follow
                ++index, ++parser.m_actualArgc, lastArg = lastArgInLevel = matchingArg, lastArgDenotation = argv;
                if (argDenotationType != Abbreviation || !argDenotation || !*argDenotation) {
                    // no further abbreviations follow -> read sub args for next argv
                    ++argv, argDenotation = nullptr;
                    read(lastArg->m_subArgs);
                    argDenotation = nullptr;
                    break;
                } else {
                    // further abbreviations follow -> remember current arg value
                    const char *const *const currentArgValue = argv;
                    // don't increment argv, keep processing outstanding chars of argDenotation
                    read(lastArg->m_subArgs);
                    // stop further processing if the denotation has been consumed or even the next value has already been loaded
                    if (!argDenotation || currentArgValue != argv) {
                        argDenotation = nullptr;
                        break;
                    }
                }
            }

            // continue with next arg if we've got a match already
            if (matchingArg) {
                continue;
            }

            // unknown argument might be a sibling of the parent element
            for (auto parentArgument = parentPath.crbegin(), pathEnd = parentPath.crend();; ++parentArgument) {
                for (Argument *const sibling : (parentArgument != pathEnd ? (*parentArgument)->subArguments() : parser.m_mainArgs)) {
                    if (sibling->occurrences() < sibling->maxOccurrences()) {
                        // check whether the denoted abbreviation matches the sibling's abbreviatiopn
                        if (argDenotationType == Abbreviation && (sibling->abbreviation() && sibling->abbreviation() == *argDenotation)) {
                            return false;
                        }
                        // check whether the denoted name matches the sibling's name
                        if (sibling->matchesDenotation(argDenotation, argDenotationLength)) {
                            return false;
                        }
                    }
                }
                if (parentArgument == pathEnd) {
                    break;
                }
            };
        }

        // unknown argument might just be a parameter value of the last argument
        if (lastArgInLevel && values->size() < lastArgInLevel->requiredValueCount()) {
            values->emplace_back(abbreviationFound ? argDenotation : *argv);
            ++index, ++argv, argDenotation = nullptr;
            continue;
        }

        // first value might denote "operation"
        for (Argument *const arg : args) {
            if (arg->denotesOperation() && arg->name() && !strcmp(arg->name(), *argv)) {
                (matchingArg = arg)->m_occurrences.emplace_back(index, parentPath, parentArg);
                lastArgDenotation = argv;
                ++index, ++argv;
                break;
            }
        }

        // use the first default argument which is not already present if there is still no match
        if (!matchingArg && (!completionMode || (argv + 1 != end))) {
            const bool uncombinableMainArgPresent = parentArg ? false : parser.isUncombinableMainArgPresent();
            for (Argument *const arg : args) {
                if (arg->isImplicit() && !arg->isPresent() && !arg->wouldConflictWithArgument()
                    && (!uncombinableMainArgPresent || !arg->isMainArgument())) {
                    (matchingArg = arg)->m_occurrences.emplace_back(index, parentPath, parentArg);
                    break;
                }
            }
        }

        if (matchingArg) {
            // an argument matched the specified denotation
            if (lastArgInLevel == matchingArg) {
                break; // break required? -> TODO: add test for this condition
            }

            // prepare reading parameter values
            values = &matchingArg->m_occurrences.back().values;

            // read sub arguments
            ++parser.m_actualArgc, lastArg = lastArgInLevel = matchingArg, argDenotation = nullptr;
            read(lastArg->m_subArgs);
            argDenotation = nullptr;
            continue;
        }

        // argument denotation is unknown -> handle error
        if (parentArg) {
            // continue with parent level
            return false;
        }
        if (completionMode) {
            // ignore unknown denotation
            ++index, ++argv, argDenotation = nullptr;
        } else {
            switch (parser.m_unknownArgBehavior) {
            case UnknownArgumentBehavior::Warn:
                cerr << Phrases::Warning << "The specified argument \"" << *argv << "\" is unknown and will be ignored." << Phrases::EndFlush;
                FALLTHROUGH;
            case UnknownArgumentBehavior::Ignore:
                // ignore unknown denotation
                ++index, ++argv, argDenotation = nullptr;
                break;
            case UnknownArgumentBehavior::Fail:
                return false;
            }
        }
    } // while(argv != end)
    return true;
}

/*!
 * \class Wrapper
 * \brief The Wrapper class is internally used print text which might needs to be wrapped preserving the indentation.
 * \remarks This class is explicitely *not* part of the public API.
 */

ostream &operator<<(ostream &os, const Wrapper &wrapper)
{
    // determine max. number of columns
    static const TerminalSize termSize(determineTerminalSize());
    const auto maxColumns = termSize.columns ? termSize.columns : numeric_limits<unsigned short>::max();

    // print wrapped string considering indentation
    unsigned short currentCol = wrapper.m_indentation.level;
    for (const char *currentChar = wrapper.m_str; *currentChar; ++currentChar) {
        const bool wrappingRequired = currentCol >= maxColumns;
        if (wrappingRequired || *currentChar == '\n') {
            // insert newline (TODO: wrap only at end of a word)
            os << '\n';
            // print indentation (if enough space)
            if (wrapper.m_indentation.level < maxColumns) {
                os << wrapper.m_indentation;
                currentCol = wrapper.m_indentation.level;
            } else {
                currentCol = 0;
            }
        }
        if (*currentChar != '\n' && (!wrappingRequired || *currentChar != ' ')) {
            os << *currentChar;
            ++currentCol;
        }
    }
    return os;
}

/// \brief Specifies the name of the application (used by ArgumentParser::printHelp()).
const char *applicationName = nullptr;
/// \brief Specifies the author of the application (used by ArgumentParser::printHelp()).
const char *applicationAuthor = nullptr;
/// \brief Specifies the version of the application (used by ArgumentParser::printHelp()).
const char *applicationVersion = nullptr;
/// \brief Specifies the URL to the application website (used by ArgumentParser::printHelp()).
const char *applicationUrl = nullptr;
/// \brief Specifies the dependency versions the application was linked against (used by ArgumentParser::printHelp()).
/// \deprecated Not used anymore. Use dependencyVersions2 instead.
std::initializer_list<const char *> dependencyVersions;
/// \brief Specifies the dependency versions the application was linked against (used by ArgumentParser::printHelp()).
std::vector<const char *> dependencyVersions2;

// TODO v5 use a struct for these properties

/*!
 * \brief Specifies a function quit the application.
 * \remarks Currently only used after printing Bash completion. Default is std::exit().
 */
void (*exitFunction)(int) = &exit;

/// \cond

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
Argument::Argument(const char *name, char abbreviation, const char *description, const char *example)
    : m_name(name)
    , m_abbreviation(abbreviation)
    , m_environmentVar(nullptr)
    , m_description(description)
    , m_example(example)
    , m_minOccurrences(0)
    , m_maxOccurrences(1)
    , m_combinable(false)
    , m_denotesOperation(false)
    , m_requiredValueCount(0)
    , m_implicit(false)
    , m_isMainArg(false)
    , m_valueCompletionBehavior(ValueCompletionBehavior::PreDefinedValues | ValueCompletionBehavior::Files | ValueCompletionBehavior::Directories
          | ValueCompletionBehavior::FileSystemIfNoPreDefinedValues)
    , m_preDefinedCompletionValues(nullptr)
{
}

/*!
 * \brief Destroys the Argument.
 */
Argument::~Argument()
{
}

/*!
 * \brief Returns the first parameter value of the first occurrence of the argument.
 * \remarks
 * - If the argument is not present and the an environment variable has been set
 *   using setEnvironmentVariable() the value of the specified variable will be returned.
 * - Returns nullptr if no value is available though.
 */
const char *Argument::firstValue() const
{
    if (!m_occurrences.empty() && !m_occurrences.front().values.empty()) {
        return m_occurrences.front().values.front();
    } else if (m_environmentVar) {
        return getenv(m_environmentVar);
    } else {
        return nullptr;
    }
}

/*!
 * \brief Writes the name, the abbreviation and other information about the Argument to the give ostream.
 */
void Argument::printInfo(ostream &os, unsigned char indentation) const
{
    Indentation ident(indentation);
    os << ident;
    EscapeCodes::setStyle(os, EscapeCodes::TextAttribute::Bold);
    if (notEmpty(name())) {
        if (!denotesOperation()) {
            os << '-' << '-';
        }
        os << name();
    }
    if (notEmpty(name()) && abbreviation()) {
        os << ',' << ' ';
    }
    if (abbreviation()) {
        os << '-' << abbreviation();
    }
    EscapeCodes::setStyle(os);
    if (requiredValueCount()) {
        unsigned int valueNamesPrint = 0;
        for (auto i = valueNames().cbegin(), end = valueNames().cend(); i != end && valueNamesPrint < requiredValueCount(); ++i) {
            os << ' ' << '[' << *i << ']';
            ++valueNamesPrint;
        }
        if (requiredValueCount() == Argument::varValueCount) {
            os << " ...";
        } else {
            for (; valueNamesPrint < requiredValueCount(); ++valueNamesPrint) {
                os << " [value " << (valueNamesPrint + 1) << ']';
            }
        }
    }
    ident.level += 2;
    if (notEmpty(description())) {
        os << '\n' << ident << Wrapper(description(), ident);
    }
    if (isRequired()) {
        os << '\n' << ident << "particularities: mandatory";
        if (!isMainArgument()) {
            os << " if parent argument is present";
        }
    }
    if (environmentVariable()) {
        os << '\n' << ident << "default environment variable: " << Wrapper(environmentVariable(), ident + 30);
    }
    os << '\n';
    for (const auto *arg : subArguments()) {
        arg->printInfo(os, ident.level);
    }
    if (notEmpty(example())) {
        if (ident.level == 2 && !subArguments().empty()) {
            os << '\n';
        }
        os << ident << "example: " << Wrapper(example(), ident + 9);
        os << '\n';
    }
}

/*!
 * \brief This function return the first present and uncombinable argument of the given list of arguments.
 *
 * The Argument \a except will be ignored.
 */
Argument *firstPresentUncombinableArg(const ArgumentVector &args, const Argument *except)
{
    for (Argument *arg : args) {
        if (arg != except && arg->isPresent() && !arg->isCombinable()) {
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
    for (Argument *arg : m_subArgs) {
        arg->m_parents.erase(remove(arg->m_parents.begin(), arg->m_parents.end(), this), arg->m_parents.end());
    }
    // assign secondary arguments
    m_subArgs.assign(secondaryArguments);
    // add this argument to the parents list of the assigned secondary arguments
    // and set the parser
    for (Argument *arg : m_subArgs) {
        if (find(arg->m_parents.cbegin(), arg->m_parents.cend(), this) == arg->m_parents.cend()) {
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
    if (find(m_subArgs.cbegin(), m_subArgs.cend(), arg) == m_subArgs.cend()) {
        m_subArgs.push_back(arg);
        if (find(arg->m_parents.cbegin(), arg->m_parents.cend(), this) == arg->m_parents.cend()) {
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
    if (isMainArgument()) {
        return true;
    }
    for (const Argument *parent : m_parents) {
        if (parent->isPresent()) {
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
 *
 * \remarks Conflicts with main arguments aren't considered by this method!
 */
Argument *Argument::conflictsWithArgument() const
{
    return isPresent() ? wouldConflictWithArgument() : nullptr;
}

/*!
 * \brief Checks if this argument would conflict with other arguments if it was present.
 *
 * If the argument is in conflict with an other argument this argument will be returned.
 * Otherwise nullptr will be returned.
 *
 * \remarks Conflicts with main arguments aren't considered by this method!
 */
Argument *Argument::wouldConflictWithArgument() const
{
    if (!isCombinable()) {
        for (Argument *parent : m_parents) {
            for (Argument *sibling : parent->subArguments()) {
                if (sibling != this && sibling->isPresent() && !sibling->isCombinable()) {
                    return sibling;
                }
            }
        }
    }
    return nullptr;
}

/*!
 * \brief Returns the first operation argument specified by the user or nullptr if no operation has been specified.
 * \remarks Only direct sub arguments of this argument are considered.
 */
Argument *Argument::specifiedOperation() const
{
    for (Argument *arg : m_subArgs) {
        if (arg->denotesOperation() && arg->isPresent()) {
            return arg;
        }
    }
    return nullptr;
}

/*!
 * \brief Resets this argument and all sub arguments recursively.
 * \sa Argument::reset()
 */
void Argument::resetRecursively()
{
    for (Argument *arg : m_subArgs) {
        arg->resetRecursively();
    }
    reset();
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
ArgumentParser::ArgumentParser()
    : m_actualArgc(0)
    , m_executable(nullptr)
    , m_unknownArgBehavior(UnknownArgumentBehavior::Fail)
    , m_defaultArg(nullptr)
{
}

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
    if (mainArguments.size()) {
        for (Argument *arg : mainArguments) {
            arg->m_isMainArg = true;
        }
        m_mainArgs.assign(mainArguments);
        if (!m_defaultArg) {
            if (!(*mainArguments.begin())->requiredValueCount()) {
                bool subArgsRequired = false;
                for (const Argument *subArg : (*mainArguments.begin())->subArguments()) {
                    if (subArg->isRequired()) {
                        subArgsRequired = true;
                        break;
                    }
                }
                if (!subArgsRequired) {
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
    if (applicationName && *applicationName) {
        os << applicationName;
        if (applicationVersion && *applicationVersion) {
            os << ',' << ' ';
        }
    }
    if (applicationVersion && *applicationVersion) {
        os << "version " << applicationVersion;
    }
    if (dependencyVersions2.size()) {
        if ((applicationName && *applicationName) || (applicationVersion && *applicationVersion)) {
            os << '\n';
            EscapeCodes::setStyle(os);
        }
        auto i = dependencyVersions2.begin(), end = dependencyVersions2.end();
        os << "Linked against: " << *i;
        for (++i; i != end; ++i) {
            os << ',' << ' ' << *i;
        }
    }
    if ((applicationName && *applicationName) || (applicationVersion && *applicationVersion) || dependencyVersions2.size()) {
        os << '\n' << '\n';
    }
    EscapeCodes::setStyle(os);
    if (!m_mainArgs.empty()) {
        bool hasOperations = false;
        for (const Argument *arg : m_mainArgs) {
            if (arg->denotesOperation()) {
                hasOperations = true;
                break;
            }
        }

        // check whether operations are available
        if (hasOperations) {
            // split top-level operations and other configurations
            os << "Available operations:";
            for (const Argument *arg : m_mainArgs) {
                if (arg->denotesOperation() && strcmp(arg->name(), "help")) {
                    os << '\n';
                    arg->printInfo(os);
                }
            }
            os << "\nAvailable top-level options:";
            for (const Argument *arg : m_mainArgs) {
                if (!arg->denotesOperation() && strcmp(arg->name(), "help")) {
                    os << '\n';
                    arg->printInfo(os);
                }
            }
        } else {
            // just show all args if no operations are available
            os << "Available arguments:";
            for (const Argument *arg : m_mainArgs) {
                if (strcmp(arg->name(), "help")) {
                    os << '\n';
                    arg->printInfo(os);
                }
            }
        }
    }
    if (applicationUrl && *applicationUrl) {
        os << "\nProject website: " << applicationUrl << endl;
    }
}

/*!
 * \brief Parses the specified command line arguments.
 * \remarks
 *  - The results are stored in the Argument instances assigned as main arguments and sub arguments.
 *  - Calls the assigned callbacks if no constraints are violated.
 *  - This method will not return in case shell completion is requested. This behavior can be altered
 *    by overriding ApplicationUtilities::exitFunction which defaults to &std::exit.
 * \throws Throws Failure if the specified arguments are invalid or violate the constraints defined
 *         by the Argument instances.
 * \sa readArgs(), parseArgsOrExit()
 */
void ArgumentParser::parseArgs(int argc, const char *const *argv)
{
    parseArgsExt(argc, argv, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
}

/*!
 * \brief Parses the specified command line arguments.
 * \remarks The same as parseArgs(), except that this method will not throw an exception in the error
 *          case. Instead, it will print an error message and terminate the application with exit
 *          code 1.
 * \sa parseArgs(), readArgs()
 * \deprecated In next major release, this method will be removed. parseArgs() can serve the same
 *             purpose then.
 */
void ArgumentParser::parseArgsOrExit(int argc, const char *const *argv)
{
    parseArgsExt(argc, argv);
}

/*!
 * \brief Parses the specified command line arguments.
 *
 * The behavior is configurable by specifying the \a behavior argument. See ParseArgumentBehavior for
 * the options. By default, all options are present.
 *
 * \remarks
 *  - The results are stored in the Argument instances assigned as main arguments and sub arguments.
 *  - This method will not return in the error case if the ParseArgumentBehavior::ExitOnFailure is present
 *    (default).
 *  - This method will not return in case shell completion is requested. This behavior can be altered
 *    by overriding ApplicationUtilities::exitFunction which defaults to &std::exit.
 * \throws Throws Failure if the specified arguments are invalid and the ParseArgumentBehavior::ExitOnFailure
 *         flag is not present.
 * \sa parseArgs(), readArgs(), parseArgsOrExit()
 * \deprecated In next major release, this method will be available as parseArgs().
 */
void ArgumentParser::parseArgsExt(int argc, const char *const *argv, ParseArgumentBehavior behavior)
{
    try {
        readArgs(argc, argv);
        if (!argc) {
            return;
        }
        if (behavior & ParseArgumentBehavior::CheckConstraints) {
            checkConstraints(m_mainArgs);
        }
        if (behavior & ParseArgumentBehavior::InvokeCallbacks) {
            invokeCallbacks(m_mainArgs);
        }
    } catch (const Failure &failure) {
        if (behavior & ParseArgumentBehavior::ExitOnFailure) {
            CMD_UTILS_START_CONSOLE;
            cerr << failure;
            exit(1);
        }
        throw;
    }
}

/*!
 * \brief Parses the specified command line arguments.
 * \remarks
 *  - The results are stored in the Argument instances assigned as main arguments and sub arguments.
 *  - In contrast to parseArgs() this method does not check whether constraints are violated and it
 *    does not call any callbacks.
 *  - This method will not return in case shell completion is requested. This behavior can be altered
 *    by overriding ApplicationUtilities::exitFunction which defaults to &std::exit.
 * \throws Throws Failure if the specified arguments are invalid.
 * \sa parseArgs(), parseArgsOrExit()
 * \deprecated In next major release, this method will be private. parseArgs() can serve the same
 *             purpose then.
 */
void ArgumentParser::readArgs(int argc, const char *const *argv)
{
    IF_DEBUG_BUILD(verifyArgs(m_mainArgs);)
    m_actualArgc = 0;

    // the first argument is the executable name
    if (!argc) {
        m_executable = nullptr;
        return;
    }
    m_executable = *argv;

    // check for further arguments
    if (!--argc) {
        // no arguments specified -> flag default argument as present if one is assigned
        if (m_defaultArg) {
            m_defaultArg->m_occurrences.emplace_back(0);
        }
        return;
    }

    // check for completion mode: if first arg (after executable name) is "--bash-completion-for", bash completion for the following arguments is requested
    const bool completionMode = !strcmp(*++argv, "--bash-completion-for");

    // determine the index of the current word for completion and the number of arguments to be passed to ArgumentReader
    unsigned int currentWordIndex, argcForReader;
    if (completionMode) {
        // the first argument after "--bash-completion-for" is the index of the current word
        try {
            currentWordIndex = (--argc ? stringToNumber<unsigned int, string>(*(++argv)) : 0);
            if (argc) {
                ++argv, --argc;
            }
        } catch (const ConversionException &) {
            currentWordIndex = static_cast<unsigned int>(argc - 1);
        }
        argcForReader = min(static_cast<unsigned int>(argc), currentWordIndex + 1);
    } else {
        argcForReader = static_cast<unsigned int>(argc);
    }

    // read specified arguments
    ArgumentReader reader(*this, argv, argv + argcForReader, completionMode);
    const bool allArgsProcessed(reader.read());
    NoColorArgument::apply();

    // fail when not all arguments could be processed, except when in completion mode
    if (!completionMode && !allArgsProcessed) {
        const auto suggestions(findSuggestions(argc, argv, static_cast<unsigned int>(argc - 1), reader));
        throw Failure(argsToString("The specified argument \"", *reader.argv, "\" is unknown.", suggestions));
    }

    // print Bash completion and prevent the applicaton to continue with the regular execution
    if (completionMode) {
        printBashCompletion(argc, argv, currentWordIndex, reader);
        exitFunction(0);
    }
}

/*!
 * \brief Resets all Argument instances assigned as mainArguments() and sub arguments.
 * \sa Argument::reset()
 */
void ArgumentParser::resetArgs()
{
    for (Argument *arg : m_mainArgs) {
        arg->resetRecursively();
    }
    m_actualArgc = 0;
}

/*!
 * \brief Returns the first operation argument specified by the user or nullptr if no operation has been specified.
 * \remarks Only main arguments are considered. See Argument::specifiedOperation() to check sub arguments of a specific
 *          argument.
 */
Argument *ArgumentParser::specifiedOperation() const
{
    for (Argument *arg : m_mainArgs) {
        if (arg->denotesOperation() && arg->isPresent()) {
            return arg;
        }
    }
    return nullptr;
}

/*!
 * \brief Checks whether at least one uncombinable main argument is present.
 */
bool ArgumentParser::isUncombinableMainArgPresent() const
{
    for (const Argument *arg : m_mainArgs) {
        if (!arg->isCombinable() && arg->isPresent()) {
            return true;
        }
    }
    return false;
}

#ifdef DEBUG_BUILD
/*!
 * \brief Verifies the specified \a argument definitions.
 *
 * Asserts that
 *  - The same argument has not been added twice to the same parent.
 *  - Only one argument within a parent is default or implicit.
 *  - Only main arguments denote operations.
 *  - Argument abbreviations are unique within the same level.
 *  - Argument names are unique within within the same level.
 *
 * \remarks
 *  - Verifies the sub arguments, too.
 *  - For debugging purposes only; hence only used in debug builds.
 */
void ApplicationUtilities::ArgumentParser::verifyArgs(const ArgumentVector &args)
{
    vector<const Argument *> verifiedArgs;
    verifiedArgs.reserve(args.size());
    vector<char> abbreviations;
    abbreviations.reserve(abbreviations.size() + args.size());
    vector<const char *> names;
    names.reserve(names.size() + args.size());
    bool hasImplicit = false;
    for (const Argument *arg : args) {
        assert(find(verifiedArgs.cbegin(), verifiedArgs.cend(), arg) == verifiedArgs.cend());
        verifiedArgs.push_back(arg);
        assert(!arg->isImplicit() || !hasImplicit);
        hasImplicit |= arg->isImplicit();
        assert(!arg->abbreviation() || find(abbreviations.cbegin(), abbreviations.cend(), arg->abbreviation()) == abbreviations.cend());
        abbreviations.push_back(arg->abbreviation());
        assert(!arg->name() || find_if(names.cbegin(), names.cend(), [arg](const char *name) { return !strcmp(arg->name(), name); }) == names.cend());
        assert(arg->requiredValueCount() == 0 || arg->subArguments().size() == 0);
        names.emplace_back(arg->name());
    }
    for (const Argument *arg : args) {
        verifyArgs(arg->subArguments());
    }
}
#endif

/*!
 * \brief Returns whether \a arg1 should be listed before \a arg2 when
 *        printing completion.
 *
 * Arguments are sorted by name (ascending order). However, all arguments
 * denoting an operation are listed before all other arguments.
 */
bool compareArgs(const Argument *arg1, const Argument *arg2)
{
    if (arg1->denotesOperation() && !arg2->denotesOperation()) {
        return true;
    } else if (!arg1->denotesOperation() && arg2->denotesOperation()) {
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
    for (const Argument *sibling : siblings) {
        if (sibling->isPresent() && !sibling->isCombinable()) {
            onlyCombinable = true;
            break;
        }
    }
    for (const Argument *sibling : siblings) {
        if ((!onlyCombinable || sibling->isCombinable()) && sibling->occurrences() < sibling->maxOccurrences()) {
            target.push_back(sibling);
        }
    }
}

/*!
 * \brief Determines arguments relevant for Bash completion or suggestions in case of typo.
 */
ArgumentCompletionInfo ArgumentParser::determineCompletionInfo(
    int argc, const char *const *argv, unsigned int currentWordIndex, const ArgumentReader &reader) const
{
    ArgumentCompletionInfo completion(reader);

    // determine last detected arg
    if (completion.lastDetectedArg) {
        completion.lastDetectedArgIndex = reader.lastArgDenotation - argv;
        completion.lastDetectedArgPath = completion.lastDetectedArg->path(completion.lastDetectedArg->occurrences() - 1);
    }

    // determine last arg, omitting trailing empty args
    if (argc) {
        completion.lastSpecifiedArgIndex = static_cast<unsigned int>(argc) - 1;
        completion.lastSpecifiedArg = argv + completion.lastSpecifiedArgIndex;
        for (; completion.lastSpecifiedArg >= argv && **completion.lastSpecifiedArg == '\0';
             --completion.lastSpecifiedArg, --completion.lastSpecifiedArgIndex)
            ;
    }

    // just return main arguments if no args detected
    if (!completion.lastDetectedArg || !completion.lastDetectedArg->isPresent()) {
        completion.nextArgumentOrValue = true;
        insertSiblings(m_mainArgs, completion.relevantArgs);
        completion.relevantArgs.sort(compareArgs);
        return completion;
    }

    completion.nextArgumentOrValue = currentWordIndex > completion.lastDetectedArgIndex;
    if (!completion.nextArgumentOrValue) {
        // since the argument could be detected (hopefully unambiguously?) just return it for "final completion"
        completion.relevantArgs.push_back(completion.lastDetectedArg);
        completion.relevantArgs.sort(compareArgs);
        return completion;
    }

    // define function to add parameter values of argument as possible completions
    const auto addValueCompletionsForArg = [&completion](const Argument *arg) {
        if (arg->valueCompletionBehaviour() & ValueCompletionBehavior::PreDefinedValues) {
            completion.relevantPreDefinedValues.push_back(arg);
        }
        if (!(arg->valueCompletionBehaviour() & ValueCompletionBehavior::FileSystemIfNoPreDefinedValues) || !arg->preDefinedCompletionValues()) {
            completion.completeFiles = completion.completeFiles || arg->valueCompletionBehaviour() & ValueCompletionBehavior::Files;
            completion.completeDirs = completion.completeDirs || arg->valueCompletionBehaviour() & ValueCompletionBehavior::Directories;
        }
    };

    // detect number of specified values
    auto currentValueCount = completion.lastDetectedArg->values(completion.lastDetectedArg->occurrences() - 1).size();
    // ignore values which are specified after the current word
    if (currentValueCount) {
        const auto currentWordIndexRelativeToLastDetectedArg = currentWordIndex - completion.lastDetectedArgIndex;
        if (currentValueCount > currentWordIndexRelativeToLastDetectedArg) {
            currentValueCount -= currentWordIndexRelativeToLastDetectedArg;
        } else {
            currentValueCount = 0;
        }
    }

    // add value completions for implicit child if there are no value specified and there are no values required by the
    // last detected argument itself
    if (!currentValueCount && !completion.lastDetectedArg->requiredValueCount()) {
        for (const Argument *child : completion.lastDetectedArg->subArguments()) {
            if (child->isImplicit() && child->requiredValueCount()) {
                addValueCompletionsForArg(child);
                break;
            }
        }
    }

    // add value completions for last argument if there are further values required
    if (completion.lastDetectedArg->requiredValueCount() == Argument::varValueCount
        || (currentValueCount < completion.lastDetectedArg->requiredValueCount())) {
        addValueCompletionsForArg(completion.lastDetectedArg);
    }

    if (completion.lastDetectedArg->requiredValueCount() == Argument::varValueCount
        || completion.lastDetectedArg->values(completion.lastDetectedArg->occurrences() - 1).size()
            >= completion.lastDetectedArg->requiredValueCount()) {
        // sub arguments of the last arg are possible completions
        for (const Argument *subArg : completion.lastDetectedArg->subArguments()) {
            if (subArg->occurrences() < subArg->maxOccurrences()) {
                completion.relevantArgs.push_back(subArg);
            }
        }

        // siblings of parents are possible completions as well
        for (auto parentArgument = completion.lastDetectedArgPath.crbegin(), end = completion.lastDetectedArgPath.crend();; ++parentArgument) {
            insertSiblings(parentArgument != end ? (*parentArgument)->subArguments() : m_mainArgs, completion.relevantArgs);
            if (parentArgument == end) {
                break;
            }
        }
    }

    return completion;
}

/*!
 * \brief Returns the suggestion string printed in error case due to unknown arguments.
 */
string ArgumentParser::findSuggestions(int argc, const char *const *argv, unsigned int cursorPos, const ArgumentReader &reader) const
{
    // determine completion info
    const auto completionInfo(determineCompletionInfo(argc, argv, cursorPos, reader));

    // determine the unknown/misspelled argument
    const auto *unknownArg(*reader.argv);
    auto unknownArgSize(strlen(unknownArg));
    // -> refuse suggestions for long args to prevent huge memory allocation for Damerau-Levenshtein algo
    if (unknownArgSize > 16) {
        return string();
    }
    // -> remove dashes since argument names internally don't have them
    if (unknownArgSize >= 2 && unknownArg[0] == '-' && unknownArg[1] == '-') {
        unknownArg += 2;
        unknownArgSize -= 2;
    }

    // find best suggestions limiting the results to 2
    multiset<ArgumentSuggestion> bestSuggestions;
    // -> consider relevant arguments
    for (const Argument *const arg : completionInfo.relevantArgs) {
        ArgumentSuggestion(unknownArg, unknownArgSize, arg->name(), !arg->denotesOperation()).addTo(bestSuggestions, 2);
    }
    // -> consider relevant values
    for (const Argument *const arg : completionInfo.relevantPreDefinedValues) {
        if (!arg->preDefinedCompletionValues()) {
            continue;
        }
        for (const char *i = arg->preDefinedCompletionValues(); *i; ++i) {
            const char *const wordStart(i);
            const char *wordEnd(wordStart + 1);
            for (; *wordEnd && *wordEnd != ' '; ++wordEnd)
                ;
            ArgumentSuggestion(unknownArg, unknownArgSize, wordStart, static_cast<size_t>(wordEnd - wordStart), false).addTo(bestSuggestions, 2);
            i = wordEnd;
        }
    }

    // format suggestion
    string suggestionStr;
    if (const auto suggestionCount = bestSuggestions.size()) {
        // allocate memory
        size_t requiredSize = 15;
        for (const auto &suggestion : bestSuggestions) {
            requiredSize += suggestion.suggestionSize + 2;
            if (suggestion.hasDashPrefix) {
                requiredSize += 2;
            }
        }
        suggestionStr.reserve(requiredSize);

        // add each suggestion to end up with something like "Did you mean status (1), pause (3), cat (4), edit (5) or rescan-all (8)?"
        suggestionStr += "\nDid you mean ";
        size_t i = 0;
        for (const auto &suggestion : bestSuggestions) {
            if (++i == suggestionCount && suggestionCount != 1) {
                suggestionStr += " or ";
            } else if (i > 1) {
                suggestionStr += ", ";
            }
            if (suggestion.hasDashPrefix) {
                suggestionStr += "--";
            }
            suggestionStr.append(suggestion.suggestion, suggestion.suggestionSize);
        }
        suggestionStr += '?';
    }
    return suggestionStr;
}

/*!
 * \brief Prints the bash completion for the specified arguments and the specified \a lastPath.
 * \remarks Arguments must have been parsed before with readSpecifiedArgs(). When calling this method, completionMode must
 *          be set to true.
 */
void ArgumentParser::printBashCompletion(int argc, const char *const *argv, unsigned int currentWordIndex, const ArgumentReader &reader) const
{
    // determine completion info and sort relevant arguments
    const auto completionInfo([&] {
        auto clutteredCompletionInfo(determineCompletionInfo(argc, argv, currentWordIndex, reader));
        clutteredCompletionInfo.relevantArgs.sort(compareArgs);
        return clutteredCompletionInfo;
    }());

    // read the "opening" (started but not finished argument denotation)
    const char *opening = nullptr;
    string compoundOpening;
    size_t openingLen = 0, compoundOpeningStartLen = 0;
    unsigned char openingDenotationType = Value;
    if (argc && completionInfo.nextArgumentOrValue) {
        if (currentWordIndex < static_cast<unsigned int>(argc)) {
            opening = argv[currentWordIndex];
            // For some reason completions for eg. "set --values disk=1 tag=a" are splitted so the
            // equation sign is an own argument ("set --values disk = 1 tag = a").
            // This is not how values are treated by the argument parser. Hence the opening
            // must be joined again. In this case only the part after the equation sign needs to be
            // provided for completion so compoundOpeningStartLen is set to number of characters to skip.
            const size_t minCurrentWordIndex = (completionInfo.lastDetectedArg ? completionInfo.lastDetectedArgIndex : 0);
            if (currentWordIndex > minCurrentWordIndex && !strcmp(opening, "=")) {
                compoundOpening.reserve(compoundOpeningStartLen = strlen(argv[--currentWordIndex]) + 1);
                compoundOpening = argv[currentWordIndex];
                compoundOpening += '=';
            } else if (currentWordIndex > (minCurrentWordIndex + 1) && !strcmp(argv[currentWordIndex - 1], "=")) {
                compoundOpening.reserve((compoundOpeningStartLen = strlen(argv[currentWordIndex -= 2]) + 1) + strlen(opening));
                compoundOpening = argv[currentWordIndex];
                compoundOpening += '=';
                compoundOpening += opening;
            }
            if (!compoundOpening.empty()) {
                opening = compoundOpening.data();
            }
        } else {
            opening = *completionInfo.lastSpecifiedArg;
        }
        *opening == '-' && (++opening, ++openingDenotationType) && *opening == '-' && (++opening, ++openingDenotationType);
        openingLen = strlen(opening);
    }

    // print "COMPREPLY" bash array
    cout << "COMPREPLY=(";
    // -> completions for parameter values
    bool noWhitespace = false;
    for (const Argument *const arg : completionInfo.relevantPreDefinedValues) {
        if (arg->valueCompletionBehaviour() & ValueCompletionBehavior::InvokeCallback && arg->m_callbackFunction) {
            arg->m_callbackFunction(arg->isPresent() ? arg->m_occurrences.front() : ArgumentOccurrence(Argument::varValueCount));
        }
        if (!arg->preDefinedCompletionValues()) {
            continue;
        }
        const bool appendEquationSign = arg->valueCompletionBehaviour() & ValueCompletionBehavior::AppendEquationSign;
        if (argc && currentWordIndex <= completionInfo.lastSpecifiedArgIndex && opening) {
            if (openingDenotationType != Value) {
                continue;
            }
            bool wordStart = true, ok = false, equationSignAlreadyPresent = false;
            size_t wordIndex = 0;
            for (const char *i = arg->preDefinedCompletionValues(), *end = opening + openingLen; *i;) {
                if (wordStart) {
                    const char *i1 = i, *i2 = opening;
                    for (; *i1 && i2 != end && *i1 == *i2; ++i1, ++i2)
                        ;
                    if ((ok = (i2 == end))) {
                        cout << '\'';
                    }
                    wordStart = false;
                    wordIndex = 0;
                } else if ((wordStart = (*i == ' ') || (*i == '\n'))) {
                    equationSignAlreadyPresent = false;
                    if (ok) {
                        cout << '\'' << ' ';
                    }
                    ++i;
                    continue;
                } else if (*i == '=') {
                    equationSignAlreadyPresent = true;
                }
                if (!ok) {
                    ++i;
                    continue;
                }
                if (!compoundOpeningStartLen || wordIndex >= compoundOpeningStartLen) {
                    if (*i == '\'') {
                        cout << "'\"'\"'";
                    } else {
                        cout << *i;
                    }
                }
                ++i, ++wordIndex;
                switch (*i) {
                case ' ':
                case '\n':
                case '\0':
                    if (appendEquationSign && !equationSignAlreadyPresent) {
                        cout << '=';
                        noWhitespace = true;
                        equationSignAlreadyPresent = false;
                    }
                    if (*i == '\0') {
                        cout << '\'';
                    }
                }
            }
            cout << ' ';
        } else if (const char *i = arg->preDefinedCompletionValues()) {
            bool equationSignAlreadyPresent = false;
            cout << '\'';
            while (*i) {
                if (*i == '\'') {
                    cout << "'\"'\"'";
                } else {
                    cout << *i;
                }
                switch (*(++i)) {
                case '=':
                    equationSignAlreadyPresent = true;
                    break;
                case ' ':
                case '\n':
                case '\0':
                    if (appendEquationSign && !equationSignAlreadyPresent) {
                        cout << '=';
                        equationSignAlreadyPresent = false;
                    }
                    if (*i != '\0') {
                        cout << '\'';
                        if (*(++i)) {
                            cout << ' ' << '\'';
                        }
                    }
                }
            }
            cout << '\'' << ' ';
        }
    }
    // -> completions for further arguments
    for (const Argument *const arg : completionInfo.relevantArgs) {
        if (argc && currentWordIndex <= completionInfo.lastSpecifiedArgIndex && opening) {
            switch (openingDenotationType) {
            case Value:
                if (!arg->denotesOperation() || strncmp(arg->name(), opening, openingLen)) {
                    continue;
                }
                break;
            case Abbreviation:
                break;
            case FullName:
                if (strncmp(arg->name(), opening, openingLen)) {
                    continue;
                }
            }
        }

        if (opening && openingDenotationType == Abbreviation && !completionInfo.nextArgumentOrValue) {
            // TODO: add test for this case
            cout << '\'' << '-' << opening << arg->abbreviation() << '\'' << ' ';
        } else if (completionInfo.lastDetectedArg && reader.argDenotationType == Abbreviation && !completionInfo.nextArgumentOrValue) {
            if (reader.argv == reader.end) {
                cout << '\'' << *(reader.argv - 1) << '\'' << ' ';
            }
        } else if (arg->denotesOperation()) {
            cout << '\'' << arg->name() << '\'' << ' ';
        } else {
            cout << '\'' << '-' << '-' << arg->name() << '\'' << ' ';
        }
    }
    // -> completions for files and dirs
    // -> if there's already an "opening", determine the dir part and the file part
    string actualDir, actualFile;
    bool haveFileOrDirCompletions = false;
    if (argc && currentWordIndex == completionInfo.lastSpecifiedArgIndex && opening) {
        // the "opening" might contain escaped characters which need to be unescaped first (let's hope this covers all possible escapings)
        string unescapedOpening(opening);
        findAndReplace<string>(unescapedOpening, "\\ ", " ");
        findAndReplace<string>(unescapedOpening, "\\,", ",");
        findAndReplace<string>(unescapedOpening, "\\[", "[");
        findAndReplace<string>(unescapedOpening, "\\]", "]");
        findAndReplace<string>(unescapedOpening, "\\!", "!");
        findAndReplace<string>(unescapedOpening, "\\#", "#");
        findAndReplace<string>(unescapedOpening, "\\$", "$");
        findAndReplace<string>(unescapedOpening, "\\'", "'");
        findAndReplace<string>(unescapedOpening, "\\\"", "\"");
        findAndReplace<string>(unescapedOpening, "\\\\", "\\");
        // determine the "directory" part
        string dir = directory(unescapedOpening);
        if (dir.empty()) {
            actualDir = ".";
        } else {
            if (dir[0] == '\"' || dir[0] == '\'') {
                dir.erase(0, 1);
            }
            if (dir.size() > 1 && (dir[dir.size() - 2] == '\"' || dir[dir.size() - 2] == '\'')) {
                dir.erase(dir.size() - 2, 1);
            }
            actualDir = move(dir);
        }
        // determine the "file" part
        string file = fileName(unescapedOpening);
        if (file[0] == '\"' || file[0] == '\'') {
            file.erase(0, 1);
        }
        if (file.size() > 1 && (file[dir.size() - 2] == '\"' || dir[file.size() - 2] == '\'')) {
            file.erase(file.size() - 2, 1);
        }
        actualFile = move(file);
    }

    // -> completion for files and dirs
    DirectoryEntryType entryTypes = DirectoryEntryType::None;
    if (completionInfo.completeFiles) {
        entryTypes |= DirectoryEntryType::File;
    }
    if (completionInfo.completeDirs) {
        entryTypes |= DirectoryEntryType::Directory;
    }
    if (entryTypes != DirectoryEntryType::None) {
        const string replace("'"), with("'\"'\"'");
        if (argc && currentWordIndex <= completionInfo.lastSpecifiedArgIndex && opening) {
            list<string> entries = directoryEntries(actualDir.c_str(), entryTypes);
            findAndReplace(actualDir, replace, with);
            for (string &dirEntry : entries) {
                if (!startsWith(dirEntry, actualFile)) {
                    continue;
                }
                cout << '\'';
                if (actualDir != ".") {
                    cout << actualDir;
                }
                findAndReplace(dirEntry, replace, with);
                cout << dirEntry << '\'' << ' ';
                haveFileOrDirCompletions = true;
            }
        } else {
            for (string &dirEntry : directoryEntries(".", entryTypes)) {
                findAndReplace(dirEntry, replace, with);
                cout << '\'' << dirEntry << '\'' << ' ';
                haveFileOrDirCompletions = true;
            }
        }
    }
    cout << ')';

    // ensure file or dir completions are formatted appropriately
    if (haveFileOrDirCompletions) {
        cout << "; compopt -o filenames";
    }

    // ensure trailing whitespace is ommitted
    if (noWhitespace) {
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
    for (const Argument *arg : args) {
        const auto occurrences = arg->occurrences();
        if (arg->isParentPresent() && occurrences > arg->maxOccurrences()) {
            throw Failure(argsToString("The argument \"", arg->name(), "\" mustn't be specified more than ", arg->maxOccurrences(),
                (arg->maxOccurrences() == 1 ? " time." : " times.")));
        }
        if (arg->isParentPresent() && occurrences < arg->minOccurrences()) {
            throw Failure(argsToString("The argument \"", arg->name(), "\" must be specified at least ", arg->minOccurrences(),
                (arg->minOccurrences() == 1 ? " time." : " times.")));
        }
        Argument *conflictingArgument = nullptr;
        if (arg->isMainArgument()) {
            if (!arg->isCombinable() && arg->isPresent()) {
                conflictingArgument = firstPresentUncombinableArg(m_mainArgs, arg);
            }
        } else {
            conflictingArgument = arg->conflictsWithArgument();
        }
        if (conflictingArgument) {
            throw Failure(argsToString("The argument \"", conflictingArgument->name(), "\" can not be combined with \"", arg->name(), "\"."));
        }
        for (size_t i = 0; i != occurrences; ++i) {
            if (!arg->allRequiredValuesPresent(i)) {
                stringstream ss(stringstream::in | stringstream::out);
                ss << "Not all parameter for argument \"" << arg->name() << "\" ";
                if (i) {
                    ss << " (" << (i + 1) << " occurrence) ";
                }
                ss << "provided. You have to provide the following parameter:";
                size_t valueNamesPrint = 0;
                for (const auto &name : arg->m_valueNames) {
                    ss << ' ' << name, ++valueNamesPrint;
                }
                if (arg->m_requiredValueCount != Argument::varValueCount) {
                    while (valueNamesPrint < arg->m_requiredValueCount) {
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
    for (const Argument *arg : args) {
        // invoke the callback for each occurrence of the argument
        if (arg->m_callbackFunction) {
            for (const auto &occurrence : arg->m_occurrences) {
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
HelpArgument::HelpArgument(ArgumentParser &parser)
    : Argument("help", 'h', "shows this information")
{
    setCallback([&parser](const ArgumentOccurrence &) {
        CMD_UTILS_START_CONSOLE;
        parser.printHelp(cout);
    });
}

/*!
 * \class OperationArgument
 * \brief The OperationArgument class is an Argument where denotesOperation() is true by default.
 */

/*!
 * \class ConfigValueArgument
 * \brief The ConfigValueArgument class is an Argument where setCombinable() is true by default.
 * \sa ConfigValueArgument::ConfigValueArgument()
 */

/*!
 * \class NoColorArgument
 * \brief The NoColorArgument class allows to specify whether use of escape codes or similar technique to provide formatted output
 *        on the terminal should be enabled/disabled.
 *
 * This argument will either prevent or explicitely allow the use of escape codes or similar technique to provide formatted output
 * on the terminal. More explicitly, the argument will always allow to negate the default value of EscapeCodes::enabled which can be
 * configured at build time by setting the CMake variable ENABLE_ESCAPE_CODES_BY_DEFAULT.
 *
 * \remarks
 * - Only the first instance is considered for actually altering the value of EscapeCodes::enabled so it makes no sense to
 *   instantiate this class multiple times.
 * - It is ensure that EscapeCodes::enabled will be set before any callback functions are invoked and even in the error case (if
 *   the error doesn't prevent the argument from being detected). Hence this feature is implemented via NoColorArgument::apply()
 *   rather than the usual callback mechanism.
 *
 * \sa NoColorArgument::NoColorArgument(), EscapeCodes::enabled
 */

NoColorArgument *NoColorArgument::s_instance = nullptr;

/*!
 * \brief Constructs a new NoColorArgument argument.
 * \remarks This will also set EscapeCodes::enabled to the value of the environment variable ENABLE_ESCAPE_CODES.
 */
NoColorArgument::NoColorArgument()
#ifdef CPP_UTILITIES_ESCAPE_CODES_ENABLED_BY_DEFAULT
    : Argument("no-color", '\0', "disables formatted/colorized output")
#else
    : Argument("enable-color", '\0', "enables formatted/colorized output")
#endif
{
    setCombinable(true);

    if (s_instance) {
        return;
    }
    s_instance = this;

    // set the environmentvariable: note that this is not directly used and just assigned for printing help
    setEnvironmentVariable("ENABLE_ESCAPE_CODES");

    // default-initialize EscapeCodes::enabled from environment variable
    const char *envValue = getenv(environmentVariable());
    if (!envValue) {
        return;
    }
    for (; *envValue; ++envValue) {
        switch (*envValue) {
        case '0':
        case ' ':
            break;
        default:
            // enable escape codes if ENABLE_ESCAPE_CODES contains anything else than spaces or zeros
            EscapeCodes::enabled = true;
            return;
        }
    }
    // disable escape codes if ENABLE_ESCAPE_CODES is empty or only contains spaces and zeros
    EscapeCodes::enabled = false;
}

/*!
 * \brief Destroys the object.
 */
NoColorArgument::~NoColorArgument()
{
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

/*!
 * \brief Sets EscapeCodes::enabled according to the presense of the first instantiation of NoColorArgument.
 */
void NoColorArgument::apply()
{
    if (NoColorArgument::s_instance && NoColorArgument::s_instance->isPresent()) {
#ifdef CPP_UTILITIES_ESCAPE_CODES_ENABLED_BY_DEFAULT
        EscapeCodes::enabled = false;
#else
        EscapeCodes::enabled = true;
#endif
    }
}

} // namespace ApplicationUtilities
