#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <string_view>
#include <optional>

namespace lgt92::core {

enum class LoRaActivation : uint8_t {
    Otaa = 0,
    Abp = 1
};

enum class LoRaClass : uint8_t {
    ClassA = 0,
    ClassC = 1
};

struct LoRaCredentials {
    LoRaActivation activation{LoRaActivation::Otaa};
    LoRaClass device_class{LoRaClass::ClassA};

    // OTAA Credentials
    std::array<uint8_t, 8>  dev_eui{};
    std::array<uint8_t, 8>  app_eui{}; // Also known as join_eui
    std::array<uint8_t, 16> app_key{};

    // ABP Credentials
    uint32_t                dev_addr{0};
    std::array<uint8_t, 16> nwk_skey{};
    std::array<uint8_t, 16> app_skey{};

    [[nodiscard]] bool is_dev_eui_valid() const noexcept {
        for (auto b : dev_eui) {
            if (b != 0) return true;
        }
        return false;
    }

    [[nodiscard]] bool is_app_key_valid() const noexcept {
        for (auto b : app_key) {
            if (b != 0) return true;
        }
        return false;
    }

    static std::optional<std::array<uint8_t, 8>> parse_hex_8(std::string_view hex_str) {
        if (hex_str.size() != 16) return std::nullopt;
        std::array<uint8_t, 8> result{};
        for (size_t i = 0; i < 8; ++i) {
            auto hi = parse_hex_nibble(hex_str[i * 2]);
            auto lo = parse_hex_nibble(hex_str[i * 2 + 1]);
            if (!hi || !lo) return std::nullopt;
            result[i] = static_cast<uint8_t>((*hi << 4) | *lo);
        }
        return result;
    }

    static std::optional<std::array<uint8_t, 16>> parse_hex_16(std::string_view hex_str) {
        if (hex_str.size() != 32) return std::nullopt;
        std::array<uint8_t, 16> result{};
        for (size_t i = 0; i < 16; ++i) {
            auto hi = parse_hex_nibble(hex_str[i * 2]);
            auto lo = parse_hex_nibble(hex_str[i * 2 + 1]);
            if (!hi || !lo) return std::nullopt;
            result[i] = static_cast<uint8_t>((*hi << 4) | *lo);
        }
        return result;
    }

private:
    static constexpr std::optional<uint8_t> parse_hex_nibble(char c) noexcept {
        if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
        if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10);
        if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);
        return std::nullopt;
    }
};

} // namespace lgt92::core

