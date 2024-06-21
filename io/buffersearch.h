#ifndef IOUTILITIES_BUFFER_SEARCH_H
#define IOUTILITIES_BUFFER_SEARCH_H

#include "../global.h"
#include "../misc/traits.h"

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace CppUtilities {

class CPP_UTILITIES_EXPORT BufferSearch {
public:
    using CallbackType = std::function<void(BufferSearch &, std::string &&)>;
    BufferSearch(std::string_view searchTerm, std::string_view terminationChars, std::string_view giveUpTerm, CallbackType &&callback);
    void operator()(std::string_view buffer);
    void operator()(const std::string_view::value_type *buffer, std::size_t bufferSize);
    template <std::size_t bufferCapacity>
    void operator()(std::shared_ptr<std::array<std::string_view::value_type, bufferCapacity>> buffer, std::size_t bufferSize);
    const std::string_view::value_type *process(std::string_view buffer);
    const std::string_view::value_type *process(const std::string_view::value_type *buffer, std::size_t bufferSize);
    void reset();
    std::string &result();
    const std::string &result() const;

private:
    const std::string_view m_searchTerm;
    const std::string_view m_terminationChars;
    const std::string_view m_terminationTerm;
    const std::string_view m_giveUpTerm;
    const CallbackType m_callback;
    std::string_view::const_iterator m_searchTermIterator;
    std::string_view::const_iterator m_giveUpTermIterator;
    std::string_view::const_iterator m_terminationTermIterator;
    std::string m_result;
    bool m_hasResult;
};

/*!
 * \brief Constructs a new BufferSearch. Might be overloaded in the future.
 */
inline BufferSearch::BufferSearch(
    std::string_view searchTerm, std::string_view terminationChars, std::string_view giveUpTerm, CallbackType &&callback)
    : m_searchTerm(searchTerm)
    , m_terminationChars(terminationChars)
    , m_giveUpTerm(giveUpTerm)
    , m_callback(std::move(callback))
    , m_searchTermIterator(m_searchTerm.begin())
    , m_giveUpTermIterator(m_giveUpTerm.begin())
    , m_terminationTermIterator(m_terminationTerm.begin())
    , m_hasResult(false)
{
}

/*!
 * \brief Processes the specified \a buffer. Invokes the callback according to the remarks mentioned in the class documentation.
 */
inline void BufferSearch::operator()(std::string_view buffer)
{
    (*this)(buffer.data(), buffer.size());
}

/*!
 * \brief Processes the specified \a buffer which is a shared array with fixed \tparam bufferCapacity. Invokes the callback according to the remarks mentioned in the class documentation.
 */
template <std::size_t bufferCapacity>
inline void BufferSearch::operator()(std::shared_ptr<std::array<std::string_view::value_type, bufferCapacity>> buffer, std::size_t bufferSize)
{
    (*this)(buffer->data(), bufferSize);
}

/*!
 * \brief Processes the specified \a buffer. Invokes the callback according to the remarks mentioned in the class documentation.
 * \returns See other overload for details.
 */
inline const std::string_view::value_type *BufferSearch::process(std::string_view buffer)
{
    return process(buffer.data(), buffer.size());
}

/*!
 * \brief Returns the search result at this point.
 */
inline std::string &BufferSearch::result()
{
    return m_result;
}

/*!
 * \brief Returns the search result at this point.
 */
inline const std::string &BufferSearch::result() const
{
    return m_result;
}

} // namespace CppUtilities

#endif // IOUTILITIES_BUFFER_SEARCH_H
