#ifndef CPP_UTILITIES_MISC_SIGNINGKEYS_H
#define CPP_UTILITIES_MISC_SIGNINGKEYS_H

#include <array>
#include <string_view>

namespace CppUtilities {

/*!
 * \brief Contains signing keys for verifying releases via OpenSSL or stsigtool.
 * \remarks
 * - Checkout the header file `verification.h` which contains the function verifySignature() for
 *   verification with OpenSSL.
 * - Checkout the Syncthing library contained in Syncthing Tray for verification with stsigtool.
 * - The arrays may contain more than one key, e.g. when rotatiung keys and an old key is in progress
 *   of being phased out.
 * - The concrete type of the contained key variables will change when the number of keys changes.
 *   So do *not* use it in e.g. a function signature or member variable that will become part of
 *   some public API/ABI. The variable is mereley supposed to be passed to verifySignature().
 * - This function is experimental and might be changed in incompatible ways (API and ABI wise) or be completely removed
 *   in further minor/patch releases.
 */
namespace SigningKeys {

// clang-format off
constexpr auto stsigtool = std::array<std::string_view, 1>{
std::string_view(
R"(-----BEGIN EC PUBLIC KEY-----
MIGbMBAGByqGSM49AgEGBSuBBAAjA4GGAAQBzGxkQSS43eE4r+A7HjlcEch5apsn
fKOgJWaRE2TOD9dNoBO2RSaJEAzzOXg2BPMsiPdr+Ty99FZtX8fmIcgJHGoB3sE1
PmSOaw3YWAXrHUYslrVRJI4iYCLuT4qjFMHgmqvphEE/zGDZ5Tyu6FwVlSjCO4Yy
FdsjpzKV6nrX6EsK++o=
-----END EC PUBLIC KEY-----
)")
};

constexpr auto openssl = std::array<std::string_view, 1>{
std::string_view(
R"(-----BEGIN PUBLIC KEY-----
MIGbMBAGByqGSM49AgEGBSuBBAAjA4GGAAQAWJAn1E7ZE5Q6H69oaV5sqCIppJdg
4bXDan9dJv6GOg70/t7q2CvwcwUXhV4FvCZxCHo25+rWYINfqKU2Utul8koAx8tK
59ohfOzI63I+CC76GfX41uRGU0P5i6hS7o/hgBLiVXqT0FgS2BMfmnLMUvUjqnI2
YQM7C55/5BM5Vrblkow=
-----END PUBLIC KEY-----
)")
};
// clang-format on

} // namespace SigningKeys
} // namespace CppUtilities

#endif // CPP_UTILITIES_MISC_SIGNINGKEYS_H
