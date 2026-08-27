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
    auto [match, _cmd, _op, _params] = ctre::match<"^AT(\\+[A-Z0-9]+)?(=\\?|\\?|=)?(.*)$">(input);

    if (!match) {
        return "ERROR\r\n";
    }

    // std::string_view cmd    = "";
    // std::string_view op     = "";
    // std::string_view params = "";

    std::string_view cmd    = _cmd.to_view();
    std::string_view op     = _op.to_view();
    std::string_view params = _params.to_view();

    // Convert 'op' string into CommandType
    // TODO: Ugly as hell, but try to make it work first.
    // TODO: Could be a single function though.
    CommandType operation;
    if (op.length() == 2) {  // Which is =?
        operation = CommandType::Help;
    }
    else if (op.length() == 1) {
        if (op.starts_with('=') == true) {
            operation = CommandType::Set;
        }
        else if (op.starts_with('?') == true) {
            operation = CommandType::Get;
        }
    }
    else if (op.length() == 0) {
        operation = CommandType::Run;
    }

    // RegEx wizardry is done here. Now we lookup the table
    switch (hash_at(cmd)) {

        case ""_hash:  // Basic AT only
            _parserOutput += "OK\r\n";
            break;
        
        case "+ACE"_hash    : cmdHandler_ACE(operation, params);     break;
        case "+ADR"_hash    : cmdHandler_ADR(operation, params);     break;
        case "+APPEUI"_hash : cmdHandler_APPEUI(operation, params);  break;
        case "+APPKEY"_hash : cmdHandler_APPKEY(operation, params);  break;
        case "+APPSKEY"_hash: cmdHandler_APPSKEY(operation, params); break;
        case "+BAT"_hash    : cmdHandler_BAT(operation, params);     break;
        case "+CAL"_hash    : cmdHandler_CAL(operation, params);     break;
        case "+CFG"_hash    : cmdHandler_CFG(operation, params);     break;
        case "+CFM"_hash    : cmdHandler_CFM(operation, params);     break;
        case "+CFS"_hash    : cmdHandler_CFS(operation, params);     break;
        case "+CHE"_hash    : cmdHandler_CHE(operation, params);     break;
        case "+CHS"_hash    : cmdHandler_CHS(operation, params);     break;
        case "+CLASS"_hash  : cmdHandler_CLASS(operation, params);   break;
        case "+DADDR"_hash  : cmdHandler_DADDR(operation, params);   break;
        case "+DCS"_hash    : cmdHandler_DCS(operation, params);     break;
        case "+DEBUG"_hash  : cmdHandler_DEBUG(operation, params);   break;
        case "+DEUI"_hash   : cmdHandler_DEUI(operation, params);    break;
        case "+DR"_hash     : cmdHandler_DR(operation, params);      break;
        case "+DWELLT"_hash : cmdHandler_DWELLT(operation, params);  break;
        case "+FCD"_hash    : cmdHandler_FCD(operation, params);     break;
        case "+FCU"_hash    : cmdHandler_FCU(operation, params);     break;
        case "+FDR"_hash    : cmdHandler_FDR(operation, params);     break;
        case "+FTIME"_hash  : cmdHandler_FTIME(operation, params);   break;
        case "+HWVER"_hash  : cmdHandler_HWVER(operation, params);   break;
        case "+JN1DL"_hash  : cmdHandler_JN1DL(operation, params);   break;
        case "+JN2DL"_hash  : cmdHandler_JN2DL(operation, params);   break;
        case "+JOIN"_hash   : cmdHandler_JOIN(operation, params);    break;
        case "+KAT"_hash    : cmdHandler_KAT(operation, params);     break;
        case "+LOGGPS"_hash : cmdHandler_LOGGPS(operation, params);  break;
        case "+LON"_hash    : cmdHandler_LON(operation, params);     break;
        case "+MD"_hash     : cmdHandler_MD(operation, params);      break;
        case "+MLON"_hash   : cmdHandler_MLON(operation, params);    break;
        case "+MOD"_hash    : cmdHandler_MOD(operation, params);     break;
        case "+NJM"_hash    : cmdHandler_NJM(operation, params);     break;
        case "+NJS"_hash    : cmdHandler_NJS(operation, params);     break;
        case "+NMEA353"_hash: cmdHandler_NMEA353(operation, params); break;
        case "+NMEA886"_hash: cmdHandler_NMEA886(operation, params); break;
        case "+NWKID"_hash  : cmdHandler_NWKID(operation, params);   break;
        case "+NWKSKEY"_hash: cmdHandler_NWKSKEY(operation, params); break;
        case "+PDOP"_hash   : cmdHandler_PDOP(operation, params);    break;
        case "+PNM"_hash    : cmdHandler_PNM(operation, params);     break;
        case "+PORT"_hash   : cmdHandler_PORT(operation, params);    break;
        case "+RECV"_hash   : cmdHandler_RECV(operation, params);    break;
        case "+RECVB"_hash  : cmdHandler_RECVB(operation, params);   break;
        case "+RJTDC"_hash  : cmdHandler_RJTDC(operation, params);   break;
        case "+RPL"_hash    : cmdHandler_RPL(operation, params);     break;
        case "+RSSI"_hash   : cmdHandler_RSSI(operation, params);    break;
        case "+RX1DL"_hash  : cmdHandler_RX1DL(operation, params);   break;
        case "+RX1WTO"_hash : cmdHandler_RX1WTO(operation, params);  break;
        case "+RX2DL"_hash  : cmdHandler_RX2DL(operation, params);   break;
        case "+RX2DR"_hash  : cmdHandler_RX2DR(operation, params);   break;
        case "+RX2FQ"_hash  : cmdHandler_RX2FQ(operation, params);   break;
        case "+RX2WTO"_hash : cmdHandler_RX2WTO(operation, params);  break;
        case "+SEND"_hash   : cmdHandler_SEND(operation, params);    break;
        case "+SENDB"_hash  : cmdHandler_SENDB(operation, params);   break;
        case "+SGM"_hash    : cmdHandler_SGM(operation, params);     break;
        case "+SNR"_hash    : cmdHandler_SNR(operation, params);     break;
        case "+TDC"_hash    : cmdHandler_TDC(operation, params);     break;
        case "+TXP"_hash    : cmdHandler_TXP(operation, params);     break;
        case "+VER"_hash    : cmdHandler_VER(operation, params);     break;
        case "Z"_hash       : cmdHandler_Z(operation, params);       break;

        default:
            _parserOutput += "ERROR\r\n";
            break;
    }

    return _parserOutput;
}

// Alarm Cycle what?
void ATParser::cmdHandler_ACE(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

// Adaptive Data Rate
void ATParser::cmdHandler_ADR(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

// LoRaWAN App EUI
void ATParser::cmdHandler_APPEUI(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

// LoRaWAN App Key
void ATParser::cmdHandler_APPKEY(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

// LoRaWAN App SKey
void ATParser::cmdHandler_APPSKEY(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

// Battery level
void ATParser::cmdHandler_BAT(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

// Perform Sensor Calibration
void ATParser::cmdHandler_CAL(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_CFG(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_CFM(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_CFS(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_CHE(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_CHS(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_CLASS(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_DADDR(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_DCS(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_DEBUG(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_DEUI(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_DR(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_DWELLT(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_FCD(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_FCU(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_FDR(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_FTIME(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_HWVER(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_JN1DL(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_JN2DL(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_JOIN(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_KAT(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_LOGGPS(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_LON(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_MD(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_MLON(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_MOD(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_NJM(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_NJS(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_NMEA353(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_NMEA886(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_NWKID(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_NWKSKEY(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_PDOP(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_PNM(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_PORT(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_RECV(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_RECVB(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_RJTDC(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_RPL(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_RSSI(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_RX1DL(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_RX1WTO(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_RX2DL(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_RX2DR(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_RX2FQ(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_RX2WTO(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_SEND(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_SENDB(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_SGM(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_SNR(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_TDC(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_TXP(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

void ATParser::cmdHandler_VER(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}

// Reset the hardware
void ATParser::cmdHandler_Z(CommandType op, std::string_view params) {
    if (op == CommandType::Get) {

    }
    else if (op == CommandType::Set) {

    }
    else if (op == CommandType::Run) {

    }
    else if (op == CommandType::Help) {

    }
    else {
        _parserOutput += "ERROR\r\n";
    }
}
