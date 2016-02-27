#ifndef TESTUTILS_H
#define TESTUTILS_H

#include "../application/argumentparser.h"

#include <string>

namespace TestUtilities {

class LIB_EXPORT TestApplication
{
public:
    TestApplication(int argc, char **argv);
    ~TestApplication();

    operator bool() const;
    std::string testFilePath(const std::string &name) const;
#ifdef PLATFORM_UNIX
    std::string workingCopyPath(const std::string &name) const;
#endif
    static const TestApplication *instance();

private:
    ApplicationUtilities::ArgumentParser m_parser;
    ApplicationUtilities::HelpArgument m_helpArg;
    ApplicationUtilities::Argument m_testFilesPathArg;
    ApplicationUtilities::Argument m_workingDirArg;
    std::string m_testFilesPathArgValue;
    std::string m_testFilesPathEnvValue;
    std::string m_workingDir;
    bool m_valid;
    static TestApplication *m_instance;
};

/*!
 * \brief Returns whether the TestApplication instance is valid.
 *
 * An instance is considered invalid if an error occured when
 * parsing the command line arguments.
 */
inline TestApplication::operator bool() const
{
    return m_valid;
}

/*!
 * \brief Returns the current TestApplication instance.
 */
inline const TestApplication *TestApplication::instance()
{
    return TestApplication::m_instance;
}

/*!
 * \brief Convenience function which returns the full path of the test file with the specified \a name.
 * \remarks A TestApplication must be present.
 */
inline LIB_EXPORT std::string testFilePath(const std::string &name)
{
    return TestApplication::instance()->testFilePath(name);
}

#ifdef PLATFORM_UNIX
/*!
 * \brief Convenience function which returns the full path to a working copy of the test file with the specified \a name.
 * \remarks A TestApplication must be present.
 */
inline LIB_EXPORT std::string workingCopyPath(const std::string &name)
{
    return TestApplication::instance()->workingCopyPath(name);
}
#endif

}

#endif // TESTUTILS_H
