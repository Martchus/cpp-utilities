#include "./testutils.h"

#include "../application/failure.h"
#include "../conversion/stringbuilder.h"
#include "../conversion/stringconversion.h"
#include "../io/ansiescapecodes.h"
#include "../io/catchiofailure.h"
#include "../io/misc.h"
#include "../io/path.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <limits>

#ifdef PLATFORM_UNIX
#include <poll.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

using namespace std;
using namespace ApplicationUtilities;
using namespace ConversionUtilities;
using namespace EscapeCodes;
using namespace IoUtilities;

/*!
 * \brief Contains classes and functions utilizing creating of test applications.
 */
namespace TestUtilities {

TestApplication *TestApplication::m_instance = nullptr;

/*!
 * \class TestApplication
 * \brief The TestApplication class simplifies writing test applications that require opening test files.
 * \remarks Only one instance is allowed at a time (singletone class).
 */

/*!
 * \brief Constructs a TestApplication instance.
 * \throws Throws std::runtime_error if an instance has already been created.
 */
TestApplication::TestApplication(int argc, char **argv)
    : m_helpArg(m_parser)
    , m_testFilesPathArg("test-files-path", 'p', "specifies the path of the directory with test files")
    , m_applicationPathArg("app-path", 'a', "specifies the path of the application to be tested")
    , m_workingDirArg("working-dir", 'w', "specifies the directory to store working copies of test files")
    , m_unitsArg("units", 'u', "specifies the units to test; omit to test all units")
{
    // check whether there is already an instance
    if (m_instance) {
        throw runtime_error("only one TestApplication instance allowed at a time");
    }
    m_instance = this;

    // determine fallback path for testfiles which is used when --test-files-path/-p not present
    // -> read TEST_FILE_PATH environment variable
    m_fallbackTestFilesPath = readTestfilePathFromEnv();
    // -> find source directory if TEST_FILE_PATH not present
    bool fallbackIsSourceDir = m_fallbackTestFilesPath.empty();
    if (fallbackIsSourceDir) {
        m_fallbackTestFilesPath = readTestfilePathFromSrcRef();
    }

    // handle specified arguments (if present)
    if (argc && argv) {
        // setup argument parser
        for (Argument *arg : initializer_list<Argument *>{ &m_testFilesPathArg, &m_applicationPathArg, &m_workingDirArg }) {
            arg->setRequiredValueCount(1);
            arg->setValueNames({ "path" });
            arg->setCombinable(true);
        }
        m_unitsArg.setRequiredValueCount(Argument::varValueCount);
        m_unitsArg.setValueNames({ "unit1", "unit2", "unit3" });
        m_unitsArg.setCombinable(true);
        m_parser.setMainArguments({ &m_testFilesPathArg, &m_applicationPathArg, &m_workingDirArg, &m_unitsArg, &m_helpArg });

        // parse arguments
        try {
            m_parser.parseArgs(argc, argv);
        } catch (const Failure &failure) {
            cerr << failure;
            m_valid = false;
            return;
        }

        // print help
        if (m_helpArg.isPresent()) {
            exit(0);
        }
    }

    // handle path for testfiles and working-copy
    cerr << "Directories used to search for testfiles:" << endl;
    if (m_testFilesPathArg.isPresent()) {
        if (*m_testFilesPathArg.values().front()) {
            cerr << ((m_testFilesPath = m_testFilesPathArg.values().front()) += '/') << endl;
        } else {
            cerr << (m_testFilesPath = "./") << endl;
        }
    } else {
        // use fallback path if --test-files-path/-p not present
        m_testFilesPath.swap(m_fallbackTestFilesPath);
    }
    // if it wasn't already the case, use the source directory as fallback dir
    if (m_fallbackTestFilesPath.empty() && !fallbackIsSourceDir) {
        m_fallbackTestFilesPath = readTestfilePathFromSrcRef();
        fallbackIsSourceDir = true;
    }
    if (!m_fallbackTestFilesPath.empty() && m_testFilesPath != m_fallbackTestFilesPath) {
        cerr << m_fallbackTestFilesPath << endl;
    }
    cerr << "./testfiles/" << endl << endl;
    cerr << "Directory used to store working copies:" << endl;
    if (m_workingDirArg.isPresent()) {
        if (*m_workingDirArg.values().front()) {
            (m_workingDir = m_workingDirArg.values().front()) += '/';
        } else {
            m_workingDir = "./";
        }
    } else if (const char *workingDirEnv = getenv("WORKING_DIR")) {
        if (*workingDirEnv) {
            m_workingDir = argsToString(workingDirEnv, '/');
        }
    } else {
        if (m_testFilesPathArg.isPresent()) {
            m_workingDir = m_testFilesPath + "workingdir/";
        } else if (!m_fallbackTestFilesPath.empty() && !fallbackIsSourceDir) {
            m_workingDir = m_fallbackTestFilesPath + "workingdir/";
        } else {
            m_workingDir = "./testfiles/workingdir/";
        }
    }
    cerr << m_workingDir << endl << endl;

    // clear list of all additional profiling files created when forking the test application
    if (const char *profrawListFile = getenv("LLVM_PROFILE_LIST_FILE")) {
        ofstream(profrawListFile, ios_base::trunc);
    }

    m_valid = true;
    cerr << TextAttribute::Bold << "Executing test cases ..." << Phrases::EndFlush;
}

/*!
 * \brief Destroys the TestApplication.
 */
TestApplication::~TestApplication()
{
    m_instance = nullptr;
}

/*!
 * \brief Returns the full path of the test file with the specified \a name.
 *
 * The specified \a name might be a relative path in the testfiles directory.
 *
 * The following directories are searched for the specified testfile:
 * 1. The directory specified as CLI argument.
 * 2. The fallback directory, which can be set by setting the environment
 *    variable `TEST_FILE_PATH`.
 * 3. The source directory, if it could be determined via "srcref"-file
 *    unless both, the CLI argument and environment variable are present.
 */
string TestApplication::testFilePath(const string &name) const
{
    string path;
    fstream file; // used to check whether the file is present

    // check the path specified by command line argument or via environment variable
    if (!m_testFilesPath.empty()) {
        file.open(path = m_testFilesPath + name, ios_base::in);
        if (file.good()) {
            return path;
        }
    }

    // check the fallback path (value from environment variable or source directory)
    if (!m_fallbackTestFilesPath.empty()) {
        file.clear();
        file.open(path = m_fallbackTestFilesPath + name, ios_base::in);
        if (file.good()) {
            return path;
        }
    }

    // file still not found -> return default path
    file.clear();
    file.open(path = "./testfiles/" + name, ios_base::in);
    if (!file.good()) {
        cerr << Phrases::Warning << "The testfile \"" << path << "\" can not be located." << Phrases::EndFlush;
    }
    return path;
}

#ifdef PLATFORM_UNIX
/*!
 * \brief Returns the full path to a working copy of the test file with the specified \a name.
 *
 * The specified \a mode controls whether a working copy is actually created or whether just the path is returned.
 * The test file is located using testFilePath().
 *
 * \remarks Currently only available under UNIX.
 */
string TestApplication::workingCopyPathMode(const string &name, WorkingCopyMode mode) const
{
    // create file streams
    fstream origFile, workingCopy;
    origFile.exceptions(ios_base::badbit | ios_base::failbit);
    workingCopy.exceptions(ios_base::badbit | ios_base::failbit);

    // ensure working directory is present
    struct stat currentStat;
    if (stat(m_workingDir.c_str(), &currentStat) || !S_ISDIR(currentStat.st_mode)) {
        if (mkdir(m_workingDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
            cerr << Phrases::Error << "Unable to create working copy for \"" << name << "\": can't create working directory." << Phrases::EndFlush;
            return string();
        }
    }

    // ensure subdirectory exists
    const auto parts = splitString<vector<string>>(name, string("/"), EmptyPartsTreat::Omit);
    if (!parts.empty()) {
        string currentLevel = m_workingDir;
        for (auto i = parts.cbegin(), end = parts.end() - 1; i != end; ++i) {
            if (currentLevel.back() != '/') {
                currentLevel += '/';
            }
            currentLevel += *i;
            if (stat(currentLevel.c_str(), &currentStat) || !S_ISDIR(currentStat.st_mode)) {
                if (mkdir(currentLevel.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
                    cerr << Phrases::Error << "Unable to create working copy for \"" << name << "\": can't create working directory."
                         << Phrases::EndFlush;
                    return string();
                }
            }
        }
    }

    // copy file
    if (mode != WorkingCopyMode::NoCopy) {
        try {
            origFile.open(testFilePath(name), ios_base::in | ios_base::binary);
            const string path = m_workingDir + name;
            workingCopy.open(path, ios_base::out | ios_base::binary | ios_base::trunc);
            workingCopy << origFile.rdbuf();
            return path;
        } catch (...) {
            catchIoFailure();
            cerr << Phrases::Error << "Unable to create working copy for \"" << name << "\": an IO error occured." << Phrases::EndFlush;
        }
    } else {
        return m_workingDir + name;
    }
    return string();
}

/*!
 * \brief Creates a working copy of the test file with the specified \a name and returns the full path of the created file.
 *
 * The test file is located using testFilePath().
 *
 * \remarks Currently only available under UNIX.
 */
string TestApplication::workingCopyPath(const string &name) const
{
    return workingCopyPathMode(name, WorkingCopyMode::CreateCopy);
}

/*!
 * \brief Executes an application with the specified \a args.
 * \remarks Provides internal implementation of execApp() and execHelperApp().
 */
int execAppInternal(const char *appPath, const char *const *args, std::string &output, std::string &errors, bool suppressLogging, int timeout,
    const std::string &newProfilingPath)
{
    // print log message
    if (!suppressLogging) {
        cout << '-';
        for (const char *const *i = args; *i; ++i) {
            cout << ' ' << *i;
        }
        cout << endl;
    }

    // create pipes
    int coutPipes[2], cerrPipes[2];
    pipe(coutPipes), pipe(cerrPipes);
    int readCoutPipe = coutPipes[0], writeCoutPipe = coutPipes[1];
    int readCerrPipe = cerrPipes[0], writeCerrPipe = cerrPipes[1];

    // create child process
    if (int child = fork()) {
        // parent process: read stdout and stderr from child
        close(writeCoutPipe), close(writeCerrPipe);

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
            ssize_t count;
            output.clear(), errors.clear();

            // poll as long as at least one pipe is open
            do {
                int retpoll = poll(fileDescriptorSet, 2, timeout);
                if (retpoll > 0) {
                    // poll succeeds
                    if (fileDescriptorSet[0].revents & POLLIN) {
                        if ((count = read(readCoutPipe, buffer, sizeof(buffer))) > 0) {
                            output.append(buffer, static_cast<size_t>(count));
                        }
                    } else if (fileDescriptorSet[0].revents & POLLHUP) {
                        close(readCoutPipe);
                        fileDescriptorSet[0].fd = -1;
                    }
                    if (fileDescriptorSet[1].revents & POLLIN) {
                        if ((count = read(readCerrPipe, buffer, sizeof(buffer))) > 0) {
                            errors.append(buffer, static_cast<size_t>(count));
                        }
                    } else if (fileDescriptorSet[1].revents & POLLHUP) {
                        close(readCerrPipe);
                        fileDescriptorSet[1].fd = -1;
                    }
                } else if (retpoll == 0) {
                    // timeout
                    throw runtime_error("Poll time-out");
                } else {
                    // fail
                    throw runtime_error("Poll failed");
                }
            } while (fileDescriptorSet[0].fd >= 0 || fileDescriptorSet[1].fd >= 0);
        } catch (...) {
            // ensure all pipes are close in the error case
            close(readCoutPipe), close(readCerrPipe);
            throw;
        }

        // get return code
        int childReturnCode;
        waitpid(child, &childReturnCode, 0);
        return childReturnCode;
    } else {
        // child process
        // -> set pipes to be used for stdout/stderr
        dup2(writeCoutPipe, STDOUT_FILENO), dup2(writeCerrPipe, STDERR_FILENO);
        close(readCoutPipe), close(writeCoutPipe), close(readCerrPipe), close(writeCerrPipe);

        // -> modify environment variable LLVM_PROFILE_FILE to apply new path for profiling output
        if (!newProfilingPath.empty()) {
            setenv("LLVM_PROFILE_FILE", newProfilingPath.data(), true);
        }

        // -> execute application
        execv(appPath, const_cast<char *const *>(args));
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
    string newProfilingPath;
    if (const char *llvmProfileFile = getenv("LLVM_PROFILE_FILE")) {
        // replace eg. "/some/path/tageditor_tests.profraw" with "/some/path/tageditor0.profraw"
        if (const char *llvmProfileFileEnd = strstr(llvmProfileFile, ".profraw")) {
            const string llvmProfileFileWithoutExtension(llvmProfileFile, llvmProfileFileEnd);
            // extract application name from path
            const char *appName = strrchr(appPath, '/');
            appName = appName ? appName + 1 : appPath;
            // concat new path
            newProfilingPath = argsToString(llvmProfileFileWithoutExtension, '_', appName, invocationCount, ".profraw");
            // append path to profiling list file
            if (const char *profrawListFile = getenv("LLVM_PROFILE_LIST_FILE")) {
                ofstream(profrawListFile, ios_base::app) << newProfilingPath << endl;
            }
        }
    }

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
#endif // PLATFORM_UNIX

string TestApplication::readTestfilePathFromEnv()
{
    const char *const testFilesPathEnv = getenv("TEST_FILE_PATH");
    if (!testFilesPathEnv || !*testFilesPathEnv) {
        return string();
    }
    return argsToString(testFilesPathEnv, '/');
}

string TestApplication::readTestfilePathFromSrcRef()
{
    try {
        // read "srcdirref" file which should contain the path of the source directory; this file should have been
        // create by the CMake module "TestTarget.cmake"
        string srcDirContent(readFile("srcdirref", 2 * 1024));
        if (srcDirContent.empty()) {
            cerr << Phrases::Warning << "The file \"srcdirref\" is empty." << Phrases::EndFlush;
            return string();
        }

        // check whether the referenced source directory contains a "testfiles" directory
#ifdef PLATFORM_UNIX // directoryEntries() is not implemented under Windows so we can only to the check under UNIX
        bool hasTestfilesDir = false;
        for (const string &dir : directoryEntries(srcDirContent.data(), DirectoryEntryType::Directory)) {
            if (dir == "testfiles") {
                hasTestfilesDir = true;
                break;
            }
        }
        if (!hasTestfilesDir) {
            cerr << Phrases::Warning
                 << "The source directory referenced by the file \"srcdirref\" does not contain a \"testfiles\" directory or does not exist."
                 << Phrases::End << "Referenced source directory: " << srcDirContent << endl;
            return string();
        }
#endif // PLATFORM_UNIX

        return srcDirContent += "/testfiles/";
    } catch (...) {
        cerr << Phrases::Warning << "The file \"srcdirref\" can not be opened. It likely just doesn't exist in the working directory."
             << Phrases::EndFlush;
        catchIoFailure();
    }
    return string();
}
} // namespace TestUtilities
