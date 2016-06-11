#include "./testutils.h"

#include "../application/failure.h"
#include "../conversion/stringconversion.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>

#include <sys/stat.h>

using namespace std;
using namespace ApplicationUtilities;
using namespace ConversionUtilities;

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
    m_testFilesPathArg.setRequiredValueCount(1);
    m_testFilesPathArg.setValueNames({"path"});
    m_testFilesPathArg.setCombinable(true);
    m_workingDirArg.setRequiredValueCount(1);
    m_workingDirArg.setValueNames({"path"});
    m_workingDirArg.setCombinable(true);
    m_unitsArg.setRequiredValueCount(-1);
    m_unitsArg.setValueNames({"unit1", "unit2", "unit3"});
    m_unitsArg.setCombinable(true);
    m_parser.setMainArguments({&m_testFilesPathArg, &m_workingDirArg, &m_unitsArg, &m_helpArg});

    // parse arguments
    try {
        m_parser.parseArgs(argc, argv);
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
        string path = m_workingDir + name;
        workingCopy.open(path, ios_base::out | ios_base::binary | ios_base::trunc);
        workingCopy << origFile.rdbuf();
        return path;
    } catch(const ios_base::failure &) {
        cerr << "Unable to create working copy for \"" << name << "\": an IO error occured." << endl;
    }
    return string();
}
#endif

}
