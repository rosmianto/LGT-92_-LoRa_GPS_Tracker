#include "LoRa.h"

#include "LoRaMac.h"
#include "lora.h"

// We're calling C++ functions from pure C LoRaWAN
// stack. So we need to create a bridge to store the _this
static void *this_ptr = nullptr;

static LoRaMainCallback_t LoRaMainCallbacks = {
	HW_GetBatteryLevel, HW_GetTemperatureLevel,
	HW_GetUniqueId,		HW_GetRandomSeed,
	LORA_RxData,		LORA_HasJoined,
	LORA_ConfirmClass};

/* !
 *Initialises the Lora Parameters
 */
static LoRaParam_t LoRaParamInit = {LORAWAN_ADR_STATE,
									LORAWAN_DEFAULT_DATA_RATE,
									LORAWAN_PUBLIC_NETWORK, JOINREQ_NBTRIALS};

LoRa::LoRa() { this_ptr = this; }

bool LoRa::init() {
	/* Configure the Lora Stack*/
	LORA_Init(&LoRaMainCallbacks, &LoRaParamInit);
	return true;
}

bool LoRa::stop() { loramacstop }

bool LoRa::sendPayload(uint8_t *payload, uint8_t len) {
	lora_AppData_t appData;
	appData.Port = 2; // TODO: Replace this to get the value from Config
	appData.Buff = payload;
	appData.BuffSize = len;

	bool txResult = LORA_send(&appData, lora_config_reqack_get());
	return txResult;
}

void LORA_ConfirmClass(DeviceClass_t Class) {
	PRINTF("switch to class %c done\n\r", "ABC"[Class]);

	/* Optionnal */
	/* informs the server that switch has occurred ASAP*/
	AppData.BuffSize = 0;
	AppData.Port = LORAWAN_APP_PORT;

	LORA_send(&AppData, LORAWAN_UNCONFIRMED_MSG);
}

static void LORA_RxData(lora_AppData_t *AppData) {

	set_at_receive(AppData->Port, AppData->Buff, AppData->BuffSize);

	switch (AppData->Buff[0] & 0xff) {
	case DOWNLINK_CMD_SET_TRANSMIT_INTERVAL: {
		if (AppData->BuffSize == 4) //---->AT+TDC
		{
			ServerSetTDC = (AppData->Buff[1] << 16 | AppData->Buff[2] << 8 |
							AppData->Buff[3]); // S

			if (ServerSetTDC < 6) {
				Server_TX_DUTYCYCLE = 6000;
			} else {
				TDC_flag = 1;
				Server_TX_DUTYCYCLE = ServerSetTDC * 1000;
			}
			rxpr_flags = 1;
		}
		break;
	}

	case DOWNLINK_CMD_DEVICE_TRIGGER: {
		if (AppData->BuffSize == 2) {
			if (AppData->Buff[1] == 0xFF) //---->ATZ
			{
				device_reset_trigger = 1;
				rxpr_flags = 1;
			} else if (AppData->Buff[1] == 0xFE) //---->AT+FDR
			{
				FLASH_erase(0x8018F80); // page 799
				FLASH_program_on_addr(0x8018F80, 0x12);
				FLASH_erase(FLASH_USER_START_ADDR_CONFIG); // Page800
				device_reset_trigger = 1;
				rxpr_flags = 1;
			}
		}
		break;
	}

	case DOWNLINK_CMD_CONFIRM_MODE: {
		if (AppData->BuffSize == 2) {
			if (AppData->Buff[1] == 0x01) //---->AT+CFM=1
			{
				lora_config_reqack_set(LORAWAN_CONFIRMED_MSG);
				Store_Config();
				rxpr_flags = 1;
			} else if (AppData->Buff[1] == 0x00) //---->AT+CFM=0
			{
				lora_config_reqack_set(LORAWAN_UNCONFIRMED_MSG);
				Store_Config();
				rxpr_flags = 1;
			}
		}
		break;
	}

	case DOWNLINK_CMD_EXIT_ALARM_MODE: {
		if (AppData->BuffSize == 2) {
			if (AppData->Buff[1] == 0x01) {
				start_time = HW_RTC_GetTimerValue();
				Alarm_times = 60;
				Alarm_times1 = 60;
				GPS_ALARM = 0;
				ALARM = 0;
				if (LON == 1) {
					BSP_sensor_Init();
					LED::ledBlueOn();
					DelayMs(1000);
				}
				ledBlueOff();
				PRINTF("Exit Alarm\r\n");
			}
		}
		break;
	}
	case DOWNLINK_CMD_MOVEMENT_DETECTION_MODE: {
		if (AppData->BuffSize == 2) {
			MD = AppData->Buff[1];
			PRINTF("MD: %02x\n\r", MD);
			if (AppData->Buff[1] != 0x00) {
				start_time = HW_RTC_GetTimerValue();
			}
		} else if (AppData->BuffSize == 4) {
			if (AppData->Buff[1] == 0x03) {
				MD = AppData->Buff[1];
				Threshold = AppData->Buff[2];
				Freq = AppData->Buff[3];
				PRINTF("Set MD: %02x,%02x,%02x\n\r", MD, Threshold, Freq);
			}
			if (AppData->Buff[1] != 0x00) {
				start_time = HW_RTC_GetTimerValue();
			}
		}
		md_flags = 1;
		Store_Config();
		break;
	}
	case DOWNLINK_CMD_GPS_FIXTIME: {
		if (AppData->BuffSize == 3) {
			Positioning_time = (AppData->Buff[1] << 8 | AppData->Buff[2]);
			if (Positioning_time == 1203) {
				LP = 2;
			} else {
				LP = 0;
			}
		}
		Store_Config();
		break;
	}
	case DOWNLINK_CMD_NAVIGATION_MODE: {
		if (AppData->BuffSize == 2) {
			gps_navigation_mode = AppData->Buff[1];
		}
		Store_Config();
		break;
	}
	case DOWNLINK_CMD_GPS_SEARCH_MODE: {
		if (AppData->BuffSize == 2) {
			gps_search_mode = AppData->Buff[1];
		}
		Store_Config();

		break;
	}
	case DOWNLINK_CMD_GPS_PDOP: {
		if (AppData->BuffSize == 3) {
			pdop_value = (AppData->Buff[1] << 8 | AppData->Buff[2]) / 10.0;
		}
		Store_Config();

		break;
	}
	case DOWNLINK_CMD_LED_ON: {
		if (AppData->BuffSize == 2) {
			LON = AppData->Buff[1];
		}
		Store_Config();

		break;
	}
	case DOWNLINK_CMD_MOVEMENT_LED_ON: {
		if (AppData->BuffSize == 2) {
			MLON = AppData->Buff[1];
		}
		Store_Config();

		break;
	}

	case DOWNLINK_CMD_SET_RGB_STATE: {
		if (AppData->BuffSize == 10) {
			struct RgbLedState state;

			// Copy the downlink payload into struct, so we
			// can easily interact with the data
			memcpy(&state, &(AppData->Buff[1]), 9);

			if (state.isRedOn == true) {
				LED::ledRedOn();
				DelayMs(state.redOnDuration);
				LED::ledRedOff();
			} else {
				LED::ledRedOff();
			}

			if (state.isGreenOn == true) {
				LED::ledGreenOn();
				DelayMs(state.greenOnDuration);
				LED::ledGreenOn();
			} else {
				LED::ledGreenOn();
			}

			if (state.isBlueOn == true) {
				LED::ledBlueOn();
				DelayMs(state.blueOnDuration);
				LED::ledBlueOn();
			} else {
				LED::ledBlueOn();
			}
		}
		break;
	}
	case DOWNLINK_CMD_INCLUDE_MOTION_DATA: {
		if (AppData->BuffSize == 2) {
			set_sgm = AppData->Buff[1];
		}
		Store_Config();

		break;
	}
	case DOWNLINK_CMD_ALARM_TX_INTERVAL: {
		ServerSetTDC = (AppData->Buff[1] << 16 | AppData->Buff[2] << 8 |
						AppData->Buff[3]); // S
		if (ServerSetTDC < 6) {
			PRINTF("ACE setting must be more than 10S\n\r");
			Alarm_TX_DUTYCYCLE = 10000;
		} else {
			TDC_flag = 1;
			Alarm_TX_DUTYCYCLE = ServerSetTDC * 1000;
			PRINTF("Set ACE: %d ms\n\r", Alarm_TX_DUTYCYCLE);
		}
		Store_Config();

		break;
	}
	case DOWNLINK_CMD_KEEPALIVE_TIME: {
		if (AppData->BuffSize == 4) {
			ServerSetTDC = (AppData->Buff[1] << 16 | AppData->Buff[2] << 8 |
							AppData->Buff[3]); // S

			if (ServerSetTDC < 360) {
				PRINTF("KAT setting must be more than 6m\n\r");
				Keep_TX_DUTYCYCLE = 360000;
			} else {
				Keep_TX_DUTYCYCLE = ServerSetTDC * 1000;
				PRINTF("Set KAT: %d ms\n\r", Keep_TX_DUTYCYCLE);
			}
			Store_Config();
		}
		break;
	}
	case DOWNLINK_CMD_NETWORK_JOINMODE: {
		if (AppData->BuffSize == 2) {
			if ((AppData->Buff[1] == 0x00) || (AppData->Buff[1] == 0x01)) {
				if (AppData->Buff[1] == 0x01) //---->AT+NJM=1
				{
					lora_config_otaa_set(LORA_ENABLE);
				} else //---->AT+NJM=0
				{
					lora_config_otaa_set(LORA_DISABLE);
				}
				Store_Config();
				device_reset_trigger = 1;
				rxpr_flags = 1;
			}
		}
		break;
	}

	case DOWNLINK_CMD_PACKET_RESPONSE_LEVEL: {
		if ((AppData->BuffSize == 2) && (AppData->Buff[1] <= 4)) {
			response_level =
				(AppData->Buff[1]); // 0~4					//---->AT++RPL
			Store_Config();
			rxpr_flags = 1;
		}
		break;
	}

	case DOWNLINK_CMD_ADAPTIVE_DATARATE: {
		MibRequestConfirm_t mib;
		if ((AppData->BuffSize == 2) &&
			(AppData->Buff[1] == 0x01)) //---->AT+ADR=1
		{
			mib.Type = MIB_ADR;
			mib.Param.AdrEnable = AppData->Buff[1];
			LoRaMacMibSetRequestConfirm(&mib);
			Store_Config();
			rxpr_flags = 1;
		} else if ((AppData->BuffSize == 4) &&
				   (AppData->Buff[1] == 0x00)) //---->AT+ADR=0
		{
			mib.Type = MIB_ADR;
			mib.Param.AdrEnable = AppData->Buff[1];
			LoRaMacMibSetRequestConfirm(&mib);
			if (AppData->Buff[2] != 0xff) //---->AT+DR
			{
				mib.Type = MIB_CHANNELS_DATARATE;
				mib.Param.ChannelsDatarate = AppData->Buff[2];
				LoRaMacMibSetRequestConfirm(&mib);
			}
			if (AppData->Buff[3] != 0xff) //---->AT+TXP
			{
				mib.Type = MIB_CHANNELS_TX_POWER;
				mib.Param.ChannelsTxPower = AppData->Buff[3];
				LoRaMacMibSetRequestConfirm(&mib);
			}
			Store_Config();
			rxpr_flags = 1;
		}
		break;
	}

	case DOWNLINK_CMD_APP_PORT: {
		if (AppData->BuffSize == 2) {
			lora_config_application_port_set(AppData->Buff[1]); //---->AT+PORT
			Store_Config();
			rxpr_flags = 1;
		}
		break;
	}

	case DOWNLINK_CMD_EIGHT_CH_MODE: {
#if defined(REGION_US915) || defined(REGION_AU915) || defined(REGION_CN470)
		if (AppData->BuffSize == 2) {
			if (AppData->Buff[1] <= 0x0C) {
				customize_set8channel_set(AppData->Buff[1]); //---->AT+CHE
				Store_Config();
				rxpr_flags = 1;
			}
		}
#endif
		break;
	}

	case DOWNLINK_CMD_DWELLTIME: {
#if defined(REGION_AS923) || defined(REGION_AU915)
		if (AppData->BuffSize == 2) {
			if ((AppData->Buff[1] == 0x00) ||
				(AppData->Buff[1] == 0x01)) //---->AT+DWELLT
			{
				dwelltime = AppData->Buff[1];
				Store_Config();
				device_reset_trigger = 1;
				rxpr_flags = 1;
			}
		}
#endif
		break;
	}

	default:
		break;
	}

	if (TDC_flag == 1) {
		Store_Config();
		TimerInit(&TxTimer, OnTxTimerEvent);
		TimerSetValue(&TxTimer, APP_TX_DUTYCYCLE);
		TimerStart(&TxTimer);
		TimerStart(&IWDGRefreshTimer);
		TDC_flag = 0;
	}

	AT_PRINTF("\r\n");
	AT_PRINTF("Receive data\n\r");
	if ((AppData->BuffSize <= 8) && (rxpr_flags == 1)) {
		AT_PRINTF("%d:", AppData->Port);
		for (int i = 0; i < AppData->BuffSize; i++) {
			AT_PRINTF("%02x ", AppData->Buff[i]);
		}
		AT_PRINTF("\n\r");
	} else {
		AT_PRINTF("BuffSize:%d,Run AT+RECVB=? to see detail\r\n",
				  AppData->BuffSize);
	}
	rxpr_flags = 0;
}

/* call back when LoRa endNode has received a frame*/
static void LORA_RxData(lora_AppData_t *AppData);

/* call back when LoRa endNode has just joined*/
static void LORA_HasJoined(void);

/* call back when LoRa endNode has just switch the class*/
static void LORA_ConfirmClass(DeviceClass_t Class);

/* LoRa endNode send request*/
static void Send(void);

static void lora_send(void);

void send_ALARM_data(void);