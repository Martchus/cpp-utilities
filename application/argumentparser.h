#ifndef ARGUMENTPARSER_H
#define ARGUMENTPARSER_H

#include "global.h"

#include <string>
#include <vector>
#include <list>
#include <initializer_list>
#include <functional>
#include <stdexcept>

namespace ApplicationUtilities {

class Argument;

class ArgumentParser;

typedef std::initializer_list<Argument *> ArgumentInitializerList;
typedef std::vector<Argument *> ArgumentVector;
typedef std::vector<std::string> StringVector;
typedef std::list<std::string> StringList;
typedef std::function<bool (Argument *)> ArgumentPredicate;

Argument LIB_EXPORT *firstPresentUncombinableArg(const ArgumentVector &args, const Argument *except);

class LIB_EXPORT Argument
{
    friend class ArgumentParser;

public:
    typedef std::function <void (const StringVector &)> CallbackFunction;

    Argument(const std::string &name, const std::string abbreviation = std::string(), const std::string &description = std::string());
    Argument(const char *name, const char *abbreviation = nullptr, const char *description = nullptr);
    ~Argument();

    const std::string &name() const;
    void setName(const std::string &name);
    const std::string &abbreviation() const;
    void setAbbreviation(const std::string &abbreviation);
    //unsigned char isAmbiguous(const ArgumentParser &parser) const;
    const std::string &description() const;
    void setDescription(const std::string &description);
    const StringVector &values() const;
    const std::string &value(StringVector::size_type index) const;
    StringVector::size_type valueCount() const;
    int requiredValueCount() const;
    void setRequiredValueCount(int requiredValueCount);
    const StringList &valueNames() const;
    void setValueNames(std::initializer_list<std::string> valueNames);
    void appendValueName(const char *valueName);
    void appendValueName(const std::string &valueName);
    bool allRequiredValuesPresent() const;
    bool isDefault() const;
    void setDefault(bool value);
    const StringVector &defaultValues() const;
    void setDefaultValues(const StringVector &defaultValues);
    bool isPresent() const;
    bool isRequired() const;
    void setRequired(bool value);
    bool isCombinable() const;
    void setCombinable(bool value);
    bool isImplicit() const;
    void setImplicit(bool value);
    bool denotesOperation() const;
    void setDenotesOperation(bool denotesOperation);
    void setCallback(CallbackFunction callback);
    void printInfo(std::ostream &os, unsigned char indentionLevel = 0) const;
    const ArgumentVector &secondaryArguments() const;
    void setSecondaryArguments(const ArgumentInitializerList &secondaryArguments);
    bool hasSecondaryArguments() const;
    const ArgumentVector parents() const;
    bool isMainArgument() const;
    std::string parentNames() const;
    bool isParentPresent() const;
    Argument *conflictsWithArgument() const;

private:
    std::string m_name;
    std::string m_abbreviation;
    std::string m_description;
    bool m_required;
    bool m_combinable;
    bool m_implicit;
    bool m_denotesOperation;
    int m_requiredValueCount;
    StringList m_valueNames;
    bool m_default;
    StringVector m_defaultValues;
    bool m_present;
    StringVector m_values;
    ArgumentVector m_secondaryArgs;
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
inline const std::string &Argument::name() const
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
inline void Argument::setName(const std::string &name)
{
    if(name.empty() || name.find(' ') != std::string::npos || name.find('=') != std::string::npos) {
        throw std::invalid_argument("name mustn't be empty or contain white spaces or equation chars");
    }
    m_name = name;
}

/*!
 * \brief Returns the abbreviation of the argument.
 *
 * The parser compares the abbreviation with the characters following a "-" prefix to
 * identify arguments.
 */
inline const std::string &Argument::abbreviation() const
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
inline void Argument::setAbbreviation(const std::string &abbreviation)
{
    if(!abbreviation.empty() && (abbreviation.find(' ') != std::string::npos || abbreviation.find('=') != std::string::npos)) {
        throw std::invalid_argument("abbreviation mustn't contain white spaces or equation chars");
    }
    m_abbreviation = abbreviation;
}

/*!
 * \brief Returns the description of the argument.
 *
 * The parser uses the description when printing help information.
 */
inline const std::string &Argument::description() const
{
    return m_description;
}

/*!
 * \brief Sets the description of the argument.
 *
 * The parser uses the description when printing help information.
 */
inline void Argument::setDescription(const std::string &description)
{
    m_description = description;
}

/*!
 * \brief Returns the additional values for the argument.
 *
 * These values set by the parser when the command line arguments.
 */
inline const StringVector &Argument::values() const
{
    return m_values;
}

/*!
 * \brief Returns the value with the give \a index.
 *
 * These values set by the parser when the command line arguments.
 */
inline const std::string &Argument::value(StringVector::size_type index) const
{
    return m_values[index];
}

/*!
 * Returns the number of values which could be found when parsing
 * the command line arguments.
 */
inline StringVector::size_type Argument::valueCount() const
{
    return m_values.size();
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
inline int Argument::requiredValueCount() const
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
inline void Argument::setRequiredValueCount(int requiredValueCount)
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
inline const StringList &Argument::valueNames() const
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
inline void Argument::setValueNames(std::initializer_list<std::string> valueNames)
{
    m_valueNames = std::list<std::string>(valueNames);
}

/*!
 * \brief Appends a value name. The value names names will be shown
 *        when printing information about the argument.
 */
inline void Argument::appendValueName(const char *valueName)
{
    m_valueNames.emplace_back(valueName);
}

/*!
 * \brief Appends a value name. The value names names will be shown
 *        when printing information about the argument.
 *
 * \sa setValueNames()
 * \sa valueNames()
 */
inline void Argument::appendValueName(const std::string &valueName)
{
    m_valueNames.push_back(valueName);
}

/*!
 * \brief Returns an indication whether all required values are present.
 */
inline bool Argument::allRequiredValuesPresent() const
{
    if(m_requiredValueCount > 0) {
        return (m_values.size() >= static_cast<size_t>(m_requiredValueCount))
                || (m_default && m_defaultValues.size() >= static_cast<size_t>(m_requiredValueCount));
    } else {
        return true;
    }
}

/*!
 * \brief Returns an indication whether the argument is a default argument.
 *
 * A default argument will be flagged as present when parsing arguments event
 * if it is not actually present if there is no uncombinable argument present
 * and it is a main argument or the parent is present.
 *
 * The callback function will be invoked in this case as the argument where
 * actually present.
 *
 * The default value (for this property) is false.
 *
 * \sa setDefault()
 */
inline bool Argument::isDefault() const
{
    return m_default;
}

/*!
 * \brief Sets whether the argument is a default argument.
 * \sa isDefault()
 */
inline void Argument::setDefault(bool value)
{
    m_default = value;
}

/*!
 * \brief Returns the default values.
 * \sa isDefault()
 * \sa setDefault()
 * \sa setDefaultValues()
 */
inline const StringVector &Argument::defaultValues() const
{
    return m_defaultValues;
}

/*!
 * \brief Sets the default values.
 *
 * There must be as many default values as required values
 * if the argument is a default argument.
 *
 * \sa isDefault()
 * \sa setDefault()
 * \sa defaultValues()
 */
inline void Argument::setDefaultValues(const StringVector &defaultValues)
{
    m_defaultValues = defaultValues;
}

/*!
 * \brief Returns an indication whether the argument could be detected
 *        when parsing.
 */
inline bool Argument::isPresent() const
{
    return m_present;
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
    return m_required;
}

/*!
 * \brief Sets if this argument is mandatory or not.
 *
 * The parser will complain if a mandatory argument is not present.
 *
 * * \sa isRequired()
 */
inline void Argument::setRequired(bool value)
{
    m_required = value;
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
 * \brief Returns an indication whether the argument can be specified implicitely.
 *
 * An implicit main argument is assumed to be present even if only its value is present.
 *
 * \sa setImplicit()
 */
inline bool Argument::isImplicit() const
{
    return m_implicit;
}

/*!
 * \brief Sets if this argument can be specified implicitely.
 *
 * \sa isImplicit()
 */
inline void Argument::setImplicit(bool value)
{
    m_implicit = value;
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
inline const ArgumentVector &Argument::secondaryArguments() const
{
    return m_secondaryArgs;
}

/*!
 * \brief Returns an indication whether the argument has secondary arguments.
 *
 * \sa secondaryArguments()
 * \sa setSecondaryArguments()
 */
inline bool Argument::hasSecondaryArguments() const
{
    return !m_secondaryArgs.empty();
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
    void printHelp(std::ostream &os) const;
    Argument *findArg(const ArgumentPredicate &predicate) const;
    static Argument *findArg(const ArgumentVector &arguments, const ArgumentPredicate &predicate);
    void verifySetup() const;
    void parseArgs(int argc, char *argv[]);
    unsigned int actualArgumentCount() const;
    const std::string &currentDirectory() const;
    bool areUnknownArgumentsIgnored() const;
    void setIgnoreUnknownArguments(bool ignore);

private:
    ArgumentVector m_mainArgs;
    unsigned int m_actualArgc;
    std::string m_currentDirectory;
    bool m_ignoreUnknownArgs;
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
 * \brief Returns the current directory.
 */
inline const std::string &ArgumentParser::currentDirectory() const
{
    return m_currentDirectory;
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

class LIB_EXPORT HelpArgument : public Argument
{
public:
    HelpArgument(ArgumentParser &parser);
};

}

#endif // ARGUMENTPARSER_H
