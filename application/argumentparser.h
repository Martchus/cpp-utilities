#ifndef APPLICATION_UTILITIES_ARGUMENTPARSER_H
#define APPLICATION_UTILITIES_ARGUMENTPARSER_H

#include "./global.h"

#include <vector>
#include <initializer_list>
#include <functional>
#ifdef DEBUG_BUILD
# include <cassert>
#endif

namespace ApplicationUtilities {

LIB_EXPORT extern const char *applicationName;
LIB_EXPORT extern const char *applicationAuthor;
LIB_EXPORT extern const char *applicationVersion;
LIB_EXPORT extern const char *applicationUrl;

#define SET_APPLICATION_INFO \
    ::ApplicationUtilities::applicationName = APP_NAME; \
    ::ApplicationUtilities::applicationAuthor = APP_AUTHOR; \
    ::ApplicationUtilities::applicationVersion = APP_VERSION; \
    ::ApplicationUtilities::applicationUrl = APP_URL

class Argument;
class ArgumentParser;

typedef std::initializer_list<Argument *> ArgumentInitializerList;
typedef std::vector<Argument *> ArgumentVector;
typedef std::function<bool (Argument *)> ArgumentPredicate;

Argument LIB_EXPORT *firstPresentUncombinableArg(const ArgumentVector &args, const Argument *except);

class LIB_EXPORT Argument
{
    friend class ArgumentParser;

public:
    typedef std::function <void (const std::vector<const char *> &)> CallbackFunction;

    Argument(const char *name, char abbreviation = '\0', const char *description = nullptr, const char *example = nullptr);
    ~Argument();

    const char *name() const;
    void setName(const char *name);
    char abbreviation() const;
    void setAbbreviation(char abbreviation);
    const char *description() const;
    void setDescription(const char *description);
    const char *example() const;
    void setExample(const char *example);
    const std::vector<const char *> &values(std::size_t occurrance = 0) const;
    std::size_t requiredValueCount() const;
    void setRequiredValueCount(std::size_t requiredValueCount);
    const std::vector<const char *> &valueNames() const;
    void setValueNames(std::initializer_list<const char *> valueNames);
    void appendValueName(const char *valueName);
    bool allRequiredValuesPresent(std::size_t occurrance = 0) const;
    bool isPresent() const;
    std::size_t occurrences() const;
    const std::vector<std::size_t> &indices() const;
    std::size_t minOccurrences() const;
    std::size_t maxOccurrences() const;
    void setConstraints(std::size_t minOccurrences, std::size_t maxOccurrences);
    bool isRequired() const;
    void setRequired(bool required);
    bool isCombinable() const;
    void setCombinable(bool value);
    bool isImplicit() const;
    void setImplicit(bool value);
    bool denotesOperation() const;
    void setDenotesOperation(bool denotesOperation);
    void setCallback(CallbackFunction callback);
    void printInfo(std::ostream &os, unsigned char indentionLevel = 0) const;
    const ArgumentVector &subArguments() const;
    void setSubArguments(const ArgumentInitializerList &subArguments);
    void addSubArgument(Argument *arg);
    bool hasSubArguments() const;
    const ArgumentVector parents() const;
    bool isMainArgument() const;
    bool isParentPresent() const;
    Argument *conflictsWithArgument() const;
    void reset();

private:
    const char *m_name;
    char m_abbreviation;
    const char *m_description;
    const char *m_example;
    std::size_t m_minOccurrences;
    std::size_t m_maxOccurrences;
    bool m_combinable;
    bool m_denotesOperation;
    std::size_t m_requiredValueCount;
    std::vector<const char *> m_valueNames;
    bool m_implicit;
    std::vector<std::size_t> m_indices;
    std::vector<std::vector<const char *> > m_values;
    ArgumentVector m_subArgs;
    CallbackFunction m_callbackFunction;
    ArgumentVector m_parents;
    bool m_isMainArg;
};

/*!
 * \brief Returns the name of the argument.
 *
 * The parser compares the name with the characters following a "--" prefix to
 * identify arguments.
 */
inline const char *Argument::name() const
{
    return m_name;
}

/*!
 * \brief Sets the name of the argument.
 *
 * The name mustn't be empty or contain white spaces or equation chars.
 *
 * The parser compares the name with the characters following a "--" prefix to
 * identify arguments.
 */
inline void Argument::setName(const char *name)
{
#ifdef DEBUG_BUILD
    if(name && *name) {
        for(const char *c = name; *c; ++c) {
            assert(*c != ' ' && *c != '=');
        }
    }
#endif
    m_name = name;
}

/*!
 * \brief Returns the abbreviation of the argument.
 *
 * The parser compares the abbreviation with the characters following a "-" prefix to
 * identify arguments.
 */
inline char Argument::abbreviation() const
{
    return m_abbreviation;
}

/*!
 * \brief Sets the abbreviation of the argument.
 *
 * The abbreviation might be empty but mustn't contain any white spaces or
 * equation chars when provided.
 *
 * The parser compares the abbreviation with the characters following a "-" prefix to
 * identify arguments.
 */
inline void Argument::setAbbreviation(char abbreviation)
{
    IF_DEBUG_BUILD(assert(abbreviation != ' ' && abbreviation != '='));
    m_abbreviation = abbreviation;
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
 * \brief Returns the additional values for the argument.
 *
 * These values set by the parser when parsing the command line arguments.
 */
inline const std::vector<const char *> &Argument::values(std::size_t occurrance) const
{
    return m_values[occurrance];
}

/*!
 * \brief Returns the number of values which are required to be given
 *        for this argument.
 *
 * The parser will expect that many values when parsing command line arguments.
 * A negative value indicates a variable number of arguments to be expected.
 *
 * The default value is 0.
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
 * A negative value indicates a variable number of arguments to be expected.
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
 * \sa appendValueNames()
 */
inline const std::vector<const char *> &Argument::valueNames() const
{
    return m_valueNames;
}

/*!
 * \brief Sets the names of the requried values. These names will be used
 *        when printing information about the argument.
 *
 * If the number of value names is higher then the number of requried values
 * the additional value names will be ignored.
 * If the number of value names is lesser then the number of requried values
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
inline bool Argument::allRequiredValuesPresent(std::size_t occurrance) const
{
    return m_requiredValueCount == static_cast<std::size_t>(-1)
            || (m_values[occurrance].size() >= static_cast<std::size_t>(m_requiredValueCount));
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
    return !m_indices.empty();
}

/*!
 * \brief Returns how often the argument could be detected when parsing.
 */
inline std::size_t Argument::occurrences() const
{
    return m_indices.size();
}

/*!
 * \brief Returns the indices of the argument's occurences which could be detected when parsing.
 */
inline const std::vector<std::size_t> &Argument::indices() const
{
    return m_indices;
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
    if(required) {
        if(!m_minOccurrences) {
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
 * \brief Sets if this argument can be combined.
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
 * The default value is false.
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
 * \brief Sets a \a callback function which will be called by the parser if
 *        the argument could be found and no parsing errors occured.
 * \remarks The \a callback will be called for each occurrance of the argument.
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

class LIB_EXPORT ArgumentParser
{
public:
    ArgumentParser();

    const ArgumentVector &mainArguments() const;
    void setMainArguments(const ArgumentInitializerList &mainArguments);
    void addMainArgument(Argument *argument);
    void printHelp(std::ostream &os) const;
    Argument *findArg(const ArgumentPredicate &predicate) const;
    static Argument *findArg(const ArgumentVector &arguments, const ArgumentPredicate &predicate);
    void parseArgs(int argc, const char *const *argv);
    unsigned int actualArgumentCount() const;
    const char *executable() const;
    bool areUnknownArgumentsIgnored() const;
    void setIgnoreUnknownArguments(bool ignore);
    Argument *defaultArgument() const;
    void setDefaultArgument(Argument *argument);

private:
    IF_DEBUG_BUILD(void verifyArgs(const ArgumentVector &args);)
    void readSpecifiedArgs(ArgumentVector &args, std::size_t &index, const char *const *&argv, const char *const *end, std::vector<Argument *> &currentPath);
    void checkConstraints(const ArgumentVector &args);
    void invokeCallbacks(const ArgumentVector &args);

    ArgumentVector m_mainArgs;
    unsigned int m_actualArgc;
    const char *m_executable;
    bool m_ignoreUnknownArgs;
    Argument *m_defaultArg;
};

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
 * \brief Returns an indication whether unknown arguments detected
 *        when parsing should be ignored.
 *
 * If unknown arguments are not ignored the parser will throw a
 * Failure when an unknown argument is detected.
 * Otherwise only a warning will be shown.
 *
 * The default value is false.
 *
 * \sa setIgnoreUnknownArguments()
 */
inline bool ArgumentParser::areUnknownArgumentsIgnored() const
{
    return m_ignoreUnknownArgs;
}

/*!
 * \brief Sets whether the parser should ignore unknown arguments
 *        when parsing.
 *
 * If set to false the parser should throw a Failure object
 * when an unknown argument is found. Otherwise only a warning
 * will be printed.
 *
 * \sa areUnknownArgumentsIgnored()
 */
inline void ArgumentParser::setIgnoreUnknownArguments(bool ignore)
{
    m_ignoreUnknownArgs = ignore;
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

class LIB_EXPORT HelpArgument : public Argument
{
public:
    HelpArgument(ArgumentParser &parser);
};

}

#endif // APPLICATION_UTILITIES_ARGUMENTPARSER_H
