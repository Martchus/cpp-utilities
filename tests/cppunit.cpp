#include "../application/argumentparser.h"
#include "../application/failure.h"

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <iostream>

using namespace std;
using namespace ApplicationUtilities;
using namespace CPPUNIT_NS;

namespace UnitTests {

string testFilesPath("tests");

}

int main(int argc, char **argv)
{
    // setup argument parser
    ArgumentParser parser;
    HelpArgument helpArg(parser);
    Argument testFilesPathArg("test-files-path", "p", "specifies the path to the directory with test files");
    testFilesPathArg.setRequiredValueCount(1);
    testFilesPathArg.setValueNames({"path"});
    testFilesPathArg.setCombinable(true);
    parser.setMainArguments({&testFilesPathArg, &helpArg});

    try {
        // parse arguments
        parser.parseArgs(argc, argv);
        if(testFilesPathArg.isPresent()) {
            UnitTests::testFilesPath = testFilesPathArg.values().front();
        }
        cerr << "Direcoty for test files: " << UnitTests::testFilesPath << endl;

        // run tests
        TextUi::TestRunner runner;
        TestFactoryRegistry &registry = TestFactoryRegistry::getRegistry();
        runner.addTest(registry.makeTest());
        return !runner.run(string(), false);
    } catch(const Failure &failure) {
        cerr << "Invalid arguments specified: " << failure.what() << endl;
        return -1;
    }


}
