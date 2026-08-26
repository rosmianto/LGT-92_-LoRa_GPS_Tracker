#pragma once

#include <array>
#include <string_view>
#include <cstring>
#include <cstdint>

enum class CommandType {
    Get,
    Set,
    Run,
    Help
};

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

    // The entire AT command handlers
    void cmdHandler_ACE(CommandType op, std::string_view params);
    void cmdHandler_ADR(CommandType op, std::string_view params);
    void cmdHandler_APPEUI(CommandType op, std::string_view params);
    void cmdHandler_APPKEY(CommandType op, std::string_view params);
    void cmdHandler_APPSKEY(CommandType op, std::string_view params);
    void cmdHandler_BAT(CommandType op, std::string_view params);
    void cmdHandler_CAL(CommandType op, std::string_view params);
    void cmdHandler_CFG(CommandType op, std::string_view params);
    void cmdHandler_CFM(CommandType op, std::string_view params);
    void cmdHandler_CFS(CommandType op, std::string_view params);
    void cmdHandler_CHE(CommandType op, std::string_view params);
    void cmdHandler_CHS(CommandType op, std::string_view params);
    void cmdHandler_CLASS(CommandType op, std::string_view params);
    void cmdHandler_DADDR(CommandType op, std::string_view params);
    void cmdHandler_DCS(CommandType op, std::string_view params);
    void cmdHandler_DEBUG(CommandType op, std::string_view params);
    void cmdHandler_DEUI(CommandType op, std::string_view params);
    void cmdHandler_DR(CommandType op, std::string_view params);
    void cmdHandler_DWELLT(CommandType op, std::string_view params);
    void cmdHandler_FCD(CommandType op, std::string_view params);
    void cmdHandler_FCU(CommandType op, std::string_view params);
    void cmdHandler_FDR(CommandType op, std::string_view params);
    void cmdHandler_FTIME(CommandType op, std::string_view params);
    void cmdHandler_HWVER(CommandType op, std::string_view params);
    void cmdHandler_JN1DL(CommandType op, std::string_view params);
    void cmdHandler_JN2DL(CommandType op, std::string_view params);
    void cmdHandler_JOIN(CommandType op, std::string_view params);
    void cmdHandler_KAT(CommandType op, std::string_view params);
    void cmdHandler_LOGGPS(CommandType op, std::string_view params);
    void cmdHandler_LON(CommandType op, std::string_view params);
    void cmdHandler_MD(CommandType op, std::string_view params);
    void cmdHandler_MLON(CommandType op, std::string_view params);
    void cmdHandler_MOD(CommandType op, std::string_view params);
    void cmdHandler_NJM(CommandType op, std::string_view params);
    void cmdHandler_NJS(CommandType op, std::string_view params);
    void cmdHandler_NMEA353(CommandType op, std::string_view params);
    void cmdHandler_NMEA886(CommandType op, std::string_view params);
    void cmdHandler_NWKID(CommandType op, std::string_view params);
    void cmdHandler_NWKSKEY(CommandType op, std::string_view params);
    void cmdHandler_PDOP(CommandType op, std::string_view params);
    void cmdHandler_PNM(CommandType op, std::string_view params);
    void cmdHandler_PORT(CommandType op, std::string_view params);
    void cmdHandler_RECV(CommandType op, std::string_view params);
    void cmdHandler_RECVB(CommandType op, std::string_view params);
    void cmdHandler_RJTDC(CommandType op, std::string_view params);
    void cmdHandler_RPL(CommandType op, std::string_view params);
    void cmdHandler_RSSI(CommandType op, std::string_view params);
    void cmdHandler_RX1DL(CommandType op, std::string_view params);
    void cmdHandler_RX1WTO(CommandType op, std::string_view params);
    void cmdHandler_RX2DL(CommandType op, std::string_view params);
    void cmdHandler_RX2DR(CommandType op, std::string_view params);
    void cmdHandler_RX2FQ(CommandType op, std::string_view params);
    void cmdHandler_RX2WTO(CommandType op, std::string_view params);
    void cmdHandler_SEND(CommandType op, std::string_view params);
    void cmdHandler_SENDB(CommandType op, std::string_view params);
    void cmdHandler_SGM(CommandType op, std::string_view params);
    void cmdHandler_SNR(CommandType op, std::string_view params);
    void cmdHandler_TDC(CommandType op, std::string_view params);
    void cmdHandler_TXP(CommandType op, std::string_view params);
    void cmdHandler_VER(CommandType op, std::string_view params);
    void cmdHandler_Z(CommandType op, std::string_view params);

};