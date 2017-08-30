#pragma once

#include <array>

#include "config.hpp"


namespace crypto {
    constexpr std::size_t getKeyBlockSizeFromCryptoLevel() {
        if (CRYPTO_LEVEL == CryptoLevel::FAST) return 16;
        if (CRYPTO_LEVEL == CryptoLevel::HIGH) return 24;
        return 32;
    }
    constexpr std::size_t KEY_BLOCKSIZE = getKeyBlockSizeFromCryptoLevel();
    constexpr std::size_t IV_BLOCKSIZE = 16;

    using Key = std::array<byte, KEY_BLOCKSIZE>;
    using IV = std::array<byte, IV_BLOCKSIZE>;

    void encrypt(byte *, const std::size_t msgLength, const Key&, const IV&);

    IV encrypt(byte *, const std::size_t msgLength, const Key&);

    void decrypt(byte *, const std::size_t msgLength, const Key&, const IV&);
}
