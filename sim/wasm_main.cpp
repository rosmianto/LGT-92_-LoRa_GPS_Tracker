#include "core/TrackerEngine.hpp"
#include "WasmAdapters.hpp"
#include <emscripten.h>
#include <memory>
#include <sstream>
#include <iomanip>

namespace lgt92::sim {

bool WasmLoRaWan::send(uint8_t port, std::span<const uint8_t> payload, bool confirmed) {
    std::ostringstream hex_stream;
    for (uint8_t byte : payload) {
        hex_stream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    std::string hex_str = hex_stream.str();

    MAIN_THREAD_EM_ASM({
        if (window.onLoRaPacket) {
            window.onLoRaPacket($0, UTF8ToString($1), $2);
        }
    }, port, hex_str.c_str(), confirmed ? 1 : 0);

    return true;
}

void WasmUi::set_led(interfaces::LedColor color, bool on) {
    MAIN_THREAD_EM_ASM({
        if (window.onLedSet) {
            window.onLedSet($0, $1);
        }
    }, static_cast<int>(color), on ? 1 : 0);
}

void WasmUi::blink_led(interfaces::LedColor color, uint16_t on_ms) {
    MAIN_THREAD_EM_ASM({
        if (window.onLedBlink) {
            window.onLedBlink($0, $1);
        }
    }, static_cast<int>(color), on_ms);
}

static std::unique_ptr<WasmGps> s_gps;
static std::unique_ptr<WasmImu> s_imu;
static std::unique_ptr<WasmBattery> s_battery;
static std::unique_ptr<WasmLoRaWan> s_lora;
static std::unique_ptr<WasmStorage> s_storage;
static std::unique_ptr<WasmTimer> s_timer;
static std::unique_ptr<WasmUi> s_ui;
static std::unique_ptr<core::TrackerEngine> s_engine;

static void main_loop() {
    if (s_engine) {
        s_engine->step();
    }
}

} // namespace lgt92::sim

extern "C" {

EMSCRIPTEN_KEEPALIVE
void sim_init() {
    using namespace lgt92::sim;
    s_gps = std::make_unique<WasmGps>();
    s_imu = std::make_unique<WasmImu>();
    s_battery = std::make_unique<WasmBattery>();
    s_lora = std::make_unique<WasmLoRaWan>();
    s_storage = std::make_unique<WasmStorage>();
    s_timer = std::make_unique<WasmTimer>();
    s_ui = std::make_unique<WasmUi>();

    s_engine = std::make_unique<lgt92::core::TrackerEngine>(
        *s_gps, *s_imu, *s_battery, *s_lora, *s_storage, *s_timer, *s_ui
    );

    s_engine->init();

    // Run main loop at 10 Hz in browser
    emscripten_set_main_loop(main_loop, 10, 1);
}

EMSCRIPTEN_KEEPALIVE
void sim_press_alarm() {
    if (lgt92::sim::s_ui) {
        lgt92::sim::s_ui->press_alarm();
    }
}

EMSCRIPTEN_KEEPALIVE
void sim_trigger_motion() {
    if (lgt92::sim::s_imu && lgt92::sim::s_engine) {
        lgt92::sim::s_imu->trigger_motion();
        lgt92::sim::s_engine->on_motion_detected();
    }
}

EMSCRIPTEN_KEEPALIVE
void sim_set_gps(double lat, double lon, float alt) {
    if (lgt92::sim::s_gps) {
        lgt92::sim::s_gps->set_coordinates(lat, lon, alt);
    }
}

EMSCRIPTEN_KEEPALIVE
void sim_set_battery(int mv) {
    if (lgt92::sim::s_battery) {
        lgt92::sim::s_battery->set_voltage(static_cast<uint16_t>(mv));
    }
}

EMSCRIPTEN_KEEPALIVE
void sim_send_downlink(int port, const char* hex_str) {
    if (!lgt92::sim::s_lora || !hex_str) return;

    std::vector<uint8_t> bytes;
    std::string_view sv(hex_str);
    for (size_t i = 0; i + 1 < sv.size(); i += 2) {
        auto hi_char = sv[i];
        auto lo_char = sv[i + 1];
        auto parse_nibble = [](char c) -> uint8_t {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return 0;
        };
        bytes.push_back(static_cast<uint8_t>((parse_nibble(hi_char) << 4) | parse_nibble(lo_char)));
    }

    lgt92::sim::s_lora->inject_downlink(static_cast<uint8_t>(port), bytes);
}

} // extern "C"
