#include "./testutils.h"

#include "../application/failure.h"
#include "../conversion/stringconversion.h"
#include "../io/catchiofailure.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <initializer_list>

#ifdef PLATFORM_UNIX
# include <unistd.h>
# include <poll.h>
# include <sys/wait.h>
# include <sys/stat.h>
#endif

using namespace std;
using namespace ApplicationUtilities;
using namespace ConversionUtilities;
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
TestApplication::TestApplication(int argc, char **argv) :
    m_helpArg(m_parser),
    m_testFilesPathArg("test-files-path", 'p', "specifies the path of the directory with test files"),
    m_applicationPathArg("app-path",'a', "specifies the path of the application to be tested"),
    m_workingDirArg("working-dir", 'w', "specifies the directory to store working copies of test files"),
    m_unitsArg("units", 'u', "specifies the units to test; omit to test all units")
{
    // check whether there is already an instance
    if(m_instance) {
        throw runtime_error("only one TestApplication instance allowed at a time");
    }
    m_instance = this;

    // read TEST_FILE_PATH environment variable
    if(const char *testFilesPathEnv = getenv("TEST_FILE_PATH")) {
        if(const auto len = strlen(testFilesPathEnv)) {
            m_testFilesPathEnvValue.reserve(len + 1);
            m_testFilesPathEnvValue += testFilesPathEnv;
            m_testFilesPathEnvValue += '/';
        }
    }

    // setup argument parser
    for(Argument *arg : initializer_list<Argument *>{&m_testFilesPathArg, &m_applicationPathArg, &m_workingDirArg}) {
        arg->setRequiredValueCount(1);
        arg->setValueNames({"path"});
        arg->setCombinable(true);
    }
    m_unitsArg.setRequiredValueCount(-1);
    m_unitsArg.setValueNames({"unit1", "unit2", "unit3"});
    m_unitsArg.setCombinable(true);
    m_parser.setMainArguments({&m_testFilesPathArg, &m_applicationPathArg, &m_workingDirArg, &m_unitsArg, &m_helpArg});

    // parse arguments
    try {
        m_parser.parseArgs(argc, argv);
        if(m_helpArg.isPresent()) {
            m_valid = false;
            exit(0);
        }
        cerr << "Directories used to search for testfiles:" << endl;
        if(m_testFilesPathArg.isPresent()) {
            if(*m_testFilesPathArg.values().front()) {
                cerr << ((m_testFilesPathArgValue = m_testFilesPathArg.values().front()) += '/') << endl;
            } else {
                cerr << (m_testFilesPathArgValue = "./") << endl;
            }
        }
        if(!m_testFilesPathEnvValue.empty()) {
            cerr << m_testFilesPathEnvValue << endl;
        }
        cerr << "./testfiles/" << endl << endl;
        cerr << "Directory used to store working copies:" << endl;
        if(m_workingDirArg.isPresent()) {
            if(*m_workingDirArg.values().front()) {
                (m_workingDir = m_workingDirArg.values().front()) += '/';
            } else {
                m_workingDir = "./";
            }
        } else if(const char *workingDirEnv = getenv("WORKING_DIR")) {
            if(const auto len = strlen(workingDirEnv)) {
                m_workingDir.reserve(len + 1);
                m_workingDir += workingDirEnv;
                m_workingDir += '/';
            }
        } else {
            if(m_testFilesPathArg.isPresent()) {
                m_workingDir = m_testFilesPathArgValue + "workingdir/";
            } else if(!m_testFilesPathEnvValue.empty()) {
                m_workingDir = m_testFilesPathEnvValue + "workingdir/";
            } else {
                m_workingDir = "./testfiles/workingdir/";
            }
        }
        cerr << m_workingDir << endl << endl;

        m_valid = true;
        cerr << "Executing test cases ..." << endl;
    } catch(const Failure &failure) {
        cerr << "Invalid arguments specified: " << failure.what() << endl;
        m_valid = false;
    }
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
 */
string TestApplication::testFilePath(const string &name) const
{
    string path;
    fstream file; // used to check whether the file is present

    // check the path specified by command line argument
    if(m_testFilesPathArg.isPresent()) {
        file.open(path = m_testFilesPathArgValue + name, ios_base::in);
        if(file.good()) {
            return path;
        }
    }

    // check the path specified by environment variable
    if(!m_testFilesPathEnvValue.empty()) {
        file.clear();
        file.open(path = m_testFilesPathEnvValue + name, ios_base::in);
        if(file.good()) {
            return path;
        }
    }

    // file still not found -> return default path
    return "./testfiles/" + name;
}

#ifdef PLATFORM_UNIX
/*!
 * \brief Returns the full path to a working copy of the test file with the specified \a name.
 * \remarks Currently only available under UNIX.
 */
string TestApplication::workingCopyPath(const string &name) const
{
    // create file streams
    fstream origFile, workingCopy;
    origFile.exceptions(ios_base::badbit | ios_base::failbit);
    workingCopy.exceptions(ios_base::badbit | ios_base::failbit);

    // ensure working directory is present
    struct stat currentStat;
    if(stat(m_workingDir.c_str(), &currentStat) || !S_ISDIR(currentStat.st_mode)) {
        if(mkdir(m_workingDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
            cerr << "Unable to create working copy for \"" << name << "\": can't create working directory." << endl;
            return string();
        }
    }

    // ensure subdirectory exists
    const auto parts = splitString<vector<string> >(name, string("/"), EmptyPartsTreat::Omit);
    if(!parts.empty()) {
        string currentLevel = m_workingDir;
        for(auto i = parts.cbegin(), end = parts.end() - 1; i != end; ++i) {
            if(currentLevel.back() != '/') {
                currentLevel += '/';
            }
            currentLevel += *i;
            if(stat(currentLevel.c_str(), &currentStat) || !S_ISDIR(currentStat.st_mode)) {
                if(mkdir(currentLevel.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
                    cerr << "Unable to create working copy for \"" << name << "\": can't create working directory." << endl;
                    return string();
                }
            }
        }
    }

    // copy file
    try {
        origFile.open(testFilePath(name), ios_base::in | ios_base::binary);
        const string path = m_workingDir + name;
        workingCopy.open(path, ios_base::out | ios_base::binary | ios_base::trunc);
        workingCopy << origFile.rdbuf();
        return path;
    } catch(...) {
        catchIoFailure();
        cerr << "Unable to create working copy for \"" << name << "\": an IO error occured." << endl;
    }
    return string();
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
int TestApplication::execApp(const char *const *args, string &stdout, string &stderr, bool suppressLogging, int timeout) const
{
    // print log message
    if(!suppressLogging) {
        cout << '-';
        for(const char *const *i = args; *i; ++i) {
            cout << ' ' << *i;
        }
        cout << endl;
    }
    // determine application path
    const char *appPath = m_applicationPathArg.firstValue();
    if(!appPath || !*appPath) {
        throw runtime_error("Unable to execute application to be tested: no application path specified");
    }
    // create pipes
    int coutPipes[2], cerrPipes[2];
    pipe(coutPipes), pipe(cerrPipes);
    int readCoutPipe = coutPipes[0], writeCoutPipe = coutPipes[1];
    int readCerrPipe = cerrPipes[0], writeCerrPipe = cerrPipes[1];
    // create child process
    if(int child = fork()) {
        // parent process: read stdout and stderr from child
        close(writeCoutPipe), close(writeCerrPipe);

        try {
            if(child == -1) {
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
            stdout.clear(), stderr.clear();

            // poll as long as at least one pipe is open
            do {
                int retpoll = poll(fileDescriptorSet, 2, timeout);
                if(retpoll > 0) {
                    // poll succeeds
                    if(fileDescriptorSet[0].revents & POLLIN) {
                        if((count = read(readCoutPipe, buffer, sizeof(buffer))) > 0) {
                            stdout.append(buffer, count);
                        }
                    } else if(fileDescriptorSet[0].revents & POLLHUP) {
                        close(readCoutPipe);
                        fileDescriptorSet[0].fd = -1;
                    }
                    if(fileDescriptorSet[1].revents & POLLIN) {
                        if((count = read(readCerrPipe, buffer, sizeof(buffer))) > 0) {
                            stderr.append(buffer, count);
                        }
                    } else if(fileDescriptorSet[1].revents & POLLHUP) {
                        close(readCerrPipe);
                        fileDescriptorSet[1].fd = -1;
                    }
                } else if(retpoll == 0) {
                    // timeout
                    throw runtime_error("Poll time-out");
                } else {
                    // fail
                    throw runtime_error("Poll failed");
                }
            } while(fileDescriptorSet[0].fd >= 0 || fileDescriptorSet[1].fd >= 0);
        } catch(...) {
            // ensure all pipes are close in the error case
            close(readCoutPipe), close(readCerrPipe);
            throw;
        }

        // get return code
        int childReturnCode;
        waitpid(child, &childReturnCode, 0);
        return childReturnCode;
    } else {
        // child process: set pipes to be used for stdout/stderr, execute application
        dup2(writeCoutPipe, STDOUT_FILENO), dup2(writeCerrPipe, STDERR_FILENO);
        close(readCoutPipe), close(writeCoutPipe), close(readCerrPipe), close(writeCerrPipe);
        execv(appPath, const_cast<char *const *>(args));
        cerr << "Unable to execute \"" << appPath << "\": execv() failed" << endl;
        exit(-101);
    }
}
#endif

}
