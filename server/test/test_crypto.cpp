#include <algorithm>

#include <gtest/gtest.h>

#include "testVector.hpp"

#include "../src/server/crypto.hpp"


constexpr std::size_t KEY_LENGTH = crypto::KEY_BLOCKSIZE;
constexpr std::size_t IV_LENGTH = crypto::IV_BLOCKSIZE;

constexpr crypto::Key KEY1 = {
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f
};

constexpr crypto::Key KEY2 = {
    0x01, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f
};

constexpr crypto::IV IV1 = {
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f
};

constexpr crypto::IV IV2 = {
    0x01, 0x01, 0x02, 0x03,
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


class EncryptionAndDecryption : public ::testing::Test {
public:
    std::string secrete;
    byte *msg;
    std::size_t msgLength;

    EncryptionAndDecryption() {
    }

    void SetUp() override {
        secrete = "secrete";
        msg = allocateWithoutTrailingZero(secrete, msgLength);
    }

    virtual ~EncryptionAndDecryption() {
        delete msg;
    }
};


TEST(Key, hasValidBlockSize) { 
    auto bs = crypto::KEY_BLOCKSIZE;
    ASSERT_EQ(KEY1.size(), bs);
}


TEST(Encryption, isNotIdentity) { 
    const std::string secrete = "secrete";

    std::size_t nChars;
    byte *msg = allocateWithoutTrailingZero(secrete, nChars);

    crypto::encrypt(msg, nChars, KEY1);
    ASSERT_FALSE(equal(msg, secrete));

    delete msg;
}


TEST_F(EncryptionAndDecryption, withSameKeysIsIdentity) { 
    auto iv = crypto::encrypt(msg, msgLength, KEY1);
    crypto::decrypt(msg, msgLength, KEY1, iv);

    ASSERT_TRUE(equal(msg, secrete));
}

TEST_F(EncryptionAndDecryption, withDifferentKeysIsNotIdentity) { 
    auto iv = crypto::encrypt(msg, msgLength, KEY1);
    crypto::decrypt(msg, msgLength, KEY2, iv);

    ASSERT_FALSE(equal(msg, secrete));
}

TEST_F(EncryptionAndDecryption, withCustomInitialVectorIsIdentity) { 
    crypto::encrypt(msg, msgLength, KEY1, IV1);
    crypto::decrypt(msg, msgLength, KEY1, IV1);

    ASSERT_TRUE(equal(msg, secrete));
}

TEST_F(EncryptionAndDecryption, withDifferentInitialVectorIsNotIdentity) { 
    crypto::encrypt(msg, msgLength, KEY1, IV1);
    crypto::decrypt(msg, msgLength, KEY1, IV2);

    ASSERT_FALSE(equal(msg, secrete));
}


void testTV(const std::string filename) {
    std::vector<TestVector::Test> tv;
    bool success = TestVector::parseRSP(filename, tv);
    ASSERT_TRUE(success);
    ASSERT_GE(tv.size(), 1);

    for (auto t : tv) {
        std::array<byte, KEY_LENGTH> key;
        std::copy_n(std::make_move_iterator(t.key.begin()), KEY_LENGTH, key.begin());

        std::array<byte, IV_LENGTH> iv;
        std::copy_n(std::make_move_iterator(t.iv.begin()), IV_LENGTH, iv.begin());

        auto msg = t.plain;
        auto nChars = msg.size();

        crypto::encrypt(&msg[0], nChars, key, iv);
        ASSERT_TRUE(msg == t.cipher);

        crypto::decrypt(&msg[0], nChars, key, iv);
        ASSERT_TRUE(msg == t.plain);
    }
}

std::vector<std::string> getTVFilenames(std::size_t keylength, std::size_t plainLength) {
    std::string path = "test/KAT_AES/";
    std::string prefix = std::to_string(plainLength * 8);
    std::string suffix = std::to_string(keylength * 8) + ".rsp";

    auto fname = [&](const std::string& method) {
        return path + "CFB" + prefix + method + suffix;
    };
    
    std::vector<std::string> filenames {
        fname("GFSbox"), 
        fname("KeySbox"), 
        fname("VarKey"), 
        fname("VarTxt")
    };

    return filenames;
}

TEST(TestVectorSetForEncryptionAndDecryption, valuesAreReproducable) { 
    for (auto fname : getTVFilenames(KEY_LENGTH, 1)) {
        SCOPED_TRACE(fname.c_str());
        testTV(fname);
    }

    for (auto fname : getTVFilenames(KEY_LENGTH, 16)) {
        SCOPED_TRACE(fname.c_str());
        testTV(fname);
    }
}
