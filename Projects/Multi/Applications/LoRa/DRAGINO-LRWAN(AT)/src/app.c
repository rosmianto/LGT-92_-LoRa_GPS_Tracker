#include "gps.h"
#include "hw.h"
#include "lora.h"
#include "low_power_manager.h"
#include "timeServer.h"

#include <LoRaWAN_Configurations.h>

uint8_t AppDataBuff[LORAWAN_APP_DATA_BUFF_SIZE];
lora_AppData_t AppData = {AppDataBuff, 0, 0};

// clang-format off
bool     rxpr_flags                          	= 0;
int      exti_flag                           	= 0;
uint32_t COUNT                               	= 0;
uint8_t  TDC_flag                            	= 0;
uint8_t  join_flag                           	= 0;
uint8_t  device_reset_trigger                   = 0;
uint8_t  payloadlens							= 0;
bool     is_time_to_IWDG_Refresh             	= 0;
bool     JoinReq_NbTrails_over               	= 0;
bool     unconfirmed_downlink_data_ans_status	= 0;
bool     confirmed_downlink_data_ans_status  	= 0;
bool     rejoin_status                       	= 0;
bool     rejoin_keep_status                  	= 0;
bool     MAC_COMMAND_ANS_status              	= 0;
uint8_t  response_level                      	= 0;
uint16_t REJOIN_TX_DUTYCYCLE                 	= 20;  // minutes

uint32_t Altitude           					= 0;
uint32_t APP_TX_DUTYCYCLE   					= 300000;
uint32_t Server_TX_DUTYCYCLE					= 300000;
uint32_t Alarm_TX_DUTYCYCLE 					= 60000;
uint32_t Keep_TX_DUTYCYCLE  					= 21600000;
uint32_t GPS_ALARM          					= 0;
uint32_t GS                 					= 0;

// GPS-related variables (?)
uint8_t  gps_setflags     = 0;
uint8_t  position_flags   = 0;
float    pdop_comp        = 7.0;
float    pdop_fixed       = 0.0;
uint32_t Start_times      = 0;
uint32_t End_times        = 0;
uint32_t gps_time         = 0;
FP32     gps_latitude     = 0.0;
FP32     gps_longitude    = 0.0;
int32_t  longitude        = 0;
int32_t  latitude         = 0;

// IMU-related variables (?)
float Roll_basic=0,Pitch_basic=0,Yaw_basic=0;
float Roll_sum=0,Pitch_sum=0,Yaw_sum=0;
float Roll=0,Pitch=0,Yaw=0;
float Roll1=0,Pitch1=0,Yaw1=0;
float Roll_new=0,Pitch_new=0,Yaw_new=0;
float Roll_old=0,Pitch_old=0,Yaw_old=0;

