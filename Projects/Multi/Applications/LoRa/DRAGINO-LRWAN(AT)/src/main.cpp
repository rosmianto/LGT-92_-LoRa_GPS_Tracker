
#include "IIC.h" // What's this? They want to bitbang I2C via GPIO?
#include "at.h" // AT command processor (via UART). Heavy dependency to other subsystems.
#include "bsp.h" //
#include "command.h"
#include "delay.h"
#include "flash_eraseprogram.h"
#include "gpio_exti.h"
#include "iwdg.h"
#include "mpu9250.h"
#include "subsystems/IMU.h"
#include "subsystems/LED.h"
#include "subsystems/LoRa.h"
#include <Downlink_Commands.h>
#include <stdint.h>

LoRa lora;
// TODO: printf not available via UART, enable or not?
// So far it's routed through TraceSend().

// TODO: The MPU9250 driver code is awful, and they
// bitbanged the I2C line when PA9 and PA10 are I2C capable?

// TODO: Some ideas around device config:
// * Skip flash, use EEPROM only. Better that way.
// * Use versioning, because EEPROM can persist between different firmware
// variants/versions
// * Add basic CRC32 hashing at the end of device config to ensure data
// integrity
// * Use packed struct and create union to simplify the read and write process.

// TODO: What subsystems we're handling anyway?
// Hardware:
// * GPS (UART)
// * IMU (I2C)
// * LoRa (SPI)
// * EEPROM (built-in)
// * LEDs (GPIO)
// * Button (GPIO)
// * RTC (built-in)
// Software:
// * NMEA parser
// * AT command parser
// *

// Teaching idea:
// * There's defensive programming,
// and there's redundant, unnecessary, paranoia-induced programming.
extern uint8_t ic_version;

uint8_t loraPayloadBuffer[100];

#define FIRMWARE_VERSION 0x04

#define Kp                                                                     \
	40.0f		 // proportional gain governs rate of convergence
				 // toaccelerometer/magnetometer
#define Ki 0.02f // integral gain governs rate of convergenceof gyroscope biases
#define halfT 0.0048f // half the sample period
#define dt 0.0096f

float pitch, roll, yaw;
float pitch_sum, roll_sum, yaw_sum;
short igx, igy, igz;
short iax, iay, iaz;
short imx, imy, imz;
float gx, gy, gz;
float ax, ay, az;
float mx, my, mz;

float gx_old, gy_old, gz_old;
float ax_old, ay_old, az_old;
float mx_old, my_old, mz_old;

static float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;
static float exInt = 0, eyInt = 0, ezInt = 0;
static short turns = 0;
static float newdata = 0.0f, olddata = 0.0f;

static float k10 = 0.0f, k11 = 0.0f, k12 = 0.0f, k13 = 0.0f;
static float k20 = 0.0f, k21 = 0.0f, k22 = 0.0f, k23 = 0.0f;
static float k30 = 0.0f, k31 = 0.0f, k32 = 0.0f, k33 = 0.0f;
static float k40 = 0.0f, k41 = 0.0f, k42 = 0.0f, k43 = 0.0f;

// various extern variables
extern uint8_t mode;
extern uint8_t inmode;
extern uint16_t power_time;
extern bool rx2_flags;
extern uint32_t LoRaMacState;
extern uint8_t dwelltime;
extern bool debug_flags;
extern uint16_t dr_power;
extern uint32_t set_sgm;
extern uint32_t LON;
extern uint32_t MD;
extern uint32_t MLON;
extern uint32_t Threshold;
extern uint32_t Freq;
extern uint8_t mpuint_flags;
extern bool button_exitflag;
extern bool moinint_exitflag;
extern uint32_t gps_search_mode;
extern uint32_t gps_navigation_mode;
extern uint8_t LP;
extern uint8_t Alarm_times;
extern uint8_t Alarm_times1;
extern uint32_t Positioning_time;
extern uint8_t md_flags;
extern float pdop_value;
extern float pdop_gps;
extern bool rx2_flags;

static uint32_t ServerSetTDC;
uint32_t CHE = 0;
int ALARM = 0;
uint32_t FLAG = 0;
uint8_t send_fail = 0;
uint32_t a = 1;
int basic_flag = 0;
uint32_t start_time = 0;
uint32_t AlarmSetTDC = 0;
uint8_t flag_1 = 1;
uint8_t alarm_flags = 0;
uint8_t stop_flag = 0;
uint32_t SendData = 0;
uint16_t batteryLevel_mV = 0;
uint16_t TIMES = 10000;
bool is_lora_joined = 0;
bool motion_flags = 0;

extern char gpsUartBuffer[500];

// clang-format on
void led_power_anim(void);
void lora_send_fsm(void);
void send_data(void);
void send_exti(void);
void send_moin(void);
void gps_Identify();

#if defined(LoRa_Sensor_Node)
/* start the tx process*/
static void LoraStartTx(TxEventType_t EventType);
static void LoraStartjoin(TxEventType_t EventType);
static void StartIWDGRefresh(TxEventType_t EventType);
static void LoraStartRejoin(TxEventType_t EventType);
static void time(TxEventType_t EventType);

TimerEvent_t TxTimer;
static TimerEvent_t TxTimer2;
static TimerEvent_t IWDGRefreshTimer; // watch dog
TimerEvent_t ReJoinTimer;
static TimerEvent_t time_TxTimer;

/* tx timer callback function*/
static void OnTxTimerEvent(void);
static void OnTxTimerEvent2(void);
static void OnIWDGRefreshTimeoutEvent(void);
static void OnReJoinTimerEvent(void);
static void timing(void);

#endif

extern void printf_joinmessage(void);

bool sleep_status = 0; // AT+SLEEP
int user_key_exti_flag = 0;
uint8_t user_key_duration = 0;
void user_key_event(void);

TimerEvent_t downlinkLedTimer;
TimerEvent_t NetworkJoinedLedTimer;
TimerEvent_t PressButtonTimesLedTimer;
TimerEvent_t PressButtonTimeoutTimer;

uint8_t press_button_times = 0; // Press the button times in a row fast
uint8_t OnPressButtonTimeout_status = 0;
extern TimerEvent_t TxDelayedTimer;

void OndownlinkLedEvent(void);
void OnNetworkJoinedLedEvent(void);
void OnPressButtonTimesLedEvent(void);
void OnPressButtonTimeoutEvent(void);

/* Private variables ---------------------------------------------------------*/
/* load Main call backs structure*/

uint8_t flag_2 = 1;

// Yeah the packed attrib is GCC specific.
// Not really focusing on full portability for now.
struct __attribute__((packed)) RgbLedState {
	uint8_t isRedOn;
	uint16_t redOnDuration;
	uint8_t isBlueOn;
	uint16_t blueOnDuration;
	uint8_t isGreenOn;
	uint16_t greenOnDuration;
};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void) {
	/* STM32 HAL library initialization*/
	HAL_Init();

	/* Configure the system clock*/
	SystemClock_Config();

	// Setup interrupt handler for button and MPU9250 INT
	EXTI4_15_IRQHandler_Config();

	/* Configure the debug mode*/
	DBG_Init();

	GPS_init();

	GPS_Run();

	/* Configure the hardware*/
	HW_Init();

	/* USER CODE BEGIN 1 */
	/* USER CODE END 1 */
	CMD_Init();

	led_power_anim();

	IIC_GPIO_MODE_Config();

	iwdg_init();

	StartIWDGRefresh(TX_ON_EVENT);

	new_firmware_update();

	/*Disbale Stand-by mode*/
	LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);

	lora.init();

	gps_Identify();

	while (1) {
		/* Handle UART commands */
		CMD_Process();

		if (device_reset_trigger == 1) {
			DelayMs(500);
			AppData.Buff[0] = 0x11;
			AppData.BuffSize = 1;
			AppData.Port = 2;
			LORA_send(&AppData, LORAWAN_UNCONFIRMED_MSG);
			device_reset_trigger++;
		} else if ((device_reset_trigger == 2) &&
				   ((LoRaMacState & 0x00000001) != 0x00000001)) {
			NVIC_SystemReset();
		}

		if (md_flags == 1) {
			MPU_INT_Init();
			md_flags = 0;
		}

		send_exti();

		send_moin();

		if (is_time_to_IWDG_Refresh == 1) {
			is_time_to_IWDG_Refresh = 0;
			IWDG_Refresh();
		}

		lora_send();

		if ((motion_flags == 1) && (mpuint_flags == 1)) {
			motion_flags = 0;
			start_time = HW_RTC_GetTimerValue();
			APP_TX_DUTYCYCLE = Server_TX_DUTYCYCLE;
			TimerInit(&TxTimer, OnTxTimerEvent);
			gps.latitude = 0;
			gps.longitude = 0;
			lora_state_GPS_Send();
			lora_send();
			TimerSetValue(&TxTimer, APP_TX_DUTYCYCLE);
			/*Wait for next tx slot*/
			TimerStart(&TxTimer);
			TimerStart(&time_TxTimer);
			TimerStart(&IWDGRefreshTimer);
			PPRINTF("Exit static mode\r\n");
		}
		user_key_event();

		DISABLE_IRQ();
		/*
		 * if an interrupt has occurred after DISABLE_IRQ, it is kept pending
		 * and cortex will not enter low power anyway
		 * don't go in low power mode if we just received a char
		 */
#ifndef LOW_POWER_DISABLE
		LPM_EnterLowPower();
#endif
		ENABLE_IRQ();

		/* USER CODE BEGIN 2 */
		/* USER CODE END 2 */
	}
}

static void LORA_HasJoined(void) {
	rx2_flags = 1;

	Read_Config();

	AT_PRINTF("JOINED\r\n");

	BSP_sensor_Init();
	LED::ledRedOn();
	LED::ledBlueOn();
	DelayMs(1000);
	LED::ledRedOff();
	LED::ledBlueOff();

	rejoin_keep_status = 0;

	if ((lora_config_otaa_get() == LORA_ENABLE ? 1 : 0)) {
		printf_joinmessage();
	}

	TimerStop(&ReJoinTimer);

	LORA_RequestClass(LORAWAN_DEFAULT_CLASS);

	start_time = HW_RTC_GetTimerValue();

#if defined(LoRa_Sensor_Node) /*LSN50 Preprocessor compile swicth:hw_conf.h*/
	LoraStartjoin(TX_ON_TIMER);
#endif
	time(TX_ON_TIMER);
	lora_state_GPS_Send();

	gps.flag = 1;
	is_lora_joined = 1;

#if defined(AT_Data_Send) /*LoRa ST Module*/
	AT_PRINTF("Please using AT+SEND or AT+SENDB to send you data!\n\r");
#endif
}

static void printf_uplink(void) {
	if (gps_latitude > 0 && gps_longitude > 0) {
		gps_state_on();

		if (gps.latNS != 'N') {
			latitude = gps_latitude * 1000000;
			latitude = (~latitude);
			gps_latitude = (float)(latitude) / 1000000;
			TimerTime_t ts = TimerGetCurrentTime();
			PPRINTF("[%lu]", ts);
			PPRINTF("%s: %.6f\n\r", (gps.latNS == 'N') ? "North" : "South",
					gps_latitude);
		} else {
			latitude = gps_latitude * 1000000;
			TimerTime_t ts = TimerGetCurrentTime();
			PPRINTF("[%lu]", ts);
			PPRINTF("%s: %.6f\n\r", (gps.latNS == 'N') ? "North" : "South",
					gps_latitude);
		}
		if (gps.lgtEW != 'E') {
			longitude = gps_longitude * 1000000;
			longitude = (~longitude);
			gps_longitude = (float)(longitude) / 1000000;
			TimerTime_t ts = TimerGetCurrentTime();
			PPRINTF("[%lu]", ts);
			PPRINTF("%s: %.6f\n\r", (gps.latNS == 'E') ? "East" : "West",
					gps_longitude);
		} else {
			longitude = gps_longitude * 1000000;
			TimerTime_t ts = TimerGetCurrentTime();
			PPRINTF("[%lu]", ts);
			PPRINTF("%s: %.6f\n\r", (gps.lgtEW == 'E') ? "East" : "West",
					gps_longitude);
		}
		TimerTime_t ts2 = TimerGetCurrentTime();
		PPRINTF("[%lu]", ts2);
		if (pdop_fixed != 0.0) {
			PPRINTF("PDOP is %.2f\n\r", pdop_fixed);
		} else if (pdop_comp != 7.0) {
			PPRINTF("PDOP is %.2f\n\r", pdop_comp);
		}
		PPRINTF("[%lu]", ts2);
		PPRINTF("Satellite:%2d.%2d\n\r", gps.usedsatnum, gps.allsatnum);
		PRINTF("Altitude:%.1f%c ", gps.altitude, gps.altitudeunit);
		PPRINTF("Fix_Time:%d \n\r", End_times);
		PPRINTF("data_success\n\r");
		pdop_fixed = 0.0;
		pdop_comp = 7.0;
		gps.latitude = 0;
		gps.longitude = 0;
		gps_latitude = 0;
		gps_longitude = 0;
	} else {
		TimerTime_t ts = TimerGetCurrentTime();
		PPRINTF("\n\r[%lu]", ts);
		if (LP == 2) {
			PPRINTF("STOP GPS \n\r");
		} else {
			PPRINTF("GPS NO FIX\n\r");
		}
	}
	if ((Alarm_times1 <= 60) && (GPS_ALARM == 1) && (GS == 0)) {
		TimerTime_t ts = TimerGetCurrentTime();
		PPRINTF("[%lu]", ts);
		PPRINTF("send NO.%d Alarm data \n\r", Alarm_times);
	}

	DelayMs(1000);
}

static void Send(void) {
	sensor_t sensor_data;

	if (LORA_JoinStatus() != LORA_SET) {
		/*Not joined, try again later*/
		return;
	}

	BSP_sensor_Read(&sensor_data);

	uint32_t i = 0;
	AppData.Port = lora_config_application_port_get();
	if (basic_flag == 1) {
		Roll_basic = 0;
		Pitch_basic = 0;
		Yaw_basic = 0;
		basic_flag = 0;
	} else if (basic_flag == 2) {
		Roll_basic = Roll;
		Pitch_basic = Pitch;
		Yaw_basic = Yaw;
		basic_flag = 0;
	}
	if (AD_code3 <= 2840) {
		LP = 1;
		PPRINTF("\n\rBattery voltage too low\r\n");
	} else {
		LP = 0;
	}

	// Enable the MPU9250 and try to calculate Pitch and Roll value
#if 0
	MPU_Write_Byte(MPU9250_ADDR, 0x6B, 0X00); // ����
	MPU_Init();
	yaw = 0;
	for (int H = 0; H < 10; H++) {
		MPU_Get_Accel(&iax, &iay, &iaz, &ax, &ay, &az);
		AHRSupdate(0, 0, 0, ax, ay, az, 0, 0, 0, &roll, &pitch, &yaw);
		olddata = newdata;
		newdata = yaw;
		CountTurns(&newdata, &olddata, &turns);
		CalYaw(&yaw, &turns);
		pitch += pitchoffset;
		roll += rolloffset;
		yaw += yawoffset;
	}
	for (int H = 0; H < 30; H++) {
		MPU_Get_Accel(&iax, &iay, &iaz, &ax, &ay, &az);
		AHRSupdate(0, 0, 0, ax, ay, az, 0, 0, 0, &roll, &pitch, &yaw);
		olddata = newdata;
		newdata = yaw;
		CountTurns(&newdata, &olddata, &turns);
		CalYaw(&yaw, &turns);
		pitch += pitchoffset;
		roll += rolloffset;
		yaw += yawoffset;

		Pitch_sum += pitch;
		Roll_sum += roll;
		Yaw_sum += yaw;
	}

	Roll_new = Roll_sum / 30.0;
	Pitch_new = Pitch_sum / 30.0;
	Yaw_new = Yaw_sum / 30;
	if (flag_1 == 1) {
		flag_1 = 0;
		Roll_old = Roll_new;
		Pitch_old = Pitch_new;
		Yaw_old = Yaw_new;
	}

	if (-0.2 < Roll_new - Roll_old && Roll_new - Roll_old < 0.2) {
		Roll1 = (Roll_new + Roll_old) / 2.0;
		Roll1 = (Roll_old + Roll1) / 2.0;
		Roll_old = Roll1;
	} else {
		Roll1 = Roll_new;
		Roll_old = Roll_new;
	}

	if (-0.2 < Pitch_new - Pitch_old && Pitch_new - Pitch_old < 0.2) {
		Pitch1 = (Pitch_new + Pitch_old) / 2.0;
		Pitch1 = (Pitch_old + Pitch1) / 2.0;
		Pitch_old = Pitch1;
	} else {
		Pitch1 = Pitch_new;
		Pitch_old = Pitch_new;
	}

	if (-0.2 < Yaw_new - Yaw_old && Yaw_new - Yaw_old < 0.2) {
		Yaw1 = (Yaw_new + Yaw_old) / 2.0;
		Yaw1 = (Yaw_old + Yaw1) / 2.0;
		Yaw_old = Yaw1;
	} else {
		Yaw1 = Yaw_new;
		Yaw_old = Yaw_new;
	}

	Roll = Roll1;
	Pitch = Pitch1;
	Roll1 = Roll1 - Roll_basic;
	Pitch1 = Pitch1 - Pitch_basic;
	yaw = Yaw1;
	Yaw1 = Yaw_old - Yaw_basic;

	Roll_sum = 0;
	Pitch_sum = 0;
	Yaw_sum = 0;
	TimerTime_t ts = TimerGetCurrentTime();
	PPRINTF("Roll=%0.2f  ", ((int)(Roll1 * 100)) / 100.0);
	PPRINTF("Pitch=%0.2f\r\n", ((int)(Pitch1 * 100)) / 100.0);

#endif

	// TODO: Will be calculated properly later.
	// This time we will assume it's done and use mock values.
	// CalculateAHRS(roll, pitch, yaw);
	Roll1 = 12.34;
	Pitch1 = 12.34;

	printf_uplink();
	FLAG = (int)(MD << 6 | LON << 5 | FIRMWARE_VERSION) & 0xFF;
	i = 0;
	if (lora_getGPSState() == STATE_GPS_OFF) {
		loraPayloadBuffer[i++] = 0x00;
		loraPayloadBuffer[i++] = 0x00;
		loraPayloadBuffer[i++] = 0x00;
		loraPayloadBuffer[i++] = 0x00;
		loraPayloadBuffer[i++] = 0x00;
		loraPayloadBuffer[i++] = 0x00;
		loraPayloadBuffer[i++] = 0x00;
		loraPayloadBuffer[i++] = 0x00;
	} else if (lora_getGPSState() == STATE_GPS_NO) {
		loraPayloadBuffer[i++] = 0xFF;
		loraPayloadBuffer[i++] = 0xFF;
		loraPayloadBuffer[i++] = 0xFF;
		loraPayloadBuffer[i++] = 0xFF;
		loraPayloadBuffer[i++] = 0xFF;
		loraPayloadBuffer[i++] = 0xFF;
		loraPayloadBuffer[i++] = 0xFF;
		loraPayloadBuffer[i++] = 0xFF;
	} else {
		loraPayloadBuffer[i++] = (int)latitude >> 24 & 0xFF;
		loraPayloadBuffer[i++] = (int)latitude >> 16 & 0xFF;
		loraPayloadBuffer[i++] = (int)latitude >> 8 & 0xFF;
		loraPayloadBuffer[i++] = (int)latitude & 0xFF;
		loraPayloadBuffer[i++] = (int)longitude >> 24 & 0xFF;
		loraPayloadBuffer[i++] = (int)longitude >> 16 & 0xFF;
		loraPayloadBuffer[i++] = (int)longitude >> 8 & 0xFF;
		loraPayloadBuffer[i++] = (int)longitude & 0xFF;
	}
	if (set_sgm == 1) {
		if (ALARM == 1) {
			loraPayloadBuffer[i++] =
				(int)(sensor_data.bat_mv) >> 8 | 0x40; // battery
			loraPayloadBuffer[i++] = (int)sensor_data.bat_mv;

		} else {
			loraPayloadBuffer[i++] = (int)(sensor_data.bat_mv) >> 8; // battery
			loraPayloadBuffer[i++] = (int)sensor_data.bat_mv;
		}
		loraPayloadBuffer[i++] = (int)FLAG;
	} else if (set_sgm == 0) {
		if (ALARM == 1) {
			loraPayloadBuffer[i++] =
				(int)(sensor_data.bat_mv) >> 8 | 0x40; // battery
			loraPayloadBuffer[i++] = (int)sensor_data.bat_mv;
		} else {
			loraPayloadBuffer[i++] = (int)(sensor_data.bat_mv) >> 8; // battery
			loraPayloadBuffer[i++] = (int)sensor_data.bat_mv;
		}
		loraPayloadBuffer[i++] = (int)FLAG;
		loraPayloadBuffer[i++] = (int)(Roll1 * 100) >> 8; // Roll
		loraPayloadBuffer[i++] = (int)(Roll1 * 100);
		loraPayloadBuffer[i++] = (int)(Pitch1 * 100) >> 8; // Pitch
		loraPayloadBuffer[i++] = (int)(Pitch1 * 100);
		loraPayloadBuffer[i++] = (int)(gps.HDOP * 100);			 // HDOP
		loraPayloadBuffer[i++] = (int)(gps.altitude * 100) >> 8; // Altitude
		loraPayloadBuffer[i++] = (int)(gps.altitude * 100);
	}

	gps.flag = 1;
	gps_setflags = 0;
	press_button_times = 0;
	payloadlens = i;
	IWDG_Refresh();
	lora.sendPayload(loraPayloadBuffer,
					 i); // TODO: Weird to use variable i to specify len.
}

#if defined(LoRa_Sensor_Node)
static void OnTxTimerEvent(void) {

	gps.flag = 1;
	gps.latitude = 0;
	gps.longitude = 0;

	if (lora_getState() != STATE_GPS_SEND) {
		lora_state_GPS_Send();
	}

	TimerSetValue(&TxTimer, APP_TX_DUTYCYCLE);

	/*Wait for next tx slot*/
	TimerStart(&TxTimer);
}

static void LoraStartTx(TxEventType_t EventType) {
	if (EventType == TX_ON_TIMER) {
		/* send everytime timer elapses */
		TimerInit(&TxTimer, OnTxTimerEvent);
		TimerSetValue(&TxTimer, APP_TX_DUTYCYCLE);
		OnTxTimerEvent();
		lora_state_GPS_Send();
		gps.flag = 1;
		gps.latitude = 0;
		gps.longitude = 0;
	}
}

static void timing(void) {
	uint32_t temp_time = 0;
	if (MD != 0) {
		if ((motion_flags == 0) && (ALARM == 0)) {
			temp_time = HW_RTC_GetTimerValue();
			//			PPRINTF("temp_time is %d\r\n",temp_time);
			if (temp_time < start_time) {
				start_time = 0;
			}

			if (mpuint_flags == 1) {
				start_time = temp_time;
				mpuint_flags = 0;
			}

			else if (temp_time - start_time >= 300000) {
				APP_TX_DUTYCYCLE = Keep_TX_DUTYCYCLE;
				TimerInit(&TxTimer, OnTxTimerEvent);
				TimerSetValue(&TxTimer, APP_TX_DUTYCYCLE);

				/*Wait for next tx slot*/
				TimerStart(&TxTimer);
				TimerStart(&IWDGRefreshTimer);
				motion_flags = 1;
				PPRINTF("Enter static mode\r\n");
			}
		}
	}
	TimerSetValue(&time_TxTimer, 40000);

	/*Wait for next tx slot*/
	TimerStart(&time_TxTimer);
}

static void time(TxEventType_t EventType) {
	if (EventType == TX_ON_TIMER) {
		/* send everytime timer elapses */
		TimerInit(&time_TxTimer, timing);
		TimerSetValue(&time_TxTimer, 40000);
		timing();
	}
}

static void OnTxTimerEvent2(void) {
	if (join_flag == 0) {
		TimerSetValue(&TxTimer2, 500);

		/*Wait for next tx slot*/
		TimerStart(&TxTimer2);

		join_flag++;
	} else if (join_flag == 1) {
		LoraStartTx(TX_ON_TIMER);
		join_flag++;
	}
}

static void LoraStartjoin(TxEventType_t EventType) {
	if (EventType == TX_ON_TIMER) {
		/* send everytime timer elapses */
		TimerInit(&TxTimer2, OnTxTimerEvent2);
		TimerSetValue(&TxTimer2, 500);

		OnTxTimerEvent2();
	}
}

static void OnIWDGRefreshTimeoutEvent(void) {
	TimerSetValue(&IWDGRefreshTimer, 18000);

	TimerStart(&IWDGRefreshTimer);

	is_time_to_IWDG_Refresh = 1;
}

static void StartIWDGRefresh(TxEventType_t EventType) {
	if (EventType == TX_ON_EVENT) {
		/* send everytime timer elapses */
		TimerInit(&IWDGRefreshTimer, OnIWDGRefreshTimeoutEvent);
		TimerSetValue(&IWDGRefreshTimer, 18000);
		TimerStart(&IWDGRefreshTimer);
	}
}

static void OnReJoinTimerEvent(void) { TimerStop(&ReJoinTimer); }

static void LoraStartRejoin(TxEventType_t EventType) {
	if (EventType == TX_ON_EVENT) {
		/* send everytime timer elapses */
		TimerInit(&ReJoinTimer, OnReJoinTimerEvent);
		TimerSetValue(&ReJoinTimer, REJOIN_TX_DUTYCYCLE * 60000);
		TimerStart(&ReJoinTimer);
	}
}
#endif

void lora_send(void) {
	switch (lora_getState()) {
	case STATE_LED: {
		gps.flag = 1;
		stop_flag = 1;
		if (Positioning_time != 0) {
			//				if(GPS_ALARM == 0)
			//				{
			GPS_POWER_OFF();
			//				}
			LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);
		}
		break;
	}
	case STATE_GPS_LOG: {
		LPM_SetOffMode(LPM_APPLI_Id, LPM_Enable);
		POWER_ON();
		GPS_INPUT();
		break;
	}
	case STATE_GPS_SEND: {
		stop_flag = 0;
		if (gps.GSA_mode2 == 3) {
			if (gps.latitude > 0 && gps.longitude > 0) {
				if (((pdop_value >= pdop_gps) && (pdop_gps != 0.0)) ||
					(ic_version == 2)) {
					gps_latitude = gps.latitude;
					gps_longitude = gps.longitude;
					pdop_fixed = pdop_gps;
					SendData = 1;
					TimerStart(&IWDGRefreshTimer);
					if (Positioning_time != 0) {
						//						 if(GPS_ALARM == 0)
						//							{
						GPS_POWER_OFF();
						//							}
					}
				} else if (pdop_value < pdop_gps) {
					if ((pdop_gps < pdop_comp) && (pdop_gps != 0.0)) {
						gps_latitude = gps.latitude;
						gps_longitude = gps.longitude;
						pdop_comp = pdop_gps;
					}
				}
				if ((((Positioning_time != 0) &&
					  (End_times >= Positioning_time)) ||
					 ((Positioning_time == 0) && (End_times >= 150))) &&
					(gps_latitude > 0 && gps_longitude > 0)) {
					SendData = 1;
					if (Positioning_time != 0) {
						//							if(GPS_ALARM == 0)
						//							{
						GPS_POWER_OFF();
						//							}
					}
				}
			}
		}

		if (SendData == 1) {
			if (GPS_ALARM == 0) {
				gps_state_on();
				if (LON == 1) {
					LED::ledBlueOn();
					DelayMs(200);
					ledBlueOff();
					DelayMs(200);
				}
				send_data();
				a = 1;
				GPS_ALARM = 0;
				SendData = 0;
			} else if (GPS_ALARM == 1) {
				if (Alarm_times <= 60) {
					gps_state_on();
					if (LON == 1) {
						LED::ledRedOn();
						DelayMs(1000);
					}
					LED::ledRedOff();
					send_ALARM_data();
					a = 100;
					GPS_ALARM = 1;
					SendData = 0;
				}
				if (Alarm_times1 == 60) {
					DelayMs(3500);
					start_time = HW_RTC_GetTimerValue();
					ALARM = 0;
					GPS_ALARM = 0;
					if (LON == 1) {
						BSP_sensor_Init();
						LED::ledBlueOn();
						DelayMs(1000);
					}
					ledBlueOff();
					PPRINTF("Exit Alarm\r\n");
				}
			}
		}

		if (LP == 0) {
			LPM_SetOffMode(LPM_APPLI_Id, LPM_Enable);
			BSP_sensor_Init();
			if (GS == 1) {
				ALARM = 1;
				gps_state_off();
				LED::ledRedOn();
				Send();
				DelayMs(5000);
				LED::ledRedOff();
				GS = 0;
				gps_time = 0;
				Alarm_times1 = 1;
				Alarm_times = 1;
				GPS_ALARM = 1;
			}

			POWER_ON();
			GPS_INPUT();
			Start_times++;
			LP = 0;
			ledGreenOff();
		} else if ((LP == 1) || (LP == 2)) {
			gps_state_no();
			if (motion_flags == 1) {
				APP_TX_DUTYCYCLE = Keep_TX_DUTYCYCLE;
			} else {
				APP_TX_DUTYCYCLE = Server_TX_DUTYCYCLE;
			}
			TimerInit(&TxTimer, OnTxTimerEvent);
			TimerSetValue(&TxTimer, APP_TX_DUTYCYCLE);
			Send();
			TimerSetValue(&TxTimer, APP_TX_DUTYCYCLE);
			/*Wait for next tx slot*/
			TimerStart(&TxTimer);
			TimerStart(&time_TxTimer);
			TimerStart(&IWDGRefreshTimer);
			LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);
			if (MD == 0) {
				MPU_Write_Byte(MPU9250_ADDR, 0x6B, 0X40); // MPU sleep
			} else {
				MPU_INT_Init();
			}
			lora_state_Led();
			a = 1;
			DISABLE_IRQ();
			/*
			 * if an interrupt has occurred after DISABLE_IRQ, it is kept
			 * pending and cortex will not enter low power anyway don't go in
			 * low power mode if we just received a char
			 */
#ifndef LOW_POWER_DISABLE
			LPM_EnterLowPower();
#endif
			ENABLE_IRQ();
		}

		if (Start_times == TIMES) {
			End_times++;
			Start_times = 0;
			gps_time++;
			TimerStart(&IWDGRefreshTimer);
			if (LON == 1) {
				LED::ledGreenOn();
			}
			TIMES = 10000;
			DelayMs(200);
		}

		if ((End_times <= Positioning_time) || (Positioning_time == 0)) {
			if (gps_time == 30) {
				PPRINTF("\r\n");
				PPRINTF("Fix Time:%d s \n\r", End_times);
				TimerStart(&IWDGRefreshTimer);
				if (Positioning_time != 0) {
					PPRINTF("Fix Timeout (FTIME):%d s \n\r", Positioning_time);
				}
				gps_time = 0;
			}
		}

		if ((position_flags == 0) &&
			(((Positioning_time != 0) && (End_times >= Positioning_time)) ||
			 ((Positioning_time == 0) && (End_times >= 150)))) {
			send_fail = 1;
			if (Positioning_time != 0) {
				//					if(GPS_ALARM == 0)
				//					{
				GPS_POWER_OFF();
				//					}
			}
			if (GPS_ALARM == 0) {
				LED::ledRedOff();
				LED::ledBlueOff();
				LED::ledGreenOff();
				gps_state_off();
				a = 100;
				if (LON == 1) {
					LED::ledRedOn();
					DelayMs(200);
					LED::ledRedOff();
					DelayMs(200);
					LED::ledRedOn();
					DelayMs(200);
					LED::ledRedOff();
					DelayMs(200);
				}
				send_data();
				GPS_ALARM = 0;
			} else if (GPS_ALARM == 1) {
				if (Alarm_times <= 60) {
					LED::ledRedOff();
					LED::ledBlueOff();
					LED::ledGreenOff();
					if (LON == 1) {
						LED::ledRedOn();
						DelayMs(1000);
					}
					LED::ledRedOff();
					gps_state_off();
					send_ALARM_data();
					GPS_ALARM = 1;
					a = 100;
					SendData = 0;
				}
				if (Alarm_times1 == 60) {
					DelayMs(3500);
					start_time = HW_RTC_GetTimerValue();
					ALARM = 0;
					GPS_ALARM = 0;
					if (LON == 1) {
						BSP_sensor_Init();
						LED::ledBlueOn();
						DelayMs(1000);
					}
					ledBlueOff();
					PPRINTF("Exit Alarm\r\n");
				}
			}
			TimerStart(&IWDGRefreshTimer);
			send_fail = 0;
		}
		position_flags = 0;
		break;
	}
	default: {
		PPRINTF("default\n\r");
		lora_state_Led();
		break;
	}
	}
}

void send_data(void) {
	if (motion_flags == 1) {
		APP_TX_DUTYCYCLE = Keep_TX_DUTYCYCLE;
	} else {
		APP_TX_DUTYCYCLE = Server_TX_DUTYCYCLE;
	}
	TimerInit(&TxTimer, OnTxTimerEvent);
	TimerSetValue(&TxTimer, APP_TX_DUTYCYCLE);
	Send();
	TimerSetValue(&TxTimer, APP_TX_DUTYCYCLE);
	/*Wait for next tx slot*/
	TimerStart(&TxTimer);
	TimerStart(&time_TxTimer);
	TimerStart(&IWDGRefreshTimer);
	LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);
	PPRINTF("Update Interval: %d ms\n\r", APP_TX_DUTYCYCLE);
	if (MD == 0) {
		MPU_Write_Byte(MPU9250_ADDR, 0x6B, 0X40); // MPU sleep
	} else {
		MPU_INT_Init();
	}
	lora_state_Led();
	gps.flag = 1;
	End_times = 0;
	gps_time = 0;
	gps.GSA_mode2 = 0;
	DISABLE_IRQ();
	/*
	 * if an interrupt has occurred after DISABLE_IRQ, it is kept pending
	 * and cortex will not enter low power anyway
	 * don't go in low power mode if we just received a char
	 */
#ifndef LOW_POWER_DISABLE
	LPM_EnterLowPower();
#endif
	ENABLE_IRQ();
	BSP_sensor_Init();

#if 0
	// TODO: Turn on LEDs but not turning off again?
	// Weird technical decision to make.
	if (red_on == 1) {
		LED::ledRedOn();
	}
	if (blue_on == 1) {
		LED::ledBlueOn();
	}
	if (green_on == 1) {
		LED::ledGreenOn();
	}
#endif

	if (Positioning_time == 0) {
		DelayMs(2000);
		LPM_SetOffMode(LPM_APPLI_Id, LPM_Enable);
	}
}

void send_ALARM_data(void) {
	APP_TX_DUTYCYCLE = Alarm_TX_DUTYCYCLE;
	TimerInit(&TxTimer, OnTxTimerEvent);
	TimerSetValue(&TxTimer, APP_TX_DUTYCYCLE);
	Send();
	TimerSetValue(&TxTimer, APP_TX_DUTYCYCLE);
	/*Wait for next tx slot*/
	TimerStart(&TxTimer);
	TimerStart(&IWDGRefreshTimer);
	LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);
	PPRINTF("Update Interval: %d ms\n\r", APP_TX_DUTYCYCLE);
	MPU_Write_Byte(MPU9250_ADDR, 0x6B, 0X40); // MPU sleep
	lora_state_Led();
	End_times = 0;
	gps_time = 0;
	a = 100;
	gps.GSA_mode2 = 0;
	//	   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
	DISABLE_IRQ();
	/*
	 * if an interrupt has occurred after DISABLE_IRQ, it is kept pending
	 * and cortex will not enter low power anyway
	 * don't go in low power mode if we just received a char
	 */
#ifndef LOW_POWER_DISABLE
	LPM_EnterLowPower();
#endif
	ENABLE_IRQ();
	if (Positioning_time == 0) {
		DelayMs(2000);
		LPM_SetOffMode(LPM_APPLI_Id, LPM_Enable);
	}
}

void user_key_event(void) {
	// TODO: What's this mess?
	// Handling different button presse patterns
	// to perform different tasks.
	// main.c shouldn't care about Button handling.
	// What does it care is what event the button triggered.
	if (user_key_exti_flag == 1) {
		user_key_exti_flag = 0;

		if (OnPressButtonTimeout_status == 0) {
			OnPressButtonTimeout_status = 1;
			TimerInit(&PressButtonTimeoutTimer, OnPressButtonTimeoutEvent);
			TimerSetValue(&PressButtonTimeoutTimer, 5000);
			TimerStart(&PressButtonTimeoutTimer);
		}
		BSP_sensor_Init();
		press_button_times++;
		HAL_GPIO_WritePin(LED1_PORT, LED0_PIN, GPIO_PIN_SET);

		uint32_t currentTime = TimerGetCurrentTime();

		while (HAL_GPIO_ReadPin(GPIO_USERKEY_PORT, GPIO_USERKEY_PIN) ==
			   GPIO_PIN_SET) {
			if (TimerGetElapsedTime(currentTime) >= 1000 &&
				TimerGetElapsedTime(currentTime) < 3000) // send
			{
				user_key_duration = 1;
			} else if (TimerGetElapsedTime(currentTime) >=
					   3000) // system reset,Activation Mode
			{
				press_button_times = 0;
				for (int i = 0; i < 10; i++) {
					HAL_GPIO_TogglePin(LED1_PORT, LED0_PIN);
					HAL_Delay(100);
				}
				user_key_duration = 4;
				break;
			}
		}
		HAL_GPIO_WritePin(LED1_PORT, LED0_PIN, GPIO_PIN_RESET);

		if (press_button_times == 5) {
			press_button_times = 0;
			user_key_duration = 5;
		}
		switch (user_key_duration) {
		case 1: {
			user_key_duration = 0;

			//				if(sleep_status==0)
			//				{
			//				  Send();
			//				}
			break;
		}
		case 2: // sleep
		{
			sleep_status = 1;
			lora.stop();
			TimerStop(&TxTimer);

			DelayMs(500);
			TimerInit(&PressButtonTimesLedTimer, OnPressButtonTimesLedEvent);
			TimerSetValue(&PressButtonTimesLedTimer, 5000);
			HAL_GPIO_WritePin(LED1_PORT, LED3_PIN, GPIO_PIN_SET);
			TimerStart(&PressButtonTimesLedTimer);
			user_key_duration = 0;
			break;
		}
		case 3: // system reset,Activation Mode
		{
			user_key_duration = 0;
			NVIC_SystemReset();
			break;
		}
		case 4: {
			GPS_ALARM = 1;
			GS = 1;
			dr_power = 1;
			ALARM = 1;
			TimerStart(&IWDGRefreshTimer);
			user_key_duration = 0;
			lora_state_GPS_Send();
#if defined(REGION_AS923) || defined(REGION_AU915)
			dwelltime = 1;
#endif
			break;
		}
		case 5: {
			PRINTF("Exit Alarm\r\n");
			start_time = HW_RTC_GetTimerValue();
			Alarm_times = 60;
			Alarm_times1 = 60;
			GPS_ALARM = 0;
			ALARM = 0;
			press_button_times = 0;
			TimerStart(&IWDGRefreshTimer);
			BSP_sensor_Init();
			HAL_GPIO_WritePin(GPIOA, LED0_PIN, GPIO_PIN_SET);
			HAL_Delay(5000);
			HAL_GPIO_WritePin(GPIOA, LED0_PIN, GPIO_PIN_RESET);
			user_key_duration = 0;
			send_data();
		}
		default:
			break;
		}
	}
}

void gps_Identify() {
	char *ublox_buff = "u-blox";
	char *l76K_buff = "IC=AT6558R";
	char *l76L_buff = "MTKGPS*08";
	if (strstr(gpsUartBuffer, ublox_buff) != NULL) {
		ic_version = 2;
		pdop_value = 7.00;
		//   AT_PRINTF("gps module:%s\n\r","ublox-MAX7");
	} else if (strstr(gpsUartBuffer, l76K_buff) != NULL) {
		ic_version = 4;
		pdop_value = 3.00;
		//		AT_PRINTF("gps module:%s\n\r","L76K");
	} else if (strstr(gpsUartBuffer, l76L_buff) != NULL) {
		ic_version = 1;
		pdop_value = 3.00;
		//		AT_PRINTF("gps module:%s\n\r","L76L");
	}

	else {
		ic_version = 0;
		pdop_value = 3.00;
	}
	Store_Config();
	memset(gpsUartBuffer, 0, sizeof(gpsUartBuffer));
}

void send_exti(void) {
	if (button_exitflag == 1) {
		lora_state_INT();
		button_exitflag = 0;
	}
}

void send_moin(void) {
	if (moinint_exitflag == 1) {
		MPU9250_INT();
		moinint_exitflag = 0;
	}
}

void OndownlinkLedEvent(void) {
	TimerStop(&downlinkLedTimer);
	LED::ledRedOff();
	LED::ledBlueOff();
}

void OnNetworkJoinedLedEvent(void) {
	TimerStop(&NetworkJoinedLedTimer);
	LED::ledRedOff();
	LED::ledGreenOff();
	LED::ledBlueOff();
}

void OnPressButtonTimesLedEvent(void) {
	TimerStop(&PressButtonTimesLedTimer);
	LED::ledRedOff();
}

void OnPressButtonTimeoutEvent(void) {
	TimerStop(&PressButtonTimeoutTimer);
	OnPressButtonTimeout_status = 0;
	press_button_times = 0;
}

void led_power_anim(void) {
	BSP_powerLED_Init();
	LED::ledGreenOn();
	DelayMs(200);
	LED::ledGreenOff();
	LED::ledBlueOn();
	DelayMs(200);
	LED::ledBlueOff();
	LED::ledRedOn();
	DelayMs(200);
	LED::ledRedOff();
}