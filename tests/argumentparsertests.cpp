#include "./testutils.h"

#include "../conversion/stringbuilder.h"

#include "../application/argumentparser.h"
#include "../application/argumentparserprivate.h"
#include "../application/failure.h"
#include "../application/fakeqtconfigarguments.h"

#include "../io/path.h"

#include "resources/config.h"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include <cstring>
#include <cstdlib>

using namespace std;
using namespace ApplicationUtilities;
using namespace ConversionUtilities;

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
    CPPUNIT_TEST(testBashCompletion);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testArgument();
    void testParsing();
    void testCallbacks();
    void testBashCompletion();

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
    CPPUNIT_ASSERT(!argument.firstValue());
    argument.setEnvironmentVariable("PATH");
    if(getenv("PATH")) {
        CPPUNIT_ASSERT(argument.firstValue());
        CPPUNIT_ASSERT(!strcmp(argument.firstValue(), getenv("PATH")));
    } else {
        CPPUNIT_ASSERT(!argument.firstValue());
    }
}

/*!
 * \brief Tests parsing command line arguments.
 */
void ArgumentParserTests::testParsing()
{
    // setup parser with some test argument definitions
    ArgumentParser parser;
    SET_APPLICATION_INFO;
    QT_CONFIG_ARGUMENTS qtConfigArgs;
    HelpArgument helpArg(parser);
    Argument verboseArg("verbose", 'v', "be verbose");
    verboseArg.setCombinable(true);
    Argument fileArg("file", 'f', "specifies the path of the file to be opened");
    fileArg.setValueNames({"path"});
    fileArg.setRequiredValueCount(1);
    fileArg.setEnvironmentVariable("PATH");
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
    Argument notAlbumArg("album", 'a', "should not be confused with album value");
    displayFileInfoArg.setDenotesOperation(true);
    displayFileInfoArg.setSubArguments({&fileArg, &verboseArg, &notAlbumArg});
    Argument fieldsArg("fields", '\0', "specifies the fields");
    fieldsArg.setRequiredValueCount(-1);
    fieldsArg.setValueNames({"title", "album", "artist", "trackpos"});
    fieldsArg.setImplicit(true);
    Argument displayTagInfoArg("get", 'p', "displays the values of all specified tag fields (displays all fields if none specified)");
    displayTagInfoArg.setDenotesOperation(true);
    displayTagInfoArg.setSubArguments({&fieldsArg, &filesArg, &verboseArg, &notAlbumArg});
    parser.setMainArguments({&qtConfigArgs.qtWidgetsGuiArg(), &printFieldNamesArg, &displayTagInfoArg, &displayFileInfoArg, &helpArg});

    // error about uncombinable arguments
    const char *argv[] = {"tageditor", "get", "album", "title", "diskpos", "-f", "somefile"};
    // try to parse, this should fail
    try {
        parser.parseArgs(7, argv);
        CPPUNIT_FAIL("Exception expected.");
    } catch(const Failure &e) {
        CPPUNIT_ASSERT(!strcmp(e.what(), "The argument \"files\" can not be combined with \"fields\"."));
    }

    // arguments read correctly after successful parse
    filesArg.setCombinable(true);
    parser.resetArgs();
    parser.parseArgs(7, argv);
    // check results
    CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());
    CPPUNIT_ASSERT(!displayFileInfoArg.isPresent());
    CPPUNIT_ASSERT(!strcmp(parser.executable(), "tageditor"));
    CPPUNIT_ASSERT(!verboseArg.isPresent());
    CPPUNIT_ASSERT(displayTagInfoArg.isPresent());
    CPPUNIT_ASSERT(fieldsArg.isPresent());
    CPPUNIT_ASSERT(!strcmp(fieldsArg.values().at(0), "album"));
    CPPUNIT_ASSERT(!strcmp(fieldsArg.values().at(1), "title"));
    CPPUNIT_ASSERT(!strcmp(fieldsArg.values().at(2), "diskpos"));
    CPPUNIT_ASSERT_THROW(displayTagInfoArg.values().at(3), out_of_range);

    // skip empty args
    const char *argv2[] = {"tageditor", "", "-p", "album", "title", "diskpos", "", "--files", "somefile"};
    // reparse the args
    parser.resetArgs();
    parser.parseArgs(9, argv2);
    // check results again
    CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());
    CPPUNIT_ASSERT(!displayFileInfoArg.isPresent());
    CPPUNIT_ASSERT(!verboseArg.isPresent());
    CPPUNIT_ASSERT(displayTagInfoArg.isPresent());
    CPPUNIT_ASSERT(fieldsArg.isPresent());
    CPPUNIT_ASSERT(!strcmp(fieldsArg.values().at(0), "album"));
    CPPUNIT_ASSERT(!strcmp(fieldsArg.values().at(1), "title"));
    CPPUNIT_ASSERT(!strcmp(fieldsArg.values().at(2), "diskpos"));
    CPPUNIT_ASSERT(!strcmp(fieldsArg.values().at(3), ""));
    CPPUNIT_ASSERT_THROW(fieldsArg.values().at(4), out_of_range);
    CPPUNIT_ASSERT(filesArg.isPresent());
    CPPUNIT_ASSERT(!strcmp(filesArg.values().at(0), "somefile"));

    // error about unknown argument: forget get/-p
    const char *argv3[] = {"tageditor", "album", "title", "diskpos", "--files", "somefile"};
    try {
        parser.resetArgs();
        parser.parseArgs(6, argv3);
        CPPUNIT_FAIL("Exception expected.");
    } catch(const Failure &e) {
        CPPUNIT_ASSERT(!strcmp(e.what(), "The specified argument \"album\" is unknown and will be ignored."));
    }

    // warning about unknown argument
    parser.setUnknownArgumentBehavior(UnknownArgumentBehavior::Warn);
    // redirect stderr to check whether warnings are printed correctly
    stringstream buffer;
    streambuf *regularCerrBuffer = cerr.rdbuf(buffer.rdbuf());
    parser.resetArgs();
    try {
        parser.parseArgs(6, argv3);
    } catch(...) {
        cerr.rdbuf(regularCerrBuffer);
        throw;
    }
    cerr.rdbuf(regularCerrBuffer);
    CPPUNIT_ASSERT(!strcmp(buffer.str().data(), "The specified argument \"album\" is unknown and will be ignored.\n"
                                                "The specified argument \"title\" is unknown and will be ignored.\n"
                                                "The specified argument \"diskpos\" is unknown and will be ignored.\n"
                                                "The specified argument \"--files\" is unknown and will be ignored.\n"
                                                "The specified argument \"somefile\" is unknown and will be ignored.\n"));
    // none of the arguments should be present now
    CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());
    CPPUNIT_ASSERT(!displayFileInfoArg.isPresent());
    CPPUNIT_ASSERT(!displayTagInfoArg.isPresent());
    CPPUNIT_ASSERT(!fieldsArg.isPresent());
    CPPUNIT_ASSERT(!filesArg.isPresent());

    // combined abbreviations like "-vf"
    const char *argv4[] = {"tageditor", "-i", "-vf", "test"};
    parser.setUnknownArgumentBehavior(UnknownArgumentBehavior::Fail);
    parser.resetArgs();
    parser.parseArgs(4, argv4);
    CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());
    CPPUNIT_ASSERT(displayFileInfoArg.isPresent());
    CPPUNIT_ASSERT(verboseArg.isPresent());
    CPPUNIT_ASSERT(!displayTagInfoArg.isPresent());
    CPPUNIT_ASSERT(!filesArg.isPresent());
    CPPUNIT_ASSERT(fileArg.isPresent());
    CPPUNIT_ASSERT(!strcmp(fileArg.values().at(0), "test"));
    CPPUNIT_ASSERT_THROW(fileArg.values().at(1), out_of_range);

    // constraint checking: no multiple occurrences (not resetting verboseArg on purpose)
    displayFileInfoArg.reset(), fileArg.reset();
    try {
        parser.parseArgs(4, argv4);
        CPPUNIT_FAIL("Exception expected.");
    } catch(const Failure &e) {
        CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());
        CPPUNIT_ASSERT(!strcmp(e.what(), "The argument \"verbose\" mustn't be specified more than 1 time."));
    }

    // constraint checking: no contraint (not resetting verboseArg on purpose)
    displayFileInfoArg.reset(), fileArg.reset();
    verboseArg.setConstraints(0, -1);
    parser.parseArgs(4, argv4);
    CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());

    // constraint checking: mandatory argument
    verboseArg.setRequired(true);
    parser.resetArgs();
    parser.parseArgs(4, argv4);
    CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());

    // contraint checking: error about missing mandatory argument
    const char *argv5[] = {"tageditor", "-i", "-f", "test"};
    displayFileInfoArg.reset(), fileArg.reset(), verboseArg.reset();
    try {
        parser.parseArgs(4, argv5);
        CPPUNIT_FAIL("Exception expected.");
    } catch(const Failure &e) {
        CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());
        CPPUNIT_ASSERT(!strcmp(e.what(), "The argument \"verbose\" must be specified at least 1 time."));
    }
    verboseArg.setRequired(false);

    // combined abbreviation with nesting "-pf"
    const char *argv10[] = {"tageditor", "-pf", "test"};
    parser.resetArgs();
    parser.parseArgs(3, argv10);
    CPPUNIT_ASSERT(displayTagInfoArg.isPresent());
    CPPUNIT_ASSERT(!displayFileInfoArg.isPresent());
    CPPUNIT_ASSERT(!fileArg.isPresent());
    CPPUNIT_ASSERT(filesArg.isPresent());
    CPPUNIT_ASSERT_EQUAL(filesArg.values(0).size(), static_cast<vector<const char *>::size_type>(1));
    CPPUNIT_ASSERT(!strcmp(filesArg.values(0).front(), "test"));
    CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());

    // constraint checking: no complains about missing -i
    const char *argv6[] = {"tageditor", "-g"};
    parser.resetArgs();
    parser.parseArgs(2, argv6);
    CPPUNIT_ASSERT(qtConfigArgs.qtWidgetsGuiArg().isPresent());

    // constraint checking: dependend arguments (-f requires -i or -p)
    const char *argv7[] = {"tageditor", "-f", "test"};
    parser.resetArgs();
    try {
        parser.parseArgs(3, argv7);
        CPPUNIT_FAIL("Exception expected.");
    } catch(const Failure &e) {
        CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());
        CPPUNIT_ASSERT(!strcmp(e.what(), "The specified argument \"-f\" is unknown and will be ignored."));
    }

    // equation sign syntax
    const char *argv11[] = {"tageditor", "-if=test"};
    parser.resetArgs();
    parser.parseArgs(2, argv11);
    CPPUNIT_ASSERT(!filesArg.isPresent());
    CPPUNIT_ASSERT(fileArg.isPresent());
    CPPUNIT_ASSERT_EQUAL(fileArg.values(0).size(), static_cast<vector<const char *>::size_type>(1));
    CPPUNIT_ASSERT(!strcmp(fileArg.values(0).front(), "test"));

    // specifying value directly after abbreviation
    const char *argv12[] = {"tageditor", "-iftest"};
    parser.resetArgs();
    parser.parseArgs(2, argv12);
    CPPUNIT_ASSERT(!filesArg.isPresent());
    CPPUNIT_ASSERT(fileArg.isPresent());
    CPPUNIT_ASSERT_EQUAL(fileArg.values(0).size(), static_cast<vector<const char *>::size_type>(1));
    CPPUNIT_ASSERT(!strcmp(fileArg.values(0).front(), "test"));

    // default argument
    const char *argv8[] = {"tageditor"};
    parser.resetArgs();
    parser.parseArgs(1, argv8);
    CPPUNIT_ASSERT(qtConfigArgs.qtWidgetsGuiArg().isPresent());
    CPPUNIT_ASSERT(!displayFileInfoArg.isPresent());
    CPPUNIT_ASSERT(!verboseArg.isPresent());
    CPPUNIT_ASSERT(!displayTagInfoArg.isPresent());
    CPPUNIT_ASSERT(!filesArg.isPresent());
    CPPUNIT_ASSERT(!fileArg.isPresent());
    if(getenv("PATH")) {
        CPPUNIT_ASSERT(fileArg.firstValue());
        CPPUNIT_ASSERT(!strcmp(fileArg.firstValue(), getenv("PATH")));
    } else {
        CPPUNIT_ASSERT(!fileArg.firstValue());
    }

    // constraint checking: required value count with sufficient number of provided parameters
    const char *argv13[] = {"tageditor", "get", "--fields", "album=test", "title", "diskpos", "--files", "somefile"};
    verboseArg.setRequired(false);
    parser.resetArgs();
    parser.parseArgs(8, argv13);
    // this should still work without complaints
    CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());
    CPPUNIT_ASSERT(!displayFileInfoArg.isPresent());
    CPPUNIT_ASSERT(!verboseArg.isPresent());
    CPPUNIT_ASSERT(displayTagInfoArg.isPresent());
    CPPUNIT_ASSERT(fieldsArg.isPresent());
    CPPUNIT_ASSERT(!strcmp(fieldsArg.values().at(0), "album=test"));
    CPPUNIT_ASSERT(!strcmp(fieldsArg.values().at(1), "title"));
    CPPUNIT_ASSERT(!strcmp(fieldsArg.values().at(2), "diskpos"));
    CPPUNIT_ASSERT_THROW(fieldsArg.values().at(3), out_of_range);
    CPPUNIT_ASSERT(filesArg.isPresent());
    CPPUNIT_ASSERT(!strcmp(filesArg.values().at(0), "somefile"));
    CPPUNIT_ASSERT(!notAlbumArg.isPresent());

    // constraint checking: required value count with insufficient number of provided parameters
    const char *argv9[] = {"tageditor", "-p", "album", "title", "diskpos"};
    fieldsArg.setRequiredValueCount(4);
    parser.resetArgs();
    try {
        parser.parseArgs(5, argv9);
        CPPUNIT_FAIL("Exception expected.");
    } catch(const Failure &e) {
        CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());
        CPPUNIT_ASSERT(!strcmp(e.what(), "Not all parameter for argument \"fields\" provided. You have to provide the following parameter: title album artist trackpos"));
    }
}

/*!
 * \brief Tests whether callbacks are called correctly.
 */
void ArgumentParserTests::testCallbacks()
{
    ArgumentParser parser;
    Argument callbackArg("with-callback", 't', "callback test");
    callbackArg.setRequiredValueCount(2);
    callbackArg.setCallback([] (const ArgumentOccurrence &occurrence) {
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), occurrence.index);
        CPPUNIT_ASSERT(occurrence.path.empty());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), occurrence.values.size());
        CPPUNIT_ASSERT(!strcmp(occurrence.values[0], "val1"));
        CPPUNIT_ASSERT(!strcmp(occurrence.values[1], "val2"));
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

/*!
 * \brief Tests bash completion.
 * \remarks This tests makes assumptions about the order and the exact output format
 *          which should be improved.
 */
void ArgumentParserTests::testBashCompletion()
{
    ArgumentParser parser;
    HelpArgument helpArg(parser);
    Argument verboseArg("verbose", 'v', "be verbose");
    verboseArg.setCombinable(true);
    Argument filesArg("files", 'f', "specifies the path of the file(s) to be opened");
    filesArg.setRequiredValueCount(-1);
    filesArg.setCombinable(true);
    Argument nestedSubArg("nested-sub", '\0', "nested sub arg");
    Argument subArg("sub", '\0', "sub arg");
    subArg.setSubArguments({&nestedSubArg});
    Argument displayFileInfoArg("display-file-info", 'i', "displays general file information");
    displayFileInfoArg.setDenotesOperation(true);
    displayFileInfoArg.setSubArguments({&filesArg, &verboseArg, &subArg});
    Argument fieldsArg("fields", '\0', "specifies the fields");
    fieldsArg.setRequiredValueCount(-1);
    fieldsArg.setPreDefinedCompletionValues("title album artist trackpos");
    fieldsArg.setImplicit(true);
    Argument valuesArg("values", '\0', "specifies the fields");
    valuesArg.setRequiredValueCount(-1);
    valuesArg.setPreDefinedCompletionValues("title album artist trackpos");
    valuesArg.setImplicit(true);
    valuesArg.setValueCompletionBehavior(ValueCompletionBehavior::PreDefinedValues | ValueCompletionBehavior::AppendEquationSign);
    Argument getArg("get", 'g', "gets tag values");
    getArg.setSubArguments({&fieldsArg, &filesArg});
    Argument setArg("set", 's', "sets tag values");
    setArg.setSubArguments({&valuesArg, &filesArg});

    parser.setMainArguments({&helpArg, &displayFileInfoArg, &getArg, &setArg});

    // redirect cout to custom buffer
    stringstream buffer;
    streambuf *regularCoutBuffer = cout.rdbuf(buffer.rdbuf());

    try {
        // fail due to operation flags not set
        const char *const argv1[] = {"se"};
        ArgumentReader reader(parser, argv1, argv1 + 1, true);
        reader.read();
        parser.printBashCompletion(1, argv1, 0, reader);
        cout.rdbuf(regularCoutBuffer);
        CPPUNIT_ASSERT_EQUAL(string("COMPREPLY=()\n"), buffer.str());

        // correct operation arg flags
        buffer.str(string());
        cout.rdbuf(buffer.rdbuf());
        getArg.setDenotesOperation(true), setArg.setDenotesOperation(true);
        reader.reset(argv1, argv1 + 1).read();
        parser.printBashCompletion(1, argv1, 0, reader);
        cout.rdbuf(regularCoutBuffer);
        CPPUNIT_ASSERT_EQUAL(string("COMPREPLY=('set' )\n"), buffer.str());

        // argument at current cursor position already specified -> the completion should just return the argument
        const char *const argv2[] = {"set"};
        buffer.str(string());
        cout.rdbuf(buffer.rdbuf());
        parser.resetArgs();
        reader.reset(argv2, argv2 + 1).read();
        parser.printBashCompletion(1, argv2, 0, reader);
        cout.rdbuf(regularCoutBuffer);
        CPPUNIT_ASSERT_EQUAL(string("COMPREPLY=('set' )\n"), buffer.str());

        // advance the cursor position -> the completion should propose the next argument
        buffer.str(string());
        cout.rdbuf(buffer.rdbuf());
        parser.resetArgs();
        reader.reset(argv2, argv2 + 1).read();
        parser.printBashCompletion(1, argv2, 1, reader);
        cout.rdbuf(regularCoutBuffer);
        CPPUNIT_ASSERT_EQUAL(string("COMPREPLY=('--files' '--values' )\n"), buffer.str());

        // specifying no args should propose all main arguments
        buffer.str(string());
        cout.rdbuf(buffer.rdbuf());
        parser.resetArgs();
        reader.reset(nullptr, nullptr).read();
        parser.printBashCompletion(0, nullptr, 0, reader);
        cout.rdbuf(regularCoutBuffer);
        CPPUNIT_ASSERT_EQUAL(string("COMPREPLY=('display-file-info' 'get' 'set' '--help' )\n"), buffer.str());

        // pre-defined values
        const char *const argv3[] = {"get", "--fields"};
        buffer.str(string());
        cout.rdbuf(buffer.rdbuf());
        parser.resetArgs();
        reader.reset(argv3, argv3 + 2).read();
        parser.printBashCompletion(2, argv3, 2, reader);
        cout.rdbuf(regularCoutBuffer);
        CPPUNIT_ASSERT_EQUAL(string("COMPREPLY=('title' 'album' 'artist' 'trackpos' '--files' )\n"), buffer.str());

        // pre-defined values with equation sign, one letter already present
        const char *const argv4[] = {"set", "--values", "a"};
        buffer.str(string());
        cout.rdbuf(buffer.rdbuf());
        parser.resetArgs();
        reader.reset(argv4, argv4 + 3).read();
        parser.printBashCompletion(3, argv4, 2, reader);
        cout.rdbuf(regularCoutBuffer);
        CPPUNIT_ASSERT_EQUAL(string("COMPREPLY=('album=' 'artist='  ); compopt -o nospace\n"), buffer.str());

        // file names
        string iniFilePath = TestUtilities::testFilePath("test.ini");
        iniFilePath.resize(iniFilePath.size() - 4);
        string mkvFilePath = TestUtilities::testFilePath("test 'with quote'.mkv");
        mkvFilePath.resize(mkvFilePath.size() - 17);
        TestUtilities::testFilePath("t.aac");
        const char *const argv5[] = {"get", "--files", iniFilePath.c_str()};
        buffer.str(string());
        cout.rdbuf(buffer.rdbuf());
        parser.resetArgs();
        reader.reset(argv5, argv5 + 3).read();
        parser.printBashCompletion(3, argv5, 2, reader);
        cout.rdbuf(regularCoutBuffer);
        // order for file names is not specified
        const string res(buffer.str());
        if(res.find(".mkv") < res.find(".ini")) {
            CPPUNIT_ASSERT_EQUAL("COMPREPLY=('" % mkvFilePath % " '\"'\"'with quote'\"'\"'.mkv' '" % iniFilePath + ".ini' ); compopt -o filenames\n", buffer.str());
        } else {
            CPPUNIT_ASSERT_EQUAL("COMPREPLY=('" % iniFilePath % ".ini' '" % mkvFilePath + " '\"'\"'with quote'\"'\"'.mkv' ); compopt -o filenames\n", buffer.str());
        }

        // sub arguments
        const char *const argv6[] = {"set", "--"};
        buffer.str(string());
        cout.rdbuf(buffer.rdbuf());
        parser.resetArgs();
        reader.reset(argv6, argv6 + 2).read();
        parser.printBashCompletion(2, argv6, 1, reader);
        cout.rdbuf(regularCoutBuffer);
        CPPUNIT_ASSERT_EQUAL(string("COMPREPLY=('--files' '--values' )\n"), buffer.str());

        // nested sub arguments
        const char *const argv7[] = {"-i", "--sub", "--"};
        buffer.str(string());
        cout.rdbuf(buffer.rdbuf());
        parser.resetArgs();
        reader.reset(argv7, argv7 + 3).read();
        parser.printBashCompletion(3, argv7, 2, reader);
        cout.rdbuf(regularCoutBuffer);
        CPPUNIT_ASSERT_EQUAL(string("COMPREPLY=('--files' '--nested-sub' '--verbose' )\n"), buffer.str());

        // started pre-defined values with equation sign, one letter already present, last value matches
        const char *const argv8[] = {"set", "--values", "t"};
        buffer.str(string());
        cout.rdbuf(buffer.rdbuf());
        parser.resetArgs();
        reader.reset(argv8, argv8 + 3).read();
        parser.printBashCompletion(3, argv8, 2, reader);
        cout.rdbuf(regularCoutBuffer);
        CPPUNIT_ASSERT_EQUAL(string("COMPREPLY=('title=' 'trackpos=' ); compopt -o nospace\n"), buffer.str());

        // combined abbreviations
        const char *const argv9[] = {"-gf"};
        buffer.str(string());
        cout.rdbuf(buffer.rdbuf());
        parser.resetArgs();
        reader.reset(argv9, argv9 + 1).read();
        parser.printBashCompletion(1, argv9, 0, reader);
        cout.rdbuf(regularCoutBuffer);
        CPPUNIT_ASSERT_EQUAL(string("COMPREPLY=('-gf' )\n"), buffer.str());

        buffer.str(string());
        cout.rdbuf(buffer.rdbuf());
        parser.resetArgs();
        reader.reset(argv9, argv9 + 1).read();
        parser.printBashCompletion(1, argv9, 1, reader);
        cout.rdbuf(regularCoutBuffer);
        CPPUNIT_ASSERT_EQUAL(static_cast<string::size_type>(0), buffer.str().find("COMPREPLY=('--fields' "));

    } catch(...) {
        cout.rdbuf(regularCoutBuffer);
        throw;
    }
}
