#pragma once

// clang-format off
// Downlink Command Definitions
#define DOWNLINK_CMD_SET_TRANSMIT_INTERVAL       0x01
#define DOWNLINK_CMD_EXIT_ALARM_MODE             0x02
#define DOWNLINK_CMD_DEVICE_TRIGGER              0x04 // reset and factory reset
#define DOWNLINK_CMD_CONFIRM_MODE                0x05 // two types of cfm
#define DOWNLINK_CMD_NETWORK_JOINMODE            0x20
#define DOWNLINK_CMD_PACKET_RESPONSE_LEVEL       0x21
#define DOWNLINK_CMD_ADAPTIVE_DATARATE           0x22
#define DOWNLINK_CMD_APP_PORT                    0x23
#define DOWNLINK_CMD_EIGHT_CH_MODE               0x24
#define DOWNLINK_CMD_DWELLTIME                   0x25
#define DOWNLINK_CMD_FIRMWARE_VERSION            0x26
#define DOWNLINK_CMD_NETWORK_REJOINING           0x26
#define DOWNLINK_CMD_TIMESYNC                    0x28
#define DOWNLINK_CMD_ONLINE_DETECT               0x32
#define DOWNLINK_CMD_NBTRANS_MAX                 0x33
#define DOWNLINK_CMD_ACK_MODE                    0x34
#define DOWNLINK_CMD_MOVEMENT_DETECTION_MODE     0xA5
#define DOWNLINK_CMD_SET_RGB_STATE               0xA8
#define DOWNLINK_CMD_KEEPALIVE_TIME              0xA9
#define DOWNLINK_CMD_GPS_FIXTIME                 0xAA
#define DOWNLINK_CMD_NAVIGATION_MODE             0xAB
#define DOWNLINK_CMD_GPS_SEARCH_MODE             0xAC
#define DOWNLINK_CMD_GPS_PDOP                    0xAD
#define DOWNLINK_CMD_LED_ON                      0xAE
#define DOWNLINK_CMD_MOVEMENT_LED_ON             0xAF
#define DOWNLINK_CMD_INCLUDE_MOTION_DATA         0xB0
#define DOWNLINK_CMD_ALARM_TX_INTERVAL           0xB1
#define DOWNLINK_CMD_DEVICE_TRIGGER_FACTORYRESET 0xFE
#define DOWNLINK_CMD_DEVICE_TRIGGER_RESET        0xFF

// clang-format on