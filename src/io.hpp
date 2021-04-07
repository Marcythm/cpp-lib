#pragma once
#include "config.hpp"

namespace __cpplib {

using namespace __config;

namespace IO {
    // configs
    namespace Config {
        constexpr i32 BUFSIZE = 131072;
        constexpr bool DEAL_WITH_EOF = false;
        constexpr bool MULTI_SIGN = true;
        constexpr bool DEAL_WITH_PARSE_FAILURE = false;
    }

    namespace Utility {
        template <size_t TABLESIZE, char... TrueIndex>
        struct TruthTable {
            static_assert(((TrueIndex < TABLESIZE) && ...), "TrueIndex > TABLESIZE");

            bool value[TABLESIZE] = {false};

            constexpr TruthTable() {
                if constexpr (sizeof...(TrueIndex) > 0) {
                    (value[TrueIndex] = ... = true);
                }
            }
            template <typename T>
            constexpr TruthTable(const std::initializer_list<T> &list): TruthTable() {
                for (const T &e: list)
                    value[static_cast<size_t>(e)] = true;
            }

            template <typename T>
            constexpr auto set(const T &index) -> void {
                value[static_cast<size_t>(index)] = true;
            }

            template <typename T>
            constexpr auto set(const std::initializer_list<T> &list) -> void {
                for (const T &e: list)
                    value[static_cast<size_t>(e)] = true;
            }

            template <typename T>
            constexpr auto operator() (const T &index) const -> bool { return value[static_cast<size_t>(index)]; }
        };
    }

    namespace Input {
        using Utility::TruthTable;

        static constexpr TruthTable<256> is_digit{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
        // static constexpr TruthTable<256, '+', '-', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'> is_digit_or_sign{};

        struct Lexer {
            FILE *inputfile = stdin;
            TruthTable<256> is_separator{' ', '\t', '\n', '\r', '\f', '\v'};

            char buf[Config::BUFSIZE];
            const char *ibuf = buf, *tbuf = buf;

            template <typename... Ts>
            Lexer(const Ts&... separators):
                is_separator{' ', '\t', '\n', '\r', '\f', '\v', static_cast<char>(separators)...} {}

            template <typename... Ts>
            Lexer(FILE *_inputfile, const Ts&... separators):
                inputfile{_inputfile}, is_separator{' ', '\t', '\n', '\r', '\f', '\v', static_cast<char>(separators)...} {}

            auto peek() -> char {
                if (ibuf < tbuf) {
                    return *ibuf;
                } else {
                    ibuf = buf;
                    tbuf = buf + std::fread(buf, 1, Config::BUFSIZE, inputfile);
                    if (ibuf != tbuf) {
                        return *ibuf;
                    }
                    if constexpr (Config::DEAL_WITH_EOF) {
                        // deal with EOF here
                        return '\0';
                    } else {
                        return EOF;
                    }
                }
            }

            auto consume() -> void {
                ++ibuf;
            }

            auto next() -> char {
                consume();
                return peek();
            }

            /// jump to next character which is not space
            auto consume_all_separators() -> void {
                while (is_separator(peek()))
                    consume();
            }
        };


        /// mode greedy: parse token until the given pattern be found
        /// mode once: return the default value when failed parsing into given type
        /// now on mode once
        struct Parser {
            Lexer *lexer;

            Parser(Lexer *_lexer): lexer{_lexer} {}
            ~Parser() = default;

            template <typename T>
            static constexpr T default_v = T();

            /// parse sign characters
            auto parse_sign() -> bool {
                bool neg = false;
                if constexpr (Config::MULTI_SIGN) {
                    for (char c = lexer->peek(); c == '-' or c == '+'; c = lexer->next()) {
                        if (c == '-') neg = not neg;
                    }
                    return neg;
                } else {
                    if (char c = lexer->peek(); c == '-') {
                        lexer->consume();
                        return true;
                    } else if (c == '+') {
                        lexer->consume();
                    }
                    return false;
                }
            }

            /// parse instantly, without ignoring white spaces
            /// return T{} if parse failed
            template <typename T>
            auto parse_unsigned_integral_type() -> T {
                static_assert(std::is_unsigned_v<T>, "in Parser::parse_unsigned_integral_type(): given type T is not an unsigned integral type");

                // parse digits
                if (char c = lexer->peek(); is_digit(c)) {
                    static constexpr T base = 10;
                    T s = static_cast<T>(c - '0');
                    while (is_digit(c = lexer->next())) {
                        s = static_cast<T>(s * base + (c - '0'));
                    }
                    return s;
                } else {
                    if constexpr (Config::DEAL_WITH_PARSE_FAILURE) {
                        // deal with parse failure
                    } else {
                        return default_v<T>;
                    }
                }
            }

            /// parse instantly, without ignoring white spaces
            /// return T{} if parse failed
            template <typename T>
            auto parse_signed_integral_type() -> T {
                static_assert(std::is_integral_v<T>, "in Parser::parse_signed_integral_type(): given type T is not an integral type");

                // parse sign characters
                if (parse_sign()) {
                    return -static_cast<T>(parse_unsigned_integral_type<std::make_unsigned_t<T>>());
                } else {
                    return static_cast<T>(parse_unsigned_integral_type<std::make_unsigned_t<T>>());
                }
            }

            /// parse instantly, without ignoring white spaces
            /// return 0 if parse failed
            template <typename T>
            auto parse_floating_point_type() -> T {
                static_assert(std::is_floating_point_v<T>, "in Parser::parse_floating_point_type(): given type T is not a floating point type");

                // parse sign characters
                bool neg = parse_sign();

                // parse digits before '.'
                T s = 0;
                if (char c = lexer->peek(); is_digit(c)) {
                    static constexpr T base = 10;

                    s = static_cast<T>(c - '0');
                    while (is_digit(c = lexer->next())) {
                        s = static_cast<T>(s * base + (c - '0'));
                    }
                } else {
                    if constexpr (Config::DEAL_WITH_PARSE_FAILURE) {
                        // deal with parse failure
                    }
                }

                if (char c = lexer->peek(); c == '.') {
                    // parse digits after '.'
                    static constexpr T Exp[] = {1e0, 1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7, 1e-8, 1e-9, 1e-10, 1e-11, 1e-12, 1e-13, 1e-14, 1e-15, 1e-16, 1e-17, -1e-18, 1e-19, 1e-20};

                    i32 exp = 0;
                    while (exp < 20 and is_digit(c = lexer->next())) {
                        s += Exp[++exp] * (c - '0');
                    }

                    // ignore digits after 1e-20
                    while (is_digit(lexer->peek())) {
                        lexer->consume();
                    }
                }

                return neg ? -s : s;
            }

            /// parse instantly, without ignoring white spaces
            /// return an empty string if parse failed
            auto parse_string() -> std::string {
                std::string str;
                if (char c = lexer->peek(); not (lexer->is_separator(c))) {
                    str.push_back(c);
                    while (not (lexer->is_separator(c = lexer->next()))) {
                        str.push_back(c);
                    }
                    return str;
                } else {
                    if constexpr (Config::DEAL_WITH_PARSE_FAILURE) {
                        // deal with parse failure
                    } else {
                        return str;
                    }
                }
            }

            inline auto parse_into_str(char *str) -> void {
                // static_assert(std::is_convertible_v<T, char*>, "in Parser::parse_into_str(): given type T can not be convert to char*");
                lexer->consume_all_separators();
                char* s = static_cast<char*>(str);
                *s++ = lexer->peek();
                while (not (lexer->is_separator(*s++ = lexer->next())));
                s[-1] = '\0';
            }

            template <typename T>
            auto parse_to() -> T {
                // jump to next character which is not space
                lexer->consume_all_separators();

                if constexpr (std::is_same_v<T, char> or std::is_same_v<T, unsigned char>) {
                    char c = lexer->peek();
                    lexer->consume();
                    return c;
                } else if constexpr (std::is_unsigned_v<T>) {
                    return parse_unsigned_integral_type<T>();
                } else if constexpr (std::is_integral_v<T>) {
                    return parse_signed_integral_type<T>();
                } else if constexpr (std::is_floating_point_v<T>) {
                    return parse_floating_point_type<T>();
                } else if constexpr (std::is_same_v<T, std::string>) {
                    return parse_string();
                } else {
                    static_assert(std::is_unsigned_v<T>, "in Parser::parse(): given type T is not supported");
                }
            }
        };
    }

    struct InputWrapper {
        Input::Lexer lexer{};
        Input::Parser parser{&lexer};

        template <typename... Ts>
        InputWrapper(const Ts&... separators):
            lexer{separators...} {}

        template <typename... Ts>
        InputWrapper(FILE *inputfile, const Ts&... separators):
            lexer{inputfile, separators...} {}

        template <typename T>
        operator T() {
            return parser.template parse_to<T>();
        }

        template <typename T>
        auto operator() () -> T {
            return parser.template parse_to<T>();
        }

        template <typename T, typename... Ts>
        auto operator() (T &&arg, Ts&&... args) -> void {
            if constexpr (std::is_convertible_v<T, char*>) {
                parser.parse_into_str(static_cast<char*>(arg));
            } else {
                arg = parser.template parse_to<std::remove_reference_t<T>>();
            }

            if constexpr (sizeof...(Ts) > 0) {
                this->operator()(std::forward<Ts&&>(args)...);
            }
        }
    };

    InputWrapper input{};
}

}
