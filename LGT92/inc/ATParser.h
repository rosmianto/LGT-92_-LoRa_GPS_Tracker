#pragma once

#include <array>
#include <string_view>
#include <cstring>
#include <cstdint>


// Our approach to build this AT command parser is to use
// Hashing. Instead of matching the AT command one by one.

// We will (ab)use C++ constexpr feature.
// We use FNV-1a 32-bit string hashing.
// It's a cool algo.
constexpr uint32_t hash_at(std::string_view str) {
    uint32_t hash = 0x811C9DC5;
    for (char c : str) {
        hash ^= static_cast<uint32_t>(c);
        hash *= 0x01000193;
    }
    return hash;
}

constexpr uint32_t operator""_hash(const char *str, size_t len) {
    return hash_at(std::string_view(str, len));
}

// We define a custom datatype called ArrayString
// so it doesn't use heap or dynamic memory allocation.
template <size_t N>
class ArrayString {

public:
    std::array<char, N> _buf;
    size_t length = 0;

    void clear() { length = 0; }

    ArrayString& operator+=(std::string_view text) {
        // Silently ignore the rest of the text
        // if it's overflowing the buffer N.
        // I don't know what's the better way though.
        if (length + text.size() <= N) {
            std::memcpy(_buf.data() + length, text.data(), text.size());
            length += text.size();
        }
        return *this;
    }

    operator std::string_view() const {
        return std::string_view(_buf.data(), length);
    }
};

class ATParser {

public:
    ATParser();
    std::string_view parseCommand(std::string_view input);

private:
    ArrayString<100> _parserOutput;
};