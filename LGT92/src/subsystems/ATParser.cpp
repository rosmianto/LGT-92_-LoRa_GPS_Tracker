#include <ATParser.h>
#include <ctre.hpp>

// static constexpr std::string_view AT_REGEX = R"()";

ATParser::ATParser() {
    _parserOutput.clear();
}

std::string_view ATParser::parseCommand(std::string_view input) {
    _parserOutput.clear();

    // Because I'm a RegEx fanboy, we will
    // use CTRE library to extract AT base command, 
    // AT command types like '?' or '=' or '=?',
    // as well as the AT params.
    auto [match, _cmd, _op, _params] = ctre::match<"^AT(\\+[A-Z0-9]+)?(=\\?|\\?|=)?(.*)\\r\\n$">(input);

    if (!match) {
        return "ERROR\r\n";
    }

    std::string_view cmd    = _cmd.to_view();
    std::string_view op     = _op.to_view();
    std::string_view params = _params.to_view();

    // RegEx wizardry is done here. Now we lookup the table
    switch (hash_at(cmd)) {

        case ""_hash:  // Basic AT only
            _parserOutput += "OK\r\n";
            break;
        
        case "+ACE"_hash:
        case "+ADR"_hash:
        case "+APPEUI"_hash:
        case "+APPKEY"_hash:
        case "+APPSKEY"_hash:
        case "+BAT"_hash:
        case "+CAL"_hash:
        case "+CFG"_hash:
        case "+CFM"_hash:
        case "+CFS"_hash:
        case "+CHE"_hash:
        case "+CHS"_hash:
        case "+CLASS"_hash:
        case "+DADDR"_hash:
        case "+DCS"_hash:
        case "+DEBUG"_hash:
        case "+DEUI"_hash:
        case "+DR"_hash:
        case "+DWELLT"_hash:
        case "+FCD"_hash:
        case "+FCU"_hash:
        case "+FDR"_hash:
        case "+FTIME"_hash:
        case "+HWVER"_hash:
        case "+JN1DL"_hash:
        case "+JN2DL"_hash:
        case "+JOIN"_hash:
        case "+KAT"_hash:
        case "+LOGGPS"_hash:
        case "+LON"_hash:
        case "+MD"_hash:
        case "+MLON"_hash:
        case "+MOD"_hash:
        case "+NJM"_hash:
        case "+NJS"_hash:
        case "+NMEA353"_hash:
        case "+NMEA886"_hash:
        case "+NWKID"_hash:
        case "+NWKSKEY"_hash:
        case "+PDOP"_hash:
        case "+PNM"_hash:
        case "+PORT"_hash:
        case "+RECV"_hash:
        case "+RECVB"_hash:
        case "Z"_hash:
        case "+RJTDC"_hash:
        case "+RPL"_hash:
        case "+RSSI"_hash:
        case "+RX1DL"_hash:
        case "+RX1WTO"_hash:
        case "+RX2DL"_hash:
        case "+RX2DR"_hash:
        case "+RX2FQ"_hash:
        case "+RX2WTO"_hash:
        case "+SEND"_hash:
        case "+SENDB"_hash:
        case "+SGM"_hash:
        case "+SNR"_hash:
        case "+TDC"_hash:
        case "+TXP"_hash:
        case "+VER"_hash:

        default:
            _parserOutput += "ERROR\r\n";
            break;
    }


    return _parserOutput;
}
