#ifndef TESTUTILS_H
#define TESTUTILS_H

#include "../application/argumentparser.h"
#include "../misc/traits.h"

#include <iomanip>
#include <ostream>
#include <string>

namespace CppUtilities {

/*!
 * \brief The WorkingCopyMode enum specifies additional options to influence behavior of TestApplication::workingCopyPath().
 */
enum class WorkingCopyMode {
    CreateCopy, /**< a working copy of the test file is created */
    NoCopy /**< only the directory for the working copy is created but not the test file itself */
};

class CPP_UTILITIES_EXPORT TestApplication {
public:
    // construction/destruction
    explicit TestApplication();
    explicit TestApplication(int argc, const char *const *argv);
    ~TestApplication();
    operator bool() const;

    // helper for tests
    std::string testFilePath(const std::string &relativeTestFilePath) const;
    std::string testDirPath(const std::string &relativeTestDirPath) const;
    std::string workingCopyPath(const std::string &relativeTestFilePath, WorkingCopyMode mode = WorkingCopyMode::CreateCopy) const;
    std::string workingCopyPathAs(const std::string &relativeTestFilePath, const std::string &relativeWorkingCopyPath,
        WorkingCopyMode mode = WorkingCopyMode::CreateCopy) const;
#ifdef PLATFORM_UNIX
    int execApp(const char *const *args, std::string &output, std::string &errors, bool suppressLogging = false, int timeout = -1) const;
#endif

    // read-only accessors
    const std::vector<std::string> &testFilePaths() const;
    const std::string &workingDirectory() const;
    const char *applicationPath();
    bool unitsSpecified() const;
    const std::vector<const char *> &units() const;
    bool onlyListUnits() const;

    // static read-only accessors
    static const TestApplication *instance();
    static const char *appPath();

private:
    static std::string readTestfilePathFromEnv();
    static std::string readTestfilePathFromSrcRef();

    ArgumentParser m_parser;
    OperationArgument m_listArg;
    OperationArgument m_runArg;
    ConfigValueArgument m_testFilesPathArg;
    ConfigValueArgument m_applicationPathArg;
    ConfigValueArgument m_workingDirArg;
    ConfigValueArgument m_unitsArg;
    std::vector<std::string> m_testFilesPaths;
    std::string m_workingDir;
    bool m_valid;
    static TestApplication *s_instance;
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
    return TestApplication::s_instance;
}

/*!
 * \brief Returns the application path or an empty string if no application path has been set.
 */
inline const char *TestApplication::appPath()
{
    return s_instance ? s_instance->applicationPath() : "";
}

/*!
 * \brief Returns the list of directories to look for test files.
 */
inline const std::vector<std::string> &TestApplication::testFilePaths() const
{
    return m_testFilesPaths;
}

/*!
 * \brief Returns the directory which is supposed to used for storing files created by tests.
 */
inline const std::string &TestApplication::workingDirectory() const
{
    return m_workingDir;
}

/*!
 * \brief Returns the application path or an empty string if no application path has been set.
 */
inline const char *TestApplication::applicationPath()
{
    return m_applicationPathArg.firstValue() ? m_applicationPathArg.firstValue() : "";
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
 * \brief Returns whether the test application should only list available units and not actually run any tests.
 */
inline bool TestApplication::onlyListUnits() const
{
    return m_listArg.isPresent();
}

/*!
 * \brief Convenience function to invoke TestApplication::testFilePath().
 * \remarks A TestApplication must be present.
 */
inline CPP_UTILITIES_EXPORT std::string testFilePath(const std::string &relativeTestFilePath)
{
    return TestApplication::instance()->testFilePath(relativeTestFilePath);
}

/*!
 * \brief Convenience function to invoke TestApplication::testDirPath().
 * \remarks A TestApplication must be present.
 */
inline CPP_UTILITIES_EXPORT std::string testDirPath(const std::string &relativeTestDirPath)
{
    return TestApplication::instance()->testDirPath(relativeTestDirPath);
}

/*!
 * \brief Convenience function to invoke TestApplication::workingCopyPath().
 * \remarks A TestApplication must be present.
 */
inline CPP_UTILITIES_EXPORT std::string workingCopyPath(const std::string &relativeTestFilePath, WorkingCopyMode mode = WorkingCopyMode::CreateCopy)
{
    return TestApplication::instance()->workingCopyPathAs(relativeTestFilePath, relativeTestFilePath, mode);
}

/*!
 * \brief Convenience function to invoke TestApplication::workingCopyPathAs().
 * \remarks A TestApplication must be present.
 */
inline CPP_UTILITIES_EXPORT std::string workingCopyPathAs(
    const std::string &relativeTestFilePath, const std::string &relativeWorkingCopyPath, WorkingCopyMode mode = WorkingCopyMode::CreateCopy)
{
    return TestApplication::instance()->workingCopyPathAs(relativeTestFilePath, relativeWorkingCopyPath, mode);
}

#ifdef PLATFORM_UNIX
/*!
 * \brief Convenience function which executes the application to be tested with the specified \a args.
 * \remarks A TestApplication must be present.
 * \sa TestApplication::execApp()
 */
inline CPP_UTILITIES_EXPORT int execApp(const char *const *args, std::string &output, std::string &errors)
{
    return TestApplication::instance()->execApp(args, output, errors);
}

CPP_UTILITIES_EXPORT int execHelperApp(
    const char *appPath, const char *const *args, std::string &output, std::string &errors, bool suppressLogging = false, int timeout = -1);
CPP_UTILITIES_EXPORT int execHelperAppInSearchPath(
    const char *appName, const char *const *args, std::string &output, std::string &errors, bool suppressLogging = false, int timeout = -1);
#endif // PLATFORM_UNIX

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
    return out << '0' << 'x' << std::hex << std::setfill('0') << std::setw(2) << unsigned(value.value) << std::dec;
}

/*!
 * \brief Wraps a value to be printed using the hex system in the error case when asserted
 *        with cppunit (or similar test framework).
 */
template <typename T> AsHexNumber<T> asHexNumber(const T &value)
{
    return AsHexNumber<T>(value);
}

/*!
 * \brief Wraps a value to be printed using the hex system in the error case when asserted
 *        with cppunit (or similar test framework).
 * \remarks Only affects integral types. Values of other types are printed as usual.
 */
template <typename T, Traits::EnableIf<std::is_integral<T>> * = nullptr> AsHexNumber<T> integralsAsHexNumber(const T &value)
{
    return AsHexNumber<T>(value);
}

/*!
 * \brief Wraps a value to be printed using the hex system in the error case when asserted
 *        with cppunit (or similar test framework).
 * \remarks Only affects integral types. Values of other types are printed as usual.
 */
template <typename T, Traits::DisableIf<std::is_integral<T>> * = nullptr> const T &integralsAsHexNumber(const T &value)
{
    return value;
}

/*!
 * \brief Asserts successful execution of the application with the specified CLI \a args.
 *
 * The application is executed via TestApplication::execApp(). Output is stored in the std::string variables stdout
 * and stderr.
 *
 * \remarks Requires cppunit.
 */
#define TESTUTILS_ASSERT_EXEC(args)                                                                                                                  \
    {                                                                                                                                                \
        const auto returnCode = execApp(args, stdout, stderr);                                                                                       \
        if (returnCode != 0) {                                                                                                                       \
            CPPUNIT_FAIL(::CppUtilities::argsToString("app failed with return code ", returnCode, "\nstdout: ", stdout, "\nstderr: ", stderr));      \
        }                                                                                                                                            \
    }

/*!
 * \brief Asserts whether the specified \a string matches the specified \a regex.
 * \remarks Requires cppunit.
 */
#define TESTUTILS_ASSERT_LIKE_FLAGS(message, expectedRegex, regexFlags, actualString)                                                                \
    (CPPUNIT_NS::Asserter::failIf(!(std::regex_match(actualString, std::regex(expectedRegex, regexFlags))),                                          \
        CPPUNIT_NS::Message(                                                                                                                         \
            CppUtilities::argsToString('\"', actualString, "\"\n    not like\n\"", expectedRegex, '\"'), "Expression: " #actualString, message),     \
        CPPUNIT_SOURCELINE()))

/*!
 * \brief Asserts whether the specified \a string matches the specified \a regex.
 * \remarks Requires cppunit.
 */
#define TESTUTILS_ASSERT_LIKE(message, expectedRegex, actualString)                                                                                  \
    TESTUTILS_ASSERT_LIKE_FLAGS(message, expectedRegex, std::regex::ECMAScript, actualString)

/*!
 * \brief Allows printing pairs so key/values of maps/hashes can be asserted using CPPUNIT_ASSERT_EQUAL.
 */
template <typename Pair, CppUtilities::Traits::EnableIf<CppUtilities::Traits::IsSpecializationOf<Pair, std::pair>> * = nullptr>
inline std::ostream &operator<<(std::ostream &out, const Pair &pair)
{
    return out << "key: " << pair.first << "; value: " << pair.second << '\n';
}

/*!
 * \brief Allows printing iteratable objects so those can be asserted using CPPUNIT_ASSERT_EQUAL.
 */
template <typename Iteratable, Traits::EnableIf<Traits::IsIteratable<Iteratable>, Traits::Not<Traits::IsString<Iteratable>>> * = nullptr>
inline std::ostream &operator<<(std::ostream &out, const Iteratable &iteratable)
{
    out << '\n';
    std::size_t index = 0;
    for (const auto &item : iteratable) {
        out << std::setw(2) << index << ':' << ' ' << integralsAsHexNumber(item) << '\n';
        ++index;
    }
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

/*!
 * \brief Literal for uint64 to ease asserting uint64 with CPPUNIT_ASSERT_EQUAL.
 * \remarks Just using "ul"-suffix does not compile under 32-bit architecture!
 */
constexpr std::uint64_t operator"" _uint64(unsigned long long size)
{
    return static_cast<std::uint64_t>(size);
}

/*!
 * \brief Literal for int64 to ease asserting int64 with CPPUNIT_ASSERT_EQUAL.
 * \remarks Just using "l"-suffix does not compile under 32-bit architecture!
 */
constexpr std::int64_t operator"" _int64(unsigned long long size)
{
    return static_cast<std::int64_t>(size);
}
} // namespace Literals
} // namespace CppUtilities

#endif // TESTUTILS_H
