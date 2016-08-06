#ifndef TESTUTILS_H
#define TESTUTILS_H

#include "../application/argumentparser.h"

#include <string>
#include <ostream>

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
    int execApp(const char *const *args, std::string &output, std::string &errors, bool suppressLogging = false) const;
#endif
    bool unitsSpecified() const;
    const std::vector<const char *> &units() const;
    static const TestApplication *instance();

private:
    ApplicationUtilities::ArgumentParser m_parser;
    ApplicationUtilities::HelpArgument m_helpArg;
    ApplicationUtilities::Argument m_testFilesPathArg;
    ApplicationUtilities::Argument m_applicationPathArg;
    ApplicationUtilities::Argument m_workingDirArg;
    ApplicationUtilities::Argument m_unitsArg;
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
 * \brief Returns whether particular units have been specified.
 */
inline bool TestApplication::unitsSpecified() const
{
    return m_unitsArg.isPresent();
}

/*!
 * \brief Returns the specified test units.
 * \remarks The units argument must be present.
 */
inline const std::vector<const char *> &TestApplication::units() const
{
    return m_unitsArg.values();
}

/*!
 * \brief Convenience function which returns the full path of the test file with the specified \a name.
 * \remarks A TestApplication must be present.
 * \sa TestApplication::testFilePath()
 */
inline LIB_EXPORT std::string testFilePath(const std::string &name)
{
    return TestApplication::instance()->testFilePath(name);
}

#ifdef PLATFORM_UNIX
/*!
 * \brief Convenience function which returns the full path to a working copy of the test file with the specified \a name.
 * \remarks A TestApplication must be present.
 * \sa TestApplication::workingCopyPath()
 */
inline LIB_EXPORT std::string workingCopyPath(const std::string &name)
{
    return TestApplication::instance()->workingCopyPath(name);
}

/*!
 * \brief Convenience function which executes the application to be tested with the specified \a args.
 * \remarks A TestApplication must be present.
 * \sa TestApplication::execApp()
 */
inline LIB_EXPORT int execApp(const char *const *args, std::string &output, std::string &errors)
{
    return TestApplication::instance()->execApp(args, output, errors);
}
#endif

/*!
 * \brief The AsHexNumber class allows printing values asserted with cppunit (or similar test framework) using the
 *        hex system in the error case.
 */
template <typename T> class AsHexNumber
{
public:
    /// \brief Constructs a new instance; use asHexNumber() for convenience instead.
    AsHexNumber(const T &value) : value(value) {}
    const T &value;
};

/*!
 * \brief Provides operator == required by CPPUNIT_ASSERT_EQUAL.
 */
template <typename T> bool operator==(const AsHexNumber<T> &lhs, const AsHexNumber<T> &rhs)
{
    return lhs.value == rhs.value;
}

/*!
 * \brief Provides the actual formatting of the output for AsHexNumber class.
 */
template <typename T> std::ostream &operator<< (std::ostream &out, const AsHexNumber<T> &value)
{
    return out << std::hex << '0' << 'x' << unsigned(value.value) << std::dec;
}

/*!
 * \brief Wraps a value to be printed using the hex system in the error case when asserted
 *        with cppunit (or similar test framework).
 */
template <typename T> AsHexNumber<T> asHexNumber(const T &value)
{
    return AsHexNumber<T>(value);
}

#ifndef TESTUTILS_ASSERT_EXEC
/*!
 * \brief Asserts successful execution of application via TestApplication::execApp(). Output is stored in stdout and stderr.
 * \remarks Requires cppunit.
 */
# define TESTUTILS_ASSERT_EXEC(args) CPPUNIT_ASSERT_EQUAL(0, execApp(args, stdout, stderr))
#endif

}

#endif // TESTUTILS_H
