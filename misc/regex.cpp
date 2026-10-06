#include "./regex.h"

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include <array>
#include <utility>

using namespace CppUtilities;

namespace CppUtilities {

namespace {

std::string makeString(std::string_view sv) noexcept
{
    try {
        return std::string(sv);
    } catch (...) {
        return std::string();
    }
}

struct MatchDataDeleter {
    void operator()(pcre2_match_data *matchData) const noexcept
    {
        pcre2_match_data_free(matchData);
    }
};
using ScopedMatchData = std::unique_ptr<pcre2_match_data, MatchDataDeleter>;

std::uint32_t toPcre2CompileOptions(RegexCompileOptions options)
{
    auto pcre2Options = std::uint32_t();
    if ((options & RegexCompileOptions::CaseInsensitive) != RegexCompileOptions::None) {
        pcre2Options |= PCRE2_CASELESS;
    }
    if ((options & RegexCompileOptions::Multiline) != RegexCompileOptions::None) {
        pcre2Options |= PCRE2_MULTILINE;
    }
    if ((options & RegexCompileOptions::DotAll) != RegexCompileOptions::None) {
        pcre2Options |= PCRE2_DOTALL;
    }
    if ((options & RegexCompileOptions::Extended) != RegexCompileOptions::None) {
        pcre2Options |= PCRE2_EXTENDED;
    }
    if ((options & RegexCompileOptions::Ungreedy) != RegexCompileOptions::None) {
        pcre2Options |= PCRE2_UNGREEDY;
    }
    if ((options & RegexCompileOptions::DollarEndOnly) != RegexCompileOptions::None) {
        pcre2Options |= PCRE2_DOLLAR_ENDONLY;
    }
    if ((options & RegexCompileOptions::Utf) != RegexCompileOptions::None) {
        pcre2Options |= PCRE2_UTF | PCRE2_UCP;
    }
    return pcre2Options;
}

std::uint32_t toPcre2MatchOptions(RegexMatchOptions options)
{
    auto pcre2Options = std::uint32_t();
    if ((options & RegexMatchOptions::NotBeginningOfLine) != RegexMatchOptions::None) {
        pcre2Options |= PCRE2_NOTBOL;
    }
    if ((options & RegexMatchOptions::NotEndOfLine) != RegexMatchOptions::None) {
        pcre2Options |= PCRE2_NOTEOL;
    }
    if ((options & RegexMatchOptions::NotEmpty) != RegexMatchOptions::None) {
        pcre2Options |= PCRE2_NOTEMPTY;
    }
    if ((options & RegexMatchOptions::NotEmptyAtStart) != RegexMatchOptions::None) {
        pcre2Options |= PCRE2_NOTEMPTY_ATSTART;
    }
    return pcre2Options;
}

} // namespace

/*!
 * \brief Constructs a new RegexException.
 */
RegexException::RegexException() noexcept
    : std::runtime_error("regular expression error")
{
}

/*!
 * \brief Constructs a new RegexException with the specified error \a message.
 */
RegexException::RegexException(std::string_view message) noexcept
    : std::runtime_error(makeString(message))
{
}

/*!
 * \brief Constructs a new RegexException with the specified error \a message and error \a offset.
 */
RegexException::RegexException(std::string_view message, std::size_t offset) noexcept
    : std::runtime_error(makeString(message))
    , m_offset(offset)
{
}

/*!
 * \brief Destroys the RegexException.
 */
RegexException::~RegexException()
{
}

/*!
 * \brief Returns the byte offset in the pattern where the error occurred.
 */
std::size_t RegexException::offset() const noexcept
{
    return m_offset;
}

/*!
 * \brief Constructs a new RegexMatch from the specified \a subject and \a captureOffsets.
 */
RegexMatch::RegexMatch(std::string_view subject, std::vector<std::pair<std::size_t, std::size_t>> &&captureOffsets) noexcept
    : m_subject(subject)
    , m_captureOffsets(std::move(captureOffsets))
{
}

/*!
 * \brief Returns whether a match was found.
 */
bool RegexMatch::hasMatch() const noexcept
{
    return !m_captureOffsets.empty() && m_captureOffsets[0].first != std::string_view::npos;
}

/*!
 * \brief Returns whether a match was found.
 */
RegexMatch::operator bool() const noexcept
{
    return hasMatch();
}

/*!
 * \brief Returns the number of captures, including the overall match at index 0.
 */
std::size_t RegexMatch::captureCount() const noexcept
{
    return m_captureOffsets.size();
}

/*!
 * \brief Returns the subject string that was matched against.
 */
std::string_view RegexMatch::subject() const noexcept
{
    return m_subject;
}

/*!
 * \brief Returns whether the capture group at the specified \a index was captured.
 */
bool RegexMatch::hasCaptured(std::size_t index) const noexcept
{
    return index < m_captureOffsets.size() && m_captureOffsets[index].first != std::string_view::npos;
}

/*!
 * \brief Returns the captured substring at the specified \a index.
 */
std::string_view RegexMatch::captured(std::size_t index) const
{
    if (!hasCaptured(index)) {
        return std::string_view();
    }
    const auto &offsets = m_captureOffsets[index];
    return m_subject.substr(offsets.first, offsets.second - offsets.first);
}

/*!
 * \brief Returns the captured substring at the specified \a index.
 */
std::string_view RegexMatch::operator[](std::size_t index) const
{
    return captured(index);
}

/*!
 * \brief Returns the captured substring at the specified \a index as a std::string.
 */
std::string RegexMatch::capturedString(std::size_t index) const
{
    return std::string(captured(index));
}

/*!
 * \brief Returns the start offset of the captured substring at the specified \a index.
 */
std::size_t RegexMatch::capturedStart(std::size_t index) const noexcept
{
    if (!hasCaptured(index)) {
        return std::string_view::npos;
    }
    return m_captureOffsets[index].first;
}

/*!
 * \brief Returns the end offset of the captured substring at the specified \a index.
 */
std::size_t RegexMatch::capturedEnd(std::size_t index) const noexcept
{
    if (!hasCaptured(index)) {
        return std::string_view::npos;
    }
    return m_captureOffsets[index].second;
}

/*!
 * \brief Returns the length of the captured substring at the specified \a index.
 */
std::size_t RegexMatch::capturedLength(std::size_t index) const noexcept
{
    if (!hasCaptured(index)) {
        return 0;
    }
    return m_captureOffsets[index].second - m_captureOffsets[index].first;
}

struct Regex::Impl {
    explicit Impl(pcre2_code *code)
        : code(code)
    {
    }

    ~Impl()
    {
        if (code) {
            pcre2_code_free(code);
        }
    }

    Impl(const Impl &) = delete;
    Impl &operator=(const Impl &) = delete;
    Impl(Impl &&) = delete;
    Impl &operator=(Impl &&) = delete;

    pcre2_code *code = nullptr;
};

/*!
 * \brief Constructs an empty, uncompiled Regex.
 */
Regex::Regex()
    : m_impl(nullptr)
{
}

/*!
 * \brief Constructs a Regex and compiles the specified \a pattern with the given \a options.
 * \throws RegexException Thrown if compilation fails.
 */
Regex::Regex(std::string_view pattern, RegexCompileOptions options)
    : m_impl(nullptr)
{
    compile(pattern, options);
}

/*!
 * \brief Destroys the Regex, freeing allocated resources.
 */
Regex::~Regex() = default;

/*!
 * \brief Constructs a copy of the specified \a other Regex.
 */
Regex::Regex(const Regex &other)
{
    if (other.isValid()) {
        compile(other.m_pattern, other.m_compileOptions);
    }
}

/*!
 * \brief Assigns the specified \a other Regex to this object.
 */
Regex &Regex::operator=(const Regex &other)
{
    if (this != &other) {
        if (other.isValid()) {
            compile(other.m_pattern, other.m_compileOptions);
        } else {
            clear();
        }
    }
    return *this;
}

/*!
 * \brief Move-constructs a Regex from \a other.
 */
Regex::Regex(Regex &&other) noexcept = default;

/*!
 * \brief Move-assigns \a other to this Regex.
 */
Regex &Regex::operator=(Regex &&other) noexcept = default;

/*!
 * \brief Compiles the specified regular expression \a pattern with the given \a options.
 * \throws RegexException Thrown if compilation fails.
 */
void Regex::compile(std::string_view pattern, RegexCompileOptions options)
{
    auto errorCode = int();
    auto errorOffset = PCRE2_SIZE();
    auto code = pcre2_compile(reinterpret_cast<PCRE2_SPTR8>(pattern.data()), static_cast<PCRE2_SIZE>(pattern.size()), toPcre2CompileOptions(options),
        &errorCode, &errorOffset, nullptr);
    if (!code) {
        auto errorBuffer = std::array<PCRE2_UCHAR, 256>();
        pcre2_get_error_message(errorCode, errorBuffer.data(), errorBuffer.size());
        throw RegexException(reinterpret_cast<const char *>(errorBuffer.data()), static_cast<std::size_t>(errorOffset));
    }
    m_pattern = pattern;
    m_compileOptions = options;
    m_impl = std::make_unique<Impl>(code);
}

/*!
 * \brief Clears the compiled regular expression.
 */
void Regex::clear() noexcept
{
    m_impl.reset();
    m_pattern.clear();
    m_compileOptions = RegexCompileOptions::None;
}

/*!
 * \brief Returns whether the regular expression is compiled and valid.
 */
bool Regex::isValid() const noexcept
{
    return m_impl && m_impl->code != nullptr;
}

/*!
 * \brief Returns whether the regular expression is compiled and valid.
 */
Regex::operator bool() const noexcept
{
    return isValid();
}

/*!
 * \brief Returns the pattern string.
 */
std::string_view Regex::pattern() const noexcept
{
    return m_pattern;
}

/*!
 * \brief Returns the compilation options.
 */
RegexCompileOptions Regex::compileOptions() const noexcept
{
    return m_compileOptions;
}

/*!
 * \brief Sets and recompiles with the new \a pattern, retaining current compilation options.
 * \throws RegexException Thrown if compilation fails.
 */
void Regex::setPattern(std::string_view pattern)
{
    const auto options = m_compileOptions;
    compile(pattern, options);
}

/*!
 * \brief Sets new compilation \a options and recompiles with the current pattern.
 * \throws RegexException Thrown if compilation fails.
 */
void Regex::setCompileOptions(RegexCompileOptions options)
{
    if (isValid()) {
        compile(m_pattern, options);
    } else {
        m_compileOptions = options;
    }
}

/*!
 * \brief Matches the regular expression against the specified \a subject starting at \a offset.
 * \return A RegexMatch object containing the results.
 * \throws RegexException Thrown if matching fails due to an internal error.
 */
RegexMatch Regex::match(std::string_view subject, std::size_t offset, RegexMatchOptions options) const
{
    if (!isValid() || offset > subject.size()) {
        return RegexMatch();
    }
    auto matchData = ScopedMatchData(pcre2_match_data_create_from_pattern(m_impl->code, nullptr));
    if (!matchData) {
        throw RegexException("unable to allocate PCRE2 match data");
    }
    const auto rc = pcre2_match(m_impl->code, reinterpret_cast<PCRE2_SPTR8>(subject.data()), static_cast<PCRE2_SIZE>(subject.size()),
        static_cast<PCRE2_SIZE>(offset), toPcre2MatchOptions(options), matchData.get(), nullptr);
    if (rc == PCRE2_ERROR_NOMATCH) {
        return RegexMatch();
    }
    if (rc < 0) {
        auto errorBuffer = std::array<PCRE2_UCHAR, 256>();
        pcre2_get_error_message(rc, errorBuffer.data(), errorBuffer.size());
        throw RegexException(reinterpret_cast<const char *>(errorBuffer.data()));
    }
    const auto count = rc == 0 ? static_cast<std::size_t>(pcre2_get_ovector_count(matchData.get())) : static_cast<std::size_t>(rc);
    const auto ovector = pcre2_get_ovector_pointer(matchData.get());
    auto captureOffsets = std::vector<std::pair<std::size_t, std::size_t>>();
    captureOffsets.reserve(count);
    for (auto i = std::size_t(); i < count; ++i) {
        const auto start = ovector[2 * i];
        const auto end = ovector[2 * i + 1];
        if (start == PCRE2_UNSET || end == PCRE2_UNSET) {
            captureOffsets.emplace_back(std::string_view::npos, std::string_view::npos);
        } else {
            captureOffsets.emplace_back(static_cast<std::size_t>(start), static_cast<std::size_t>(end));
        }
    }
    return RegexMatch(subject, std::move(captureOffsets));
}

/*!
 * \brief Matches the regular expression repeatedly against \a subject, returning all matches.
 */
std::vector<RegexMatch> Regex::matchAll(std::string_view subject, std::size_t offset, RegexMatchOptions options) const
{
    auto matches = std::vector<RegexMatch>();
    if (!isValid() || offset > subject.size()) {
        return matches;
    }
    auto currentOffset = offset;
    while (currentOffset <= subject.size()) {
        auto m = match(subject, currentOffset, options);
        if (!m) {
            break;
        }
        const auto matchEnd = m.capturedEnd(0);
        matches.emplace_back(std::move(m));
        if (matchEnd > currentOffset) {
            currentOffset = matchEnd;
        } else {
            // zero-length match: advance by 1
            ++currentOffset;
        }
    }
    return matches;
}

/*!
 * \brief Returns whether the regular expression matches the specified \a subject starting at \a offset.
 * \throws RegexException Thrown if matching fails due to an internal error.
 */
bool Regex::isMatch(std::string_view subject, std::size_t offset, RegexMatchOptions options) const
{
    if (!isValid() || offset > subject.size()) {
        return false;
    }
    auto matchData = ScopedMatchData(pcre2_match_data_create_from_pattern(m_impl->code, nullptr));
    if (!matchData) {
        throw RegexException("unable to allocate PCRE2 match data");
    }
    const auto rc = pcre2_match(m_impl->code, reinterpret_cast<PCRE2_SPTR8>(subject.data()), static_cast<PCRE2_SIZE>(subject.size()),
        static_cast<PCRE2_SIZE>(offset), toPcre2MatchOptions(options), matchData.get(), nullptr);
    if (rc == PCRE2_ERROR_NOMATCH) {
        return false;
    }
    if (rc < 0) {
        auto errorBuffer = std::array<PCRE2_UCHAR, 256>();
        pcre2_get_error_message(rc, errorBuffer.data(), errorBuffer.size());
        throw RegexException(reinterpret_cast<const char *>(errorBuffer.data()));
    }
    return true;
}

} // namespace CppUtilities
