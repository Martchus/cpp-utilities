#include "./testutils.h"

#include "../application/failure.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>

using namespace std;
using namespace ApplicationUtilities;

namespace TestUtilities {

TestApplication *TestApplication::m_instance = nullptr;

/*!
 * \class TestApplication
 * \brief The TestApplication class simplifies writing test applications.
 * \remarks Only one instance is allowed at a time (singletone class).
 */

/*!
 * \brief Constructs a TestApplication instance.
 * \throws Throws std::runtime_error if an instance has already been created.
 */
TestApplication::TestApplication(int argc, char **argv) :
    m_helpArg(m_parser),
    m_testFilesPathArg("test-files-path", "p", "specifies the path of the directory with test files")
{
    if(m_instance) {
        throw runtime_error("only one TestApplication instance allowed at a time");
    }
    m_instance = this;
    if(const char *testFilesPathEnv = getenv("TEST_FILE_PATH")) {
        if(const auto len = strlen(testFilesPathEnv)) {
            m_testFilesPathEnvValue.reserve(len + 1);
            m_testFilesPathEnvValue += testFilesPathEnv;
            m_testFilesPathEnvValue += '/';
        }
    }
    m_testFilesPathArg.setRequiredValueCount(1);
    m_testFilesPathArg.setValueNames({"path"});
    m_testFilesPathArg.setCombinable(true);
    m_parser.setMainArguments({&m_testFilesPathArg, &m_helpArg});
    try {
        m_parser.parseArgs(argc, argv);
        cerr << "Directories used to search for testfiles: " << endl;
        if(m_testFilesPathArg.isPresent()) {
            if(!m_testFilesPathArg.values().front().empty()) {
                cerr << (m_testFilesPathArgValue = m_testFilesPathArg.values().front() + '/') << endl;
            } else {
                cerr << (m_testFilesPathArgValue = "./") << endl;
            }
        }
        if(!m_testFilesPathEnvValue.empty()) {
            cerr << m_testFilesPathEnvValue << endl;
        }
        cerr << "./testfiles/" << endl << endl;
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
 * \brief Returns the full path of tbe test file with the specified \a name.
 */
string TestApplication::testFilePath(const string &name) const
{
    string path;
    fstream file;
    if(m_testFilesPathArg.isPresent()) {
        file.open(path = m_testFilesPathArgValue + name, ios_base::in);
        if(file.good()) {
            return path;
        }
    }
    if(!m_testFilesPathEnvValue.empty()) {
        file.clear();
        file.open(path = m_testFilesPathEnvValue + name, ios_base::in);
        if(file.good()) {
            return path;
        }
    }
    return "./testfiles/" + name;
}

}
