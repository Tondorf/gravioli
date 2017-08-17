#pragma once

#include <crypto++/aes.h>
#include <crypto++/osrng.h>
#include <cryptopp/modes.h>

#include <array>

#include "config.hpp"


class Crypto {
public:
    static constexpr std::size_t
    KEY_BLOCKSIZE = (CRYPTO_LEVEL == CryptoLevel::FAST) ? 16 :
                    ((CRYPTO_LEVEL == CryptoLevel::HIGH) ? 24 : 32);
    static constexpr std::size_t IV_BLOCKSIZE = 16;

    using Key = std::array<byte, KEY_BLOCKSIZE>;
    using IV = std::array<byte, IV_BLOCKSIZE>;


    Crypto() = delete;


    static void encrypt(byte *msg, const std::size_t msgLength, const Key& key, const IV& iv) {
        CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption cfbEncryption(key.data(), key.size(), iv.data());
        cfbEncryption.ProcessData(msg, msg, msgLength);
    }


    static IV encrypt(byte *msg, const std::size_t msgLength, const Key& key) {
        IV iv;
        generateIV(iv);

        encrypt(msg, msgLength, key, iv);

        return iv;
    }


    static void decrypt(byte *msg, const std::size_t msgLength, const Key& key, const IV& iv) {
        CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption cfbDecryption(key.data(), key.size(), iv.data());
        cfbDecryption.ProcessData(msg, msg, msgLength);
    }

private:
    static void generateIV(IV& iv) {
        CryptoPP::AutoSeededRandomPool rnd;
        rnd.GenerateBlock(iv.data(), IV_BLOCKSIZE);
    }
};
