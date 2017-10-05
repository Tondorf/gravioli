#pragma once


#include <tuple>


namespace httpserver {
    struct Request;

    class RequestParser {
    public:
        enum class ResultType {
            GOOD, BAD, INDETERMINATE
        };

    private:
        ResultType consume(Request&, char);

        static bool is_char(int);

        static bool is_ctl(int);

        static bool is_tspecial(int);

        static bool is_digit(int);

        enum class State {
            METHOD_START,
            METHOD,
            URI,
            HTTP_VERSION_H,
            HTTP_VERSION_T_1,
            HTTP_VERSION_T_2,
            HTTP_VERSION_P,
            HTTP_VERSION_SLASH,
            HTTP_VERSION_MAJOR_START,
            HTTP_VERSION_MAJOR,
            HTTP_VERSION_MINOR_START,
            HTTP_VERSION_MINOR,
            EXPECTING_NEWLINE_1,
            HEADER_LINE_START,
            HEADER_LWS,
            HEADER_NAME,
            SPACE_BEFORE_HEADER_VALUE,
            HEADER_VALUE,
            EXPECTING_NEWLINE_2,
            EXPECTING_NEWLINE_3
        } _state;

    public:
        RequestParser();

        virtual ~RequestParser() = default;

        virtual void reset();

        template <class T>
        std::tuple<ResultType, T> parse(Request& req, T begin, T end) {
            while (begin != end) {
                auto result = consume(req, *begin++);
                if (result == ResultType::GOOD || result == ResultType::BAD) {
                    return std::make_tuple(result, begin);
                }
            }

            return std::make_tuple(ResultType::INDETERMINATE, begin);
        }
    };
}
