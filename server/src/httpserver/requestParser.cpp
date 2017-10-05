#include "httpserver/requestParser.hpp"

#include "httpserver/request.hpp"


namespace httpserver {
    RequestParser::RequestParser(): _state(State::METHOD_START) {
    }


    void RequestParser::reset() {
        _state = State::METHOD_START;
    }


    RequestParser::ResultType RequestParser::consume(Request& req, char input) {
        auto getHttpVersionMinor = [&req,input]() {
            return req.http_version_minor * 10 + input - '0';
        };

        auto getHttpVersionMajor = [&req,input]() {
            return req.http_version_major * 10 + input - '0';
        };

        switch (_state) {
        case State::METHOD_START:
            if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
                return ResultType::BAD;
            } else {
                _state = State::METHOD;
                req.method.push_back(input);
                return ResultType::INDETERMINATE;
            }
        case State::METHOD:
            if (input == ' ') {
                _state = State::URI;
                return ResultType::INDETERMINATE;
            } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
                return ResultType::BAD;
            } else {
                req.method.push_back(input);
                return ResultType::INDETERMINATE;
            }
        case State::URI:
            if (input == ' ') {
                _state = State::HTTP_VERSION_H;
                return ResultType::INDETERMINATE;
            } else if (is_ctl(input)) {
                return ResultType::BAD;
            } else {
                req.uri.push_back(input);
                return ResultType::INDETERMINATE;
            }
        case State::HTTP_VERSION_H:
            if (input == 'H') {
                _state = State::HTTP_VERSION_T_1;
                return ResultType::INDETERMINATE;
            } else {
                return ResultType::BAD;
            }
        case State::HTTP_VERSION_T_1:
            if (input == 'T') {
                _state = State::HTTP_VERSION_T_2;
                return ResultType::INDETERMINATE;
            } else {
                return ResultType::BAD;
            }
        case State::HTTP_VERSION_T_2:
            if (input == 'T') {
                _state = State::HTTP_VERSION_P;
                return ResultType::INDETERMINATE;
            } else {
                return ResultType::BAD;
            }
        case State::HTTP_VERSION_P:
            if (input == 'P') {
                _state = State::HTTP_VERSION_SLASH;
                return ResultType::INDETERMINATE;
            } else {
                return ResultType::BAD;
            }
        case State::HTTP_VERSION_SLASH:
            if (input == '/') {
                req.http_version_major = 0;
                req.http_version_minor = 0;
                _state = State::HTTP_VERSION_MAJOR_START;
                return ResultType::INDETERMINATE;
            } else {
                return ResultType::BAD;
            }
        case State::HTTP_VERSION_MAJOR_START:
            if (is_digit(input)) {
                req.http_version_major = getHttpVersionMajor();
                _state = State::HTTP_VERSION_MAJOR;
                return ResultType::INDETERMINATE;
            } else {
                return ResultType::BAD;
            }
        case State::HTTP_VERSION_MAJOR:
            if (input == '.') {
                _state = State::HTTP_VERSION_MINOR_START;
                return ResultType::INDETERMINATE;
            } else if (is_digit(input)) {
                req.http_version_major = getHttpVersionMajor();
                return ResultType::INDETERMINATE;
            } else {
                return ResultType::BAD;
            }
        case State::HTTP_VERSION_MINOR_START:
            if (is_digit(input)) {
                req.http_version_minor = getHttpVersionMinor();
                _state = State::HTTP_VERSION_MINOR;
                return ResultType::INDETERMINATE;
            } else {
                return ResultType::BAD;
            }
        case State::HTTP_VERSION_MINOR:
            if (input == '\r') {
                _state = State::EXPECTING_NEWLINE_1;
                return ResultType::INDETERMINATE;
            } else if (is_digit(input)) {
                req.http_version_minor = getHttpVersionMinor();
                return ResultType::INDETERMINATE;
            } else {
                return ResultType::BAD;
            }
        case State::EXPECTING_NEWLINE_1:
            if (input == '\n') {
                _state = State::HEADER_LINE_START;
                return ResultType::INDETERMINATE;
            } else {
                return ResultType::BAD;
            }
        case State::HEADER_LINE_START:
            if (input == '\r') {
                _state = State::EXPECTING_NEWLINE_3;
                return ResultType::INDETERMINATE;
            } else if (!req.headers.empty() && 
                       (input == ' ' || input == '\t')) {
                _state = State::HEADER_LWS;
                return ResultType::INDETERMINATE;
            } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
                return ResultType::BAD;
            } else {
                req.headers.push_back(Header());
                req.headers.back().name.push_back(input);
                _state = State::HEADER_NAME;
                return ResultType::INDETERMINATE;
            }
        case State::HEADER_LWS:
            if (input == '\r') {
                _state = State::EXPECTING_NEWLINE_2;
                return ResultType::INDETERMINATE;
            } else if (input == ' ' || input == '\t') {
                return ResultType::INDETERMINATE;
            } else if (is_ctl(input)) {
                return ResultType::BAD;
            } else {
                _state = State::HEADER_VALUE;
                req.headers.back().value.push_back(input);
                return ResultType::INDETERMINATE;
            }
        case State::HEADER_NAME:
            if (input == ':') {
                _state = State::SPACE_BEFORE_HEADER_VALUE;
                return ResultType::INDETERMINATE;
            } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
                return ResultType::BAD;
            } else {
                req.headers.back().name.push_back(input);
                return ResultType::INDETERMINATE;
            }
        case State::SPACE_BEFORE_HEADER_VALUE:
            if (input == ' ') {
                _state = State::HEADER_VALUE;
                return ResultType::INDETERMINATE;
            } else {
                return ResultType::BAD;
            }
        case State::HEADER_VALUE:
            if (input == '\r') {
                _state = State::EXPECTING_NEWLINE_2;
                return ResultType::INDETERMINATE;
            } else if (is_ctl(input)) {
                return ResultType::BAD;
            } else {
                req.headers.back().value.push_back(input);
                return ResultType::INDETERMINATE;
            }
        case State::EXPECTING_NEWLINE_2:
            if (input == '\n') {
                _state = State::HEADER_LINE_START;
                return ResultType::INDETERMINATE;
            } else {
                return ResultType::BAD;
            }
        case State::EXPECTING_NEWLINE_3:
            return (input == '\n') ? ResultType::GOOD : ResultType::BAD;
        default:
            return ResultType::BAD;
        }
    }


    bool RequestParser::is_char(int c) {
        return c >= 0 && c <= 127;
    }


    bool RequestParser::is_ctl(int c) {
        return (c >= 0 && c <= 31) || (c == 127);
    }


    bool RequestParser::is_tspecial(int c) {
        switch (c) {
        case '(': case ')': case '<': case '>': case '@':
        case ',': case ';': case ':': case '\\': case '"':
        case '/': case '[': case ']': case '?': case '=':
        case '{': case '}': case ' ': case '\t':
            return true;
        default:
            return false;
        }
    }


    bool RequestParser::is_digit(int c) {
        return c >= '0' && c <= '9';
    }
}
