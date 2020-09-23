#include "./testutils.h"

#include "../conversion/stringbuilder.h"
#include "../conversion/stringconversion.h"
#include "../io/ansiescapecodes.h"
#include "../io/misc.h"
#include "../io/nativefilestream.h"
#include "../io/path.h"
#include "../misc/parseerror.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <limits>

#ifdef PLATFORM_UNIX
#ifdef CPP_UTILITIES_USE_STANDARD_FILESYSTEM
#include <filesystem>
#endif
#include <poll.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

using namespace std;
using namespace CppUtilities::EscapeCodes;

/*!
 * \brief Contains classes and functions utilizing creating of test applications.
 */
namespace CppUtilities {

/// \cond
bool fileSystemItemExists(const string &path)
{
#ifdef PLATFORM_UNIX
    struct stat res;
    return stat(path.data(), &res) == 0;
#else
    const auto widePath(convertMultiByteToWide(path));
    if (!widePath.first) {
        return false;
    }
    const auto fileType(GetFileAttributesW(widePath.first.get()));
    return fileType != INVALID_FILE_ATTRIBUTES;
#endif
}

bool fileExists(const string &path)
{
#ifdef PLATFORM_UNIX
    struct stat res;
    return stat(path.data(), &res) == 0 && !S_ISDIR(res.st_mode);
#else
    const auto widePath(convertMultiByteToWide(path));
    if (!widePath.first) {
        return false;
    }
    const auto fileType(GetFileAttributesW(widePath.first.get()));
    return (fileType != INVALID_FILE_ATTRIBUTES) && !(fileType & FILE_ATTRIBUTE_DIRECTORY) && !(fileType & FILE_ATTRIBUTE_DEVICE);
#endif
}

bool dirExists(const string &path)
{
#ifdef PLATFORM_UNIX
    struct stat res;
    return stat(path.data(), &res) == 0 && S_ISDIR(res.st_mode);
#else
    const auto widePath(convertMultiByteToWide(path));
    if (!widePath.first) {
        return false;
    }
    const auto fileType(GetFileAttributesW(widePath.first.get()));
    return (fileType != INVALID_FILE_ATTRIBUTES) && (fileType & FILE_ATTRIBUTE_DIRECTORY);
#endif
}

bool makeDir(const string &path)
{
#ifdef PLATFORM_UNIX
    return mkdir(path.data(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
#else
    const auto widePath(convertMultiByteToWide(path));
    if (!widePath.first) {
        return false;
    }
    return CreateDirectoryW(widePath.first.get(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS;
#endif
}
/// \endcond

TestApplication *TestApplication::s_instance = nullptr;

/*!
 * \class TestApplication
 * \brief The TestApplication class simplifies writing test applications that require opening test files.
 * \remarks Only one instance is allowed at a time (singletone class).
 */

/*!
 * \brief Constructs a TestApplication instance without further arguments.
 * \remarks This constructor skips parsing CLI arguments. Other initialization like reading environment variables
 *          for test file paths and working directories is still done.
 * \throws Throws std::runtime_error if an instance has already been created.
 */
TestApplication::TestApplication()
    : TestApplication(0, nullptr)
{
}

/*!
 * \brief Constructs a TestApplication instance for the specified arguments.
 * \throws Throws std::runtime_error if an instance has already been created.
 */
TestApplication::TestApplication(int argc, const char *const *argv)
    : m_listArg("list", 'l', "lists available test units")
    , m_runArg("run", 'r', "runs the tests")
    , m_testFilesPathArg("test-files-path", 'p', "specifies the path of the directory with test files", { "path" })
    , m_applicationPathArg("app-path", 'a', "specifies the path of the application to be tested", { "path" })
    , m_workingDirArg("working-dir", 'w', "specifies the directory to store working copies of test files", { "path" })
    , m_unitsArg("units", 'u', "specifies the units to test; omit to test all units", { "unit1", "unit2", "unit3" })
{
    // check whether there is already an instance
    if (s_instance) {
        throw runtime_error("only one TestApplication instance allowed at a time");
    }
    s_instance = this;

    // handle specified arguments (if present)
    if (argc && argv) {
        // setup argument parser
        m_testFilesPathArg.setRequiredValueCount(Argument::varValueCount);
        m_unitsArg.setRequiredValueCount(Argument::varValueCount);
        m_runArg.setImplicit(true);
        m_runArg.setSubArguments({ &m_testFilesPathArg, &m_applicationPathArg, &m_workingDirArg, &m_unitsArg });
        m_parser.setMainArguments({ &m_runArg, &m_listArg, &m_parser.noColorArg(), &m_parser.helpArg() });

        // parse arguments
        try {
            m_parser.parseArgs(argc, argv, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
        } catch (const ParseError &failure) {
            cerr << failure;
            m_valid = false;
            return;
        }

        // print help
        if (m_parser.helpArg().isPresent()) {
            exit(0);
        }
    }

    // set paths for testfiles
    // -> set paths set via CLI argument
    if (m_testFilesPathArg.isPresent()) {
        for (const char *const testFilesPath : m_testFilesPathArg.values()) {
            if (*testFilesPath) {
                m_testFilesPaths.emplace_back(argsToString(testFilesPath, '/'));
            } else {
                m_testFilesPaths.emplace_back("./");
            }
        }
    }
    // -> read TEST_FILE_PATH environment variable
    bool hasTestFilePathFromEnv;
    if (auto testFilePathFromEnv = readTestfilePathFromEnv(); (hasTestFilePathFromEnv = !testFilePathFromEnv.empty())) {
        m_testFilesPaths.emplace_back(move(testFilePathFromEnv));
    }
    // -> find source directory
    if (auto testFilePathFromSrcDirRef = readTestfilePathFromSrcRef(); !testFilePathFromSrcDirRef.empty()) {
        m_testFilesPaths.emplace_back(move(testFilePathFromSrcDirRef));
    }
    // -> try testfiles directory in working directory
    m_testFilesPaths.emplace_back("./testfiles/");
    for (const auto &testFilesPath : m_testFilesPaths) {
        cerr << testFilesPath << '\n';
    }

    // set path for working-copy
    if (m_workingDirArg.isPresent()) {
        if (*m_workingDirArg.values().front()) {
            (m_workingDir = m_workingDirArg.values().front()) += '/';
        } else {
            m_workingDir = "./";
        }
    } else if (const char *const workingDirEnv = getenv("WORKING_DIR")) {
        if (*workingDirEnv) {
            m_workingDir = argsToString(workingDirEnv, '/');
        }
    } else {
        if ((m_testFilesPathArg.isPresent() && !m_testFilesPathArg.values().empty()) || hasTestFilePathFromEnv) {
            m_workingDir = m_testFilesPaths.front() + "workingdir/";
        } else {
            m_workingDir = "./testfiles/workingdir/";
        }
    }
    cerr << "Directory used to store working copies:\n" << m_workingDir << '\n';

    // clear list of all additional profiling files created when forking the test application
    if (const char *const profrawListFile = getenv("LLVM_PROFILE_LIST_FILE")) {
        ofstream(profrawListFile, ios_base::trunc);
    }

    m_valid = true;
}

/*!
 * \brief Destroys the TestApplication.
 */
TestApplication::~TestApplication()
{
    s_instance = nullptr;
}

/*!
 * \brief Returns the full path of the test file with the specified \a relativeTestFilePath.
 *
 * The specified \a relativeTestFilePath is considered to be a path to a test file which is relative
 * to at least one of the considered test file search directories.
 *
 * The following directories are searched for test files in the given order:
 * 1. The directories specified as CLI argument.
 * 2. The directory set via the environment variable `TEST_FILE_PATH`.
 * 3. The subdirectory "testfiles" within the source directory, if it could be determined via "srcref"-file.
 * 4. The subdirectory "testfiles" within present working directory.
 */
std::string TestApplication::testFilePath(const std::string &relativeTestFilePath) const
{
    std::string path;
    for (const auto &testFilesPath : m_testFilesPaths) {
        if (fileExists(path = testFilesPath + relativeTestFilePath)) {
            return path;
        }
    }
    throw std::runtime_error("The test file \"" % relativeTestFilePath % "\" can not be located. Was looking under:"
        + joinStrings(m_testFilesPaths, "\n", false, std::string(), relativeTestFilePath));
}

/*!
 * \brief Returns the full path of the test directory with the specified \a relativeTestDirPath.
 *
 * This is the same as TestApplication::testFilePath() but for directories. Checkout the documentation of
 * TestApplication::testFilePath() for details about the lookup.
 */
std::string TestApplication::testDirPath(const std::string &relativeTestDirPath) const
{
    std::string path;
    for (const auto &testFilesPath : m_testFilesPaths) {
        if (dirExists(path = testFilesPath + relativeTestDirPath)) {
            return path;
        }
    }
    throw std::runtime_error("The test directory \"" % relativeTestDirPath % "\" can not be located. Was looking under:"
        + joinStrings(m_testFilesPaths, "\n", false, std::string(), relativeTestDirPath));
}

/*!
 * \brief Returns the full path to a working copy of the test file with the specified \a relativeTestFilePath.
 *
 * The specified \a mode controls whether a working copy is actually created or whether just the path is returned.
 *
 * \remarks The test file is located using testFilePath().
 */
string TestApplication::workingCopyPath(const string &relativeTestFilePath, WorkingCopyMode mode) const
{
    return workingCopyPathAs(relativeTestFilePath, relativeTestFilePath, mode);
}

/*!
 * \brief Returns the full path to a working copy of the test file with the specified \a relativeTestFilePath.
 *
 * The specified \a mode controls whether a working copy is actually created or whether just the path is returned. If only the
 * path is returned, the \a relativeTestFilePath is ignored.
 *
 * In contrast to workingCopyPath(), this method allows to adjust the relative path of the working copy within the working copy
 * directory via \a relativeWorkingCopyPath.
 *
 * \remarks
 * - The test file specified via \a relativeTestFilePath is located using testFilePath().
 * - The name of the working copy file specified via \a relativeWorkingCopyPath will be adjusted if it already exists in the file
 *   system and can not be truncated.
 */
string TestApplication::workingCopyPathAs(
    const std::string &relativeTestFilePath, const std::string &relativeWorkingCopyPath, WorkingCopyMode mode) const
{
    // ensure working directory is present
    if (!dirExists(m_workingDir) && !makeDir(m_workingDir)) {
        cerr << Phrases::Error << "Unable to create working copy for \"" << relativeTestFilePath << "\": can't create working directory \""
             << m_workingDir << "\"." << Phrases::EndFlush;
        return string();
    }

    // ensure subdirectory exists
    const auto parts = splitString<vector<string>>(relativeWorkingCopyPath, "/", EmptyPartsTreat::Omit);
    if (!parts.empty()) {
        // create subdirectory level by level
        string currentLevel;
        currentLevel.reserve(m_workingDir.size() + relativeWorkingCopyPath.size() + 1);
        currentLevel.assign(m_workingDir);
        for (auto i = parts.cbegin(), end = parts.end() - 1; i != end; ++i) {
            if (currentLevel.back() != '/') {
                currentLevel += '/';
            }
            currentLevel += *i;

            // continue if subdirectory level already exists or we can successfully create the directory
            if (dirExists(currentLevel) || makeDir(currentLevel)) {
                continue;
            }
            // fail otherwise
            cerr << Phrases::Error << "Unable to create working copy for \"" << relativeWorkingCopyPath << "\": can't create directory \""
                 << currentLevel << "\" (inside working directory)." << Phrases::EndFlush;
            return string();
        }
    }

    // just return the path if we don't want to actually create a copy
    if (mode == WorkingCopyMode::NoCopy) {
        return m_workingDir + relativeWorkingCopyPath;
    }

    // copy the file
    const auto origFilePath(testFilePath(relativeTestFilePath));
    auto workingCopyPath(m_workingDir + relativeWorkingCopyPath);
    size_t workingCopyPathAttempt = 0;
    NativeFileStream origFile, workingCopy;
    origFile.open(origFilePath, ios_base::in | ios_base::binary);
    if (origFile.fail()) {
        cerr << Phrases::Error << "Unable to create working copy for \"" << relativeTestFilePath
             << "\": an IO error occurred when opening original file \"" << origFilePath << "\"." << Phrases::EndFlush;
        cerr << "error: " << strerror(errno) << endl;
        return string();
    }
    workingCopy.open(workingCopyPath, ios_base::out | ios_base::binary | ios_base::trunc);
    while (workingCopy.fail() && fileSystemItemExists(workingCopyPath)) {
        // adjust the working copy path if the target file already exists and can not be truncated
        workingCopyPath = argsToString(m_workingDir, relativeWorkingCopyPath, '.', ++workingCopyPathAttempt);
        workingCopy.clear();
        workingCopy.open(workingCopyPath, ios_base::out | ios_base::binary | ios_base::trunc);
    }
    if (workingCopy.fail()) {
        cerr << Phrases::Error << "Unable to create working copy for \"" << relativeTestFilePath
             << "\": an IO error occurred when opening target file \"" << workingCopyPath << "\"." << Phrases::EndFlush;
        cerr << "error: " << strerror(errno) << endl;
        return string();
    }
    workingCopy << origFile.rdbuf();
    workingCopy.close();
    if (!origFile.fail() && !workingCopy.fail()) {
        return workingCopyPath;
    }

    cerr << Phrases::Error << "Unable to create working copy for \"" << relativeTestFilePath << "\": ";
    if (origFile.fail()) {
        cerr << "an IO error occurred when reading original file \"" << origFilePath << "\"";
        return string();
    }
    if (workingCopy.fail()) {
        if (origFile.fail()) {
            cerr << " and ";
        }
        cerr << " an IO error occurred when writing to target file \"" << workingCopyPath << "\".";
    }
    cerr << "error: " << strerror(errno) << endl;
    return string();
}

#ifdef PLATFORM_UNIX
/*!
 * \brief Executes an application with the specified \a args.
 * \remarks Provides internal implementation of execApp() and execHelperApp().
 */
static int execAppInternal(const char *appPath, const char *const *args, std::string &output, std::string &errors, bool suppressLogging, int timeout,
    const std::string &newProfilingPath, bool enableSearchPath = false)
{
    // print log message
    if (!suppressLogging) {
        // print actual appPath and skip first argument instead
        cout << '-' << ' ' << appPath;
        if (*args) {
            for (const char *const *i = args + 1; *i; ++i) {
                cout << ' ' << *i;
            }
        }
        cout << endl;
    }

    // create pipes
    int coutPipes[2], cerrPipes[2];
    pipe(coutPipes);
    pipe(cerrPipes);
    const auto readCoutPipe = coutPipes[0], writeCoutPipe = coutPipes[1];
    const auto readCerrPipe = cerrPipes[0], writeCerrPipe = cerrPipes[1];

    // create child process
    if (const auto child = fork()) {
        // parent process: read stdout and stderr from child
        close(writeCoutPipe);
        close(writeCerrPipe);

        try {
            if (child == -1) {
                throw runtime_error("Unable to create fork");
            }

            // init file descriptor set for poll
            struct pollfd fileDescriptorSet[2];
            fileDescriptorSet[0].fd = readCoutPipe;
            fileDescriptorSet[1].fd = readCerrPipe;
            fileDescriptorSet[0].events = fileDescriptorSet[1].events = POLLIN;

            // init variables for reading
            char buffer[512];
            output.clear();
            errors.clear();

            // poll as long as at least one pipe is open
            do {
                const auto retpoll = poll(fileDescriptorSet, 2, timeout);
                if (retpoll == 0) {
                    throw runtime_error("Poll time-out");
                }
                if (retpoll < 0) {
                    throw runtime_error("Poll failed");
                }
                if (fileDescriptorSet[0].revents & POLLIN) {
                    const auto count = read(readCoutPipe, buffer, sizeof(buffer));
                    if (count > 0) {
                        output.append(buffer, static_cast<size_t>(count));
                    }
                } else if (fileDescriptorSet[0].revents & POLLHUP) {
                    close(readCoutPipe);
                    fileDescriptorSet[0].fd = -1;
                }
                if (fileDescriptorSet[1].revents & POLLIN) {
                    const auto count = read(readCerrPipe, buffer, sizeof(buffer));
                    if (count > 0) {
                        errors.append(buffer, static_cast<size_t>(count));
                    }
                } else if (fileDescriptorSet[1].revents & POLLHUP) {
                    close(readCerrPipe);
                    fileDescriptorSet[1].fd = -1;
                }
            } while (fileDescriptorSet[0].fd >= 0 || fileDescriptorSet[1].fd >= 0);
        } catch (...) {
            // ensure all pipes are closed in the error case
            close(readCoutPipe);
            close(readCerrPipe);
            throw;
        }

        // get return code
        int childReturnCode;
        waitpid(child, &childReturnCode, 0);
        return childReturnCode;
    } else {
        // child process
        // -> set pipes to be used for stdout/stderr
        dup2(writeCoutPipe, STDOUT_FILENO);
        dup2(writeCerrPipe, STDERR_FILENO);
        close(readCoutPipe);
        close(writeCoutPipe);
        close(readCerrPipe);
        close(writeCerrPipe);

        // -> modify environment variable LLVM_PROFILE_FILE to apply new path for profiling output
        if (!newProfilingPath.empty()) {
            setenv("LLVM_PROFILE_FILE", newProfilingPath.data(), true);
        }

        // -> execute application
        if (enableSearchPath) {
            execvp(appPath, const_cast<char *const *>(args));

        } else {
            execv(appPath, const_cast<char *const *>(args));
        }
        cerr << Phrases::Error << "Unable to execute \"" << appPath << "\": execv() failed" << Phrases::EndFlush;
        exit(-101);
    }
}

/*!
 * \brief Executes the application to be tested with the specified \a args and stores the standard output and
 *        errors in \a stdout and \a stderr.
 * \throws Throws std::runtime_error when the application can not be executed.
 * \remarks
 *  - The specified \a args must be 0 terminated. The first argument is the application name.
 *  - Currently only supported under UNIX.
 *  - \a stdout and \a stderr are cleared before.
 */
int TestApplication::execApp(const char *const *args, string &output, string &errors, bool suppressLogging, int timeout) const
{
    // increase counter used for giving profiling files unique names
    static unsigned int invocationCount = 0;
    ++invocationCount;

    // determine the path of the application to be tested
    const char *appPath = m_applicationPathArg.firstValue();
    string fallbackAppPath;
    if (!appPath || !*appPath) {
        // try to find the path by removing "_tests"-suffix from own executable path
        // (the own executable path is the path of the test application and its name is usually the name of the application
        //  to be tested with "_tests"-suffix)
        const char *const testAppPath = m_parser.executable();
        const size_t testAppPathLength = strlen(testAppPath);
        if (testAppPathLength > 6 && !strcmp(testAppPath + testAppPathLength - 6, "_tests")) {
            fallbackAppPath.assign(testAppPath, testAppPathLength - 6);
            appPath = fallbackAppPath.data();
            // TODO: it would not hurt to verify whether "fallbackAppPath" actually exists and is executalbe
        } else {
            throw runtime_error("Unable to execute application to be tested: no application path specified");
        }
    }

    // determine new path for profiling output (to not override profiling output of parent and previous invocations)
    const auto newProfilingPath = [appPath] {
        string newProfilingPath;
        const char *const llvmProfileFile = getenv("LLVM_PROFILE_FILE");
        if (!llvmProfileFile) {
            return newProfilingPath;
        }
        // replace eg. "/some/path/tageditor_tests.profraw" with "/some/path/tageditor0.profraw"
        const char *const llvmProfileFileEnd = strstr(llvmProfileFile, ".profraw");
        if (!llvmProfileFileEnd) {
            return newProfilingPath;
        }
        const string llvmProfileFileWithoutExtension(llvmProfileFile, llvmProfileFileEnd);
        // extract application name from path
        const char *appName = strrchr(appPath, '/');
        appName = appName ? appName + 1 : appPath;
        // concat new path
        newProfilingPath = argsToString(llvmProfileFileWithoutExtension, '_', appName, invocationCount, ".profraw");
        // append path to profiling list file
        if (const char *const profrawListFile = getenv("LLVM_PROFILE_LIST_FILE")) {
            ofstream(profrawListFile, ios_base::app) << newProfilingPath << endl;
        }
        return newProfilingPath;
    }();

    return execAppInternal(appPath, args, output, errors, suppressLogging, timeout, newProfilingPath);
}

/*!
 * \brief Executes an application with the specified \a args.
 * \remarks
 * - Intended to invoke helper applications (eg. to setup test files). Use execApp() and TestApplication::execApp() to
 *   invoke the application to be tested itself.
 * - Currently only supported under UNIX.
 */
int execHelperApp(const char *appPath, const char *const *args, std::string &output, std::string &errors, bool suppressLogging, int timeout)
{
    return execAppInternal(appPath, args, output, errors, suppressLogging, timeout, string());
}

/*!
 * \brief Executes an application with the specified \a args.
 *
 * Searches for the location of \a appName among the directories specified by the PATH environment variable.
 *
 * \remarks
 * - Intended to invoke helper applications (eg. to setup test files). Use execApp() and TestApplication::execApp() to
 *   invoke the application to be tested itself.
 * - Currently only supported under UNIX.
 */
int execHelperAppInSearchPath(
    const char *appName, const char *const *args, std::string &output, std::string &errors, bool suppressLogging, int timeout)
{
    return execAppInternal(appName, args, output, errors, suppressLogging, timeout, string(), true);
}
#endif // PLATFORM_UNIX

/*!
 * \brief Reads the path of the test file directory from the environment variable TEST_FILE_PATH.
 */
string TestApplication::readTestfilePathFromEnv()
{
    const char *const testFilesPathEnv = getenv("TEST_FILE_PATH");
    if (!testFilesPathEnv || !*testFilesPathEnv) {
        return string();
    }
    return argsToString(testFilesPathEnv, '/');
}

/*!
 * \brief Reads the path of the test file directory from the "srcdirref" file.
 * \remarks That file is supposed to contain the path the the source directory. It is supposed to be stored by the build system in the
 *          same directory as the test executable. The CMake modules contained of these utilities ensure that's the case.
 */
string TestApplication::readTestfilePathFromSrcRef()
{
    // find the path of the current executable on platforms supporting "/proc/self/exe"; otherwise assume the current working directory
    // is the executable path
    std::string binaryPath;
#if defined(CPP_UTILITIES_USE_STANDARD_FILESYSTEM) && defined(PLATFORM_UNIX)
    try {
        binaryPath = std::filesystem::read_symlink("/proc/self/exe").parent_path();
        binaryPath += '/';
    } catch (const std::filesystem::filesystem_error &e) {
        cerr << Phrases::Warning << "Unable to detect binary path for finding \"srcdirref\": " << e.what() << Phrases::EndFlush;
    }
#endif
    try {
        // read "srcdirref" file which should contain the path of the source directory
        auto srcDirContent(readFile(binaryPath + "srcdirref", 2 * 1024));
        if (srcDirContent.empty()) {
            cerr << Phrases::Warning << "The file \"srcdirref\" is empty." << Phrases::EndFlush;
            return string();
        }
        srcDirContent += "/testfiles/";

        // check whether the referenced source directory contains a "testfiles" directory
        if (!dirExists(srcDirContent)) {
            cerr << Phrases::Warning
                 << "The source directory referenced by the file \"srcdirref\" does not contain a \"testfiles\" directory or does not exist."
                 << Phrases::End << "Referenced source directory: " << srcDirContent << endl;
            return string();
        }
        return srcDirContent;

    } catch (const std::ios_base::failure &) {
        cerr << Phrases::Warning << "The file \"srcdirref\" can not be opened. It likely just doesn't exist in the working directory."
             << Phrases::EndFlush;
    }
    return string();
}
} // namespace CppUtilities
