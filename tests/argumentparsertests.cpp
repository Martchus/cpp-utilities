#include "./testutils.h"

#include "../application/argumentparser.h"
#include "../application/failure.h"
#include "../application/fakeqtconfigarguments.h"

#include "resources/config.h"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include <cstring>

using namespace std;
using namespace ApplicationUtilities;

using namespace CPPUNIT_NS;

/*!
 * \brief The ArgumentParserTests class tests the ArgumentParser and Argument classes.
 */
class ArgumentParserTests : public TestFixture
{
    CPPUNIT_TEST_SUITE(ArgumentParserTests);
    CPPUNIT_TEST(testArgument);
    CPPUNIT_TEST(testParsing);
    CPPUNIT_TEST(testCallbacks);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testArgument();
    void testParsing();
    void testCallbacks();

private:
    void callback();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ArgumentParserTests);

void ArgumentParserTests::setUp()
{}

void ArgumentParserTests::tearDown()
{}

/*!
 * \brief Tests the behaviour of the argument class.
 */
void ArgumentParserTests::testArgument()
{
    Argument argument("test", 't', "some description");
    CPPUNIT_ASSERT_EQUAL(argument.isRequired(), false);
    argument.setConstraints(1, 10);
    CPPUNIT_ASSERT_EQUAL(argument.isRequired(), true);
    Argument subArg("sub", 's', "sub arg");
    argument.addSubArgument(&subArg);
    CPPUNIT_ASSERT_EQUAL(subArg.parents().at(0), &argument);
    CPPUNIT_ASSERT(!subArg.conflictsWithArgument());

}

/*!
 * \brief Tests parsing command line arguments.
 */
void ArgumentParserTests::testParsing()
{
    ArgumentParser parser;

    // add some test argument definitions
    SET_APPLICATION_INFO;
    QT_CONFIG_ARGUMENTS qtConfigArgs;
    HelpArgument helpArg(parser);
    Argument verboseArg("verbose", 'v', "be verbose");
    verboseArg.setCombinable(true);
    Argument fileArg("file", 'f', "specifies the path of the file to be opened");
    fileArg.setValueNames({"path"});
    fileArg.setRequiredValueCount(1);
    Argument filesArg("files", 'f', "specifies the path of the file(s) to be opened");
    filesArg.setValueNames({"path 1", "path 2"});
    filesArg.setRequiredValueCount(-1);
    Argument outputFileArg("output-file", 'o', "specifies the path of the output file");
    outputFileArg.setValueNames({"path"});
    outputFileArg.setRequiredValueCount(1);
    outputFileArg.setRequired(true);
    outputFileArg.setCombinable(true);
    Argument printFieldNamesArg("print-field-names", '\0', "prints available field names");
    Argument displayFileInfoArg("display-file-info", 'i', "displays general file information");
    displayFileInfoArg.setDenotesOperation(true);
    displayFileInfoArg.setSubArguments({&fileArg, &verboseArg});
    Argument fieldsArg("fields", '\0', "specifies the fields");
    fieldsArg.setRequiredValueCount(-1);
    fieldsArg.setValueNames({"title", "album", "artist", "trackpos"});
    fieldsArg.setDefault(true);
    Argument displayTagInfoArg("get", 'p', "displays the values of all specified tag fields (displays all fields if none specified)");
    displayTagInfoArg.setDenotesOperation(true);
    displayTagInfoArg.setSubArguments({&fieldsArg, &filesArg, &verboseArg});
    parser.setMainArguments({&qtConfigArgs.qtWidgetsGuiArg(), &printFieldNamesArg, &displayTagInfoArg, &displayFileInfoArg, &helpArg});

    // define some argument values
    const char *argv[] = {"tageditor", "get", "album", "title", "diskpos", "-f", "somefile"};
    // try to parse, this should fail
    try {
        parser.parseArgs(7, argv);
        CPPUNIT_FAIL("Exception expected.");
    } catch(const Failure &e) {
        CPPUNIT_ASSERT(!strcmp(e.what(), "The argument \"files\" can not be combined with \"fields\"."));
    }

    // try to parse again, but adjust the configuration for a successful parse
    displayTagInfoArg.reset(), fieldsArg.reset(), filesArg.reset();
    filesArg.setCombinable(true);
    parser.parseArgs(7, argv);
    // check results
    CPPUNIT_ASSERT(!displayFileInfoArg.isPresent());
    CPPUNIT_ASSERT(!strcmp(parser.currentDirectory(), "tageditor"));
    CPPUNIT_ASSERT(!verboseArg.isPresent());
    CPPUNIT_ASSERT(displayTagInfoArg.isPresent());
    CPPUNIT_ASSERT(fieldsArg.isPresent());
    CPPUNIT_ASSERT(!strcmp(fieldsArg.values().at(0), "album"));
    CPPUNIT_ASSERT(!strcmp(fieldsArg.values().at(1), "title"));
    CPPUNIT_ASSERT(!strcmp(fieldsArg.values().at(2), "diskpos"));
    CPPUNIT_ASSERT_THROW(displayTagInfoArg.values().at(3), out_of_range);

    // define the same arguments in a different way
    const char *argv2[] = {"tageditor", "-p", "album", "title", "diskpos", "--file", "somefile"};
    // reparse the args
    displayTagInfoArg.reset(), fieldsArg.reset(), filesArg.reset();
    parser.parseArgs(7, argv2);
    // check results again
    CPPUNIT_ASSERT(!displayFileInfoArg.isPresent());
    CPPUNIT_ASSERT(!verboseArg.isPresent());
    CPPUNIT_ASSERT(displayTagInfoArg.isPresent());
    CPPUNIT_ASSERT(fieldsArg.isPresent());
    CPPUNIT_ASSERT(!strcmp(fieldsArg.values().at(0), "album"));
    CPPUNIT_ASSERT(!strcmp(fieldsArg.values().at(1), "title"));
    CPPUNIT_ASSERT(!strcmp(fieldsArg.values().at(2), "diskpos"));
    CPPUNIT_ASSERT_THROW(displayTagInfoArg.values().at(3), out_of_range);
    CPPUNIT_ASSERT(filesArg.isPresent());
    CPPUNIT_ASSERT(!strcmp(filesArg.values().at(0), "somefile"));

    // forget "get"/"-p"
    const char *argv3[] = {"tageditor", "album", "title", "diskpos", "--file", "somefile"};
    displayTagInfoArg.reset(), fieldsArg.reset(), filesArg.reset();

    // a parsing error should occur because the argument "album" is not defined
    try {
        parser.parseArgs(6, argv3);
        CPPUNIT_FAIL("Exception expected.");
    } catch(const Failure &e) {
        CPPUNIT_ASSERT(!strcmp(e.what(), "The specified argument \"album\" is unknown and will be ignored."));
    }

    // repeat the test, but this time just ignore the undefined argument
    displayTagInfoArg.reset(), fieldsArg.reset(), filesArg.reset();
    parser.setIgnoreUnknownArguments(true);
    parser.parseArgs(6, argv3);
    // none of the arguments should be present now
    CPPUNIT_ASSERT(!displayFileInfoArg.isPresent());
    CPPUNIT_ASSERT(!displayTagInfoArg.isPresent());
    CPPUNIT_ASSERT(!fieldsArg.isPresent());
    CPPUNIT_ASSERT(!filesArg.isPresent());

    // test abbreviations like "-vf"
    const char *argv4[] = {"tageditor", "-i", "-vf", "test"};
    displayTagInfoArg.reset(), fieldsArg.reset(), filesArg.reset();
    parser.setIgnoreUnknownArguments(false);
    parser.parseArgs(4, argv4);
    CPPUNIT_ASSERT(displayFileInfoArg.isPresent());
    CPPUNIT_ASSERT(verboseArg.isPresent());
    CPPUNIT_ASSERT(!displayTagInfoArg.isPresent());
    CPPUNIT_ASSERT(!filesArg.isPresent());
    CPPUNIT_ASSERT(fileArg.isPresent());
    CPPUNIT_ASSERT(!strcmp(fileArg.values().at(0), "test"));
    CPPUNIT_ASSERT_THROW(fileArg.values().at(1), out_of_range);

    // don't reset verbose argument to test constraint checking
    displayFileInfoArg.reset(), fileArg.reset();
    try {
        parser.parseArgs(4, argv4);
        CPPUNIT_FAIL("Exception expected.");
    } catch(const Failure &e) {
        CPPUNIT_ASSERT(!strcmp(e.what(), "The argument \"verbose\" mustn't be specified more than 1 time."));
    }

    // relax constraint
    displayFileInfoArg.reset(), fileArg.reset(), verboseArg.reset();
    verboseArg.setConstraints(0, -1);
    parser.parseArgs(4, argv4);

    // make verbose mandatory
    verboseArg.setRequired(true);
    displayFileInfoArg.reset(), fileArg.reset(), verboseArg.reset();
    parser.parseArgs(4, argv4);

    // make it complain about missing argument
    const char *argv5[] = {"tageditor", "-i", "-f", "test"};
    displayFileInfoArg.reset(), fileArg.reset(), verboseArg.reset();
    try {
        parser.parseArgs(4, argv5);
        CPPUNIT_FAIL("Exception expected.");
    } catch(const Failure &e) {
        CPPUNIT_ASSERT(!strcmp(e.what(), "The argument \"verbose\" must be specified at least 1 time."));
    }

    // it should not complain if -i is not present
    const char *argv6[] = {"tageditor", "-g"};
    displayFileInfoArg.reset(), fileArg.reset(), verboseArg.reset();
    parser.parseArgs(2, argv6);

    // it should not be possible to specify -f without -i or -p
    const char *argv7[] = {"tageditor", "-f", "test"};
    displayFileInfoArg.reset(), fileArg.reset(), verboseArg.reset();
    try {
        parser.parseArgs(3, argv7);
        CPPUNIT_FAIL("Exception expected.");
    } catch(const Failure &e) {
        CPPUNIT_ASSERT(!strcmp(e.what(), "The specified argument \"-f\" is unknown and will be ignored."));
    }
}

void ArgumentParserTests::testCallbacks()
{
    ArgumentParser parser;
    Argument callbackArg("with-callback", 't', "callback test");
    callbackArg.setRequiredValueCount(2);
    callbackArg.setCallback([] (const vector<const char *> &values) {
        CPPUNIT_ASSERT(values.size() == 2);
        CPPUNIT_ASSERT(!strcmp(values[0], "val1"));
        CPPUNIT_ASSERT(!strcmp(values[1], "val2"));
        throw 42;
    });
    Argument noCallbackArg("no-callback", 'l', "callback test");
    noCallbackArg.setRequiredValueCount(2);
    parser.setMainArguments({&callbackArg, &noCallbackArg});

    // test whether callback is invoked when argument with callback is specified
    const char *argv[] = {"test", "-t", "val1", "val2"};
    try {
        parser.parseArgs(4, argv);
    } catch(int i) {
        CPPUNIT_ASSERT_EQUAL(i, 42);
    }

    // test whether callback is not invoked when argument with callback is not specified
    callbackArg.reset();
    const char *argv2[] = {"test", "-l", "val1", "val2"};
    parser.parseArgs(4, argv2);

}
