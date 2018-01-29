#include "./testutils.h"

#include "../conversion/conversionexception.h"
#include "../io/ansiescapecodes.h"
#include "../io/binaryreader.h"
#include "../io/binarywriter.h"
#include "../io/bitreader.h"
#include "../io/catchiofailure.h"
#include "../io/copy.h"
#include "../io/inifile.h"
#include "../io/misc.h"
#include "../io/path.h"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <algorithm>
#include <fstream>
#include <sstream>

using namespace std;
using namespace IoUtilities;
using namespace TestUtilities;
using namespace TestUtilities::Literals;
using namespace ConversionUtilities;

using namespace CPPUNIT_NS;

/*!
 * \brief The IoTests class tests classes and methods of the IoUtilities namespace.
 */
class IoTests : public TestFixture {
    CPPUNIT_TEST_SUITE(IoTests);
    CPPUNIT_TEST(testFailure);
    CPPUNIT_TEST(testBinaryReader);
    CPPUNIT_TEST(testBinaryWriter);
    CPPUNIT_TEST(testBitReader);
    CPPUNIT_TEST(testPathUtilities);
    CPPUNIT_TEST(testIniFile);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST(testMisc);
    CPPUNIT_TEST(testAnsiEscapeCodes);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testFailure();
    void testBinaryReader();
    void testBinaryWriter();
    void testBitReader();
    void testPathUtilities();
    void testIniFile();
    void testCopy();
    void testMisc();
    void testAnsiEscapeCodes();
};

CPPUNIT_TEST_SUITE_REGISTRATION(IoTests);

void IoTests::setUp()
{
}

void IoTests::tearDown()
{
}

/*!
 * \brief Tests for GCC Bug 66145.
 * \sa https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145
 * \remarks Using workaround now; hence testing workaround instead.
 */
void IoTests::testFailure()
{
    //fstream stream;
    //stream.exceptions(ios_base::failbit | ios_base::badbit);
    //CPPUNIT_ASSERT_THROW(stream.open("path/to/file/which/does/not/exist", ios_base::in), ios_base::failure);
    // check other exceptions used by my applications, too
    vector<int> testVec;
    map<string, string> testMap;
    CPPUNIT_ASSERT_THROW(testVec.at(1), out_of_range);
    CPPUNIT_ASSERT_THROW(testMap.at("test"), out_of_range);

    // check workaround
    try {
        fstream stream;
        stream.exceptions(ios_base::failbit | ios_base::badbit);
        stream.open("path/to/file/which/does/not/exist", ios_base::in);
    } catch (...) {
        catchIoFailure();
    }
}

/*!
 * \brief Tests the most important methods of the BinaryReader.
 */
void IoTests::testBinaryReader()
{
    // read test file
    fstream testFile;
    testFile.exceptions(ios_base::failbit | ios_base::badbit);
    testFile.open(TestUtilities::testFilePath("some_data"), ios_base::in | ios_base::binary);
    BinaryReader reader(&testFile);
    CPPUNIT_ASSERT_EQUAL(reader.readStreamsize(), static_cast<istream::pos_type>(398));
    CPPUNIT_ASSERT(reader.readUInt16LE() == 0x0102u);
    CPPUNIT_ASSERT(reader.readUInt16BE() == 0x0102u);
    CPPUNIT_ASSERT(reader.readUInt24LE() == 0x010203u);
    CPPUNIT_ASSERT(reader.readUInt24BE() == 0x010203u);
    CPPUNIT_ASSERT(reader.readUInt32LE() == 0x01020304u);
    CPPUNIT_ASSERT(reader.readUInt32BE() == 0x01020304u);
    CPPUNIT_ASSERT(reader.readUInt40LE() == 0x0102030405u);
    CPPUNIT_ASSERT(reader.readUInt40BE() == 0x0102030405u);
    CPPUNIT_ASSERT(reader.readUInt56LE() == 0x01020304050607u);
    CPPUNIT_ASSERT(reader.readUInt56BE() == 0x01020304050607u);
    CPPUNIT_ASSERT(reader.readUInt64LE() == 0x0102030405060708u);
    CPPUNIT_ASSERT(reader.readUInt64BE() == 0x0102030405060708u);
    testFile.seekg(0);
    CPPUNIT_ASSERT(reader.readInt16LE() == 0x0102);
    CPPUNIT_ASSERT(reader.readInt16BE() == 0x0102);
    CPPUNIT_ASSERT(reader.readInt24LE() == 0x010203);
    CPPUNIT_ASSERT(reader.readInt24BE() == 0x010203);
    CPPUNIT_ASSERT(reader.readInt32LE() == 0x01020304);
    CPPUNIT_ASSERT(reader.readInt32BE() == 0x01020304);
    CPPUNIT_ASSERT(reader.readInt40LE() == 0x0102030405);
    CPPUNIT_ASSERT(reader.readInt40BE() == 0x0102030405);
    CPPUNIT_ASSERT(reader.readInt56LE() == 0x01020304050607);
    CPPUNIT_ASSERT(reader.readInt56BE() == 0x01020304050607);
    CPPUNIT_ASSERT(reader.readInt64LE() == 0x0102030405060708);
    CPPUNIT_ASSERT(reader.readInt64BE() == 0x0102030405060708);
    CPPUNIT_ASSERT(reader.readFloat32LE() == 1.125);
    CPPUNIT_ASSERT(reader.readFloat64LE() == 1.625);
    CPPUNIT_ASSERT(reader.readFloat32BE() == 1.125);
    CPPUNIT_ASSERT(reader.readFloat64BE() == 1.625);
    CPPUNIT_ASSERT(reader.readBool() == false);
    CPPUNIT_ASSERT(reader.readBool() == true);
    CPPUNIT_ASSERT(reader.readString(3) == "abc");
    CPPUNIT_ASSERT(reader.readLengthPrefixedString() == "ABC");
    CPPUNIT_ASSERT(reader.readLengthPrefixedString()
        == "01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901"
           "23456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123"
           "45678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345"
           "678901234567890123456789");
    CPPUNIT_ASSERT_EQUAL("def"s, reader.readTerminatedString());
    testFile.seekg(-4, ios_base::cur);
    CPPUNIT_ASSERT_EQUAL("def"s, reader.readTerminatedString(5, 0));
    testFile.seekg(-4, ios_base::cur);
    CPPUNIT_ASSERT_EQUAL("de"s, reader.readMultibyteTerminatedStringBE(5, 0x6600));
    testFile.seekg(-4, ios_base::cur);
    CPPUNIT_ASSERT_EQUAL("de"s, reader.readMultibyteTerminatedStringLE(5, 0x0066));
    testFile.seekg(-4, ios_base::cur);
    CPPUNIT_ASSERT_EQUAL("de"s, reader.readMultibyteTerminatedStringBE(static_cast<uint16>(0x6600)));
    testFile.seekg(-4, ios_base::cur);
    CPPUNIT_ASSERT_EQUAL("de"s, reader.readMultibyteTerminatedStringLE(static_cast<uint16>(0x0066)));
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
    testFile.open(TestUtilities::testFilePath("some_data"), ios_base::in | ios_base::binary);

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
    const byte testData[] = { 0x81, 0x90, 0x3C, 0x44, 0x28, 0x00, 0x44, 0x10, 0x20, 0xFF, 0xFA };
    BitReader reader(reinterpret_cast<const char *>(testData), sizeof(testData));
    CPPUNIT_ASSERT(reader.readBit() == 1);
    reader.skipBits(6);
    CPPUNIT_ASSERT(reader.showBits<byte>(2) == 3);
    CPPUNIT_ASSERT(reader.readBits<byte>(2) == 3);
    CPPUNIT_ASSERT(reader.readBits<uint32>(32) == (0x103C4428 << 1));
    reader.align();
    CPPUNIT_ASSERT(reader.readBits<byte>(8) == 0x44);
    CPPUNIT_ASSERT(reader.readUnsignedExpGolombCodedBits<byte>() == 7);
    CPPUNIT_ASSERT(reader.readSignedExpGolombCodedBits<sbyte>() == 4);
    CPPUNIT_ASSERT(reader.readBit() == 0);
    CPPUNIT_ASSERT(reader.readBit() == 0);
    reader.skipBits(8 + 4);
    CPPUNIT_ASSERT_EQUAL(4_st, reader.bitsAvailable());
    CPPUNIT_ASSERT_EQUAL(static_cast<byte>(0xA), reader.readBits<byte>(4));
    try {
        reader.readBit();
        CPPUNIT_FAIL("no exception");
    } catch (...) {
        catchIoFailure();
    }
    try {
        reader.skipBits(1);
        CPPUNIT_FAIL("no exception");
    } catch (...) {
        catchIoFailure();
    }
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
    CPPUNIT_ASSERT_EQUAL("/usr/lib/"s, directory("/usr/lib/libc++utilities.so"));
    CPPUNIT_ASSERT_EQUAL(string(), directory("libc++utilities.so"));
    CPPUNIT_ASSERT_EQUAL("C:\\libs\\"s, directory("C:\\libs\\libc++utilities.so"));
    CPPUNIT_ASSERT_EQUAL("C:\\libs/"s, directory("C:\\libs/libc++utilities.so"));
    string invalidPath("lib/c++uti*lities.so?");
    removeInvalidChars(invalidPath);
    CPPUNIT_ASSERT(invalidPath == "libc++utilities.so");
#ifdef PLATFORM_UNIX
    const string iniFilePath = TestUtilities::testFilePath("test.ini");
    const string testFilesDir = iniFilePath.substr(0, iniFilePath.size() - 9);
    auto testFilesDirEntries = directoryEntries(testFilesDir.c_str(), DirectoryEntryType::All);
    CPPUNIT_ASSERT(find(testFilesDirEntries.cbegin(), testFilesDirEntries.cend(), "test.ini") != testFilesDirEntries.cend());
    CPPUNIT_ASSERT(find(testFilesDirEntries.cbegin(), testFilesDirEntries.cend(), ".") != testFilesDirEntries.cend());
    CPPUNIT_ASSERT(find(testFilesDirEntries.cbegin(), testFilesDirEntries.cend(), "..") != testFilesDirEntries.cend());
    testFilesDirEntries = directoryEntries(testFilesDir.c_str(), DirectoryEntryType::Directory);
    CPPUNIT_ASSERT(find(testFilesDirEntries.cbegin(), testFilesDirEntries.cend(), "test.ini") == testFilesDirEntries.cend());
    CPPUNIT_ASSERT(find(testFilesDirEntries.cbegin(), testFilesDirEntries.cend(), ".") != testFilesDirEntries.cend());
    CPPUNIT_ASSERT(find(testFilesDirEntries.cbegin(), testFilesDirEntries.cend(), "..") != testFilesDirEntries.cend());
    testFilesDirEntries = directoryEntries(testFilesDir.c_str(), DirectoryEntryType::File);
    CPPUNIT_ASSERT(find(testFilesDirEntries.cbegin(), testFilesDirEntries.cend(), "test.ini") != testFilesDirEntries.cend());
    CPPUNIT_ASSERT(find(testFilesDirEntries.cbegin(), testFilesDirEntries.cend(), ".") == testFilesDirEntries.cend());
    CPPUNIT_ASSERT(find(testFilesDirEntries.cbegin(), testFilesDirEntries.cend(), "..") == testFilesDirEntries.cend());
#endif
}

/*!
 * \brief Tests IniFile.
 */
void IoTests::testIniFile()
{
    // prepare reading test file
    fstream inputFile;
    inputFile.exceptions(ios_base::failbit | ios_base::badbit);
    inputFile.open(TestUtilities::testFilePath("test.ini"), ios_base::in);

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
    outputFile.open(workingCopyPathMode("output.ini", WorkingCopyMode::NoCopy), ios_base::out | ios_base::trunc);
    ini.make(outputFile);

    // parse written values (again)
    outputFile.close();
    outputFile.open(workingCopyPathMode("output.ini", WorkingCopyMode::NoCopy), ios_base::in);
    IniFile ini2;
    ini2.parse(outputFile);
    CPPUNIT_ASSERT(ini.data() == ini2.data());
}

/*!
 * \brief Tests CopyHelper.
 */
void IoTests::testCopy()
{
    // prepare streams
    fstream testFile;
    testFile.exceptions(ios_base::failbit | ios_base::badbit);
    testFile.open(TestUtilities::testFilePath("some_data"), ios_base::in | ios_base::binary);
    stringstream outputStream(ios_base::in | ios_base::out | ios_base::binary);
    outputStream.exceptions(ios_base::failbit | ios_base::badbit);

    // copy
    CopyHelper<13> copyHelper;
    copyHelper.copy(testFile, outputStream, 50);

    // test
    testFile.seekg(0);
    for (byte i = 0; i < 50; ++i) {
        CPPUNIT_ASSERT(testFile.get() == outputStream.get());
    }
}

/*!
 * \brief Tests misc IO utilities.
 */
void IoTests::testMisc()
{
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
    try {
        readFile(iniFilePath, 10);
        CPPUNIT_FAIL("no exception");
    } catch (...) {
        catchIoFailure();
    }
}

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
