#pragma once

#include <fstream>
#include <iostream>
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
    static std::vector<byte> getFromString(const std::string& line,
                                           const int pos) {
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
    static bool parseRSP(const std::string& filename, std::vector<Test> &tv) {
        std::string line;
        std::ifstream f(filename);
        if (!f.is_open()) {
            std::cerr << "Could not open file " << filename << std::endl;
            return false;
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
        Keyword kwCounter("COUNT");

        auto startsWith = [](const std::string& line, Keyword kw) {
            if (line.length() < kw.length) return false;

            for (std::size_t i = 0; i < kw.length; ++i) {
                if (line[i] != kw.name[i]) return false;
            }

            return true;
        };

        Test t;
        int counter = -1;
        int offset = 0;
        while (getline(f, line)) {
            try {
                if (startsWith(line, kwKey)) {
                    t.key = getFromString(line, kwKey.length);
                } else if (startsWith(line, kwIV)) {
                    t.iv = getFromString(line, kwIV.length);
                } else if (startsWith(line, kwPlain)) {
                    t.plain = getFromString(line, kwPlain.length);
                } else if (startsWith(line, kwCipher)) {
                    t.cipher = getFromString(line, kwCipher.length);

                    /*
                     * New Test is not yet pushed, but COUNT starts with 0,
                     * hence counter == tv.size() is expected.
                     */
                    if (counter < 0 ||
                        static_cast<std::size_t>(counter) != tv.size()) {
                        std::cerr << "Invalid counter" << std::endl;
                        return false;
                    }
                    
                    tv.push_back(t);
                } else if (startsWith(line, kwCounter)) {
                    auto newCounter = std::stoi(
                        line.substr(kwCounter.length + 3)
                    );

                    // COUNT is reset to zero after transition, i.e.
                    // [ENCRYPT] -> [DECRYPT]
                    if (newCounter == 0 && counter >= 0) {
                        offset = counter + 1;
                    }

                    counter = newCounter;
                    counter += offset;
                }
            } catch (...) {
                f.close();

                std::cerr << "Error during line parsing." << std::endl;
                return false;
            }
        }

        f.close();

        return true;
    }
};
