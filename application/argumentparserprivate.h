#ifndef APPLICATION_UTILITIES_ARGUMENTPARSER_PRIVATE_H
#define APPLICATION_UTILITIES_ARGUMENTPARSER_PRIVATE_H

#include "./argumentparser.h"
#include "./commandlineutils.h"

namespace ApplicationUtilities {

class CPP_UTILITIES_EXPORT ArgumentReader {
public:
    ArgumentReader(ArgumentParser &parser, const char *const *argv, const char *const *end, bool completionMode = false);
    ApplicationUtilities::ArgumentReader &reset(const char *const *argv, const char *const *end);
    bool read();
    bool read(ArgumentVector &args);

    /// \brief The associated ArgumentParser instance.
    ArgumentParser &parser;
    /// \brief The Argument instances to store the results. Sub arguments of args are considered as well.
    ArgumentVector &args;
    /// \brief An index which is incremented when an argument is encountered (the current index is stored in the occurrence) or a value is encountered.
    size_t index;
    /// \brief Points to the first argument denotation and will be incremented when a denotation has been processed.
    const char *const *argv;
    /// \brief Points to the end of the \a argv array.
    const char *const *end;
    /// \brief The last Argument instance which could be detected. Set to nullptr in the initial call. Used for Bash completion.
    Argument *lastArg;
    /// \brief Points to the element in argv where lastArg was encountered. Unspecified if lastArg is not set.
    const char *const *lastArgDenotation;
    /// \brief The currently processed abbreviation denotation (should be substring of one of the args in argv). Set to nullptr for processing argv from the beginning (default).
    const char *argDenotation;
    /// \brief The type of the currently processed abbreviation denotation. Unspecified if argDenotation is not set.
    unsigned char argDenotationType;
    /// \brief Whether completion mode is enabled. In this case reading args will be continued even if an denotation is unknown (regardless of unknownArgumentBehavior()).
    bool completionMode;
};

class Wrapper;

std::ostream &operator<<(std::ostream &os, const Wrapper &wrapper);

class Wrapper {
    friend std::ostream &operator<<(std::ostream &os, const Wrapper &wrapper);

public:
    Wrapper(const char *str, Indentation currentIndentation = Indentation());

private:
    const char *const m_str;
    Indentation m_indentation;
};

Wrapper::Wrapper(const char *str, Indentation currentIndentation)
    : m_str(str)
    , m_indentation(currentIndentation)
{
}

} // namespace ApplicationUtilities

#endif // APPLICATION_UTILITIES_ARGUMENTPARSER_PRIVATE_H
