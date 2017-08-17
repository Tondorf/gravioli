#pragma once

#include <fstream>
#include <string>
#include <vector>


class TestVector {
public:
    using byte = unsigned char;

    struct Test {
        std::vector<byte> key;
        std::vector<byte> iv;
        std::vector<byte> plain;
        std::vector<byte> cipher;
    };

private:
    static std::vector<byte> getFromString(const std::string& line, const int pos) {
        auto value = line.substr(pos + 3);
        const std::size_t valueLength = value.length() - 1;

        std::vector<byte> bytes;
        char lastDigit;
        for (std::size_t i = 0; i < valueLength; ++i) {
            char digit = [](char c) {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                throw std::invalid_argument("Value is not hex.");
            }(value[i]);

            if (i % 2 == 0) {
                lastDigit = digit;
            } else {
                bytes.push_back(static_cast<byte>(lastDigit * 16 + digit));
            }
        }

        return bytes;
    }

public:
    static std::vector<Test> parseRSP(const std::string& filename) {
        std::vector<Test> tv;

        std::string line;
        std::ifstream f(filename);
        if (!f.is_open()) {
            return tv;
        }

        struct Keyword {
            const std::string name;
            const std::size_t length;

            Keyword(const std::string& s):
                name(s), length(s.length()) {
            }
        };
        Keyword kwKey("KEY");
        Keyword kwIV("IV");
        Keyword kwPlain("PLAINTEXT");
        Keyword kwCipher("CIPHERTEXT");

        auto startsWith = [](const std::string& line, Keyword kw) {
            if (line.length() < kw.length) return false;

            for (std::size_t i = 0; i < kw.length; ++i) {
                if (line[i] != kw.name[i]) return false;
            }

            return true;
        };

        Test t;
        while (getline(f, line)) {
            if (startsWith(line, kwKey)) {
                t.key = getFromString(line, kwKey.length);
            } else if (startsWith(line, kwIV)) {
                t.iv = getFromString(line, kwIV.length);
            } else if (startsWith(line, kwPlain)) {
                t.plain = getFromString(line, kwPlain.length);
            } else if (startsWith(line, kwCipher)) {
                t.cipher = getFromString(line, kwCipher.length);
                
                tv.push_back(t);
            }
        }

        f.close();

        return tv;
    }
};

