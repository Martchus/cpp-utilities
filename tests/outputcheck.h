#ifndef TESTUTILS_OUTPUTCHECK_H
#define TESTUTILS_OUTPUTCHECK_H

#include "../conversion/stringbuilder.h"

#include <cppunit/extensions/HelperMacros.h>

#include <functional>
#include <iostream>
#include <sstream>
#include <string>

namespace TestUtilities {

/*!
 * \brief The StandardOutputCheck class asserts whether the (standard) output written in the enclosing code block
 *        matches the expected output.
 */
class OutputCheck {
public:
    OutputCheck(const std::string &expectedOutput, std::ostream &os = std::cout);
    OutputCheck(std::string &&expectedOutput, std::string &&alternativeOutput, std::ostream &os = std::cout);
    OutputCheck(std::function<void(const std::string &output)> &&customCheck, std::ostream &os = std::cout);
    ~OutputCheck() noexcept(false);

private:
    std::ostream &m_os;
    const std::function<void(const std::string &output)> m_customCheck;
    const std::string m_expectedOutput;
    const std::string m_alternativeOutput;
    std::stringstream m_buffer;
    std::streambuf *const m_regularOutputBuffer;
};

/*!
 * \brief Redirects standard output to an internal buffer.
 */
inline OutputCheck::OutputCheck(const std::string &expectedOutput, std::ostream &os)
    : m_os(os)
    , m_expectedOutput(expectedOutput)
    , m_buffer()
    , m_regularOutputBuffer(os.rdbuf(m_buffer.rdbuf()))
{
}

/*!
 * \brief Redirects standard output to an internal buffer.
 */
inline OutputCheck::OutputCheck(std::string &&expectedOutput, std::string &&alternativeOutput, std::ostream &os)
    : m_os(os)
    , m_expectedOutput(expectedOutput)
    , m_alternativeOutput(alternativeOutput)
    , m_buffer()
    , m_regularOutputBuffer(os.rdbuf(m_buffer.rdbuf()))
{
}

/*!
 * \brief Redirects standard output to an internal buffer.
 */
inline OutputCheck::OutputCheck(std::function<void(const std::string &)> &&customCheck, std::ostream &os)
    : m_os(os)
    , m_customCheck(customCheck)
    , m_buffer()
    , m_regularOutputBuffer(os.rdbuf(m_buffer.rdbuf()))
{
}

/*!
 * \brief Asserts the buffered standard output and restores the regular behaviour of std::cout.
 */
inline OutputCheck::~OutputCheck() noexcept(false)
{
    m_os.rdbuf(m_regularOutputBuffer);
    if (m_customCheck) {
        m_customCheck(m_buffer.str());
        return;
    }
    if (m_alternativeOutput.empty()) {
        CPPUNIT_ASSERT_EQUAL(m_expectedOutput, m_buffer.str());
        return;
    }
    const std::string actualOutput(m_buffer.str());
    if (m_expectedOutput != actualOutput && m_alternativeOutput != actualOutput) {
        using namespace ConversionUtilities;
        CPPUNIT_FAIL("Output is not either \"" % m_expectedOutput % "\" or \"" % m_alternativeOutput % "\". Got instead:\n" + actualOutput);
    }
}

} // namespace TestUtilities

#endif // TESTUTILS_OUTPUTCHECK_H
