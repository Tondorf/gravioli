#pragma once

#include <crypto++/aes.h>
#include <crypto++/osrng.h>
#include <cryptopp/modes.h>

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


    void encrypt(byte *msg,
                 const std::size_t msgLength,
                 const Key& key,
                 const IV& iv) {
        CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption cfbEncryption(
            key.data(),
            key.size(),
            iv.data()
        );
        cfbEncryption.ProcessData(msg, msg, msgLength);
    }


    IV encrypt(byte *msg, const std::size_t msgLength, const Key& key) {
        IV iv;
        CryptoPP::AutoSeededRandomPool rnd;
        rnd.GenerateBlock(iv.data(), IV_BLOCKSIZE);

        encrypt(msg, msgLength, key, iv);

        return iv;
    }


    void decrypt(byte *msg,
                 const std::size_t msgLength,
                 const Key& key,
                 const IV& iv) {
        CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption cfbDecryption(
            key.data(),
            key.size(),
            iv.data()
        );
        cfbDecryption.ProcessData(msg, msg, msgLength);
    }
}
