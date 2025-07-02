#ifndef CPP_UTILITIES_MISC_VERIFICATION_H
#define CPP_UTILITIES_MISC_VERIFICATION_H

#include "../conversion/stringconversion.h"

#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <string>
#include <string_view>

namespace CppUtilities {

namespace Detail {
/// \brief Initializes OpenSSL.
/// \remarks This function is an implementation detail and must not be called by users this library.
inline void initOpenSsl()
{
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
}

/// \brief Returns the current OpenSSL error.
/// \remarks This function is an implementation detail and must not be called by users this library.
inline std::string getOpenSslError()
{
    const auto errCode = ERR_get_error();
    if (errCode == 0) {
        return "unknown OpenSSL error";
    }
    auto buffer = std::array<char, 256>();
    ERR_error_string_n(errCode, buffer.data(), buffer.size());
    return std::string(buffer.data());
}

/// \brief Extracts the base64-encoded body from a PEM block.
/// \remarks This function is an implementation detail and must not be called by users of this library.
inline std::string extractPemBody(std::string_view pem, std::string_view header)
{
    auto body = std::string();
    auto begin = pem.find(header);
    if (begin == std::string_view::npos) {
        return body;
    }
    begin += header.size();

    auto end = pem.find("-----END", begin);
    if (end == std::string_view::npos) {
        return body;
    }

    body = std::string(pem.data() + begin, end - begin);
    body.erase(std::remove_if(body.begin(), body.end(), ::isspace), body.end());
    return body;
}

/// \brief Converts PEM-encoded signature into DER-encoded signature.
/// \remarks This function is an implementation detail and must not be called by users of this library.
inline std::string parsePemSignature(std::string_view pemSignature, std::pair<std::unique_ptr<std::uint8_t[]>, std::uint32_t> &decodedSignature)
{
    const auto pemSignatureBody = extractPemBody(pemSignature, "-----BEGIN SIGNATURE-----");
    if (pemSignatureBody.empty()) {
        return "invalid or missing PEM signature block";
    }
    try {
        decodedSignature = decodeBase64(pemSignatureBody.data(), static_cast<std::uint32_t>(pemSignatureBody.size()));
        return std::string();
    } catch (const ConversionException &e) {
        return "unable to decode PEM signature block";
    }
}

} // namespace Detail

/*!
 * \brief Verifies \a data with the specified public key \a publicKeyPem and signature \a signaturePem.
 * \returns Returns an empty string if \a data and \a signature are correct and an error message otherwise.
 * \remarks
 * - The digest algorithm is assumed to be SHA256.
 * - The key and signature must both be provided in PEM format.
 * - This function requires linking with the OpenSSL crypto library and will initialize OpenSSL.
 * - This function is experimental and might be changed in incompatible ways (API and ABI wise) or be completely removed
 *   in further minor/patch releases.
 *
 * A key pair for signing can be created with the following commands:
 * ```
 * openssl ecparam -name secp521r1 -genkey -noout -out release-signing-private-openssl-secp521r1.pem
 * openssl ec -in release-signing-private-openssl-secp521r1.pem -pubout > release-signing-public-openssl-secp521r1.pem
 * ```
 *
 * A signature can be created an verified using the following commands:
 * ```
 * openssl dgst -sha256 -sign release-signing-private-openssl-secp521r1.pem test_msg.txt > test_msg-secp521r1.txt.sig
 * openssl dgst -sha256 -verify release-signing-public-openssl-secp521r1.pem -signature test_msg-secp521r1.txt.sig test_msg.txt
 * ```
 *
 * The signature can be converted to the PEM format using the following commands:
 * ```
 * echo "-----BEGIN SIGNATURE-----" > test_msg-secp521r1.txt.sig.pem
 * cat test_msg-secp521r1.txt.sig | base64 -w 64 >> test_msg-secp521r1.txt.sig.pem
 * echo "-----END SIGNATURE-----" >> test_msg-secp521r1.txt.sig.pem
 * ```
 */
inline std::string verifySignature(std::string_view publicKeyPem, std::string_view signaturePem, std::string_view data)
{
    auto error = std::string();
    Detail::initOpenSsl();

    auto derSignature = std::pair<std::unique_ptr<std::uint8_t[]>, std::uint32_t>();
    if (error = Detail::parsePemSignature(signaturePem, derSignature); !error.empty()) {
        return error;
    }

    BIO *const keyBio = BIO_new_mem_buf(publicKeyPem.data(), static_cast<int>(publicKeyPem.size()));
    if (!keyBio) {
        return error = "BIO_new_mem_buf failed: " + Detail::getOpenSslError();
    }

    EVP_PKEY *const publicKey = PEM_read_bio_PUBKEY(keyBio, nullptr, nullptr, nullptr);
    BIO_free(keyBio);
    if (!publicKey) {
        return error = "PEM_read_bio_PUBKEY failed: " + Detail::getOpenSslError();
    }

    EVP_MD_CTX *const mdCtx = EVP_MD_CTX_new();
    if (!mdCtx) {
        EVP_PKEY_free(publicKey);
        return error = "EVP_MD_CTX_new failed: " + Detail::getOpenSslError();
    }

    if (EVP_DigestVerifyInit(mdCtx, nullptr, EVP_sha256(), nullptr, publicKey) != 1) {
        error = "EVP_DigestVerifyInit failed: " + Detail::getOpenSslError();
    } else if (EVP_DigestVerifyUpdate(mdCtx, data.data(), data.size()) != 1) {
        error = "EVP_DigestVerifyUpdate failed: " + Detail::getOpenSslError();
    } else {
        switch (EVP_DigestVerifyFinal(mdCtx, derSignature.first.get(), derSignature.second)) {
        case 0:
            error = "incorrect signature";
            break;
        case 1:
            break; // signature is correct
        default:
            error = "EVP_DigestVerifyFinal failed: " + Detail::getOpenSslError();
            break;
        }
    }

    EVP_MD_CTX_free(mdCtx);
    EVP_PKEY_free(publicKey);
    return error;
}

} // namespace CppUtilities

#endif // CPP_UTILITIES_MISC_VERIFICATION_H
