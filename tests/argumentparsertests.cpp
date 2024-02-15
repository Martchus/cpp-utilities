#include "./outputcheck.h"
#include "./testutils.h"

#include "../conversion/stringbuilder.h"
#include "../conversion/stringconversion.h"

#include "../application/argumentparser.h"
#include "../application/argumentparserprivate.h"
#include "../application/commandlineutils.h"
#include "../application/fakeqtconfigarguments.h"

#include "../io/ansiescapecodes.h"
#include "../io/path.h"

#include "../misc/parseerror.h"

#include "resources/config.h"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <cstdlib>
#include <cstring>
#include <regex>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

using namespace std;
using namespace CppUtilities;
using namespace CppUtilities::Literals;

using namespace CPPUNIT_NS;

/*!
 * \brief The ArgumentParserTests class tests the ArgumentParser and Argument classes.
 */
class ArgumentParserTests : public TestFixture {
    CPPUNIT_TEST_SUITE(ArgumentParserTests);
    CPPUNIT_TEST(testArgument);
    CPPUNIT_TEST(testParsing);
    CPPUNIT_TEST(testCallbacks);
    CPPUNIT_TEST(testSetMainArguments);
    CPPUNIT_TEST(testValueConversion);
#ifndef PLATFORM_WINDOWS
    CPPUNIT_TEST(testBashCompletion);
    CPPUNIT_TEST(testHelp);
    CPPUNIT_TEST(testNoColorArgument);
#endif
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void testArgument();
    void testParsing();
    void testCallbacks();
    void testSetMainArguments();
    void testValueConversion();
#ifndef PLATFORM_WINDOWS
    void testBashCompletion();
    void testHelp();
    void testNoColorArgument();
#endif

private:
    void callback();
    [[noreturn]] void failOnExit(int code);
};

CPPUNIT_TEST_SUITE_REGISTRATION(ArgumentParserTests);

void ArgumentParserTests::setUp()
{
#ifndef PLATFORM_WINDOWS
    setenv("ENABLE_ESCAPE_CODES", "0", 1);
#endif
    EscapeCodes::enabled = false;
}

void ArgumentParserTests::tearDown()
{
}

[[noreturn]] void ArgumentParserTests::failOnExit(int code)
{
    CPPUNIT_FAIL(argsToString("Exited unexpectedly with code ", code));
}

/*!
 * \brief Tests the behaviour of the argument class.
 */
void ArgumentParserTests::testArgument()
{
    Argument argument("test", 't', "some description");
    CPPUNIT_ASSERT_EQUAL(false, argument.isRequired());
    argument.setConstraints(1, 10);
    CPPUNIT_ASSERT_EQUAL(true, argument.isRequired());
    Argument subArg("sub", 's', "sub arg");
    argument.addSubArgument(&subArg);
    CPPUNIT_ASSERT_EQUAL(&argument, subArg.parents().at(0));
    CPPUNIT_ASSERT(!subArg.conflictsWithArgument());
    CPPUNIT_ASSERT(!argument.firstValue());
    argument.setEnvironmentVariable("FOO_ENV_VAR");
#ifndef PLATFORM_WINDOWS // disabled under Windows for same reason as testNoColorArgument()
    setenv("FOO_ENV_VAR", "foo", 1);
    CPPUNIT_ASSERT_EQUAL("foo"s, string(argument.firstValue()));
#endif
    ArgumentOccurrence occurrence(0, vector<Argument *>(), nullptr);
    occurrence.values.emplace_back("bar");
    argument.m_occurrences.emplace_back(std::move(occurrence));
    CPPUNIT_ASSERT_EQUAL("bar"s, string(argument.firstValue()));
}

/*!
 * \brief Tests parsing command line arguments.
 */
void ArgumentParserTests::testParsing()
{
    // setup parser with some test argument definitions
    auto parser = ArgumentParser();
    parser.setExitFunction(std::bind(&ArgumentParserTests::failOnExit, this, std::placeholders::_1));
    SET_APPLICATION_INFO;
    auto qtConfigArgs = QT_CONFIG_ARGUMENTS();
    auto verboseArg = Argument("verbose", 'v', "be verbose");
    verboseArg.setCombinable(true);
    auto fileArg = Argument("file", 'f', "specifies the path of the file to be opened");
    fileArg.setValueNames({ "path" });
    fileArg.setRequiredValueCount(1);
    fileArg.setEnvironmentVariable("PATH");
    auto filesArg = Argument("files", 'f', "specifies the path of the file(s) to be opened");
    filesArg.setValueNames({ "path 1", "path 2" });
    filesArg.setRequiredValueCount(Argument::varValueCount);
    auto outputFileArg = Argument("output-file", 'o', "specifies the path of the output file");
    outputFileArg.setValueNames({ "path" });
    outputFileArg.setRequiredValueCount(1);
    outputFileArg.setRequired(true);
    outputFileArg.setCombinable(true);
    auto printFieldNamesArg = Argument("print-field-names", '\0', "prints available field names");
    auto displayFileInfoArg = Argument("display-file-info", 'i', "displays general file information");
    auto notAlbumArg = Argument("album", 'a', "should not be confused with album value");
    displayFileInfoArg.setDenotesOperation(true);
    displayFileInfoArg.setSubArguments({ &fileArg, &verboseArg, &notAlbumArg });
    auto fieldsArg = Argument("fields", '\0', "specifies the fields");
    fieldsArg.setRequiredValueCount(Argument::varValueCount);
    fieldsArg.setValueNames({ "title", "album", "artist", "trackpos" });
    fieldsArg.setImplicit(true);
    auto displayTagInfoArg = Argument("get", 'p', "displays the values of all specified tag fields (displays all fields if none specified)");
    displayTagInfoArg.setDenotesOperation(true);
    displayTagInfoArg.setSubArguments({ &fieldsArg, &filesArg, &verboseArg, &notAlbumArg });
    parser.setMainArguments(
        { &qtConfigArgs.qtWidgetsGuiArg(), &printFieldNamesArg, &displayTagInfoArg, &displayFileInfoArg, &parser.noColorArg(), &parser.helpArg() });

    // no args present
    parser.parseArgs(0, nullptr, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
    CPPUNIT_ASSERT(!parser.executable());
    CPPUNIT_ASSERT(!parser.specifiedOperation());
    CPPUNIT_ASSERT_EQUAL(0u, parser.actualArgumentCount());

    // error about uncombinable arguments
    const char *argv[] = { "tageditor", "get", "album", "title", "diskpos", "-f", "somefile" };
    // try to parse, this should fail
    try {
        parser.parseArgs(7, argv, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
        CPPUNIT_FAIL("Exception expected.");
    } catch (const ParseError &e) {
        CPPUNIT_ASSERT_EQUAL("The argument \"files\" can not be combined with \"fields\"."s, string(e.what()));
        // test printing btw
        auto ss = std::stringstream();
        ss << e;
        CPPUNIT_ASSERT_EQUAL(
            "Error: Unable to parse arguments: The argument \"files\" can not be combined with \"fields\".\nSee --help for available commands.\n"sv,
            std::string_view(ss.str()));
    }
    CPPUNIT_ASSERT(parser.isUncombinableMainArgPresent());

    // arguments read correctly after successful parse
    filesArg.setCombinable(true);
    parser.resetArgs();
    parser.parseArgs(7, argv, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
    // check results
    CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());
    CPPUNIT_ASSERT(!displayFileInfoArg.isPresent());
    CPPUNIT_ASSERT_EQUAL("tageditor"sv, std::string_view(parser.executable()));
    CPPUNIT_ASSERT(!verboseArg.isPresent());
    CPPUNIT_ASSERT(displayTagInfoArg.isPresent());
    CPPUNIT_ASSERT(fieldsArg.isPresent());
    CPPUNIT_ASSERT_EQUAL("album"sv, std::string_view(fieldsArg.values().at(0)));
    CPPUNIT_ASSERT_EQUAL("title"sv, std::string_view(fieldsArg.values().at(1)));
    CPPUNIT_ASSERT_EQUAL("diskpos"sv, std::string_view(fieldsArg.values().at(2)));
    CPPUNIT_ASSERT_EQUAL(3_st, fieldsArg.values().size());
    CPPUNIT_ASSERT_EQUAL(&displayTagInfoArg, parser.specifiedOperation());

    // skip empty args
    const char *argv2[] = { "tageditor", "", "-p", "album", "title", "diskpos", "", "--files", "somefile" };
    // reparse the args
    parser.resetArgs();
    parser.parseArgs(9, argv2, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
    // check results again
    CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());
    CPPUNIT_ASSERT(!displayFileInfoArg.isPresent());
    CPPUNIT_ASSERT(!verboseArg.isPresent());
    CPPUNIT_ASSERT(displayTagInfoArg.isPresent());
    CPPUNIT_ASSERT(fieldsArg.isPresent());
    CPPUNIT_ASSERT_EQUAL("album"sv, std::string_view(fieldsArg.values().at(0)));
    CPPUNIT_ASSERT_EQUAL("title"sv, std::string_view(fieldsArg.values().at(1)));
    CPPUNIT_ASSERT_EQUAL("diskpos"sv, std::string_view(fieldsArg.values().at(2)));
    CPPUNIT_ASSERT_EQUAL(""sv, std::string_view(fieldsArg.values().at(3)));
    CPPUNIT_ASSERT_EQUAL(4_st, fieldsArg.values().size());
    CPPUNIT_ASSERT(filesArg.isPresent());
    CPPUNIT_ASSERT_EQUAL("somefile"sv, std::string_view(filesArg.values().at(0)));

    // error about unknown argument: forget get/-p
    const char *argv3[] = { "tageditor", "album", "title", "diskpos", "--files", "somefile" };
    try {
        parser.resetArgs();
        parser.parseArgs(6, argv3, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
        CPPUNIT_FAIL("Exception expected.");
    } catch (const ParseError &e) {
        CPPUNIT_ASSERT_EQUAL("The specified argument \"album\" is unknown.\nDid you mean get or --help?"sv, std::string_view(e.what()));
    }

    // error about unknown argument: mistake in final argument
    const char *argv18[] = { "tageditor", "get", "album", "title", "diskpos", "--verbose", "--fi" };
    try {
        parser.resetArgs();
        parser.parseArgs(7, argv18, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
        CPPUNIT_FAIL("Exception expected.");
    } catch (const ParseError &e) {
        CPPUNIT_ASSERT_EQUAL("The specified argument \"--fi\" is unknown.\nDid you mean --files or --no-color?"sv, std::string_view(e.what()));
    }

    // warning about unknown argument
    parser.setUnknownArgumentBehavior(UnknownArgumentBehavior::Warn);
    {
#ifndef PLATFORM_WINDOWS
        const OutputCheck outputCheck("Warning: The specified argument \"album\" is unknown and will be ignored.\n"s
                                      "Warning: The specified argument \"title\" is unknown and will be ignored.\n"s
                                      "Warning: The specified argument \"diskpos\" is unknown and will be ignored.\n"s
                                      "Warning: The specified argument \"--files\" is unknown and will be ignored.\n"s
                                      "Warning: The specified argument \"somefile\" is unknown and will be ignored.\n"s,
            cerr);
#endif
        parser.resetArgs();
        EscapeCodes::enabled = false;
        parser.parseArgs(6, argv3, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);

        // none of the arguments should be present now
        CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());
        CPPUNIT_ASSERT(!displayFileInfoArg.isPresent());
        CPPUNIT_ASSERT(!displayTagInfoArg.isPresent());
        CPPUNIT_ASSERT(!fieldsArg.isPresent());
        CPPUNIT_ASSERT(!filesArg.isPresent());
    }

    // combined abbreviations like "-vf"
    const char *argv4[] = { "tageditor", "-i", "-vf", "test" };
    parser.setUnknownArgumentBehavior(UnknownArgumentBehavior::Fail);
    parser.resetArgs();
    parser.parseArgs(4, argv4, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
    CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());
    CPPUNIT_ASSERT(displayFileInfoArg.isPresent());
    CPPUNIT_ASSERT(verboseArg.isPresent());
    CPPUNIT_ASSERT(!displayTagInfoArg.isPresent());
    CPPUNIT_ASSERT(!filesArg.isPresent());
    CPPUNIT_ASSERT(fileArg.isPresent());
    CPPUNIT_ASSERT_EQUAL("test"sv, std::string_view(fileArg.values().at(0)));
    CPPUNIT_ASSERT_EQUAL(1_st, fileArg.values().size());

    // constraint checking: no multiple occurrences (not resetting verboseArg on purpose)
    displayFileInfoArg.reset();
    fileArg.reset();
    try {
        parser.parseArgs(4, argv4, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
        CPPUNIT_FAIL("Exception expected.");
    } catch (const ParseError &e) {
        CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());
        CPPUNIT_ASSERT_EQUAL("The argument \"verbose\" mustn't be specified more than 1 time."sv, std::string_view(e.what()));
    }

    // constraint checking: no constraint (not resetting verboseArg on purpose)
    displayFileInfoArg.reset();
    fileArg.reset();
    verboseArg.setConstraints(0, Argument::varValueCount);
    parser.parseArgs(4, argv4, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
    CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());

    // constraint checking: mandatory argument
    verboseArg.setRequired(true);
    parser.resetArgs();
    parser.parseArgs(4, argv4, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
    CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());

    // constraint checking: error about missing mandatory argument
    const char *argv5[] = { "tageditor", "-i", "-f", "test" };
    displayFileInfoArg.reset();
    fileArg.reset();
    verboseArg.reset();
    try {
        parser.parseArgs(4, argv5, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
        CPPUNIT_FAIL("Exception expected.");
    } catch (const ParseError &e) {
        CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());
        CPPUNIT_ASSERT_EQUAL("The argument \"verbose\" must be specified at least 1 time."sv, std::string_view(e.what()));
    }
    verboseArg.setRequired(false);

    // combined abbreviation with nesting "-pf"
    const char *argv10[] = { "tageditor", "-pf", "test" };
    parser.resetArgs();
    parser.parseArgs(3, argv10, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
    CPPUNIT_ASSERT(displayTagInfoArg.isPresent());
    CPPUNIT_ASSERT(!displayFileInfoArg.isPresent());
    CPPUNIT_ASSERT(!fileArg.isPresent());
    CPPUNIT_ASSERT(filesArg.isPresent());
    CPPUNIT_ASSERT_EQUAL(1_st, filesArg.values(0).size());
    CPPUNIT_ASSERT_EQUAL("test"sv, std::string_view(filesArg.values(0).at(0)));
    CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());

    // constraint checking: no complains about missing -i
    const char *argv6[] = { "tageditor", "-g" };
    parser.resetArgs();
    parser.parseArgs(2, argv6, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
    CPPUNIT_ASSERT(qtConfigArgs.qtWidgetsGuiArg().isPresent());

    // constraint checking: dependent arguments (-f requires -i or -p)
    const char *argv7[] = { "tageditor", "-f", "test" };
    parser.resetArgs();
    try {
        parser.parseArgs(3, argv7, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
        CPPUNIT_FAIL("Exception expected.");
    } catch (const ParseError &e) {
        CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());
        CPPUNIT_ASSERT_EQUAL("The specified argument \"-f\" is unknown.\nDid you mean get or --help?"sv, std::string_view(e.what()));
    }

    // equation sign syntax
    const char *argv11[] = { "tageditor", "-if=test-v" };
    parser.resetArgs();
    parser.parseArgs(2, argv11, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
    CPPUNIT_ASSERT(!filesArg.isPresent());
    CPPUNIT_ASSERT(fileArg.isPresent());
    CPPUNIT_ASSERT(!verboseArg.isPresent());
    CPPUNIT_ASSERT_EQUAL(1_st, fileArg.values(0).size());
    CPPUNIT_ASSERT_EQUAL("test-v"s, string(fileArg.values(0).front()));
    const char *argv15[] = { "tageditor", "-i", "--file=test", "-v" };
    parser.resetArgs();
    parser.parseArgs(4, argv15, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
    CPPUNIT_ASSERT(!filesArg.isPresent());
    CPPUNIT_ASSERT(fileArg.isPresent());
    CPPUNIT_ASSERT(verboseArg.isPresent());
    CPPUNIT_ASSERT_EQUAL(1_st, fileArg.values(0).size());
    CPPUNIT_ASSERT_EQUAL("test"sv, std::string_view(fileArg.values(0).front()));

    // specifying value directly after abbreviation
    const char *argv12[] = { "tageditor", "-iftest" };
    parser.resetArgs();
    parser.parseArgs(2, argv12, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
    CPPUNIT_ASSERT(!filesArg.isPresent());
    CPPUNIT_ASSERT(fileArg.isPresent());
    CPPUNIT_ASSERT_EQUAL(1_st, fileArg.values(0).size());
    CPPUNIT_ASSERT_EQUAL("test"sv, std::string_view(fileArg.values().at(0)));

    // specifying top-level argument after abbreviation
    const char *argv17[] = { "tageditor", "-if=test-v", "--no-color" };
    parser.resetArgs();
    parser.parseArgs(3, argv17, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
    CPPUNIT_ASSERT(!filesArg.isPresent());
    CPPUNIT_ASSERT(fileArg.isPresent());
    CPPUNIT_ASSERT(!verboseArg.isPresent());
    CPPUNIT_ASSERT(parser.noColorArg().isPresent());
    CPPUNIT_ASSERT_EQUAL(1_st, fileArg.values(0).size());
    CPPUNIT_ASSERT_EQUAL("test-v"sv, std::string_view(fileArg.values(0).front()));

    // default argument
    const char *argv8[] = { "tageditor" };
    parser.resetArgs();
    parser.parseArgs(1, argv8, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
    CPPUNIT_ASSERT(qtConfigArgs.qtWidgetsGuiArg().isPresent());
    CPPUNIT_ASSERT(!displayFileInfoArg.isPresent());
    CPPUNIT_ASSERT(!verboseArg.isPresent());
    CPPUNIT_ASSERT(!displayTagInfoArg.isPresent());
    CPPUNIT_ASSERT(!filesArg.isPresent());
    CPPUNIT_ASSERT(!fileArg.isPresent());
    if (const auto path = std::getenv("PATH")) {
        CPPUNIT_ASSERT(fileArg.firstValue());
        CPPUNIT_ASSERT_EQUAL(std::string_view(path), std::string_view(fileArg.firstValue()));
    } else {
        CPPUNIT_ASSERT(!fileArg.firstValue());
    }

    // constraint checking: required value count with sufficient number of provided parameters
    const char *argv13[] = { "tageditor", "get", "--fields", "album=test", "title", "diskpos", "--files", "somefile" };
    verboseArg.setRequired(false);
    parser.resetArgs();
    parser.parseArgs(8, argv13, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
    // this should still work without complaints
    CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());
    CPPUNIT_ASSERT(!displayFileInfoArg.isPresent());
    CPPUNIT_ASSERT(!verboseArg.isPresent());
    CPPUNIT_ASSERT(displayTagInfoArg.isPresent());
    CPPUNIT_ASSERT(fieldsArg.isPresent());
    CPPUNIT_ASSERT_EQUAL("album=test"sv, std::string_view(fieldsArg.values().at(0)));
    CPPUNIT_ASSERT_EQUAL("title"sv, std::string_view(fieldsArg.values().at(1)));
    CPPUNIT_ASSERT_EQUAL("diskpos"sv, std::string_view(fieldsArg.values().at(2)));
    CPPUNIT_ASSERT_EQUAL(3_st, fieldsArg.values().size());
    CPPUNIT_ASSERT(filesArg.isPresent());
    CPPUNIT_ASSERT_EQUAL("somefile"sv, std::string_view(filesArg.values().at(0)));
    CPPUNIT_ASSERT(!notAlbumArg.isPresent());

    // constraint checking: required value count with insufficient number of provided parameters
    const char *argv9[] = { "tageditor", "-p", "album", "title", "diskpos" };
    fieldsArg.setRequiredValueCount(4);
    parser.resetArgs();
    try {
        parser.parseArgs(5, argv9, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
        CPPUNIT_FAIL("Exception expected.");
    } catch (const ParseError &e) {
        CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());
        CPPUNIT_ASSERT_EQUAL(
            "Not all parameters for argument \"fields\" provided. You have to provide the following parameters: title album artist trackpos"sv,
            std::string_view(e.what()));
    }

    // constraint checking: truncated argument not wrongly detected
    const char *argv16[] = { "tageditor", "--hel", "-p", "album", "title", "diskpos" };
    fieldsArg.setRequiredValueCount(Argument::varValueCount);
    parser.resetArgs();
    try {
        parser.parseArgs(6, argv16, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
        CPPUNIT_FAIL("Exception expected.");
    } catch (const ParseError &e) {
        CPPUNIT_ASSERT(!qtConfigArgs.qtWidgetsGuiArg().isPresent());
        CPPUNIT_ASSERT_EQUAL("The specified argument \"--hel\" is unknown.\nDid you mean --help or get?"sv, std::string_view(e.what()));
    }

    // nested operations
    const char *argv14[] = { "tageditor", "get", "fields", "album=test", "-f", "somefile" };
    parser.resetArgs();
    fieldsArg.setDenotesOperation(true);
    parser.parseArgs(6, argv14, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
    CPPUNIT_ASSERT(displayTagInfoArg.isPresent());
    CPPUNIT_ASSERT(fieldsArg.isPresent());
    CPPUNIT_ASSERT_EQUAL("album=test"sv, std::string_view(fieldsArg.values().at(0)));

    // implicit flag still works when argument doesn't denote operation
    parser.resetArgs();
    fieldsArg.setDenotesOperation(false);
    parser.parseArgs(6, argv14, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
    CPPUNIT_ASSERT(displayTagInfoArg.isPresent());
    CPPUNIT_ASSERT(fieldsArg.isPresent());
    CPPUNIT_ASSERT_EQUAL("fields"sv, std::string_view(fieldsArg.values().at(0)));
    CPPUNIT_ASSERT_EQUAL("album=test"sv, std::string_view(fieldsArg.values().at(1)));

    // greedy-flag
    const char *argv19[] = { "tageditor", "get", "--fields", "foo", "bar", "--help" };
    parser.resetArgs();
    try {
        parser.parseArgs(6, argv19, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
        CPPUNIT_FAIL("Exception expected.");
    } catch (const ParseError &e) {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("--help assumed to be an argument without greedy-flag (leading to error)",
            "The argument \"help\" can not be combined with \"get\"."sv, std::string_view(e.what()));
    }
    parser.resetArgs();
    fieldsArg.setFlags(Argument::Flags::Greedy, true);
    parser.parseArgs(6, argv19, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
    CPPUNIT_ASSERT(displayTagInfoArg.isPresent());
    CPPUNIT_ASSERT(fieldsArg.isPresent());
    CPPUNIT_ASSERT_MESSAGE("--help not considered an argument with greedy-flag", !parser.helpArg().isPresent());
    CPPUNIT_ASSERT_EQUAL("foo"sv, std::string_view(fieldsArg.values().at(0)));
    CPPUNIT_ASSERT_EQUAL("bar"sv, std::string_view(fieldsArg.values().at(1)));
    CPPUNIT_ASSERT_EQUAL("--help"sv, std::string_view(fieldsArg.values().at(2)));
    CPPUNIT_ASSERT_EQUAL(3_st, fieldsArg.values().size());
}

/*!
 * \brief Tests whether callbacks are called correctly.
 */
void ArgumentParserTests::testCallbacks()
{
    ArgumentParser parser;
    parser.setExitFunction(std::bind(&ArgumentParserTests::failOnExit, this, std::placeholders::_1));
    Argument callbackArg("with-callback", 't', "callback test");
    callbackArg.setRequiredValueCount(2);
    callbackArg.setCallback([](const ArgumentOccurrence &occurrence) {
        CPPUNIT_ASSERT_EQUAL(0_st, occurrence.index);
        CPPUNIT_ASSERT(occurrence.path.empty());
        CPPUNIT_ASSERT_EQUAL(2_st, occurrence.values.size());
        CPPUNIT_ASSERT(!strcmp(occurrence.values[0], "val1"));
        CPPUNIT_ASSERT(!strcmp(occurrence.values[1], "val2"));
        throw 42;
    });
    Argument noCallbackArg("no-callback", 'l', "callback test");
    noCallbackArg.setRequiredValueCount(2);
    parser.setMainArguments({ &callbackArg, &noCallbackArg });

    // test whether callback is invoked when argument with callback is specified
    const char *argv[] = { "test", "-t", "val1", "val2" };
    try {
        parser.parseArgs(4, argv, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
    } catch (int i) {
        CPPUNIT_ASSERT_EQUAL(i, 42);
    }

    // test whether callback is not invoked when argument with callback is not specified
    callbackArg.reset();
    const char *argv2[] = { "test", "-l", "val1", "val2" };
    parser.parseArgs(4, argv2, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
}

#ifndef PLATFORM_WINDOWS
/*!
 * \brief Used to check whether the exit() function is called when printing bash completion.
 */
static bool exitCalled = false;

/*!
 * \brief Tests bash completion.
 * \remarks
 * - Disabled under Windows because OutputCheck isn't working.
 * - This tests makes assumptions about the order and the exact output format.
 */
void ArgumentParserTests::testBashCompletion()
{
    ArgumentParser parser;
    parser.setExitFunction(std::bind(&ArgumentParserTests::failOnExit, this, std::placeholders::_1));
    Argument verboseArg("verbose", 'v', "be verbose");
    verboseArg.setCombinable(true);
    Argument filesArg("files", 'f', "specifies the path of the file(s) to be opened");
    filesArg.setRequiredValueCount(Argument::varValueCount);
    filesArg.setCombinable(true);
    Argument nestedSubArg("nested-sub", '\0', "nested sub arg");
    Argument subArg("sub", '\0', "sub arg");
    subArg.setSubArguments({ &nestedSubArg });
    Argument displayFileInfoArg("display-file-info", 'i', "displays general file information");
    displayFileInfoArg.setDenotesOperation(true);
    displayFileInfoArg.setSubArguments({ &filesArg, &verboseArg, &subArg });
    Argument fieldsArg("fields", '\0', "specifies the fields");
    fieldsArg.setRequiredValueCount(Argument::varValueCount);
    fieldsArg.setPreDefinedCompletionValues("title album artist trackpos");
    fieldsArg.setImplicit(true);
    Argument valuesArg("values", '\0', "specifies the fields");
    valuesArg.setRequiredValueCount(Argument::varValueCount);
    valuesArg.setPreDefinedCompletionValues("title album artist trackpos");
    valuesArg.setImplicit(false);
    valuesArg.setValueCompletionBehavior(ValueCompletionBehavior::PreDefinedValues | ValueCompletionBehavior::AppendEquationSign);
    Argument selectorsArg("selectors", '\0', "has some more pre-defined values");
    selectorsArg.setRequiredValueCount(Argument::varValueCount);
    selectorsArg.setPreDefinedCompletionValues("tag=id3v1 tag=id3v2 tag=matroska target=file target=track");
    selectorsArg.setImplicit(false);
    selectorsArg.setValueCompletionBehavior(ValueCompletionBehavior::PreDefinedValues);
    selectorsArg.setCallback(
        [&selectorsArg](const ArgumentOccurrence &) { selectorsArg.setPreDefinedCompletionValues("tag=matroska tag=mp4 tag=vorbis"); });
    Argument getArg("get", 'g', "gets tag values");
    getArg.setSubArguments({ &fieldsArg, &filesArg });
    Argument setArg("set", 's', "sets tag values");
    setArg.setSubArguments({ &valuesArg, &filesArg, &selectorsArg });

    parser.setMainArguments({ &displayFileInfoArg, &getArg, &setArg, &parser.noColorArg(), &parser.helpArg() });

    // fail due to operation flags not set
    const char *const argv1[] = { "se" };
    ArgumentReader reader(parser, argv1, argv1 + 1, true);
    {
        const OutputCheck c("COMPREPLY=()\n");
        CPPUNIT_ASSERT(reader.read());
        parser.printBashCompletion(1, argv1, 0, reader);
    }

    // correct operation arg flags
    getArg.setDenotesOperation(true);
    setArg.setDenotesOperation(true);
    {
        const OutputCheck c("COMPREPLY=('set' )\n");
        reader.reset(argv1, argv1 + 1).read();
        parser.printBashCompletion(1, argv1, 0, reader);
    }

    // argument at current cursor position already specified -> the completion should just return the argument
    const char *const argv2[] = { "set" };
    parser.resetArgs();
    {
        const OutputCheck c("COMPREPLY=('set' )\n");
        reader.reset(argv2, argv2 + 1).read();
        parser.printBashCompletion(1, argv2, 0, reader);
    }

    // advance the cursor position -> the completion should propose the next argument
    parser.resetArgs();
    {
        const OutputCheck c("COMPREPLY=('--files' '--no-color' '--selectors' '--values' )\n");
        reader.reset(argv2, argv2 + 1).read();
        parser.printBashCompletion(1, argv2, 1, reader);
    }

    // nested operations should be proposed as operations
    parser.resetArgs();
    filesArg.setDenotesOperation(true);
    {
        const OutputCheck c("COMPREPLY=('files' '--no-color' '--selectors' '--values' )\n");
        reader.reset(argv2, argv2 + 1).read();
        parser.printBashCompletion(1, argv2, 1, reader);
    }

    // specifying no args should propose all main arguments
    parser.resetArgs();
    filesArg.setDenotesOperation(false);
    {
        const OutputCheck c("COMPREPLY=('display-file-info' 'get' 'set' '--help' '--no-color' )\n");
        reader.reset(nullptr, nullptr).read();
        parser.printBashCompletion(0, nullptr, 0, reader);
    }

    // pre-defined values
    const char *const argv3[] = { "get", "--fields" };
    parser.resetArgs();
    {
        const OutputCheck c("COMPREPLY=('title' 'album' 'artist' 'trackpos' '--files' '--no-color' )\n");
        reader.reset(argv3, argv3 + 2).read();
        parser.printBashCompletion(2, argv3, 2, reader);
    }

    // pre-defined values with equation sign, one letter already present
    const char *const argv4[] = { "set", "--values", "a" };
    parser.resetArgs();
    {
        const OutputCheck c("COMPREPLY=('album=' 'artist='  ); compopt -o nospace\n");
        reader.reset(argv4, argv4 + 3).read();
        parser.printBashCompletion(3, argv4, 2, reader);
    }

    // pre-defined values containing equation sign, equation sign already present
    const char *const argv12[] = { "set", "--selectors", "tag=id3" };
    parser.resetArgs();
    {
        const OutputCheck c("COMPREPLY=('tag=id3v1' 'tag=id3v2'  )\n");
        reader.reset(argv12, argv12 + 3).read();
        parser.printBashCompletion(3, argv12, 2, reader);
    }

    // recombining pre-defined values containing equation sign, equation sign already present
    const char *const argv13[] = { "set", "--selectors", "tag", "=", "id3" };
    parser.resetArgs();
    {
        const OutputCheck c("COMPREPLY=('id3v1' 'id3v2'  )\n");
        reader.reset(argv13, argv13 + 5).read();
        parser.printBashCompletion(5, argv13, 4, reader);
    }
    parser.resetArgs();
    {
        const OutputCheck c("COMPREPLY=('id3v1' 'id3v2' 'matroska'  )\n");
        reader.reset(argv13, argv13 + 5).read();
        parser.printBashCompletion(5, argv13, 3, reader);
    }

    // computing pre-defined values just in time using callback
    selectorsArg.setValueCompletionBehavior(selectorsArg.valueCompletionBehaviour() | ValueCompletionBehavior::InvokeCallback);
    parser.resetArgs();
    {
        const OutputCheck c("COMPREPLY=('matroska' 'mp4' 'vorbis' )\n");
        reader.reset(argv13, argv13 + 5).read();
        parser.printBashCompletion(5, argv13, 3, reader);
    }

    // pre-defined values for implicit argument
    parser.resetArgs();
    {
        const OutputCheck c("COMPREPLY=('title' 'album' 'artist' 'trackpos' '--fields' '--files' '--no-color' )\n");
        reader.reset(argv3, argv3 + 1).read();
        parser.printBashCompletion(1, argv3, 2, reader);
    }

    // file names
    string iniFilePath = CppUtilities::testFilePath("test.ini");
    iniFilePath.resize(iniFilePath.size() - 4);
    string mkvFilePath = CppUtilities::testFilePath("test 'with quote'.mkv");
    mkvFilePath.resize(mkvFilePath.size() - 17);
    parser.resetArgs();
    const char *const argv5[] = { "get", "--files", iniFilePath.c_str() };
    {
#ifdef CPP_UTILITIES_USE_STANDARD_FILESYSTEM
        // order for file names is not specified
        const OutputCheck c("COMPREPLY=('" % mkvFilePath % " '\"'\"'with quote'\"'\"'.mkv' '" % iniFilePath + ".ini' ); compopt -o filenames\n",
            "COMPREPLY=('" % iniFilePath % ".ini' '" % mkvFilePath + " '\"'\"'with quote'\"'\"'.mkv' ); compopt -o filenames\n");
#else
        const OutputCheck c("COMPREPLY=()\n");
#endif
        reader.reset(argv5, argv5 + 3).read();
        parser.printBashCompletion(3, argv5, 2, reader);
    }

    // directory names
    string directoryPath = CppUtilities::testFilePath("subdir/foo/bar");
    directoryPath.resize(directoryPath.size() - 4);
    filesArg.setValueCompletionBehavior(ValueCompletionBehavior::Directories);
    parser.resetArgs();
    const char *const argv14[] = { "get", "--files", directoryPath.c_str() };
    {
#ifdef CPP_UTILITIES_USE_STANDARD_FILESYSTEM
        const OutputCheck c("COMPREPLY=('" % directoryPath + "' ); compopt -o filenames\n");
#else
        const OutputCheck c("COMPREPLY=()\n");
#endif
        reader.reset(argv14, argv14 + 3).read();
        parser.printBashCompletion(3, argv14, 2, reader);
    }

    // sub arguments
    const char *const argv6[] = { "set", "--" };
    parser.resetArgs();
    {
        const OutputCheck c("COMPREPLY=('--files' '--no-color' '--selectors' '--values' )\n");
        reader.reset(argv6, argv6 + 2).read();
        parser.printBashCompletion(2, argv6, 1, reader);
    }

    // nested sub arguments
    const char *const argv7[] = { "-i", "--sub", "--" };
    parser.resetArgs();
    {
        const OutputCheck c("COMPREPLY=('--files' '--nested-sub' '--no-color' '--verbose' )\n");
        reader.reset(argv7, argv7 + 3).read();
        parser.printBashCompletion(3, argv7, 2, reader);
    }

    // started pre-defined values with equation sign, one letter already present, last value matches
    const char *const argv8[] = { "set", "--values", "t" };
    parser.resetArgs();
    {
        const OutputCheck c("COMPREPLY=('title=' 'trackpos=' ); compopt -o nospace\n");
        reader.reset(argv8, argv8 + 3).read();
        parser.printBashCompletion(3, argv8, 2, reader);
    }

    // combined abbreviations
    const char *const argv9[] = { "-gf" };
    parser.resetArgs();
    {
        const OutputCheck c("COMPREPLY=('-gf' )\n");
        reader.reset(argv9, argv9 + 1).read();
        parser.printBashCompletion(1, argv9, 0, reader);
    }
    parser.resetArgs();
    {
        const OutputCheck c([](const string &actualOutput) { CPPUNIT_ASSERT_EQUAL(0_st, actualOutput.find("COMPREPLY=('--fields' ")); });
        reader.reset(argv9, argv9 + 1).read();
        parser.printBashCompletion(1, argv9, 1, reader);
    }

    // override exit function to prevent readArgs() from terminating the test run
    parser.setExitFunction([](int) { exitCalled = true; });

    // call completion via readArgs() with current word index
    const char *const argv10[] = { "/some/path/tageditor", "--bash-completion-for", "0" };
    parser.resetArgs();
    {
        const OutputCheck c("COMPREPLY=('display-file-info' 'get' 'set' '--help' '--no-color' )\n");
        parser.readArgs(3, argv10);
    }
    CPPUNIT_ASSERT(!strcmp("/some/path/tageditor", parser.executable()));
    CPPUNIT_ASSERT(exitCalled);

    // call completion via readArgs() without current word index
    const char *const argv11[] = { "/some/path/tageditor", "--bash-completion-for", "ge" };
    parser.resetArgs();
    {
        const OutputCheck c("COMPREPLY=('get' )\n");
        parser.readArgs(3, argv11);
    }
}
#endif

#ifndef PLATFORM_WINDOWS
/*!
 * \brief Tests --help output.
 * \remarks Disabled under Windows because OutputCheck isn't working.
 */
void ArgumentParserTests::testHelp()
{
    // indentation
    Indentation indent;
    indent = indent + 3;
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(4 + 3), indent.level);

    // setup parser
    ArgumentParser parser;
    parser.setExitFunction(std::bind(&ArgumentParserTests::failOnExit, this, std::placeholders::_1));
    OperationArgument verboseArg("verbose", 'v', "be verbose", "actually not an operation");
    verboseArg.setCombinable(true);
    ConfigValueArgument nestedSubArg("nested-sub", '\0', "nested sub arg", { "value1", "value2" });
    nestedSubArg.setRequiredValueCount(Argument::varValueCount);
    Argument subArg("foo", 'f', "dummy");
    subArg.setName("sub");
    subArg.setAbbreviation('\0');
    subArg.setDescription("sub arg");
    subArg.setExample("sub arg example");
    subArg.setRequired(true);
    subArg.addSubArgument(&nestedSubArg);
    Argument filesArg("files", 'f', "specifies the path of the file(s) to be opened");
    filesArg.setCombinable(true);
    filesArg.addSubArgument(&subArg);
    filesArg.setSubArguments({ &subArg }); // test re-assignment btw
    Argument envArg("env", '\0', "env");
    envArg.setEnvironmentVariable("FILES");
    envArg.setRequiredValueCount(2);
    envArg.appendValueName("file");
    Argument deprecatedArg("deprecated");
    deprecatedArg.markAsDeprecated(&filesArg);
    parser.helpArg().setRequired(true);
    parser.setMainArguments({ &verboseArg, &filesArg, &envArg, &deprecatedArg, &parser.noColorArg(), &parser.helpArg() });
    applicationInfo.dependencyVersions = { "somelib", "some other lib" };

    // parse args and assert output
    const char *const argv[] = { "app", "-h" };
    {
        const OutputCheck c("\033[1m" APP_NAME ", version " APP_VERSION "\n"
                            "\n"
                            "\033[0m" APP_DESCRIPTION "\n"
                            "\n"
                            "Available operations:\n"
                            "\033[1mverbose, -v\033[0m\n"
                            "  be verbose\n"
                            "  example: actually not an operation\n"
                            "\n"
                            "Available top-level options:\n"
                            "\033[1m--files, -f\033[0m\n"
                            "  specifies the path of the file(s) to be opened\n"
                            "  \033[1m--sub\033[0m\n"
                            "    sub arg\n"
                            "    particularities: mandatory if parent argument is present\n"
                            "    \033[1m--nested-sub\033[0m [value1] [value2] ...\n"
                            "      nested sub arg\n"
                            "    example: sub arg example\n"
                            "\n"
                            "\033[1m--env\033[0m [file] [value 2]\n"
                            "  env\n"
                            "  default environment variable: FILES\n"
                            "\n"
                            "\033[1m--no-color\033[0m\n"
                            "  disables formatted/colorized output\n"
                            "  default environment variable: ENABLE_ESCAPE_CODES\n"
                            "\n"
                            "Linked against: somelib, some other lib\n"
                            "\n"
                            "Project website: " APP_URL "\n");
        EscapeCodes::enabled = true;
        parser.parseArgs(2, argv, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
    }

    verboseArg.setDenotesOperation(false);
    parser.setMainArguments({ &verboseArg, &filesArg, &envArg, &parser.helpArg() });
    {
        const OutputCheck c(APP_NAME ", version " APP_VERSION "\n"
                                     "\n" APP_DESCRIPTION "\n"
                                     "\n"
                                     "Available arguments:\n"
                                     "--verbose, -v\n"
                                     "  be verbose\n"
                                     "  example: actually not an operation\n"
                                     "\n"
                                     "--files, -f\n"
                                     "  specifies the path of the file(s) to be opened\n"
                                     "  --sub\n"
                                     "    sub arg\n"
                                     "    particularities: mandatory if parent argument is present\n"
                                     "    --nested-sub [value1] [value2] ...\n"
                                     "      nested sub arg\n"
                                     "    example: sub arg example\n"
                                     "\n"
                                     "--env [file] [value 2]\n"
                                     "  env\n"
                                     "  default environment variable: FILES\n"
                                     "\n"
                                     "Linked against: somelib, some other lib\n"
                                     "\n"
                                     "Project website: " APP_URL "\n");
        EscapeCodes::enabled = false;
        parser.resetArgs();
        parser.parseArgs(2, argv, ParseArgumentBehavior::CheckConstraints | ParseArgumentBehavior::InvokeCallbacks);
    }
}
#endif

/*!
 * \brief Tests some corner cases in setMainArguments() which are not already checked in the other tests.
 */
void ArgumentParserTests::testSetMainArguments()
{
    ArgumentParser parser;
    parser.setExitFunction(std::bind(&ArgumentParserTests::failOnExit, this, std::placeholders::_1));
    HelpArgument &helpArg(parser.helpArg());
    Argument subArg("sub-arg", 's', "mandatory sub arg");
    subArg.setRequired(true);
    helpArg.addSubArgument(&subArg);
    parser.addMainArgument(&helpArg);
    parser.setMainArguments({});
    CPPUNIT_ASSERT_MESSAGE("clear main args", parser.mainArguments().empty());
    parser.setMainArguments({ &helpArg });
    CPPUNIT_ASSERT_MESSAGE("no default due to required sub arg", !parser.defaultArgument());
    subArg.setConstraints(0, 20);
    parser.setMainArguments({ &helpArg });
    CPPUNIT_ASSERT_MESSAGE("default if no required sub arg", &helpArg == parser.defaultArgument());
}

#ifndef PLATFORM_WINDOWS
/*!
 * \brief Tests whether NocolorArgument toggles escape codes correctly.
 * \remarks
 * Disabled under Windows. Under that platform we could alter the environment using
 * SetEnvironmentVariableW. However, that doesn't seem to have an effect on further
 * getenv() or _wgetenv() calls.
 */
void ArgumentParserTests::testNoColorArgument()
{
    // assume escape codes are enabled by default
    EscapeCodes::enabled = true;

    {
        unsetenv("ENABLE_ESCAPE_CODES");
        NoColorArgument noColorArg;
        noColorArg.apply();
        CPPUNIT_ASSERT_MESSAGE("default used if not present", EscapeCodes::enabled);
        noColorArg.m_occurrences.emplace_back(0);
        noColorArg.apply();
        CPPUNIT_ASSERT_MESSAGE("default negated if present", !EscapeCodes::enabled);
    }
    {
        setenv("ENABLE_ESCAPE_CODES", "  0 ", 1);
        const NoColorArgument noColorArg;
        CPPUNIT_ASSERT(!EscapeCodes::enabled);
    }
    {
        setenv("ENABLE_ESCAPE_CODES", "  1 ", 1);
        const NoColorArgument noColorArg;
        CPPUNIT_ASSERT(EscapeCodes::enabled);
    }
}
#endif

template <typename ValueTuple> void checkConvertedValues(const std::string &message, const ValueTuple &values)
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, "foo"s, get<0>(values));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, 42u, get<1>(values));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, 7.5, get<2>(values));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, -42, get<3>(values));
}

/*!
 * \brief Tests value conversion provided by Argument and ArgumentOccurrence.
 */
void ArgumentParserTests::testValueConversion()
{
    // convert values directly from ArgumentOccurrence
    ArgumentOccurrence occurrence(0);
    occurrence.values = { "foo", "42", "7.5", "-42" };
    checkConvertedValues("values from ArgumentOccurrence::convertValues", occurrence.convertValues<string, unsigned int, double, int>());
    static_assert(std::is_same<std::tuple<>, decltype(occurrence.convertValues<>())>::value, "specifying no types yields empty tuple");

    // convert values via Argument's API
    Argument arg("test", '\0');
    arg.m_occurrences = { occurrence, occurrence };
    checkConvertedValues("values from Argument::convertValues", arg.valuesAs<string, unsigned int, double, int>());
    checkConvertedValues("values from Argument::convertValues(1)", arg.valuesAs<string, unsigned int, double, int>(1));
    const auto allValues = arg.allValuesAs<string, unsigned int, double, int>();
    CPPUNIT_ASSERT_EQUAL(2_st, allValues.size());
    for (const auto &values : allValues) {
        checkConvertedValues("values from Argument::convertAllValues", values);
    }
    static_assert(std::is_same<std::tuple<>, decltype(arg.valuesAs<>())>::value, "specifying no types yields empty tuple");

    // error handling
    try {
        occurrence.convertValues<string, unsigned int, double, int, int>();
        CPPUNIT_FAIL("Expected exception");
    } catch (const ParseError &failure) {
        CPPUNIT_ASSERT_EQUAL("Expected 5 top-level values to be present but only 4 have been specified."s, string(failure.what()));
    }
    try {
        occurrence.convertValues<int>();
        CPPUNIT_FAIL("Expected exception");
    } catch (const ParseError &failure) {
        TESTUTILS_ASSERT_LIKE_FLAGS("conversion error of top-level value",
            "Conversion of top-level value \"foo\" to type \".*\" failed: The character \"f\" is no valid digit."s, std::regex::extended,
            std::string(failure.what()));
    }
    occurrence.path = { &arg };
    try {
        occurrence.convertValues<string, unsigned int, double, int, int>();
        CPPUNIT_FAIL("Expected exception");
    } catch (const ParseError &failure) {
        CPPUNIT_ASSERT_EQUAL("Expected 5 values for argument --test to be present but only 4 have been specified."s, string(failure.what()));
    }
    try {
        occurrence.convertValues<int>();
        CPPUNIT_FAIL("Expected exception");
    } catch (const ParseError &failure) {
        TESTUTILS_ASSERT_LIKE_FLAGS("conversion error of argument value",
            "Conversion of value \"foo\" \\(for argument --test\\) to type \".*\" failed: The character \"f\" is no valid digit."s,
            std::regex::extended, std::string(failure.what()));
    }
}
