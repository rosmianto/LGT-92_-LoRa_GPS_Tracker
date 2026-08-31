/* Includes ------------------------------------------------------------------*/
#include "IIC.h"
#include "at.h"
#include "bsp.h"
#include "command.h"
#include "delay.h"
#include "flash_eraseprogram.h"
#include "gpio_exti.h"
#include "gps.h"
#include "hw_rtc.h"
#include "iwdg.h"
#include "lora.h"
#include "low_power_manager.h"
#include "mpu9250.h"
#include "timeServer.h"
#include "vcom.h"
#include "version.h"
#include <Button.h>
#include <Commissioning.h>
#include <IMU.h>
#include <Region.h>
#include <Temporary.h>
#include <debug.h>
#include <hw_conf.h>
#include <hw_msp.h>
#include <led.h>
#include <math.h>

// Include drivers
#include <drivers/ConfigStorage_Dummy.h>
#include <drivers/IMUDriver_Dummy.h>

// Include subsystems
#include <ATParser.h>
#include <ConfigManager.h>

IMUDriver_Dummy imuDummy;
ConfigStorage_Dummy stgDummy;

IMU imu(imuDummy);
ConfigManager cfgMgr(stgDummy);
ATParser atParser;
Button btn;

lora_AppData_t AppData = {AppDataBuff, 0, 0};

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/


/* TODO: Add struct to represent System state:

   * Debug Mode (used to print out timestamp in various location)
   * FSM State (Sending, Init, waiting GPS, Idle) -> Need to properly design the
   state transitions
   *
*/

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
extern "C" void printf_joinmessage(void);

bool sleep_status = 0; // AT+SLEEP
int user_key_exti_flag = 0;
uint8_t user_key_duration = 0;
void user_key_event(void);

extern TimerEvent_t MacStateCheckTimer;
extern TimerEvent_t TxDelayedTimer;
extern TimerEvent_t AckTimeoutTimer;
extern TimerEvent_t RxWindowTimer1;
extern TimerEvent_t RxWindowTimer2;

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
static LoRaMainCallback_t LoRaMainCallbacks = {
    HW_GetBatteryLevel, HW_GetTemperatureLevel,
    HW_GetUniqueId,     HW_GetRandomSeed,
    LORA_RxData,        LORA_HasJoined,
    LORA_ConfirmClass};

/* !
 *Initialises the Lora Parameters
 */
static LoRaParam_t LoRaParamInit = {LORAWAN_ADR_STATE,
                                    LORAWAN_DEFAULT_DATA_RATE,
                                    LORAWAN_PUBLIC_NETWORK, JOINREQ_NBTRIALS};

/* Private functions ---------------------------------------------------------*/

void handleButtonEvent(ButtonEvent ev);
void handleCommandDownlink(uint8_t cmd, const uint8_t *data, uint8_t len);

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

  led_init();

  led_run_animation();

  IIC_GPIO_MODE_Config();

  iwdg_init();

  StartIWDGRefresh(TX_ON_EVENT);

  /*Disable Stand-by mode*/
  LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);

  /* Configure the Lora Stack*/
  LORA_Init(&LoRaMainCallbacks, &LoRaParamInit);

  // C++ starts here
  imu.init();

  // TODO: Ensure config is loaded and validated
  // with standard CRC32.
  cfgMgr.init();
  cfgMgr.loadConfig();

  gps_Identify();

  while (1) {
    /* Handle UART commands */
    CMD_Process();

    std::string_view line = "AT"; // For example.
    auto response = atParser.parseCommand(line);

    // Serial.write(response);

    if (restartRequested == 1) {
      DelayMs(500);
      AppData.Buff[0] = 0x11;
      AppData.BuffSize = 1;
      AppData.Port = 2;
      LORA_send(&AppData, LORAWAN_UNCONFIRMED_MSG);
      restartRequested++;
    } else if ((restartRequested == 2) && !isLoRaMacBusy()) {
      NVIC_SystemReset();
    }

    if (md_flags == 1) {
      MPU_INT_Init();
      md_flags = 0;
    }

#if 0
    // TODO: Code remnant. The logic below should be handled
    // in the main loop, not stm32l0xx_it.c
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_12) != RESET) {
      if (is_lora_joined == 1) {
        if (motionDetectMode != DISABLED) {
          if (isLoRaMacBusy() == false) {
            if ((stop_flag == 1) && (GPS_ALARM == 0)) {
              moinint_exitflag = 1;
            }
          }
        }
      }
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_12);
      HAL_GPIO_EXTI_Callback(GPIO_PIN_12);
    }

    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_14) != RESET) {
      if (is_lora_joined == 1) {
        button_exitflag = 1;
      }
      user_key_exti_flag = 1;
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_14);
      HAL_GPIO_EXTI_Callback(GPIO_PIN_14);
    }
#endif

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

    ButtonEvent userPress = btn.getButtonEvent();

    handleButtonEvent(userPress);

    // TODO: Eradicate this
    // user_key_event();

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
  led_red_on();
  led_blue_on();
  DelayMs(1000);
  led_red_off();
  led_blue_off(); // LoRa Joined

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
    // Roll_basic = 0;
    // Pitch_basic = 0;
    // Yaw_basic = 0;
    // basic_flag = 0;
  } else if (basic_flag == 2) {
    // Roll_basic = Roll;
    // Pitch_basic = Pitch;
    // Yaw_basic = Yaw;
    // basic_flag = 0;
  }
  if (sensor_data.bat_mv <= 2840.0) {
    LP = 1;
    PPRINTF("\n\rBattery voltage too low\r\n");
  } else {
    LP = 0;
  }

  // TODO: Wakeup MPU9250 before sampling
  // MPU9250_wakeup();

  FusionEuler ahrsEuler = imu.getImuOrientation();

  loraPayloadRoll = ahrsEuler.angle.roll;
  loraPayloadPitch = ahrsEuler.angle.pitch;

  printf_uplink();

  FLAG = (int)(motionDetectMode << 6 | LON << 5 | Firmware) & 0xFF;
  if (lora_getGPSState() == STATE_GPS_OFF) {
    AppData.Buff[i++] = 0x00;
    AppData.Buff[i++] = 0x00;
    AppData.Buff[i++] = 0x00;
    AppData.Buff[i++] = 0x00;
    AppData.Buff[i++] = 0x00;
    AppData.Buff[i++] = 0x00;
    AppData.Buff[i++] = 0x00;
    AppData.Buff[i++] = 0x00;
  } else if (lora_getGPSState() == STATE_GPS_NO) {
    AppData.Buff[i++] = 0xFF;
    AppData.Buff[i++] = 0xFF;
    AppData.Buff[i++] = 0xFF;
    AppData.Buff[i++] = 0xFF;
    AppData.Buff[i++] = 0xFF;
    AppData.Buff[i++] = 0xFF;
    AppData.Buff[i++] = 0xFF;
    AppData.Buff[i++] = 0xFF;
  } else {
    AppData.Buff[i++] = (int)latitude >> 24 & 0xFF;
    AppData.Buff[i++] = (int)latitude >> 16 & 0xFF;
    AppData.Buff[i++] = (int)latitude >> 8 & 0xFF;
    AppData.Buff[i++] = (int)latitude & 0xFF;
    AppData.Buff[i++] = (int)longitude >> 24 & 0xFF;
    AppData.Buff[i++] = (int)longitude >> 16 & 0xFF;
    AppData.Buff[i++] = (int)longitude >> 8 & 0xFF;
    AppData.Buff[i++] = (int)longitude & 0xFF;
  }
  if (set_sgm == 1) {
    if (ALARM == 1) {
      AppData.Buff[i++] = (int)(sensor_data.bat_mv) >> 8 | 0x40; // battery
      AppData.Buff[i++] = (int)sensor_data.bat_mv;

    } else {
      AppData.Buff[i++] = (int)(sensor_data.bat_mv) >> 8; // battery
      AppData.Buff[i++] = (int)sensor_data.bat_mv;
    }
    AppData.Buff[i++] = (int)FLAG;
  } else if (set_sgm == 0) {
    if (ALARM == 1) {
      AppData.Buff[i++] = (int)(sensor_data.bat_mv) >> 8 | 0x40; // battery
      AppData.Buff[i++] = (int)sensor_data.bat_mv;
    } else {
      AppData.Buff[i++] = (int)(sensor_data.bat_mv) >> 8; // battery
      AppData.Buff[i++] = (int)sensor_data.bat_mv;
    }
    AppData.Buff[i++] = (int)FLAG;
    AppData.Buff[i++] = (int)(loraPayloadRoll * 100) >> 8; // Roll
    AppData.Buff[i++] = (int)(loraPayloadRoll * 100);
    AppData.Buff[i++] = (int)(loraPayloadPitch * 100) >> 8; // Pitch
    AppData.Buff[i++] = (int)(loraPayloadPitch * 100);
    AppData.Buff[i++] = (int)(gps.HDOP * 100);          // HDOP
    AppData.Buff[i++] = (int)(gps.altitude * 100) >> 8; // Altitude
    AppData.Buff[i++] = (int)(gps.altitude * 100);
  }

  gps_setflags = 0;
  press_button_times = 0;
  AppData.BuffSize = i;
  payloadlens = i;
  IWDG_Refresh();
  LORA_send(&AppData, lora_config_reqack_get());
}

static void LORA_RxData(lora_AppData_t *AppData) {

  set_at_receive(AppData->Port, AppData->Buff, AppData->BuffSize);

  uint8_t cmd = AppData->Buff[0] & 0xff;

  handleCommandDownlink(cmd, AppData->Buff, AppData->BuffSize);

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
  if ((AppData->BuffSize <= 8) && (printResponse == 1)) {
    AT_PRINTF("%d:", AppData->Port);
    for (int i = 0; i < AppData->BuffSize; i++) {
      AT_PRINTF("%02x ", AppData->Buff[i]);
    }
    AT_PRINTF("\n\r");
  } else {
    AT_PRINTF("BuffSize:%d,Run AT+RECVB=? to see detail\r\n",
              AppData->BuffSize);
  }
  printResponse = 0;
}

#if defined(LoRa_Sensor_Node)
static void OnTxTimerEvent(void) {

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
    gps.latitude = 0;
    gps.longitude = 0;
  }
}

static void timing(void) {
  uint32_t temp_time = 0;
  if (motionDetectMode != DISABLED) {
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

static void LORA_ConfirmClass(DeviceClass_t Class) {
  PRINTF("switch to class %c done\n\r", "ABC"[Class]);

  /*Optionnal*/
  /*informs the server that switch has occurred ASAP*/
  AppData.BuffSize = 0;
  AppData.Port = LORAWAN_APP_PORT;

  LORA_send(&AppData, LORAWAN_UNCONFIRMED_MSG);
}

void lora_send(void) {
  switch (lora_getState()) {
  case STATE_LED: {
    stop_flag = 1;
    if (Positioning_time != 0) {
      GPS_POWER_OFF();
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
            (gpsModel == GPS_UBLOX_MAX7)) {
          gps_latitude = gps.latitude;
          gps_longitude = gps.longitude;
          pdop_fixed = pdop_gps;
          SendData = 1;
          TimerStart(&IWDGRefreshTimer);
          if (Positioning_time != 0) {
            GPS_POWER_OFF();
          }
        } else if (pdop_value < pdop_gps) {
          if ((pdop_gps < pdop_comp) && (pdop_gps != 0.0)) {
            gps_latitude = gps.latitude;
            gps_longitude = gps.longitude;
            pdop_comp = pdop_gps;
          }
        }
        if ((((Positioning_time != 0) && (End_times >= Positioning_time)) ||
             ((Positioning_time == 0) && (End_times >= 150))) &&
            (gps_latitude > 0 && gps_longitude > 0)) {
          SendData = 1;
          if (Positioning_time != 0) {
            GPS_POWER_OFF();
          }
        }
      }
    }

    if (SendData == 1) {
      if (GPS_ALARM == 0) {
        gps_state_on();
        if (LON == 1) {
          led_blue_on();
          DelayMs(200);
          led_blue_off();
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
            led_red_on();
            DelayMs(1000);
          }
          led_red_off();
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
            led_blue_on();
            DelayMs(1000);
          }
          led_blue_off();
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
        led_red_on();
        Send();
        DelayMs(5000);
        led_red_off();
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
      led_green_off();
    }

    else if ((LP == 1) || (LP == 2)) {
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
      if (motionDetectMode == DISABLED) {
        MPU_Write_Byte(MPU9250_ADDR, 0x6B, 0X40); // MPU sleep
      } else {
        MPU_INT_Init();
      }
      lora_state_Led();
      a = 1;
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
    }

    if (Start_times == TIMES) {
      End_times++;
      Start_times = 0;
      gps_time++;
      TimerStart(&IWDGRefreshTimer);
      if (LON == 1) {
        led_green_on();
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

    if (
        (((Positioning_time != 0) && (End_times >= Positioning_time)) ||
         ((Positioning_time == 0) && (End_times >= 150)))) {
      send_fail = 1;
      if (Positioning_time != 0) {
        GPS_POWER_OFF();
      }
      if (GPS_ALARM == 0) {
        led_red_off();
        led_blue_off();
        led_green_off();
        gps_state_off();
        a = 100;
        if (LON == 1) {
          led_red_on();
          DelayMs(200);
          led_red_off();
          DelayMs(200);
          led_red_on();
          DelayMs(200);
          led_red_off();
          DelayMs(200);
        }
        send_data();
        GPS_ALARM = 0;
      } else if (GPS_ALARM == 1) {
        if (Alarm_times <= 60) {
          led_red_off();
          led_blue_off();
          led_green_off();
          if (LON == 1) {
            led_red_on();
            DelayMs(1000);
          }
          led_red_off();
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
            led_blue_on();
            DelayMs(1000);
          }
          led_blue_off();
          PPRINTF("Exit Alarm\r\n");
        }
      }
      TimerStart(&IWDGRefreshTimer);
      send_fail = 0;
    }
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
  if (motionDetectMode == DISABLED) {
    MPU_Write_Byte(MPU9250_ADDR, 0x6B, 0X40); // MPU sleep
  } else {
    MPU_INT_Init();
  }
  lora_state_Led();
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
  if (red == 1) {
    led_red_on();
  }
  if (blue == 1) {
    led_blue_on();
  }
  if (greed == 1) {
    led_green_on();
  }
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
      break;
    }
    case 2: // sleep
    {
      sleep_status = 1;
      TimerStop(&MacStateCheckTimer);
      TimerStop(&TxDelayedTimer);
      TimerStop(&AckTimeoutTimer);

      TimerStop(&RxWindowTimer1);
      TimerStop(&RxWindowTimer2);
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

// TODO: We want to identify different GPS modules at runtime
void gps_Identify() {
  char *ublox_buff = "u-blox";
  char *l76K_buff = "IC=AT6558R";
  char *l76L_buff = "MTKGPS*08";
  if (strstr(DATABUFF, ublox_buff) != NULL) {
    gpsModel = GPS_UBLOX_MAX7;
    pdop_value = 7.00;
  } else if (strstr(DATABUFF, l76K_buff) != NULL) {
    gpsModel = GPS_L76K;
    pdop_value = 3.00;
  } else if (strstr(DATABUFF, l76L_buff) != NULL) {
    gpsModel = GPS_L76L;
    pdop_value = 3.00;
  } else {
    gpsModel = GPS_L70RL;
    pdop_value = 3.00;
  }
  Store_Config();
  memset(DATABUFF, 0, sizeof(DATABUFF));
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
  HAL_GPIO_WritePin(LED1_PORT, LED3_PIN | LED1_PIN, GPIO_PIN_RESET);
}

void OnNetworkJoinedLedEvent(void) {
  TimerStop(&NetworkJoinedLedTimer);
  HAL_GPIO_WritePin(LED1_PORT, LED3_PIN | LED0_PIN | LED1_PIN, GPIO_PIN_RESET);
}

void OnPressButtonTimesLedEvent(void) {
  TimerStop(&PressButtonTimesLedTimer);
  HAL_GPIO_WritePin(LED1_PORT, LED3_PIN, GPIO_PIN_RESET);
}

void OnPressButtonTimeoutEvent(void) {
  TimerStop(&PressButtonTimeoutTimer);
  OnPressButtonTimeout_status = 0;
  press_button_times = 0;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

extern "C" {
int _close(int file) { return -1; }
int _lseek(int file, int ptr, int dir) { return 0; }
int _read(int file, char *ptr, int len) { return 0; }
int _write(int file, char *ptr, int len) { return len; }
}

void handleButtonEvent(ButtonEvent ev) {
  if (ev == ButtonEvent::Click_long) {
    // Send LoRaWAN payload
  } else if (ev == ButtonEvent::Click_2times) {
    // Enter Deep Sleep Mode
  } else if (ev == ButtonEvent::Click_3times) {
    // System Wake / Rejoin Network
  } else if (ev == ButtonEvent::Click_4times) {
    // GPS Test Mode (always on?)
  } else if (ev == ButtonEvent::Click_5times) {
    // Exit Panic Mode
  }
}

void handleCommandDownlink(uint8_t cmd, const uint8_t *data, uint8_t len) {
  switch (cmd) {
  case 0x01: {
    if (len == 4) //---->AT+TDC
    {
      ServerSetTDC = (data[1] << 16 | data[2] << 8 |
                      data[3]); // S

      if (ServerSetTDC < 6) {
        Server_TX_DUTYCYCLE = 6000;
      } else {
        TDC_flag = 1;
        Server_TX_DUTYCYCLE = ServerSetTDC * 1000;
      }
      printResponse = 1;
    }
    break;
  }

  case 0x04: {
    if (len == 2) {
      if (data[1] == 0xFF) //---->ATZ
      {
        restartRequested = 1;
        printResponse = 1;
      } else if (data[1] == 0xFE) //---->AT+FDR
      {
        FLASH_erase(0x8018F80); // page 799
        FLASH_program_on_addr(0x8018F80, 0x12);
        FLASH_erase(FLASH_USER_START_ADDR_CONFIG); // Page800
        restartRequested = 1;
        printResponse = 1;
      }
    }
    break;
  }

  case 0x05: {
    if (len == 2) {
      if (data[1] == 0x01) //---->AT+CFM=1
      {
        lora_config_reqack_set(LORAWAN_CONFIRMED_MSG);
        Store_Config();
        printResponse = 1;
      } else if (data[1] == 0x00) //---->AT+CFM=0
      {
        lora_config_reqack_set(LORAWAN_UNCONFIRMED_MSG);
        Store_Config();
        printResponse = 1;
      }
    }
    break;
  }

  case 2: {
    if (len == 2) {
      if (data[1] == 0x01) {
        start_time = HW_RTC_GetTimerValue();
        Alarm_times = 60;
        Alarm_times1 = 60;
        GPS_ALARM = 0;
        ALARM = 0;
        if (LON == 1) {
          BSP_sensor_Init();
          led_blue_on();
          DelayMs(1000);
        }
        led_blue_off(); // Exit Alarm
        PRINTF("Exit Alarm\r\n");
      }
    }
    break;
  }
  case 0xa5: {
    if (len == 2) {
      motionDetectMode = static_cast<MotionDetectionMode>(data[1]);
      PRINTF("motionDetectMode: %02x\n\r", motionDetectMode);
      if (data[1] != 0x00) {
        start_time = HW_RTC_GetTimerValue();
      }
    } else if (len == 4) {
      if (data[1] == 0x03) {
        motionDetectMode = static_cast<MotionDetectionMode>(data[1]);
        Threshold = data[2];
        Freq = data[3];
        PRINTF("Set motionDetectMode: %02x,%02x,%02x\n\r", motionDetectMode,
               Threshold, Freq);
      }
      if (data[1] != 0x00) {
        start_time = HW_RTC_GetTimerValue();
      }
    }
    md_flags = 1;
    Store_Config();
    break;
  }
  case 0xaa: {
    if (len == 3) {
      Positioning_time = (data[1] << 8 | data[2]);
      if (Positioning_time == 1203) {
        LP = 2;
      } else {
        LP = 0;
      }
    }
    Store_Config();
    break;
  }
  case 0xab: {
    if (len == 2) {
      navMode = static_cast<GPSNavMode>(data[1]);
    }
    Store_Config();
    break;
  }
  case 0xac: {
    if (len == 2) {
      searchMode = static_cast<GPSSearchMode>(data[1]);
    }
    Store_Config();

    break;
  }
  case 0xad: {
    if (len == 3) {
      pdop_value = (data[1] << 8 | data[2]) / 10.0;
    }
    Store_Config();

    break;
  }
  case 0xae: {
    if (len == 2) {
      LON = data[1];
    }
    Store_Config();

    break;
  }
  case 0xaf: {
    if (len == 2) {
      MLON = data[1];
    }
    Store_Config();

    break;
  }
  case 0xa8: {
    if (len == 10) {
      if (data[1] == 0x01) {
        BSP_powerLED_Init();
        led_red_on();
        red = 1;
        led_red = data[2] << 8 | data[3];
        if (led_red != 0) {
          DelayMs(led_red);
          led_red_off();
          red = 0;
        }
      } else {
        red = 0;
        led_red_off();
      }
      if (data[4] == 0x01) {
        BSP_powerLED_Init();
        led_blue_on();
        blue = 1;
        led_blue = data[5] << 8 | data[6];
        if (led_blue != 0) {
          DelayMs(led_blue);
          led_blue_off(); // Debug LED
          blue = 0;
        }
      } else {
        blue = 0;
        led_blue_off(); // Debug LED
      }
      if (data[7] == 0x01) {
        BSP_powerLED_Init();
        led_green_on();
        greed = 1;
        led_greed = data[8] << 8 | data[9];
        if (led_blue != 0) {
          DelayMs(led_greed);
          led_green_off();
          greed = 0;
        }
      } else {
        greed = 0;
        led_green_off();
      }
    }
    Store_Config();

    break;
  }
  case 0xb0: {
    if (len == 2) {
      set_sgm = data[1];
    }
    Store_Config();

    break;
  }
  case 0xb1: {
    ServerSetTDC = (data[1] << 16 | data[2] << 8 |
                    data[3]); // S
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
  case 0xa9: {
    if (len == 4) {
      ServerSetTDC = (data[1] << 16 | data[2] << 8 |
                      data[3]); // S

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
  case 0x20: {
    if (len == 2) {
      if ((data[1] == 0x00) || (data[1] == 0x01)) {
        if (data[1] == 0x01) //---->AT+NJM=1
        {
          lora_config_otaa_set(LORA_ENABLE);
        } else //---->AT+NJM=0
        {
          lora_config_otaa_set(LORA_DISABLE);
        }
        Store_Config();
        restartRequested = 1;
        printResponse = 1;
      }
    }
    break;
  }

  case 0x21: {
    if ((len == 2) && (data[1] <= 4)) {
      response_level = (data[1]); // 0~4
                                           // //---->AT++RPL
      Store_Config();
      printResponse = 1;
    }
    break;
  }

  case 0x22: {
    MibRequestConfirm_t mib;
    if ((len == 2) && (data[1] == 0x01)) //---->AT+ADR=1
    {
      mib.Type = MIB_ADR;
      mib.Param.AdrEnable = data[1];
      LoRaMacMibSetRequestConfirm(&mib);
      Store_Config();
      printResponse = 1;
    } else if ((len == 4) &&
               (data[1] == 0x00)) //---->AT+ADR=0
    {
      mib.Type = MIB_ADR;
      mib.Param.AdrEnable = data[1];
      LoRaMacMibSetRequestConfirm(&mib);
      if (data[2] != 0xff) //---->AT+DR
      {
        mib.Type = MIB_CHANNELS_DATARATE;
        mib.Param.ChannelsDatarate = data[2];
        LoRaMacMibSetRequestConfirm(&mib);
      }
      if (data[3] != 0xff) //---->AT+TXP
      {
        mib.Type = MIB_CHANNELS_TX_POWER;
        mib.Param.ChannelsTxPower = data[3];
        LoRaMacMibSetRequestConfirm(&mib);
      }
      Store_Config();
      printResponse = 1;
    }
    break;
  }

  case 0x23: {
    if (len == 2) {
      lora_config_application_port_set(data[1]); //---->AT+PORT
      Store_Config();
      printResponse = 1;
    }
    break;
  }

  case 0x24: {
#if defined(REGION_US915) || defined(REGION_AU915) || defined(REGION_CN470)
    if (len == 2) {
      if (data[1] <= 0x0C) {
        customize_set8channel_set(data[1]); //---->AT+CHE
        Store_Config();
        printResponse = 1;
      }
    }
#endif
    break;
  }

  case 0x25: {
#if defined(REGION_AS923) || defined(REGION_AU915)
    if (len == 2) {
      if ((data[1] == 0x00) ||
          (data[1] == 0x01)) //---->AT+DWELLT
      {
        dwelltime = data[1];
        Store_Config();
        restartRequested = 1;
        printResponse = 1;
      }
    }
#endif
    break;
  }

  default:
    break;
  }
}