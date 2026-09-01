#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <lora.h>
#include <timeServer.h>

// TODO: I prefer enum class for type-safety.
typedef enum {
  DISABLED = 0,
  ON_MOVE = 1,
  ON_COLLIDE = 2,
  USER_DEFINED = 3
} MotionDetectionMode;

typedef enum {
  GPS_L70RL = 0,
  GPS_L76L = 1,
  GPS_UBLOX_MAX7 = 2,
  GPS_UBLOX_MAX8 = 3,
  GPS_L76K = 4
} GPSModel;

typedef enum {
  SEARCHMODE_DEFAULT = 0,
  SEARCHMODE_GPS_GLONASS = 1,
  SEARCHMODE_GPS_BEIDOU = 2,
  SEARCHMODE_GPS_GALILEO = 3,
  SEARCHMODE_GPS_GLONASS_GALILEO = 4
} GPSSearchMode;

typedef enum {
  NAVMODE_DEFAULT = 0,
  NAVMODE_FITNESS = 1,
  NAVMODE_AVIATION = 2,
  NAVMODE_BALLOON = 3,
  NAVMODE_STATIONARY = 4
} GPSNavMode;

// extern MotionDetectionMode motionDetectMode; // Declared in at.c
// extern GPSModel gpsModel;                    // Declared in at.c
// extern GPSSearchMode searchMode;             // Declared in at.c
// extern GPSNavMode navMode;                   // Declared in at.c

#define Firmware 0x04
#define LORAWAN_ADR_STATE LORAWAN_ADR_ON
#define LORAWAN_DEFAULT_DATA_RATE DR_0
#define LORAWAN_APP_PORT 2
#define JOINREQ_NBTRIALS 200
#define LORAWAN_DEFAULT_CLASS CLASS_A
#define LORAWAN_DEFAULT_CONFIRM_MSG_STATE LORAWAN_UNCONFIRMED_MSG
#define MAX_RECEIVED_DATA 255
#define LORAWAN_APP_DATA_BUFF_SIZE 200

struct Globals {
  bool button_exitflag;
  bool debug_flags;
  bool fdr_flags;
  bool is_lora_joined;
  bool is_time_to_IWDG_Refresh;
  bool moinint_exitflag;
  bool motion_flags;
  bool printResponse;
  bool red, blue, greed;
  bool rejoin_keep_status;
  bool rejoin_status;
  bool rx2_flags;

  char DATABUFF[500];

  float gps_latitude, gps_longitude;
  float loraPayloadPitch;
  float loraPayloadRoll;
  float pdop_comp;
  float pdop_fixed;
  float pdop_gps;
  float pdop_value;

  GPSModel gpsModel;
  GPSNavMode navMode;
  GPSSearchMode searchMode;

  int ALARM;
  int basic_flag;

  int16_t _lastKnownSNR;

  int32_t latitude;
  int32_t longitude;

  lora_AppData_t AppData;
  ;
  MotionDetectionMode motionDetectMode;

  TimerEvent_t AckTimeoutTimer;
  TimerEvent_t MacStateCheckTimer;
  TimerEvent_t ReJoinTimer;
  TimerEvent_t RxWindowTimer1;
  TimerEvent_t RxWindowTimer2;
  TimerEvent_t TxDelayedTimer;
  TimerEvent_t TxTimer;

  uint16_t dr_power;
  uint16_t fire_frequcy;
  uint16_t fire_version;
  uint16_t hardware_version;
  uint16_t REJOIN_TX_DUTYCYCLE; // min
  uint16_t TIMES;
  uint32_t a;
  uint32_t Alarm_TX_DUTYCYCLE;
  uint32_t APP_TX_DUTYCYCLE;
  uint32_t FLAG;
  uint32_t Freq;
  uint32_t GPS_ALARM;
  uint32_t GS;
  uint32_t Keep_TX_DUTYCYCLE;
  uint32_t led_red, led_blue, led_greed;
  uint32_t loggps;
  uint32_t LON;
  uint32_t MLON;
  uint32_t Positioning_time;
  uint32_t rx1_de, rx2_de;
  uint32_t s_config[32]; // store config
  uint32_t s_hard[1];    // store hardware version
  uint32_t s_key[32];    // store key
  uint32_t s_timer;
  uint32_t SendData;
  uint32_t Server_TX_DUTYCYCLE;
  uint32_t ServerSetTDC;
  uint32_t set_sgm;
  uint32_t start_time;
  uint32_t Start_times, End_times, gps_time;
  uint32_t Threshold;
  uint8_t _lastKnownRSSI;
  uint8_t Alarm_times;
  uint8_t Alarm_times1;
  uint8_t config_count;
  uint8_t dwelltime;
  uint8_t flag1;
  uint8_t flag2;
  uint8_t gps_setflags;
  uint8_t join_flag;
  uint8_t joinrx2_dr;
  uint8_t key_count;
  uint8_t LP;
  uint8_t md_flags;
  uint8_t payloadlens;
  uint8_t response_level;
  uint8_t restartRequested;
  uint8_t rx_flags;
  uint8_t send_fail;
  uint8_t stop_flag;
  uint8_t symbtime1_value; // RX1windowtimeout
  uint8_t symbtime2_value; // RX2windowtimeout
  uint8_t TDC_flag;
};

extern struct Globals glob;