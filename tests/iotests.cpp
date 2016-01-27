#include "../io/binaryreader.h"
#include "../io/binarywriter.h"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include <fstream>
#include <sstream>

using namespace std;
using namespace IoUtilities;

using namespace CPPUNIT_NS;

namespace UnitTests {
extern string testFilesPath;
}

class IoTests : public TestFixture
{
    CPPUNIT_TEST_SUITE(IoTests);
    CPPUNIT_TEST(testFailure);
    CPPUNIT_TEST(testBinaryReader);
    CPPUNIT_TEST(testBinaryWriter);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testFailure();
    void testBinaryReader();
    void testBinaryWriter();
};

CPPUNIT_TEST_SUITE_REGISTRATION(IoTests);

void IoTests::setUp()
{}

void IoTests::tearDown()
{}

/*!
 * \brief Tests for GCC Bug 66145.
 * \sa https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145
 */
void IoTests::testFailure()
{
    fstream stream;
    stream.exceptions(ios_base::failbit | ios_base::badbit);
    CPPUNIT_ASSERT_THROW(stream.open("path/to/file/which/does/not/exist", ios_base::in), ios_base::failure);
}

/*!
 * \brief Tests the most important methods of the BinaryReader.
 */
void IoTests::testBinaryReader()
{
    // read test file
    fstream testFile;
    testFile.open(UnitTests::testFilesPath + "/some_data", ios_base::in | ios_base::binary);
    BinaryReader reader(&testFile);
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
    CPPUNIT_ASSERT(reader.readTerminatedString() == "def");
}

/*!
 * \brief Tests the most important methods of the BinaryWriter.
 */
void IoTests::testBinaryWriter()
{
    // prepare reading expected data
    fstream testFile;
    testFile.open(UnitTests::testFilesPath + "/some_data", ios_base::in | ios_base::binary);

    // prepare output stream
    stringstream outputStream(ios_base::in | ios_base::out | ios_base::binary);
    outputStream.exceptions(ios_base::failbit | ios_base::badbit);
    char testData[95];
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
    for(char c : testData) {
        CPPUNIT_ASSERT(c == static_cast<char>(testFile.get()));
        if(testFile.tellg() >= 58) {
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
    writer.writeTerminatedString("def");

    // test written values
    for(char c : testData) {
        CPPUNIT_ASSERT(c == static_cast<char>(testFile.get()));
    }
}
