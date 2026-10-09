#ifndef CPP_UTILITIES_REGEX_H
#define CPP_UTILITIES_REGEX_H

#include "../global.h"
#include "../misc/flagenumclass.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace CppUtilities {

/*!
 * \class RegexException
 * \brief The RegexException class is thrown when compiling or matching a regular expression fails.
 * \remarks Still experimental. Might be removed/adjusted in next minor release.
 */
class CPP_UTILITIES_EXPORT RegexException : public std::runtime_error {
public:
    explicit RegexException() noexcept;
    explicit RegexException(std::string_view message) noexcept;
    explicit RegexException(std::string_view message, std::size_t offset) noexcept;
    ~RegexException() override;

    std::size_t offset() const noexcept;

private:
    std::size_t m_offset = 0;
};

/*!
 * \brief Returns the byte offset in the pattern where the error occurred.
 */
inline std::size_t RegexException::offset() const noexcept
{
    return m_offset;
}

/*!
 * \brief The RegexCompileOptions enum specifies options for compiling regular expressions.
 * \remarks Still experimental. Might be removed/adjusted in next minor release.
 */
enum class RegexCompileOptions : std::uint32_t {
    None = 0,
    CaseInsensitive = (1 << 0),
    Multiline = (1 << 1),
    DotAll = (1 << 2),
    Extended = (1 << 3),
    Ungreedy = (1 << 4),
    DollarEndOnly = (1 << 5),
    Utf = (1 << 6),
};

/*!
 * \brief The RegexMatchOptions enum specifies options for matching regular expressions.
 * \remarks Still experimental. Might be removed/adjusted in next minor release.
 */
enum class RegexMatchOptions : std::uint32_t {
    None = 0,
    NotBeginningOfLine = (1 << 0),
    NotEndOfLine = (1 << 1),
    NotEmpty = (1 << 2),
    NotEmptyAtStart = (1 << 3),
};

/*!
 * \class RegexMatch
 * \brief The RegexMatch class represents the results of a regular expression match.
 * \remarks The RegexMatch object references the subject string as a std::string_view.
 *          The subject string must outlive the RegexMatch object.
 * \remarks Still experimental. Might be removed/adjusted in next minor release.
 */
class CPP_UTILITIES_EXPORT RegexMatch {
public:
    explicit RegexMatch() noexcept = default;
    explicit RegexMatch(std::string_view subject, std::vector<std::pair<std::size_t, std::size_t>> &&captureOffsets) noexcept;
    ~RegexMatch() = default;

    RegexMatch(const RegexMatch &other) = default;
    RegexMatch &operator=(const RegexMatch &other) = default;
    RegexMatch(RegexMatch &&other) noexcept = default;
    RegexMatch &operator=(RegexMatch &&other) noexcept = default;

    bool hasMatch() const noexcept;
    explicit operator bool() const noexcept;

    std::size_t captureCount() const noexcept;

    std::string_view subject() const noexcept;
    std::string_view captured(std::size_t index = 0) const;
    std::string_view operator[](std::size_t index) const;
    std::string capturedString(std::size_t index = 0) const;

    std::size_t capturedStart(std::size_t index = 0) const noexcept;
    std::size_t capturedEnd(std::size_t index = 0) const noexcept;
    std::size_t capturedLength(std::size_t index = 0) const noexcept;

    bool hasCaptured(std::size_t index = 0) const noexcept;

private:
    std::string_view m_subject;
    std::vector<std::pair<std::size_t, std::size_t>> m_captureOffsets;
};

/*!
 * \brief Returns whether a match was found.
 */
inline bool RegexMatch::hasMatch() const noexcept
{
    return !m_captureOffsets.empty() && m_captureOffsets[0].first != std::string_view::npos;
}

/*!
 * \brief Returns whether a match was found.
 */
inline RegexMatch::operator bool() const noexcept
{
    return hasMatch();
}

/*!
 * \brief Returns the number of captures, including the overall match at index 0.
 */
inline std::size_t RegexMatch::captureCount() const noexcept
{
    return m_captureOffsets.size();
}

/*!
 * \brief Returns the subject string that was matched against.
 */
inline std::string_view RegexMatch::subject() const noexcept
{
    return m_subject;
}

/*!
 * \brief Returns whether the capture group at the specified \a index was captured.
 */
inline bool RegexMatch::hasCaptured(std::size_t index) const noexcept
{
    return index < m_captureOffsets.size() && m_captureOffsets[index].first != std::string_view::npos;
}

/*!
 * \brief Returns the captured substring at the specified \a index.
 */
inline std::string_view RegexMatch::operator[](std::size_t index) const
{
    return captured(index);
}

/*!
 * \brief Returns the captured substring at the specified \a index as a std::string.
 */
inline std::string RegexMatch::capturedString(std::size_t index) const
{
    return std::string(captured(index));
}

/*!
 * \brief Returns the start offset of the captured substring at the specified \a index.
 */
inline std::size_t RegexMatch::capturedStart(std::size_t index) const noexcept
{
    return hasCaptured(index) ? m_captureOffsets[index].first : std::string_view::npos;
}

/*!
 * \brief Returns the end offset of the captured substring at the specified \a index.
 */
inline std::size_t RegexMatch::capturedEnd(std::size_t index) const noexcept
{
    return hasCaptured(index) ? m_captureOffsets[index].second : std::string_view::npos;
}

/*!
 * \brief Returns the length of the captured substring at the specified \a index.
 */
inline std::size_t RegexMatch::capturedLength(std::size_t index) const noexcept
{
    return hasCaptured(index) ? (m_captureOffsets[index].second - m_captureOffsets[index].first) : 0;
}

/*!
 * \class Regex
 * \brief The Regex class represents a compiled regular expression.
 * \remarks Still experimental. Might be removed/adjusted in next minor release.
 */
class CPP_UTILITIES_EXPORT Regex {
public:
    explicit Regex();
    explicit Regex(std::string_view pattern, RegexCompileOptions options = RegexCompileOptions::None);
    ~Regex();

    Regex(const Regex &other);
    Regex &operator=(const Regex &other);
    Regex(Regex &&other) noexcept;
    Regex &operator=(Regex &&other) noexcept;

    void compile(std::string_view pattern, RegexCompileOptions options = RegexCompileOptions::None);
    void clear() noexcept;

    bool isValid() const noexcept;
    explicit operator bool() const noexcept;

    std::string_view pattern() const noexcept;
    RegexCompileOptions compileOptions() const noexcept;

    void setPattern(std::string_view pattern);
    void setCompileOptions(RegexCompileOptions options);

    RegexMatch match(std::string_view subject, std::size_t offset = 0, RegexMatchOptions options = RegexMatchOptions::None) const;
    std::vector<RegexMatch> matchAll(std::string_view subject, std::size_t offset = 0, RegexMatchOptions options = RegexMatchOptions::None) const;
    bool isMatch(std::string_view subject, std::size_t offset = 0, RegexMatchOptions options = RegexMatchOptions::None) const;

private:
    struct Impl;
    std::string m_pattern;
    RegexCompileOptions m_compileOptions = RegexCompileOptions::None;
    std::unique_ptr<Impl> m_impl;
};

} // namespace CppUtilities

CPP_UTILITIES_MARK_FLAG_ENUM_CLASS(CppUtilities, RegexCompileOptions)
CPP_UTILITIES_MARK_FLAG_ENUM_CLASS(CppUtilities, RegexMatchOptions)

#endif // CPP_UTILITIES_REGEX_H
