#include <algorithm>

#include <gtest/gtest.h>

#include "testVector.hpp"

#include "../src/server/crypto.hpp"


constexpr crypto::Key TEST_KEY = {
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f
};


byte *allocateWithoutTrailingZero(const std::string msg, std::size_t& nChars) {
    nChars = msg.length();

    byte *chars = new byte[nChars];
    for (std::size_t i = 0; i < nChars; ++i) {
        chars[i] = static_cast<byte>(msg[i]);
    }

    return chars;
}


bool equal(byte *a, char *b, const std::size_t size) {
    for (std::size_t i = 0; i < size; ++i) {
        if (a[i] != static_cast<byte>(b[i])) {
            return false;
        }
    }

    return true;
}

bool equal(byte *a, std::string b) {
    return equal(a, &b[0], b.length());
}


TEST(Key, hasValidBlockSize) { 
    auto bs = crypto::KEY_BLOCKSIZE;
    ASSERT_EQ(TEST_KEY.size(), bs);
}


TEST(Encryption, isNotIdentity) { 
    const std::string secrete = "secrete";

    std::size_t nChars;
    byte *msg = allocateWithoutTrailingZero(secrete, nChars);

    crypto::encrypt(msg, nChars, TEST_KEY);
    ASSERT_FALSE(equal(msg, secrete));

    delete msg;
}


TEST(EncryptionAndDecryption, withSameKeysIsIdentity) { 
    const std::string secrete = "secrete";

    std::size_t nChars;
    byte *msg = allocateWithoutTrailingZero(secrete, nChars);

    auto iv = crypto::encrypt(msg, nChars, TEST_KEY);
    crypto::decrypt(msg, nChars, TEST_KEY, iv);

    ASSERT_TRUE(equal(msg, secrete));

    delete msg;
}

TEST(EncryptionAndDecryption, withDifferentKeysIsNotIdentity) { 
    const std::string secrete = "secrete";

    std::size_t nChars;
    byte *msg = allocateWithoutTrailingZero(secrete, nChars);

    constexpr crypto::Key OTHER_KEY = {
        0x01, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f
    };

    auto iv = crypto::encrypt(msg, nChars, TEST_KEY);
    crypto::decrypt(msg, nChars, OTHER_KEY, iv);

    ASSERT_FALSE(equal(msg, secrete));

    delete msg;
}

TEST(EncryptionAndDecryption, withDifferentInitialVectorIsNotIdentity) { 
    const std::string secrete = "secrete";

    std::size_t nChars;
    byte *msg = allocateWithoutTrailingZero(secrete, nChars);

    constexpr crypto::Key IV1 = {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f
    };

    constexpr crypto::Key IV2 = {
        0x01, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f
    };

    crypto::encrypt(msg, nChars, TEST_KEY, IV1);
    crypto::decrypt(msg, nChars, TEST_KEY, IV2);

    ASSERT_FALSE(equal(msg, secrete));

    delete msg;
}

TEST(EncryptionAndDecryption, matchValuesOfTestVector) { 
    constexpr std::size_t KEY_LENGTH = crypto::KEY_BLOCKSIZE;
    constexpr std::size_t IV_LENGTH = crypto::IV_BLOCKSIZE;
    constexpr std::size_t PLAIN_LENGTH = 16;

    std::vector<std::string> prefixes {
        "CFB128GFSbox",
        "CFB128KeySbox",
        "CFB128VarKey",
        "CFB128VarTxt"
    };

    std::string suffix = std::to_string(KEY_LENGTH * 8) + ".rsp";
    for (auto prefix : prefixes) {
        std::string filename = "test/KAT_AES/";
        filename += prefix;
        filename += suffix;

        auto tv = TestVector::parseRSP(filename);
        ASSERT_GE(tv.size(), 1);

        for (auto t : tv) {
            std::array<byte, KEY_LENGTH> key;
            std::copy_n(std::make_move_iterator(t.key.begin()), KEY_LENGTH, key.begin());

            std::array<byte, IV_LENGTH> iv;
            std::copy_n(std::make_move_iterator(t.iv.begin()), IV_LENGTH, iv.begin());

            auto msg = t.plain;

            crypto::encrypt(&msg[0], PLAIN_LENGTH, key, iv);
            ASSERT_TRUE(msg == t.cipher);

            crypto::decrypt(&msg[0], PLAIN_LENGTH, key, iv);
            ASSERT_TRUE(msg == t.plain);
        }
    }
}
