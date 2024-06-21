#include "./buffersearch.h"

using namespace std;

namespace CppUtilities {

/*!
 * \class BufferSearch
 * \brief The BufferSearch struct invokes a callback if an initially given search term occurs in consecutively provided buffers.
 * \remarks
 * - The class works without making internal copies of the specified buffers, except for the search result.
 * - The callback is invoked after the search term has been found and one of the specified termination characters occurred. The
 *   search result is passed to the callback.
 * - The "search result" is the data after the last character of the search term and before any of the specified termination
 *   characters.
 * - If no termination characters are specified, the callback is invoked directly after the search term occurred (with an empty
 *   search result).
 * - If the specified give-up term has occurred, operator() will exit early and the specified callback will not be invoked
 *   anymore.
 * - If the callback has been invoked, operator() will exit early and the callback will not be invoked anymore (even if the
 *   search term occurs again). Call reset() after consuming the result within the callback to continue the search.
 *
 */

/*!
 * \brief Processes the specified \a buffer. Invokes the callback according to the remarks mentioned in the class documentation.
 * \returns
 * - Returns the offset in \a buffer after the search term and search result. This is the first character after the search term if
 *   no termination characters have been specified; otherwiese it is the offset of the termination character.
 * - Returns nullptr if the search term could not be found.
 */
const std::string_view::value_type *BufferSearch::process(const std::string_view::value_type *buffer, std::size_t bufferSize)
{
    if (m_hasResult || (!m_giveUpTerm.empty() && m_giveUpTermIterator == m_giveUpTerm.end())) {
        return nullptr;
    }
    for (auto i = buffer, end = buffer + bufferSize; i != end; ++i) {
        const auto currentChar = *i;
        if (m_searchTermIterator == m_searchTerm.end()) {
            if (m_terminationChars.empty()) {
                m_hasResult = true;
            } else {
                for (const auto &terminationChar : m_terminationChars) {
                    if (currentChar == terminationChar) {
                        m_hasResult = true;
                        break;
                    }
                }
            }
            if (m_hasResult) {
                m_callback(*this, std::move(m_result));
                return i;
            }
            m_result += currentChar;
            continue;
        }
        if (currentChar == *m_searchTermIterator) {
            ++m_searchTermIterator;
        } else {
            m_searchTermIterator = m_searchTerm.begin();
        }
        if (m_giveUpTerm.empty()) {
            continue;
        }
        if (currentChar == *m_giveUpTermIterator) {
            ++m_giveUpTermIterator;
        } else {
            m_giveUpTermIterator = m_giveUpTerm.begin();
        }
    }
    return nullptr;
}

/*!
 * \brief Processes the specified \a buffer. Invokes the callback according to the remarks mentioned in the class documentation.
 * \todo Make inline in v6.
 */
void BufferSearch::operator()(const std::string_view::value_type *buffer, std::size_t bufferSize)
{
    process(buffer, bufferSize);
}

/*!
 * \brief Resets the search to its initial state (assuming no characters of the search term or give-up term have been found yet).
 */
void BufferSearch::reset()
{
    m_searchTermIterator = m_searchTerm.begin();
    m_giveUpTermIterator = m_giveUpTerm.begin();
    m_terminationTermIterator = m_terminationTerm.begin();
    m_hasResult = false;
    m_result.clear();
}

} // namespace CppUtilities
