#ifndef TESTUTILS_H
#define TESTUTILS_H

#include "../application/argumentparser.h"
#include "../misc/traits.h"

#include <ostream>
#include <string>

namespace TestUtilities {

/*!
 * \brief The WorkingCopyMode enum specifies additional options to influence behavior of TestApplication::workingCopyPathMode().
 */
enum class WorkingCopyMode {
    CreateCopy, /**< a working copy of the test file is created */
    NoCopy /**< only the directory for the working copy is created but not the test file itself */
};

class CPP_UTILITIES_EXPORT TestApplication {
public:
    TestApplication(int argc, char **argv);
    ~TestApplication();

    operator bool() const;
    std::string testFilePath(const std::string &name) const;
#ifdef PLATFORM_UNIX
    std::string workingCopyPathMode(const std::string &name, WorkingCopyMode mode) const;
    std::string workingCopyPath(const std::string &name) const;
    int execApp(const char *const *args, std::string &output, std::string &errors, bool suppressLogging = false, int timeout = -1) const;
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
inline CPP_UTILITIES_EXPORT std::string testFilePath(const std::string &name)
{
    return TestApplication::instance()->testFilePath(name);
}

#ifdef PLATFORM_UNIX
/*!
 * \brief Convenience function which returns the full path to a working copy of the test file with the specified \a name.
 * \remarks A TestApplication must be present.
 * \sa TestApplication::workingCopyPath()
 */
inline CPP_UTILITIES_EXPORT std::string workingCopyPath(const std::string &name)
{
    return TestApplication::instance()->workingCopyPath(name);
}

/*!
 * \brief Convenience function which returns the full path to a working copy of the test file with the specified \a name.
 * \remarks A TestApplication must be present.
 * \sa TestApplication::workingCopyPathEx()
 */
inline CPP_UTILITIES_EXPORT std::string workingCopyPathMode(const std::string &name, WorkingCopyMode mode)
{
    return TestApplication::instance()->workingCopyPathMode(name, mode);
}

/*!
 * \brief Convenience function which executes the application to be tested with the specified \a args.
 * \remarks A TestApplication must be present.
 * \sa TestApplication::execApp()
 */
inline CPP_UTILITIES_EXPORT int execApp(const char *const *args, std::string &output, std::string &errors)
{
    return TestApplication::instance()->execApp(args, output, errors);
}

/*!
 * \brief Executes an application with the specified \a args.
 * \remarks Intended to invoke helper applications (eg. to setup test files). Use execApp() to invoke the application
 *          to be tested itself.
 */
CPP_UTILITIES_EXPORT int execHelperApp(
    const char *appPath, const char *const *args, std::string &output, std::string &errors, bool suppressLogging = false, int timeout = -1);
#endif

/*!
 * \brief The AsHexNumber class allows printing values asserted with cppunit (or similar test framework) using the
 *        hex system in the error case.
 */
template <typename T> class AsHexNumber {
public:
    /// \brief Constructs a new instance; use asHexNumber() for convenience instead.
    AsHexNumber(const T &value)
        : value(value)
    {
    }
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
template <typename T> std::ostream &operator<<(std::ostream &out, const AsHexNumber<T> &value)
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
#define TESTUTILS_ASSERT_EXEC(args) CPPUNIT_ASSERT_EQUAL(0, execApp(args, stdout, stderr))
#endif

/*!
 * \brief Allows printing iteratable objects so those can be asserted using CPPUNIT_ASSERT_EQUAL.
 */
template <typename Iteratable, Traits::EnableIf<Traits::IsIteratable<Iteratable>, Traits::Not<Traits::IsString<Iteratable>>>...>
inline std::ostream &operator<<(std::ostream &out, const Iteratable &iteratable)
{
    for (const auto &item : iteratable)
        out << item << '\n';
    return out;
}

/*!
 * \brief Contains literals to ease asserting with CPPUNIT_ASSERT_EQUAL.
 */
namespace Literals {
/*!
 * \brief Literal for std::size_t to ease asserting std::size_t with CPPUNIT_ASSERT_EQUAL.
 * \remarks Just using "ul"-suffix does not compile under 32-bit architecture!
 */
constexpr std::size_t operator"" _st(unsigned long long size)
{
    return static_cast<std::size_t>(size);
}
}
}

#endif // TESTUTILS_H
