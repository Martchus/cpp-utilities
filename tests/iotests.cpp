#include "./testutils.h"

#include "../conversion/conversionexception.h"
#include "../conversion/stringbuilder.h"

#include "../io/ansiescapecodes.h"
#include "../io/binaryreader.h"
#include "../io/binarywriter.h"
#include "../io/bitreader.h"
#include "../io/copy.h"
#include "../io/inifile.h"
#include "../io/misc.h"
#include "../io/nativefilestream.h"
#include "../io/path.h"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <algorithm>
#include <fstream>
#include <regex>
#include <sstream>

#ifdef PLATFORM_WINDOWS
#include <cstdio>
#endif

#ifdef PLATFORM_UNIX
#include <sys/fcntl.h>
#include <sys/types.h>
#endif

using namespace std;
using namespace CppUtilities;
using namespace CppUtilities::Literals;
using namespace CPPUNIT_NS;

/*!
 * \brief The IoTests class tests classes and functions provided by the files inside the io directory.
 */
class IoTests : public TestFixture {
    CPPUNIT_TEST_SUITE(IoTests);
    CPPUNIT_TEST(testBinaryReader);
    CPPUNIT_TEST(testBinaryWriter);
    CPPUNIT_TEST(testBitReader);
    CPPUNIT_TEST(testPathUtilities);
    CPPUNIT_TEST(testIniFile);
    CPPUNIT_TEST(testAdvancedIniFile);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST(testReadFile);
    CPPUNIT_TEST(testWriteFile);
    CPPUNIT_TEST(testAnsiEscapeCodes);
#ifdef CPP_UTILITIES_USE_NATIVE_FILE_BUFFER
    CPPUNIT_TEST(testNativeFileStream);
#endif
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void testBinaryReader();
    void testBinaryWriter();
    void testBitReader();
    void testPathUtilities();
    void testIniFile();
    void testAdvancedIniFile();
    void testCopy();
    void testReadFile();
    void testWriteFile();
    void testAnsiEscapeCodes();
#ifdef CPP_UTILITIES_USE_NATIVE_FILE_BUFFER
    void testNativeFileStream();
#endif
};

CPPUNIT_TEST_SUITE_REGISTRATION(IoTests);

void IoTests::setUp()
{
}

void IoTests::tearDown()
{
}

/*!
 * \brief Tests the most important methods of the BinaryReader.
 */
void IoTests::testBinaryReader()
{
    // read test file
    fstream testFile;
    testFile.exceptions(ios_base::failbit | ios_base::badbit);
    testFile.open(testFilePath("some_data"), ios_base::in | ios_base::binary);
    BinaryReader reader(&testFile);
    CPPUNIT_ASSERT_EQUAL(static_cast<istream::pos_type>(398), reader.readStreamsize());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0102u), reader.readUInt16LE());
    CPPUNIT_ASSERT_EQUAL(static_cast<istream::pos_type>(396), reader.readRemainingBytes());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0102u), reader.readUInt16BE());
    CPPUNIT_ASSERT_EQUAL(0x010203u, reader.readUInt24LE());
    CPPUNIT_ASSERT_EQUAL(0x010203u, reader.readUInt24BE());
    CPPUNIT_ASSERT_EQUAL(0x01020304u, reader.readUInt32LE());
    CPPUNIT_ASSERT_EQUAL(0x01020304u, reader.readUInt32BE());
    CPPUNIT_ASSERT_EQUAL(0x0102030405u, reader.readUInt40LE());
    CPPUNIT_ASSERT_EQUAL(0x0102030405u, reader.readUInt40BE());
    CPPUNIT_ASSERT_EQUAL(0x01020304050607u, reader.readUInt56LE());
    CPPUNIT_ASSERT_EQUAL(0x01020304050607u, reader.readUInt56BE());
    CPPUNIT_ASSERT_EQUAL(0x0102030405060708u, reader.readUInt64LE());
    CPPUNIT_ASSERT_EQUAL(0x0102030405060708u, reader.readUInt64BE());
    testFile.seekg(0);
    CPPUNIT_ASSERT_EQUAL(reader.readInt16LE(), static_cast<std::int16_t>(0x0102));
    CPPUNIT_ASSERT_EQUAL(reader.readInt16BE(), static_cast<std::int16_t>(0x0102));
    CPPUNIT_ASSERT_EQUAL(0x010203, reader.readInt24LE());
    CPPUNIT_ASSERT_EQUAL(0x010203, reader.readInt24BE());
    CPPUNIT_ASSERT_EQUAL(0x01020304, reader.readInt32LE());
    CPPUNIT_ASSERT_EQUAL(0x01020304, reader.readInt32BE());
    CPPUNIT_ASSERT_EQUAL(0x0102030405, reader.readInt40LE());
    CPPUNIT_ASSERT_EQUAL(0x0102030405, reader.readInt40BE());
    CPPUNIT_ASSERT_EQUAL(0x01020304050607, reader.readInt56LE());
    CPPUNIT_ASSERT_EQUAL(0x01020304050607, reader.readInt56BE());
    CPPUNIT_ASSERT_EQUAL(0x0102030405060708, reader.readInt64LE());
    CPPUNIT_ASSERT_EQUAL(0x0102030405060708, reader.readInt64BE());
    CPPUNIT_ASSERT_EQUAL(1.125f, reader.readFloat32LE());
    CPPUNIT_ASSERT_EQUAL(1.625, reader.readFloat64LE());
    CPPUNIT_ASSERT_EQUAL(1.125f, reader.readFloat32BE());
    CPPUNIT_ASSERT_EQUAL(reader.readFloat64BE(), 1.625);
    CPPUNIT_ASSERT_EQUAL(false, reader.readBool());
    CPPUNIT_ASSERT_EQUAL(true, reader.readBool());
    CPPUNIT_ASSERT_EQUAL("abc"s, reader.readString(3));
    CPPUNIT_ASSERT_EQUAL(reader.readLengthPrefixedString(), "ABC"s);
    CPPUNIT_ASSERT_EQUAL("01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901"
                         "23456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123"
                         "45678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345"
                         "678901234567890123456789"s,
        reader.readLengthPrefixedString());
    CPPUNIT_ASSERT_EQUAL("def"s, reader.readTerminatedString());
    testFile.seekg(-4, ios_base::cur);
    CPPUNIT_ASSERT_EQUAL("def"s, reader.readTerminatedString(5, 0));
    CPPUNIT_ASSERT_THROW(reader.readLengthPrefixedString(), ConversionException);
    CPPUNIT_ASSERT_MESSAGE("pos in stream not advanced on conversion error", reader.readByte() == 0);

    // test ownership
    reader.setStream(nullptr, true);
    reader.setStream(new fstream(), true);
    BinaryReader reader2(reader);
    CPPUNIT_ASSERT(reader2.stream() == reader.stream());
    CPPUNIT_ASSERT(!reader2.hasOwnership());
    reader.setStream(&testFile, false);
    reader.setStream(new fstream(), true);
}

/*!
 * \brief Tests the most important methods of the BinaryWriter.
 */
void IoTests::testBinaryWriter()
{
    // prepare reading expected data
    fstream testFile;
    testFile.exceptions(ios_base::failbit | ios_base::badbit);
    testFile.open(testFilePath("some_data"), ios_base::in | ios_base::binary);

    // prepare output stream
    stringstream outputStream(ios_base::in | ios_base::out | ios_base::binary);
    outputStream.exceptions(ios_base::failbit | ios_base::badbit);
    char testData[397];
    outputStream.rdbuf()->pubsetbuf(testData, sizeof(testData));

    // write test data
    BinaryWriter writer(&outputStream);
    writer.writeUInt16LE(0x0102u);
    writer.writeUInt16BE(0x0102u);
    writer.writeUInt24LE(0x010203u);
    writer.writeUInt24BE(0x010203u);
    writer.writeUInt32LE(0x01020304u);
    writer.writeUInt32BE(0x01020304u);
    writer.writeUInt40LE(0x0102030405u);
    writer.writeUInt40BE(0x0102030405u);
    writer.writeUInt56LE(0x01020304050607u);
    writer.writeUInt56BE(0x01020304050607u);
    writer.writeUInt64LE(0x0102030405060708u);
    writer.writeUInt64BE(0x0102030405060708u);

    // test written values
    for (char c : testData) {
        CPPUNIT_ASSERT(c == static_cast<char>(testFile.get()));
        if (testFile.tellg() >= 58) {
            break;
        }
    }
    testFile.seekg(0);
    outputStream.seekp(0);

    // write more test data
    writer.writeInt16LE(0x0102);
    writer.writeInt16BE(0x0102);
    writer.writeInt24LE(0x010203);
    writer.writeInt24BE(0x010203);
    writer.writeInt32LE(0x01020304);
    writer.writeInt32BE(0x01020304);
    writer.writeInt40LE(0x0102030405);
    writer.writeInt40BE(0x0102030405);
    writer.writeInt56LE(0x01020304050607);
    writer.writeInt56BE(0x01020304050607);
    writer.writeInt64LE(0x0102030405060708);
    writer.writeInt64BE(0x0102030405060708);
    writer.writeFloat32LE(1.125);
    writer.writeFloat64LE(1.625);
    writer.writeFloat32BE(1.125);
    writer.writeFloat64BE(1.625);
    writer.writeBool(false);
    writer.writeBool(true);
    writer.writeString("abc");
    writer.writeLengthPrefixedString("ABC");
    writer.writeLengthPrefixedString("012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                                     "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901"
                                     "234567890123456789012345678901234567890123456789012345678901234567890123456789");
    writer.writeTerminatedString("def");

    // test written values
    for (char c : testData) {
        CPPUNIT_ASSERT(c == static_cast<char>(testFile.get()));
    }

    // test ownership
    writer.setStream(nullptr, true);
    writer.setStream(new fstream(), true);
    BinaryWriter writer2(writer);
    CPPUNIT_ASSERT(writer2.stream() == writer.stream());
    CPPUNIT_ASSERT(!writer2.hasOwnership());
    writer.setStream(&testFile, false);
    writer.setStream(new fstream(), true);
}

/*!
 * \brief Tests the BitReader.
 */
void IoTests::testBitReader()
{
    const std::uint8_t testData[] = { 0x81, 0x90, 0x3C, 0x44, 0x28, 0x00, 0x44, 0x10, 0x20, 0xFF, 0xFA };
    BitReader reader(reinterpret_cast<const char *>(testData), sizeof(testData));
    CPPUNIT_ASSERT(reader.readBit() == 1);
    reader.skipBits(6);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(3), reader.showBits<std::uint8_t>(2));
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(3), reader.readBits<std::uint8_t>(2));
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0x103C4428 << 1), reader.readBits<std::uint32_t>(32));
    reader.align();
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0x44), reader.readBits<std::uint8_t>(8));
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(7), reader.readUnsignedExpGolombCodedBits<std::uint8_t>());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::int8_t>(4), reader.readSignedExpGolombCodedBits<std::int8_t>());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), reader.readBit());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0), reader.readBit());
    reader.skipBits(8 + 4);
    CPPUNIT_ASSERT_EQUAL(4_st, reader.bitsAvailable());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::uint8_t>(0xA), reader.readBits<std::uint8_t>(4));
    CPPUNIT_ASSERT_THROW(reader.readBit(), std::ios_base::failure);
    CPPUNIT_ASSERT_THROW(reader.skipBits(1), std::ios_base::failure);
    reader.reset(reinterpret_cast<const char *>(testData), sizeof(testData));
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(8 * sizeof(testData)), reader.bitsAvailable());
}

/*!
 * \brief Tests fileName() and removeInvalidChars().
 */
void IoTests::testPathUtilities()
{
    CPPUNIT_ASSERT_EQUAL("libc++utilities.so"s, fileName("C:\\libs\\libc++utilities.so"));
    CPPUNIT_ASSERT_EQUAL("libc++utilities.so"s, fileName("C:\\libs/libc++utilities.so"));
    CPPUNIT_ASSERT_EQUAL("libc++utilities.so"s, fileName("/usr/lib/libc++utilities.so"));
    CPPUNIT_ASSERT_EQUAL("libc++utilities.so"s, fileName("libc++utilities.so"));
    CPPUNIT_ASSERT_EQUAL("/usr/lib/"s, directory("/usr/lib/libc++utilities.so"));
    CPPUNIT_ASSERT_EQUAL(string(), directory("libc++utilities.so"));
    CPPUNIT_ASSERT_EQUAL("C:\\libs\\"s, directory("C:\\libs\\libc++utilities.so"));
    CPPUNIT_ASSERT_EQUAL("C:\\libs/"s, directory("C:\\libs/libc++utilities.so"));
    string invalidPath("lib/c++uti*lities.so?");
    removeInvalidChars(invalidPath);
    CPPUNIT_ASSERT(invalidPath == "libc++utilities.so");
}

/*!
 * \brief Tests IniFile.
 */
void IoTests::testIniFile()
{
    // prepare reading test file
    fstream inputFile;
    inputFile.exceptions(ios_base::failbit | ios_base::badbit);
    inputFile.open(testFilePath("test.ini"), ios_base::in);

    IniFile ini;
    ini.parse(inputFile);
    const auto globalScope = ini.data().at(0);
    const auto scope1 = ini.data().at(1);
    const auto scope2 = ini.data().at(2);
    CPPUNIT_ASSERT(globalScope.first.empty());
    CPPUNIT_ASSERT(globalScope.second.find("key0") != globalScope.second.cend());
    CPPUNIT_ASSERT(globalScope.second.find("key0")->second == "value 0");
    CPPUNIT_ASSERT(globalScope.second.find("key1") == globalScope.second.cend());
    CPPUNIT_ASSERT(scope1.first == "scope 1");
    CPPUNIT_ASSERT(scope1.second.find("key1") != scope1.second.cend());
    CPPUNIT_ASSERT(scope1.second.find("key1")->second == "value 1");
    CPPUNIT_ASSERT(scope1.second.find("key2") != scope1.second.cend());
    CPPUNIT_ASSERT(scope1.second.find("key2")->second == "value=2");
    CPPUNIT_ASSERT(scope2.first == "scope 2");
    CPPUNIT_ASSERT(scope2.second.find("key5") == scope2.second.cend());

    // write values to another file
    fstream outputFile;
    outputFile.exceptions(ios_base::failbit | ios_base::badbit);
    outputFile.open(workingCopyPath("output.ini", WorkingCopyMode::NoCopy), ios_base::out | ios_base::trunc);
    ini.make(outputFile);

    // parse written values (again)
    outputFile.close();
    outputFile.open(workingCopyPath("output.ini", WorkingCopyMode::NoCopy), ios_base::in);
    IniFile ini2;
    ini2.parse(outputFile);
    CPPUNIT_ASSERT(ini.data() == ini2.data());
}

/*!
 * \brief Tests AdvancedIniFile.
 */
void IoTests::testAdvancedIniFile()
{
    // prepare reading test file
    fstream inputFile;
    inputFile.exceptions(ios_base::failbit | ios_base::badbit);
    inputFile.open(testFilePath("pacman.conf"), ios_base::in);

    // parse the test file
    AdvancedIniFile ini;
    ini.parse(inputFile);

    // check whether scope data is as expected
    CPPUNIT_ASSERT_EQUAL_MESSAGE("5 scopes (taking implicit empty section at the end into account)", 5_st, ini.sections.size());
    auto options = ini.findSection("options");
    CPPUNIT_ASSERT(options != ini.sectionEnd());
    TESTUTILS_ASSERT_LIKE_FLAGS(
        "comment block before section", "# Based on.*\n.*# GENERAL OPTIONS\n#\n"s, std::regex::extended, options->precedingCommentBlock);
    CPPUNIT_ASSERT_EQUAL(7_st, options->fields.size());
    CPPUNIT_ASSERT_EQUAL("HoldPkg"s, options->fields[0].key);
    CPPUNIT_ASSERT_EQUAL("pacman glibc"s, options->fields[0].value);
    CPPUNIT_ASSERT_MESSAGE("value present", options->fields[0].flags & IniFileFieldFlags::HasValue);
    TESTUTILS_ASSERT_LIKE_FLAGS("comment block between section header and first field",
        "# The following paths are.*\n.*#HookDir     = /etc/pacman\\.d/hooks/\n"s, std::regex::extended, options->fields[0].precedingCommentBlock);
    CPPUNIT_ASSERT_EQUAL(""s, options->fields[0].followingInlineComment);
    CPPUNIT_ASSERT_EQUAL("Foo"s, options->fields[1].key);
    CPPUNIT_ASSERT_EQUAL("bar"s, options->fields[1].value);
    CPPUNIT_ASSERT_MESSAGE("value present", options->fields[1].flags & IniFileFieldFlags::HasValue);
    TESTUTILS_ASSERT_LIKE_FLAGS("comment block between fields", "#XferCommand.*\n.*#CleanMethod = KeepInstalled\n"s, std::regex::extended,
        options->fields[1].precedingCommentBlock);
    CPPUNIT_ASSERT_EQUAL("# inline comment"s, options->fields[1].followingInlineComment);
    CPPUNIT_ASSERT_EQUAL("CheckSpace"s, options->fields[3].key);
    CPPUNIT_ASSERT_EQUAL(""s, options->fields[3].value);
    CPPUNIT_ASSERT_MESSAGE("no value present", !(options->fields[3].flags & IniFileFieldFlags::HasValue));
    TESTUTILS_ASSERT_LIKE_FLAGS("empty lines in comments preserved", "\n# Pacman.*\n.*\n\n#NoUpgrade   =\n.*#TotalDownload\n"s, std::regex::extended,
        options->fields[3].precedingCommentBlock);
    CPPUNIT_ASSERT_EQUAL(""s, options->fields[3].followingInlineComment);
    auto extraScope = ini.findSection(options, "extra");
    CPPUNIT_ASSERT(extraScope != ini.sectionEnd());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("comment block which is only an empty line", "\n"s, extraScope->precedingCommentBlock);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("inline comment after scope", "# an inline comment after a scope name"s, extraScope->followingInlineComment);
    CPPUNIT_ASSERT_EQUAL(1_st, extraScope->fields.size());
    CPPUNIT_ASSERT(ini.sections.back().flags & IniFileSectionFlags::Implicit);
    TESTUTILS_ASSERT_LIKE_FLAGS("comment block after last field present in implicitly added last scope", "\n# If you.*\n.*custompkgs\n"s,
        std::regex::extended, ini.sections.back().precedingCommentBlock);

    // test finding a field from file level and const access
    const auto *const constIniFile = &ini;
    auto includeField = constIniFile->findField("extra", "Include");
    CPPUNIT_ASSERT(includeField.has_value());
    CPPUNIT_ASSERT_EQUAL("Include"s, includeField.value()->key);
    CPPUNIT_ASSERT_EQUAL("/etc/pacman.d/mirrorlist"s, includeField.value()->value);
    CPPUNIT_ASSERT_MESSAGE("field not present", !constIniFile->findField("extra", "Includ").has_value());
    CPPUNIT_ASSERT_MESSAGE("scope not present", !constIniFile->findField("extr", "Includ").has_value());

    // write values again; there shouldn't be a difference as the parser and the writer are supposed to
    // preserve the order of all elements and comments
    std::stringstream newFile;
    ini.make(newFile);
    std::string originalContents;
    inputFile.clear();
    inputFile.seekg(std::ios_base::beg);
    originalContents.assign((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
    CPPUNIT_ASSERT_EQUAL(originalContents, newFile.str());
}

/*!
 * \brief Tests CopyHelper.
 */
void IoTests::testCopy()
{
    // prepare streams
    fstream testFile;
    testFile.exceptions(ios_base::failbit | ios_base::badbit);
    testFile.open(testFilePath("some_data"), ios_base::in | ios_base::binary);
    stringstream outputStream(ios_base::in | ios_base::out | ios_base::binary);
    outputStream.exceptions(ios_base::failbit | ios_base::badbit);

    // copy
    CopyHelper<13> copyHelper;
    copyHelper.copy(testFile, outputStream, 50);

    // test
    testFile.seekg(0);
    for (auto i = 0; i < 50; ++i) {
        CPPUNIT_ASSERT(testFile.get() == outputStream.get());
    }
}

/*!
 * \brief Tests readFile().
 */
void IoTests::testReadFile()
{
    // read a file successfully
    const string iniFilePath(testFilePath("test.ini"));
    CPPUNIT_ASSERT_EQUAL("# file for testing INI parser\n"
                         "key0=value 0\n"
                         "\n"
                         "[scope 1]\n"
                         "key1=value 1 # comment\n"
                         "key2=value=2\n"
                         "key3=value 3\n"
                         "\n"
                         "[scope 2]\n"
                         "key4=value 4\n"
                         "#key5=value 5\n"
                         "key6=value 6\n"s,
        readFile(iniFilePath));

    // fail by exceeding max size
    CPPUNIT_ASSERT_THROW(readFile(iniFilePath, 10), std::ios_base::failure);

    // handle UTF-8 in path and file contents correctly via NativeFileStream
#if !defined(PLATFORM_WINDOWS) || defined(CPP_UTILITIES_USE_NATIVE_FILE_BUFFER)
    CPPUNIT_ASSERT_EQUAL("file with non-ASCII character 'ä' in its name\n"s, readFile(testFilePath("täst.txt")));
#endif
}

/*!
 * \brief Tests writeFile().
 */
void IoTests::testWriteFile()
{
    const string path(workingCopyPath("test.ini", WorkingCopyMode::NoCopy));
    writeFile(path, "some contents");
    CPPUNIT_ASSERT_EQUAL("some contents"s, readFile(path));
}

/*!
 * \brief Tests formatting functions of CppUtilities::EscapeCodes namespace.
 */
void IoTests::testAnsiEscapeCodes()
{
    stringstream ss1;
    EscapeCodes::enabled = true;
    ss1 << EscapeCodes::Phrases::Error << "some error" << EscapeCodes::Phrases::End;
    ss1 << EscapeCodes::Phrases::Warning << "some warning" << EscapeCodes::Phrases::End;
    ss1 << EscapeCodes::Phrases::Info << "some info" << EscapeCodes::Phrases::End;
    ss1 << EscapeCodes::Phrases::ErrorMessage << "Arch-style error" << EscapeCodes::Phrases::End;
    ss1 << EscapeCodes::Phrases::WarningMessage << "Arch-style warning" << EscapeCodes::Phrases::End;
    ss1 << EscapeCodes::Phrases::PlainMessage << "Arch-style message" << EscapeCodes::Phrases::End;
    ss1 << EscapeCodes::Phrases::SuccessMessage << "Arch-style success" << EscapeCodes::Phrases::End;
    ss1 << EscapeCodes::Phrases::SubMessage << "Arch-style sub-message" << EscapeCodes::Phrases::End;
    ss1 << EscapeCodes::color(EscapeCodes::Color::Blue, EscapeCodes::Color::Red, EscapeCodes::TextAttribute::Blink)
        << "blue, blinking text on red background" << EscapeCodes::TextAttribute::Reset << '\n';
    cout << "\noutput for formatting with ANSI escape codes:\n" << ss1.str() << "---------------------------------------------\n";
    fstream("/tmp/test.txt", ios_base::out | ios_base::trunc) << ss1.str();
    CPPUNIT_ASSERT_EQUAL("\e[1;31mError: \e[0m\e[1msome error\e[0m\n"
                         "\e[1;33mWarning: \e[0m\e[1msome warning\e[0m\n"
                         "\e[1;34mInfo: \e[0m\e[1msome info\e[0m\n"
                         "\e[1;31m==> ERROR: \e[0m\e[1mArch-style error\e[0m\n"
                         "\e[1;33m==> WARNING: \e[0m\e[1mArch-style warning\e[0m\n"
                         "    \e[0m\e[1mArch-style message\e[0m\n"
                         "\e[1;32m==> \e[0m\e[1mArch-style success\e[0m\n"
                         "\e[1;32m  -> \e[0m\e[1mArch-style sub-message\e[0m\n"
                         "\e[5;34;41mblue, blinking text on red background\e[0m\n"s,
        ss1.str());

    stringstream ss2;
    EscapeCodes::enabled = false;
    ss2 << EscapeCodes::Phrases::Info << "some info" << EscapeCodes::Phrases::End;
    CPPUNIT_ASSERT_EQUAL("Info: some info\n"s, ss2.str());
}

#ifdef CPP_UTILITIES_USE_NATIVE_FILE_BUFFER
/*!
 * \brief Tests the NativeFileStream class.
 */
void IoTests::testNativeFileStream()
{
    // open file by path
    const auto txtFilePath(workingCopyPath("täst.txt"));
    NativeFileStream fileStream;
    fileStream.exceptions(ios_base::badbit | ios_base::failbit);
    CPPUNIT_ASSERT(!fileStream.is_open());
    fileStream.open(txtFilePath, ios_base::in);
    CPPUNIT_ASSERT(fileStream.is_open());
#if defined(PLATFORM_WINDOWS) && defined(CPP_UTILITIES_USE_BOOST_IOSTREAMS)
    CPPUNIT_ASSERT(fileStream.fileHandle() != nullptr);
#else
    CPPUNIT_ASSERT(fileStream.fileDescriptor() != -1);
#endif
    CPPUNIT_ASSERT_EQUAL(static_cast<char>(fileStream.get()), 'f');
    fileStream.seekg(0, ios_base::end);
    CPPUNIT_ASSERT_EQUAL(fileStream.tellg(), static_cast<NativeFileStream::pos_type>(47));
    fileStream.close();
    CPPUNIT_ASSERT(!fileStream.is_open());
    try {
        fileStream.open("non existing file", ios_base::in | ios_base::out | ios_base::binary);
        CPPUNIT_FAIL("expected exception");
    } catch (const std::ios_base::failure &failure) {
#ifdef PLATFORM_WINDOWS
#ifdef CPP_UTILITIES_USE_BOOST_IOSTREAMS
        TESTUTILS_ASSERT_LIKE("expected error with some message", "CreateFileW failed: .+", failure.what());
#else
        TESTUTILS_ASSERT_LIKE("expected error with some message", "_wopen failed: .+", failure.what());
#endif
#else
        TESTUTILS_ASSERT_LIKE("expected error with some message", "open failed: .+", failure.what());
#endif
    }
    fileStream.clear();

    // open file from file descriptor
#ifndef PLATFORM_WINDOWS
    auto readWriteFileDescriptor = open(txtFilePath.data(), O_RDWR);
    CPPUNIT_ASSERT(readWriteFileDescriptor);
    fileStream.open(readWriteFileDescriptor, ios_base::in | ios_base::out | ios_base::binary);
    CPPUNIT_ASSERT(fileStream.is_open());
    CPPUNIT_ASSERT_EQUAL(static_cast<char>(fileStream.get()), 'f');
    fileStream.seekg(0, ios_base::end);
    CPPUNIT_ASSERT_EQUAL(fileStream.tellg(), static_cast<NativeFileStream::pos_type>(47));
    fileStream.flush();
    fileStream.close();
    CPPUNIT_ASSERT(!fileStream.is_open());
#endif
    try {
        fileStream.open(-1, ios_base::in | ios_base::out | ios_base::binary);
        fileStream.get();
        CPPUNIT_FAIL("expected exception");
    } catch (const std::ios_base::failure &failure) {
#ifndef PLATFORM_WINDOWS
        TESTUTILS_ASSERT_LIKE(
            "expected error message", "(basic_ios::clear|failed reading: Bad file descriptor): iostream error"s, string(failure.what()));
#else
        CPP_UTILITIES_UNUSED(failure)
#endif
    }
    fileStream.clear();

    // append + write file via path
    NativeFileStream fileStream2;
    fileStream2.exceptions(ios_base::failbit | ios_base::badbit);
    fileStream2.open(txtFilePath, ios_base::in | ios_base::out | ios_base::app);
    CPPUNIT_ASSERT(fileStream2.is_open());
    fileStream2 << "foo";
    fileStream2.flush();
    fileStream2.close();
    CPPUNIT_ASSERT(!fileStream2.is_open());
    CPPUNIT_ASSERT_EQUAL("file with non-ASCII character 'ä' in its name\nfoo"s, readFile(txtFilePath, 50));

    // truncate + write file via path
    fileStream2.open(txtFilePath, ios_base::out | ios_base::trunc);
    CPPUNIT_ASSERT(fileStream2.is_open());
    fileStream2 << "bar";
    fileStream2.close();
    CPPUNIT_ASSERT(!fileStream2.is_open());
    CPPUNIT_ASSERT_EQUAL("bar"s, readFile(txtFilePath, 4));

    // append + write via file descriptor from file handle
#ifdef PLATFORM_WINDOWS
    const auto wideTxtFilePath = NativeFileStream::makeWidePath(txtFilePath);
    const auto appendFileHandle = _wfopen(wideTxtFilePath.get(), L"a+");
#else
    const auto appendFileHandle = fopen(txtFilePath.data(), "a");
#endif
    CPPUNIT_ASSERT(appendFileHandle);
    fileStream2.open(fileno(appendFileHandle), ios_base::out | ios_base::app);
    CPPUNIT_ASSERT(fileStream2.is_open());
    fileStream2 << "foo";
    fileStream2.close();
    CPPUNIT_ASSERT(!fileStream2.is_open());
    CPPUNIT_ASSERT_EQUAL("barfoo"s, readFile(txtFilePath, 7));
}
#endif
