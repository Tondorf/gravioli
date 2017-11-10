#include <cryptopp/aes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/modes.h>
#include "crypto.hpp"


namespace crypto {
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
