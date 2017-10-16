#ifndef APPLICATION_UTILITIES_ARGUMENTPARSER_H
#define APPLICATION_UTILITIES_ARGUMENTPARSER_H

#include "../global.h"

#include <functional>
#include <initializer_list>
#include <limits>
#include <vector>
#ifdef DEBUG_BUILD
#include <cassert>
#endif

class ArgumentParserTests;

namespace ApplicationUtilities {

CPP_UTILITIES_EXPORT extern const char *applicationName;
CPP_UTILITIES_EXPORT extern const char *applicationAuthor;
CPP_UTILITIES_EXPORT extern const char *applicationVersion;
CPP_UTILITIES_EXPORT extern const char *applicationUrl;
CPP_UTILITIES_EXPORT extern std::initializer_list<const char *> dependencyVersions;

/*!
 * \macro SET_DEPENDENCY_INFO
 * \brief Sets meta data about the dependencies the application was linked against which is
 *        used by ArgumentParser::printHelp().
 * \remarks Reads those data from the config header so "config.h" must be included.
 */
#ifndef APP_STATICALLY_LINKED
#define SET_DEPENDENCY_INFO ::ApplicationUtilities::dependencyVersions = DEPENCENCY_VERSIONS
#else
#define SET_DEPENDENCY_INFO ::ApplicationUtilities::dependencyVersions = STATIC_DEPENCENCY_VERSIONS
#endif

/*!
 * \macro SET_APPLICATION_INFO
 * \brief Sets application meta data (including SET_DEPENDENCY_INFO) used by ArgumentParser::printHelp().
 * \remarks Reads those data from the config header so "config.h" must be included.
 */
#define SET_APPLICATION_INFO                                                                                                                         \
    ::ApplicationUtilities::applicationName = APP_NAME;                                                                                              \
    ::ApplicationUtilities::applicationAuthor = APP_AUTHOR;                                                                                          \
    ::ApplicationUtilities::applicationVersion = APP_VERSION;                                                                                        \
    ::ApplicationUtilities::applicationUrl = APP_URL;                                                                                                \
    SET_DEPENDENCY_INFO

CPP_UTILITIES_EXPORT extern void (*exitFunction)(int);

class Argument;
class ArgumentParser;
class ArgumentReader;

typedef std::initializer_list<Argument *> ArgumentInitializerList;
typedef std::vector<Argument *> ArgumentVector;
typedef std::function<bool(Argument *)> ArgumentPredicate;

/*!
 * \brief The UnknownArgumentBehavior enum specifies the behavior of the argument parser when an unknown
 *        argument is detected.
 */
enum class UnknownArgumentBehavior {
    Ignore, /**< Unknown arguments are ignored without warnings. */
    Warn, /**< A warning is printed to std::cerr if an unknown argument is detected. */
    Fail /**< Further parsing is aborted and an ApplicationUtilities::Failure instance with an error message is thrown. */
};

/*!
 * \brief The ValueCompletionBehavior enum specifies the items to be considered when generating completion for an argument value.
 * \remarks
 * - The enumeration items are meant to be combined using the |-operator.
 * - ValueCompletionBehavior::InvokeCallback is meant to initialize pre-defined values only when required in the callback assigned
 *   via Argument::setCallback(). Hence it makes sense to combine it with ValueCompletionBehavior::PreDefinedValues.
 * - When ValueCompletionBehavior::InvokeCallback is present, the callback assigned via Argument::setCallback() might be called
 *   even when not all constraints are fulfilled. So, for instance, there might not be all required values present.
 */
enum class ValueCompletionBehavior : unsigned char {
    None = 0, /**< no auto-completion */
    PreDefinedValues = 2, /**< values assigned with Argument::setPreDefinedCompletionValues() */
    Files = 4, /**< files */
    Directories = 8, /**< directories */
    FileSystemIfNoPreDefinedValues = 16, /**< files and directories but only if no values have been assigned (default behavior) */
    AppendEquationSign = 32, /**< an equation sign is appended to values which not contain an equation sign already */
    InvokeCallback = 64, /**< whether to invoke the callback before reading pre-defined values */
};

/// \cond
constexpr ValueCompletionBehavior operator|(ValueCompletionBehavior lhs, ValueCompletionBehavior rhs)
{
    return static_cast<ValueCompletionBehavior>(static_cast<unsigned char>(lhs) | static_cast<unsigned char>(rhs));
}

constexpr bool operator&(ValueCompletionBehavior lhs, ValueCompletionBehavior rhs)
{
    return static_cast<bool>(static_cast<unsigned char>(lhs) & static_cast<unsigned char>(rhs));
}
/// \endcond

Argument CPP_UTILITIES_EXPORT *firstPresentUncombinableArg(const ArgumentVector &args, const Argument *except);

/*!
 * \brief The ArgumentOccurrence struct holds argument values for an occurrence of an argument.
 */
struct CPP_UTILITIES_EXPORT ArgumentOccurrence {
    ArgumentOccurrence(std::size_t index);
    ArgumentOccurrence(std::size_t index, const std::vector<Argument *> parentPath, Argument *parent);

    /*!
     * \brief The index of the occurrence. This is not necessarily the index in the argv array.
     */
    std::size_t index;

    /*!
     * \brief The parameter values which have been specified after the occurrence of the argument.
     */
    std::vector<const char *> values;

    /*!
     * \brief The "path" of the occurrence (the parent elements which have been specified before).
     * \remarks Empty for top-level occurrences.
     */
    std::vector<Argument *> path;
};

/*!
 * \brief Constructs an argument occurrence for the specified \a index.
 */
inline ArgumentOccurrence::ArgumentOccurrence(std::size_t index)
    : index(index)
{
}

/*!
 * \brief Constructs an argument occurrence.
 * \param index Specifies the index.
 * \param parentPath Specifies the path of \a parent.
 * \param parent Specifies the parent which might be nullptr for top-level occurrences.
 *
 * The path of the new occurrence is built from the specified \a parentPath and \a parent.
 */
inline ArgumentOccurrence::ArgumentOccurrence(std::size_t index, const std::vector<Argument *> parentPath, Argument *parent)
    : index(index)
    , path(parentPath)
{
    if (parent) {
        path.push_back(parent);
    }
}

class CPP_UTILITIES_EXPORT Argument {
    friend ArgumentParser;
    friend ArgumentReader;
    friend ArgumentParserTests;

public:
    typedef std::function<void(const ArgumentOccurrence &)> CallbackFunction;

    Argument(const char *name, char abbreviation = '\0', const char *description = nullptr, const char *example = nullptr);
    ~Argument();

    const char *name() const;
    void setName(const char *name);
    char abbreviation() const;
    void setAbbreviation(char abbreviation);
    const char *environmentVariable() const;
    void setEnvironmentVariable(const char *environmentVariable);
    const char *description() const;
    void setDescription(const char *description);
    const char *example() const;
    void setExample(const char *example);
    const std::vector<const char *> &values(std::size_t occurrence = 0) const;
    const char *firstValue() const;
    std::size_t requiredValueCount() const;
    void setRequiredValueCount(std::size_t requiredValueCount);
    const std::vector<const char *> &valueNames() const;
    void setValueNames(std::initializer_list<const char *> valueNames);
    void appendValueName(const char *valueName);
    bool allRequiredValuesPresent(std::size_t occurrence = 0) const;
    bool isPresent() const;
    std::size_t occurrences() const;
    std::size_t index(std::size_t occurrence) const;
    std::size_t minOccurrences() const;
    std::size_t maxOccurrences() const;
    void setConstraints(std::size_t minOccurrences, std::size_t maxOccurrences);
    const std::vector<Argument *> &path(std::size_t occurrence = 0) const;
    bool isRequired() const;
    void setRequired(bool required);
    bool isCombinable() const;
    void setCombinable(bool value);
    bool isImplicit() const;
    void setImplicit(bool value);
    bool denotesOperation() const;
    void setDenotesOperation(bool denotesOperation);
    const CallbackFunction &callback() const;
    void setCallback(CallbackFunction callback);
    void printInfo(std::ostream &os, unsigned char indentation = 0) const;
    const ArgumentVector &subArguments() const;
    void setSubArguments(const ArgumentInitializerList &subArguments);
    void addSubArgument(Argument *arg);
    bool hasSubArguments() const;
    const ArgumentVector parents() const;
    bool isMainArgument() const;
    bool isParentPresent() const;
    ValueCompletionBehavior valueCompletionBehaviour() const;
    void setValueCompletionBehavior(ValueCompletionBehavior valueCompletionBehaviour);
    const char *preDefinedCompletionValues() const;
    void setPreDefinedCompletionValues(const char *preDefinedCompletionValues);
    Argument *conflictsWithArgument() const;
    Argument *wouldConflictWithArgument() const;
    Argument *specifiedOperation() const;
    void reset();
    void resetRecursively();

    /*!
     * \brief Denotes a variable number of values.
     * \sa setRequiredValueCount()
     */
    static constexpr std::size_t varValueCount = std::numeric_limits<std::size_t>::max();

private:
    const char *m_name;
    char m_abbreviation;
    const char *m_environmentVar;
    const char *m_description;
    const char *m_example;
    std::size_t m_minOccurrences;
    std::size_t m_maxOccurrences;
    bool m_combinable;
    bool m_denotesOperation;
    std::size_t m_requiredValueCount;
    std::vector<const char *> m_valueNames;
    bool m_implicit;
    std::vector<ArgumentOccurrence> m_occurrences;
    ArgumentVector m_subArgs;
    CallbackFunction m_callbackFunction;
    ArgumentVector m_parents;
    bool m_isMainArg;
    ValueCompletionBehavior m_valueCompletionBehavior;
    const char *m_preDefinedCompletionValues;
};

class CPP_UTILITIES_EXPORT ArgumentParser {
    friend ArgumentParserTests;
    friend ArgumentReader;

public:
    ArgumentParser();

    const ArgumentVector &mainArguments() const;
    void setMainArguments(const ArgumentInitializerList &mainArguments);
    void addMainArgument(Argument *argument);
    void printHelp(std::ostream &os) const;
    void parseArgs(int argc, const char *const *argv);
    void parseArgsOrExit(int argc, const char *const *argv);
    void readArgs(int argc, const char *const *argv);
    void resetArgs();
    unsigned int actualArgumentCount() const;
    const char *executable() const;
    UnknownArgumentBehavior unknownArgumentBehavior() const;
    void setUnknownArgumentBehavior(UnknownArgumentBehavior behavior);
    Argument *defaultArgument() const;
    void setDefaultArgument(Argument *argument);
    Argument *specifiedOperation() const;
    void checkConstraints();
    void invokeCallbacks();
    bool isUncombinableMainArgPresent() const;

private:
    IF_DEBUG_BUILD(void verifyArgs(const ArgumentVector &args, std::vector<char> abbreviations, std::vector<const char *> names);)
    void printBashCompletion(int argc, const char *const *argv, unsigned int cursorPos, const ArgumentReader &reader);
    void checkConstraints(const ArgumentVector &args);
    static void invokeCallbacks(const ArgumentVector &args);

    ArgumentVector m_mainArgs;
    unsigned int m_actualArgc;
    const char *m_executable;
    UnknownArgumentBehavior m_unknownArgBehavior;
    Argument *m_defaultArg;
};

/*!
 * \brief Returns the name of the argument.
 *
 * The parser compares the name with the characters following a "--" prefix to identify arguments.
 */
inline const char *Argument::name() const
{
    return m_name;
}

/*!
 * \brief Sets the name of the argument.
 *
 * The name mustn't be empty, start with a minus or contain white spaces, equation chars, quotes and newlines.
 *
 * The parser compares the name with the characters following a "--" prefix to identify arguments.
 */
inline void Argument::setName(const char *name)
{
#ifdef DEBUG_BUILD
    if (name && *name) {
        assert(*name != '-');
        for (const char *c = name; *c; ++c) {
            assert(*c != ' ' && *c != '=' && *c != '\'' && *c != '\"' && *c != '\n' && *c != '\r');
        }
    }
#endif
    m_name = name;
}

/*!
 * \brief Returns the abbreviation of the argument.
 *
 * The parser compares the abbreviation with the characters following a "-" prefix to identify arguments.
 */
inline char Argument::abbreviation() const
{
    return m_abbreviation;
}

/*!
 * \brief Sets the abbreviation of the argument.
 *
 * The abbreviation might be empty but mustn't be white spaces, equation char, single quote, double quote or newline.
 *
 * The parser compares the abbreviation with the characters following a "-" prefix to identify arguments.
 */
inline void Argument::setAbbreviation(char abbreviation)
{
    IF_DEBUG_BUILD(assert(abbreviation != ' ' && abbreviation != '=' && abbreviation != '-' && abbreviation != '\'' && abbreviation != '"'
        && abbreviation != '\n' && abbreviation != '\r'));
    m_abbreviation = abbreviation;
}

/*!
 * \brief Returns the environment variable queried when firstValue() is called.
 * \sa firstValue(), setEnvironmentVariable()
 */
inline const char *Argument::environmentVariable() const
{
    return m_environmentVar;
}

/*!
 * \brief Sets the environment variable queried when firstValue() is called.
 * \sa firstValue(), environmentVariable()
 */
inline void Argument::setEnvironmentVariable(const char *environmentVariable)
{
    m_environmentVar = environmentVariable;
}

/*!
 * \brief Returns the description of the argument.
 *
 * The parser uses the description when printing help information.
 */
inline const char *Argument::description() const
{
    return m_description;
}

/*!
 * \brief Sets the description of the argument.
 *
 * The parser uses the description when printing help information.
 */
inline void Argument::setDescription(const char *description)
{
    m_description = description;
}

/*!
 * \brief Returns the usage example of the argument.
 *
 * The parser uses the description when printing help information.
 */
inline const char *Argument::example() const
{
    return m_example;
}

/*!
 * \brief Sets the a usage example for the argument.
 *
 * The parser uses the description when printing help information.
 */
inline void Argument::setExample(const char *example)
{
    m_example = example;
}

/*!
 * \brief Returns the parameter values for the specified \a occurrence of argument.
 * \remarks
 *  - The values are set by the parser when parsing the command line arguments.
 *  - The specified \a occurrence must be less than occurrences().
 */
inline const std::vector<const char *> &Argument::values(std::size_t occurrence) const
{
    return m_occurrences[occurrence].values;
}

/*!
 * \brief Returns the number of values which are required to be given
 *        for this argument.
 *
 * The parser will expect that many values when parsing command line arguments.
 * A negative value indicates a variable number of arguments to be expected.
 *
 * The default value is 0, except for ConfigValueArgument instances.
 *
 * \sa setRequiredValueCount()
 * \sa valueNames()
 * \sa setValueNames()
 */
inline std::size_t Argument::requiredValueCount() const
{
    return m_requiredValueCount;
}

/*!
 * \brief Sets the number of values which are required to be given
 *        for this argument.
 *
 * The parser will expect that many values when parsing command line arguments.
 * Pass Argument::varValueCount for a variable number of arguments
 * to be expected.
 *
 * \sa requiredValueCount()
 * \sa valueNames()
 * \sa setValueNames()
 */
inline void Argument::setRequiredValueCount(std::size_t requiredValueCount)
{
    m_requiredValueCount = requiredValueCount;
}

/*!
 * \brief Returns the names of the requried values.
 *
 * These names will be shown when printing information about the argument.
 *
 * \sa setValueNames()
 * \sa appendValueName()
 */
inline const std::vector<const char *> &Argument::valueNames() const
{
    return m_valueNames;
}

/*!
 * \brief Sets the names of the requried values. These names will be used
 *        when printing information about the argument.
 *
 * If the number of value names is higher than the number of requried values
 * the additional value names will be ignored.
 * If the number of value names is lesser than the number of requried values
 * generic values will be used for the missing names.
 *
 * \sa appendValueName()
 * \sa valueNames()
 * \sa requiredValueCount()
 */
inline void Argument::setValueNames(std::initializer_list<const char *> valueNames)
{
    m_valueNames.assign(valueNames);
}

/*!
 * \brief Appends a value name. The value names names will be shown
 *        when printing information about the argument.
 * \sa setValueNames()
 * \sa valueNames()
 */
inline void Argument::appendValueName(const char *valueName)
{
    m_valueNames.emplace_back(valueName);
}

/*!
 * \brief Returns an indication whether all required values are present.
 */
inline bool Argument::allRequiredValuesPresent(std::size_t occurrence) const
{
    return m_requiredValueCount == static_cast<std::size_t>(-1)
        || (m_occurrences[occurrence].values.size() >= static_cast<std::size_t>(m_requiredValueCount));
}

/*!
 * \brief Returns an indication whether the argument is an implicit argument.
 * \sa setImplicit()
 */
inline bool Argument::isImplicit() const
{
    return m_implicit;
}

/*!
 * \brief Sets whether the argument is an implicit argument.
 * \sa isImplicit()
 */
inline void Argument::setImplicit(bool implicit)
{
    m_implicit = implicit;
}

/*!
 * \brief Returns an indication whether the argument could be detected when parsing.
 */
inline bool Argument::isPresent() const
{
    return !m_occurrences.empty();
}

/*!
 * \brief Returns how often the argument could be detected when parsing.
 */
inline std::size_t Argument::occurrences() const
{
    return m_occurrences.size();
}

/*!
 * \brief Returns the indices of the argument's occurences which could be detected when parsing.
 */
inline std::size_t Argument::index(std::size_t occurrence) const
{
    return m_occurrences[occurrence].index;
}

/*!
 * \brief Returns the minimum number of occurrences.
 *
 * If the argument occurs not that many times, the parser will complain.
 */
inline std::size_t Argument::minOccurrences() const
{
    return m_minOccurrences;
}

/*!
 * \brief Returns the maximum number of occurrences.
 *
 * If the argument occurs more often, the parser will complain.
 */
inline std::size_t Argument::maxOccurrences() const
{
    return m_maxOccurrences;
}

/*!
 * \brief Sets the allowed number of occurrences.
 * \sa minOccurrences()
 * \sa maxOccurrences()
 */
inline void Argument::setConstraints(std::size_t minOccurrences, std::size_t maxOccurrences)
{
    m_minOccurrences = minOccurrences;
    m_maxOccurrences = maxOccurrences;
}

/*!
 * \brief Returns the path of the specified \a occurrence.
 */
inline const std::vector<Argument *> &Argument::path(std::size_t occurrence) const
{
    return m_occurrences[occurrence].path;
}

/*!
 * \brief Returns an indication whether the argument is mandatory.
 *
 * The parser will complain if a mandatory argument is not present.
 *
 * The default value is false.
 *
 * \sa setRequired()
 */
inline bool Argument::isRequired() const
{
    return m_minOccurrences;
}

/*!
 * \brief Sets whether this argument is mandatory or not.
 *
 * The parser will complain if a mandatory argument is not present.
 *
 * * \sa isRequired()
 */
inline void Argument::setRequired(bool required)
{
    if (required) {
        if (!m_minOccurrences) {
            m_minOccurrences = 1;
        }
    } else {
        m_minOccurrences = 0;
    }
}

/*!
 * \brief Returns an indication whether the argument is combinable.
 *
 * The parser will complain if two arguments labeled as uncombinable are
 * present at the same time.
 *
 * \sa setCombinable()
 */
inline bool Argument::isCombinable() const
{
    return m_combinable;
}

/*!
 * \brief Sets whether this argument can be combined.
 *
 * The parser will complain if two arguments labeled as uncombinable are
 * present at the same time.
 *
 * \sa isCombinable()
 */
inline void Argument::setCombinable(bool value)
{
    m_combinable = value;
}

/*!
 * \brief Returns whether the argument denotes the operation.
 *
 * An argument which denotes the operation might be specified
 * withouth "--" or "-" prefix as first main argument.
 *
 * The default value is false, except for OperationArgument instances.
 *
 * \sa setDenotesOperation()
 */
inline bool Argument::denotesOperation() const
{
    return m_denotesOperation;
}

/*!
 * \brief Sets whether the argument denotes the operation.
 * \sa denotesOperation()
 */
inline void Argument::setDenotesOperation(bool denotesOperation)
{
    m_denotesOperation = denotesOperation;
}

/*!
 * \brief Returns the assigned callback function.
 * \sa setCallback()
 */
inline const Argument::CallbackFunction &Argument::callback() const
{
    return m_callbackFunction;
}

/*!
 * \brief Sets a \a callback function which will be called by the parser if
 *        the argument could be found and no parsing errors occured.
 * \remarks The \a callback will be called for each occurrence of the argument.
 * \sa callback()
 */
inline void Argument::setCallback(Argument::CallbackFunction callback)
{
    m_callbackFunction = callback;
}

/*!
 * \brief Returns the secondary arguments for this argument.
 *
 * \sa setSecondaryArguments()
 * \sa hasSecondaryArguments()
 */
inline const ArgumentVector &Argument::subArguments() const
{
    return m_subArgs;
}

/*!
 * \brief Returns an indication whether the argument has secondary arguments.
 *
 * \sa secondaryArguments()
 * \sa setSecondaryArguments()
 */
inline bool Argument::hasSubArguments() const
{
    return !m_subArgs.empty();
}

/*!
 * \brief Returns the parents of this argument.
 *
 * If this argument is used as secondary argument, the arguments which
 * contain this argument as secondary arguments are returned
 * as "parents" of this argument.
 *
 * If this argument is used as a main argument shouldn't be used as
 * secondary argument at the same time and thus have no parents.
 */
inline const ArgumentVector Argument::parents() const
{
    return m_parents;
}

/*!
 * \brief Returns an indication whether the argument is used as main argument.
 *
 * An argument used as main argument shouldn't be used as secondary
 * arguments at the same time.
 */
inline bool Argument::isMainArgument() const
{
    return m_isMainArg;
}

/*!
 * \brief Returns the items to be considered when generating completion for the values.
 */
inline ValueCompletionBehavior Argument::valueCompletionBehaviour() const
{
    return m_valueCompletionBehavior;
}

/*!
 * \brief Sets the items to be considered when generating completion for the values.
 */
inline void Argument::setValueCompletionBehavior(ValueCompletionBehavior completionValues)
{
    m_valueCompletionBehavior = completionValues;
}

/*!
 * \brief Returns the assigned values used when generating completion for the values.
 */
inline const char *Argument::preDefinedCompletionValues() const
{
    return m_preDefinedCompletionValues;
}

/*!
 * \brief Assignes the values to be used when generating completion for the values.
 */
inline void Argument::setPreDefinedCompletionValues(const char *preDefinedCompletionValues)
{
    m_preDefinedCompletionValues = preDefinedCompletionValues;
}

/*!
 * \brief Resets occurrences (indices, values and paths).
 */
inline void Argument::reset()
{
    m_occurrences.clear();
}

/*!
 * \brief Returns the main arguments.
 * \sa setMainArguments()
 */
inline const ArgumentVector &ArgumentParser::mainArguments() const
{
    return m_mainArgs;
}

/*!
 * \brief Returns the actual number of arguments that could be found when parsing.
 */
inline unsigned int ArgumentParser::actualArgumentCount() const
{
    return m_actualArgc;
}

/*!
 * \brief Returns the name of the current executable.
 */
inline const char *ArgumentParser::executable() const
{
    return m_executable;
}

/*!
 * \brief Returns how unknown arguments are treated.
 *
 * The default value is UnknownArgumentBehavior::Fail.
 */
inline UnknownArgumentBehavior ArgumentParser::unknownArgumentBehavior() const
{
    return m_unknownArgBehavior;
}

/*!
 * \brief Sets how unknown arguments are treated.
 *
 * The default value is UnknownArgumentBehavior::Fail.
 */
inline void ArgumentParser::setUnknownArgumentBehavior(UnknownArgumentBehavior behavior)
{
    m_unknownArgBehavior = behavior;
}

/*!
 * \brief Returns the default argument.
 * \remarks The default argument is assumed to be present if no other arguments have been specified.
 */
inline Argument *ArgumentParser::defaultArgument() const
{
    return m_defaultArg;
}

/*!
 * \brief Sets the default argument.
 * \remarks The default argument is assumed to be present if no other arguments have been specified.
 */
inline void ArgumentParser::setDefaultArgument(Argument *argument)
{
    m_defaultArg = argument;
}

/*!
 * \brief Checks whether contraints are violated.
 * \remarks Automatically called by parseArgs().
 * \throws Throws Failure if constraints are violated.
 */
inline void ArgumentParser::checkConstraints()
{
    checkConstraints(m_mainArgs);
}

/*!
 * \brief Invokes all assigned callbacks.
 * \remarks Automatically called by parseArgs().
 */
inline void ArgumentParser::invokeCallbacks()
{
    invokeCallbacks(m_mainArgs);
}

class CPP_UTILITIES_EXPORT HelpArgument : public Argument {
public:
    HelpArgument(ArgumentParser &parser);
};

class CPP_UTILITIES_EXPORT OperationArgument : public Argument {
public:
    OperationArgument(const char *name, char abbreviation = '\0', const char *description = nullptr, const char *example = nullptr);
};

inline OperationArgument::OperationArgument(const char *name, char abbreviation, const char *description, const char *example)
    : Argument(name, abbreviation, description, example)
{
    setDenotesOperation(true);
}

class CPP_UTILITIES_EXPORT ConfigValueArgument : public Argument {
public:
    ConfigValueArgument(const char *name, char abbreviation = '\0', const char *description = nullptr,
        std::initializer_list<const char *> valueNames = std::initializer_list<const char *>());
};

/*!
 * \brief Constructs a new ConfigValueArgument with the specified parameter. The initial value of requiredValueCount() is set to size of specified \a valueNames.
 */
inline ConfigValueArgument::ConfigValueArgument(
    const char *name, char abbreviation, const char *description, std::initializer_list<const char *> valueNames)
    : Argument(name, abbreviation, description)
{
    setCombinable(true);
    setRequiredValueCount(valueNames.size());
    setValueNames(valueNames);
}

class CPP_UTILITIES_EXPORT NoColorArgument : public Argument {
public:
    NoColorArgument();
    ~NoColorArgument();
    static void apply();

private:
    static NoColorArgument *s_instance;
};

} // namespace ApplicationUtilities

#endif // APPLICATION_UTILITIES_ARGUMENTPARSER_H
