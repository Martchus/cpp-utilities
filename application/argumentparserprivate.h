#ifndef APPLICATION_UTILITIES_ARGUMENTPARSER_PRIVATE_H
#define APPLICATION_UTILITIES_ARGUMENTPARSER_PRIVATE_H

namespace ApplicationUtilities {

struct CPP_UTILITIES_EXPORT ArgumentReader
{
    ArgumentReader(ArgumentParser &parser, const char *const *argv, const char *const *end, bool completionMode = false);
    ApplicationUtilities::ArgumentReader &reset(const char *const *argv, const char *const *end);
    void read();
    void read(ArgumentVector &args);

    /// \brief Specifies the associated ArgumentParser instance.
    ArgumentParser &parser;
    /// \brief Specifies the Argument instances to store the results. Sub arguments of args are considered as well.
    ArgumentVector &args;
    /// \brief Specifies and index which is incremented when an argument is encountered (the current index is stored in the occurrence) or a value is encountered.
    size_t index;
    /// \brief Points to the first argument denotation and will be incremented when a denotation has been processed.
    const char *const *argv;
    /// \brief Points to the end of the \a argv array.
    const char *const *end;
    /// \brief Specifies the last Argument instance which could be detected. Set to nullptr in the initial call. Used for Bash completion.
    Argument *lastArg;
    /// \brief Specifies the currently processed abbreviation denotation (should be substring of \a argv). Set to nullptr for processing \a argv from the beginning (default).
    const char *argDenotation;
    /// \brief Specifies whether completion mode is enabled. In this case reading args will be continued even if an denotation is unknown (regardless of unknownArgumentBehavior()).
    bool completionMode;
};

}

#endif // APPLICATION_UTILITIES_ARGUMENTPARSER_PRIVATE_H
