#include <Temporary.h>

#define Firmware 0x04
#define LORAWAN_ADR_STATE LORAWAN_ADR_ON
#define LORAWAN_DEFAULT_DATA_RATE DR_0
#define LORAWAN_APP_PORT 2
#define JOINREQ_NBTRIALS 200
#define LORAWAN_DEFAULT_CLASS CLASS_A
#define LORAWAN_DEFAULT_CONFIRM_MSG_STATE LORAWAN_UNCONFIRMED_MSG
#define LORAWAN_APP_DATA_BUFF_SIZE 256

uint8_t AppDataBuff[LORAWAN_APP_DATA_BUFF_SIZE];

bool printResponse = 0;
uint8_t TDC_flag = 0;
uint8_t join_flag = 0;
uint8_t restartRequested = 0;
bool is_time_to_IWDG_Refresh = 0;
bool rejoin_status = 0;
bool rejoin_keep_status = 0;
uint8_t response_level = 0;
uint16_t REJOIN_TX_DUTYCYCLE = 20; // min

extern bool rx2_flags;
extern uint8_t dwelltime;

uint32_t APP_TX_DUTYCYCLE = 300000;
uint32_t Server_TX_DUTYCYCLE = 300000;
uint32_t Alarm_TX_DUTYCYCLE = 60000;
uint32_t Keep_TX_DUTYCYCLE = 21600000;
uint32_t GPS_ALARM = 0;
uint32_t GS = 0;

uint16_t dr_power = 0;

extern uint32_t set_sgm;
extern uint32_t LON;
extern uint32_t MLON;
extern uint32_t Threshold;
extern uint32_t Freq;
extern uint8_t mpuint_flags;
extern bool button_exitflag;
extern bool moinint_exitflag;

int ALARM = 0;
uint32_t FLAG = 0;
uint8_t send_fail = 0;
uint32_t a = 1;
int basic_flag = 0;
uint32_t ServerSetTDC;
uint32_t start_time = 0;
extern uint8_t LP;
uint8_t stop_flag = 0;
uint8_t payloadlens = 0;
uint8_t gps_setflags = 0;
float pdop_comp = 7.0;
float pdop_fixed = 0.0;

extern uint8_t Alarm_times;
extern uint8_t Alarm_times1;
extern uint32_t Positioning_time;
extern uint8_t md_flags;
extern float pdop_value;
extern float pdop_gps;

uint32_t Start_times = 0, End_times = 0, gps_time = 0;
FP32 gps_latitude, gps_longitude;
int32_t longitude;
int32_t latitude;
uint32_t SendData = 0;
uint16_t TIMES = 10000;
bool is_lora_joined = 0;
bool motion_flags = 0;

extern char DATABUFF[500];

uint32_t led_red = 0, led_blue = 0, led_greed = 0;
bool red = 0, blue = 0, greed = 0;

void send_data(void);
void send_exti(void);
void send_moin(void);
void gps_Identify();

float loraPayloadRoll = 0.0;
float loraPayloadPitch = 0.0;

lora_AppData_t AppData = {AppDataBuff, 0, 0};

bool debug_flags = 0;
uint8_t symbtime1_value = 0; // RX1windowtimeout
uint8_t flag1 = 0;

uint8_t symbtime2_value = 0; // RX2windowtimeout
uint8_t flag2 = 0;
uint8_t dwelltime;

extern uint32_t Server_TX_DUTYCYCLE;
extern uint32_t Alarm_TX_DUTYCYCLE;
extern uint32_t Keep_TX_DUTYCYCLE;
extern uint32_t start_time;
GPSModel gpsModel = GPS_UBLOX_MAX7;
GPSSearchMode searchMode = SEARCHMODE_DEFAULT;
GPSNavMode navMode = NAVMODE_DEFAULT;
extern uint16_t hardware_version;
extern float pdop_value;

uint8_t LP = 0;
uint32_t Positioning_time = 150;
uint32_t set_sgm = 0;
uint32_t s_timer = 1;
uint8_t md_flags = 0;
uint32_t LON = 1;
MotionDetectionMode motionDetectMode = ON_MOVE;
uint32_t MLON = 0;
uint32_t Threshold = 0;
uint32_t Freq = 0;
uint32_t loggps = 0;

#define MAX_RECEIVED_DATA 255

extern bool fdr_flags;
extern uint16_t REJOIN_TX_DUTYCYCLE;
extern uint8_t response_level;
extern TimerEvent_t MacStateCheckTimer;
extern TimerEvent_t TxDelayedTimer;
extern TimerEvent_t AckTimeoutTimer;
extern TimerEvent_t RxWindowTimer1;
extern TimerEvent_t RxWindowTimer2;
extern TimerEvent_t TxTimer;
extern TimerEvent_t ReJoinTimer;

int16_t _lastKnownSNR = 0;
uint8_t _lastKnownRSSI = 0;

uint16_t fire_version = 0;
uint16_t fire_frequcy = 0;
uint8_t joinrx2_dr;
bool rx2_flags = 0;
bool fdr_flags = 0;

extern uint8_t dwelltime;
extern uint8_t symbtime1_value;
extern uint8_t flag1;

extern uint8_t symbtime2_value;
extern uint8_t flag2;
extern uint8_t rx_flags;
extern uint32_t rx1_de, rx2_de;

extern uint16_t REJOIN_TX_DUTYCYCLE;
extern uint8_t response_level;
extern bool rejoin_status;

uint8_t config_count = 0;
uint8_t key_count = 0;

uint32_t s_config[32]; // store config
uint32_t s_key[32];    // store key
uint32_t s_hard[1];    // store hardware version

uint8_t mpuint_flags = 0;
uint16_t hardware_version =
    167; // TODO: This actually serves no purpose on firmware behavior
uint8_t joinrx2_dr;
float pdop_value;

extern uint8_t symbtime1_value;
extern uint8_t flag1;

extern uint8_t symbtime2_value;
extern uint8_t flag2;

extern uint32_t Server_TX_DUTYCYCLE;

extern uint32_t Alarm_TX_DUTYCYCLE;

extern uint32_t Keep_TX_DUTYCYCLE;

extern uint32_t set_sgm;

extern uint32_t Positioning_time;

extern uint32_t s_timer;

extern uint8_t Alarm_times;

extern uint8_t Alarm_times1;

extern uint32_t LON;
extern uint32_t MLON;
extern uint32_t Threshold;
extern uint32_t Freq;
extern uint8_t LP;