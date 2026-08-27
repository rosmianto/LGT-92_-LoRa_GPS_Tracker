#include "gps.h"
#include "at.h"
#include "delay.h"
#include "vcom.h"

#include "stm32l072xx.h"
#include "stm32l0xx_hal.h"
#include "stm32l0xx_nucleo.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bsp_usart2.h"
#include <ctype.h>

#define SEMICOLON ','
#define ASTERISK '*'

/*
    TODO:
    This module has way more responsibilities than it should:
    * HAL GPS UART init/deinit (It's HAL's job)
    * Parsing NMEA sentence (it's NMEA parser's job)
    * Managing GPS power state (it's correct here)
    * Copying GPS command data
    * Pausing GPS NMEA sentence parsing when isFirmwareUpdate == true
    *
*/

GPSINFO gps;

char lasttime[20];
_Bool isrunning;
uint32_t isFirmwareUpdate = 0;
int count = 0;
uint8_t gpspower_flag = 0;
float pdop_gps;
char *txdata353;
char *txdata886;

extern UART_HandleTypeDef uart1;
extern uint8_t gps_setflags;
extern uint8_t se_mode;
extern uint8_t fr_mode;
extern uint8_t ic_version;
extern uint32_t loggps;

_Bool GPS_Run(void) {
  if (isrunning)
    return 0;

  isrunning = 1;
  GPS_init();
  GPS_STANDBY_L();
  // Delay_ms(100);
  GPS_RESET_OFF();
  GPS_POWER_ON();

  return isrunning;
}

void GPS_Stop(void) { isrunning = 0; }

void GPS_FirmwareUpdate(void) { isFirmwareUpdate = 1; }

_Bool GPS_IsRunning(void) { return isrunning; }

void GPS_DegreeToDMS(FP32 deg, int *d, int *m, FP32 *s) {

  *d = (int)deg;
  *m = (int)((deg - *d) * 60);
  *s = (((deg - *d) * 60) - *m) * 60;
  if (60.0 - *s < 0.01) {
    *s = 0.0;
    *m++;
    if (*m == 60) {
      *d++;
      *m = 0;
    }
  }
}

int my_atoi(const char *str) {
  int result = 0;
  int signal = 1; /* Ĭ��Ϊ���� */
  if ((*str >= '0' && *str <= '9') || *str == '-' || *str == '+') {
    if (*str == '-' || *str == '+') {
      if (*str == '-')
        signal = -1; /* ���븺�� */
      str++;
    }
  } else
    return 0;

  /* ��ʼת�� */
  while (*str >= '0' && *str <= '9')
    result = result * 10 + (*str++ - '0');

  return signal * result;
}

FP32 my_strtod(const char *s, char **endptr)

{

  const char *p = s;

  FP32 value = 0.L;

  int sign = 0;

  FP32 factor;

  unsigned int expo;

  while (isspace(*p)) // ����ǰ��Ŀո�

    p++;

  if (*p == '-' || *p == '+')

    sign = *p++; // �ѷ��Ÿ����ַ�sign��ָ����ơ�

  // ���������ַ�

  while ((unsigned int)(*p - '0') < 10u) // ת����������

    value = value * 10 + (*p++ - '0');

  // ����������ı�ʾ��ʽ���磺1234.5678��

  if (*p == '.')

  {

    factor = 1.;

    p++;

    while ((unsigned int)(*p - '0') < 10u)

    {

      factor *= 0.1;

      value += (*p++ - '0') * factor;
    }
  }

  // �����IEEE754��׼�ĸ�ʽ���磺1.23456E+3��

  if ((*p | 32) == 'e') {

    expo = 0;

    factor = 10.L;

    switch (*++p)

    {

    case '-':

      factor = 0.1;

    case '+':

      p++;

      break;

    case '0':

    case '1':

    case '2':

    case '3':

    case '4':

    case '5':

    case '6':

    case '7':

    case '8':

    case '9':

      break;

    default:

      value = 0.L;

      p = s;

      goto done;
    }

    while ((unsigned int)(*p - '0') < 10u)

      expo = 10 * expo + (*p++ - '0');

    while (1)

    {

      if (expo & 1)

        value *= factor;

      if ((expo >>= 1) == 0)

        break;

      factor *= factor;
    }
  }

done:

  if (endptr != 0)

    *endptr = (char *)p;

  return (sign == '-' ? -value : value);
}

FP32 my_atof(char *str)

{

  return my_strtod(str, 0);
}

char *match(char *str, char *patten) {
  while (*str != 0 && *patten != 0 && *str == *patten) {
    str++;
    patten++;
  }

  if (*patten == 0)
    return str + 1;
  else
    return NULL;
}

char *next_fld(char *str) {
  char *ts = str;

  while (*ts != ',' && *ts != 0)
    ts++;
  *ts = '\0';

  return str;
}
char *split(char *buf, char s, char **left) {

  char *p = buf, *ret = buf;

  if (buf == NULL || buf[0] == 0) {
    *left = NULL;
    return NULL;
  }

  while (*p != 0 && *p != s && *p != '\r' && *p != '\n') {
    p++;
  }

  if (*p != 0) {
    *left = p + 1;
    *p = 0;
  } else {
    *left = NULL;
  }

  return ret;
}

unsigned char digits[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                            '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
int check(char *sentence, char *cksum) {

  unsigned char *p = (unsigned char *)sentence, sum = 0 /*,ts*/;
  if (sentence == NULL || cksum == NULL)
    return 0;

  for (; *p != 0; p++) {
    sum ^= *p;
  }

  if (digits[sum >> 4] == cksum[0] && digits[sum & 0x0f] == cksum[1])
    return 1;
  /*
      sscanf(cksum,"%x",&ts);
      if(sum != ts)
          return 0;
  */
  return 0;
}

uint8_t GPS_parse(char *buf) {
  // printf("%s\n",buf);
  int d, m, mm;
  uint8_t i;
  char *word, *left = buf + 1;
  static uint8_t msgcount = 0, msgid = 0, satcount = 0; // ����GSV�õ��ı���
                                                        // ��ͨ�����õ����Ǳ��
  uint8_t usedsatcount = 0;

  if (buf[0] != '$')
    return 0;

  word = split(left, ASTERISK, &left);
  if (check(word, left) != 1)
    return 0;

  left = word;

  word = split(left, SEMICOLON, &left);
  if (!strcmp(word, "PMTK001")) {
    gps_setflags = 1;
    gpspower_flag = 0;
  }

  if (!strcmp(word, "GNRRMC")) {
    // ʱ���
    //        AT_PRINTF("GNRMC:%s\n\r",word);
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GNRMC1:%s\n\r",word);
    if (word != NULL) {
      if (strncmp(lasttime, word, 20)) {
        sscanf(word, "%2d%2d%2d.%4d", &gps.hh, &gps.mm, &gps.ss, &gps.ms);
        //                AT_PRINTF("%2d%2d%2d.%4d",&gps.hh,&gps.mm,&gps.ss,&gps.ms);
        strncpy(lasttime, word, 20);
      }
    }

    // ��λ״̬
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GNRMC2:%s\n\r",word);
    if (word != NULL) {
      if (word[0] == 'A')
        gps.isvalid = 1;
      else
        gps.isvalid = 0;
    }

    // γ��
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GNRMC3:%s\n\r",word);
    if (word != NULL) {

      sscanf(word, "%2d%2d.%4d", &d, &m, &mm);
      gps.latitude = (float)d + (float)m / 60.0 + (float)mm / 600000.0;
      PRINTF("%s: %.6f��\n\r", gps.latitude);
    }

    // �ϱ������־
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GNRMC4:%s\n\r",word);
    if (word != NULL) {
      if (word[0] == 'S') { // gps.latitude=-gps.latitude;
        gps.latNS = 'S';
      } else {
        gps.latNS = 'N';
      }
    }

    // ����
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GNRMC5:%s\n\r",word);
    if (word != NULL) {
      int d, m, mm;
      sscanf(word, "%3d%2d.%4d", &d, &m, &mm);
      gps.longitude = (float)d + (float)m / 60.0 + (float)mm / 600000.0;
    }

    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GNRMC6:%s\n\r",word);
    if (word != NULL) {
      if (word[0] == 'W') {
        gps.lgtEW = 'W';
        // gps.longitude=-gps.longitude;
      } else {
        gps.lgtEW = 'E';
      }
    }

    // �Ե��˶��ٶ�
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GNRMC7:%s\n\r",word);
    if (word != NULL) {
      gps.speed = my_atof(word) * 1.852;
    }

    // �Ե��˶�����
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GNRMC8:%s\n\r",word);
    if (word != NULL) {
      gps.direction = my_atof(word);
    }

    // utc ����
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GNRMC9:%s\n\r",word);
    if (word != NULL) {
      sscanf(word, "%2d%2d%2d", &gps.DD, &gps.MM, &gps.YY);
    }
  } else if (!strcmp(word, "GPRMC")) {
    // ʱ���
    //			  AT_PRINTF("GPRMC:%s\n\r",word);
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPRMC0:%s\n\r",word);
    if (word != NULL) {
      if (strncmp(lasttime, word, 20)) {
        sscanf(word, "%2d%2d%2d.%4d", &gps.hh, &gps.mm, &gps.ss, &gps.ms);
        strncpy(lasttime, word, 20);
      }
    }

    // ��λ״̬
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPRMC1:%s\n\r",word);
    if (word != NULL) {
      if (word[0] == 'A')
        gps.isvalid = 1;
      else
        gps.isvalid = 0;
    }

    // γ��
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPRMC2:%s\n\r",word);
    if (word != NULL) {

      sscanf(word, "%2d%2d.%4d", &d, &m, &mm);
      gps.latitude = (float)d + (float)m / 60.0 + (float)mm / 600000.0;
    }

    // �ϱ������־
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPRMC3:%s\n\r",word);
    if (word != NULL) {
      if (word[0] == 'S') { // gps.latitude=-gps.latitude;
        gps.latNS = 'S';
      } else {
        gps.latNS = 'N';
      }
    }

    // ����
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPRMC4:%s\n\r",word);
    if (word != NULL) {
      int d, m, mm;
      sscanf(word, "%3d%2d.%4d", &d, &m, &mm);
      gps.longitude = (float)d + (float)m / 60.0 + (float)mm / 600000.0;
    }

    // ���������־
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPRMC5:%s\n\r",word);
    if (word != NULL) {
      if (word[0] == 'W') {
        gps.lgtEW = 'W';
        // gps.longitude=-gps.longitude;
      } else {
        gps.lgtEW = 'E';
      }
    }

    // �Ե��˶��ٶ�
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPRMC6:%s\n\r",word);
    if (word != NULL) {
      gps.speed = my_atof(word) * 1.852;
    }

    // �Ե��˶�����
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPRMC7:%s\n\r",word);
    if (word != NULL) {
      gps.direction = my_atof(word);
    }

    // utc ����
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPRMC8:%s\n\r",word);
    if (word != NULL) {
      sscanf(word, "%2d%2d%2d", &gps.DD, &gps.MM, &gps.YY);
    }
  } else if ((!strcmp(word, "GPGGA")) || (!strcmp(word, "GNGGA"))) {
    // ʱ���
    //        AT_PRINTF("GPGGA:%s\n\r",word);
    word = split(left, SEMICOLON, &left);
    if (word != NULL) {
      if (strncmp(lasttime, word, 20)) {
        sscanf(word, "%2d%2d%2d.%4d", &gps.hh, &gps.mm, &gps.ss, &gps.ms);
        strncpy(lasttime, word, 20);
      }
    }

    // γ��
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPGGA1:%s\n\r",word);
    if (word != NULL) {
      int d, m, mm;
      sscanf(word, "%2d%2d.%4d", &d, &m, &mm);
      gps.latitude = (float)d + (float)m / 60.0 + (float)mm / 600000.0;
    }

    // �ϱ������־
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPGGA2:%s\n\r",word);
    if (word != NULL) {
      if (word[0] == 'S') { // gps.latitude=-gps.latitude;
        gps.latNS = 'S';
      } else {
        gps.latNS = 'N';
      }
    }

    // ����
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPGGA3:%s\n\r",word);
    if (word != NULL) {
      int d, m, mm;
      sscanf(word, "%3d%2d.%4d", &d, &m, &mm);
      gps.longitude = (float)d + (float)m / 60.0 + (float)mm / 600000.0;
    }

    // ���������־
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPGGA4:%s\n\r",word);
    if (word != NULL) {
      if (word[0] == 'W') {
        gps.lgtEW = 'W';
        // gps.longitude=-gps.longitude;
      } else {
        gps.lgtEW = 'E';
      }
    }

    // ��λ��Ч�Լ���ʽ
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPGGA5:%s\n\r",word);
    if (word != NULL) {
      gps.FixMode = my_atoi(word);
    }

    // ��׽��������
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPGGA6:%s\n\r",word);
    if (word != NULL) {
      gps.usedsatnum = my_atoi(word);
    }

    // �������
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPGGA7:%s\n\r",word);
    if (word != NULL) {
      gps.HDOP = my_atof(word);

      // sscanf(word,"%f",&gps.HDOP);
    }

    // ���θ߶�
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPGGA8:%s\n\r",word);
    if (word != NULL) {
      gps.altitude = my_atof(word);
    }

    // �߶ȵ�λ
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPGGA9:%s\n\r",word);
    if (word != NULL) {
      gps.altitudeunit = word[0];
    }
  } else if (!strcmp(word, "GPGSV")) {

    // ��Ϣ����
    //        AT_PRINTF("GPGSV:%s\n\r",word);
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPGSV1:%s\n\r",word);
    if (word != NULL) {
      msgcount = my_atoi(word);
    }

    // ��Ϣ���
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPGSV2:%s\n\r",word);
    if (word != NULL) {
      msgid = my_atoi(word);
    }

    if (msgid == 1) {
      satcount = 0;
      // memset(satinfo,0,sizeof(SatelliteInfo)*38);
    }

    // ��������
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPGSV4:%s\n\r",word);
    if (word != NULL) {
      gps.allsatnum = my_atoi(word);
    }

    // printf("%s\n",left);
    for (i = 0; i < 4; i++) {
      // ���Ǳ��
      word = split(left, SEMICOLON, &left);
      if (word != NULL) {
        gps.satinfo[satcount].satid = my_atoi(word);
      }

      // ��������
      word = split(left, SEMICOLON, &left);
      //            AT_PRINTF("GPGSV5:%s\n\r",word);
      if (word != NULL) {
        gps.satinfo[satcount].elevation = my_atoi(word);
      }

      // ���Ƿ�λ��
      word = split(left, SEMICOLON, &left);
      //            AT_PRINTF("GPGSV6:%s\n\r",word);
      if (word != NULL) {
        gps.satinfo[satcount].azimuth = my_atoi(word);
      }

      // �����ź������
      word = split(left, SEMICOLON, &left);
      //            AT_PRINTF("GPGSV7:%s\n\r",word);
      if (word != NULL) {
        gps.satinfo[satcount].snr = my_atoi(word);
      }

      satcount++;

      if (word == NULL)
        break;
    }

    if (msgid == msgcount)
      ;
  } else if ((!strcmp(word, "GPGSA")) || (!strcmp(word, "GNGSA"))) {
    //			  AT_PRINTF("GPGSA:%s\n\r",word);
    // ��λģʽ1
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPGSA1:%s\n\r",word);
    if (word != NULL) {
      gps.GSA_mode1 = my_atoi(word);
    }

    // ��λģʽ2
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPGSA2:%s\n\r",word);
    if (word != NULL) {
      gps.GSA_mode2 = my_atoi(word);
    }

    for (i = 0; i < 12; i++) {
      word = split(left, SEMICOLON, &left);
      if (word != NULL) {
        gps.usedsat[i] = my_atoi(word);
        usedsatcount++;
      }
    }

    // λ�þ���ֵ
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPGSA3:%s\n\r",word);
    if (word != NULL) {
      gps.PDOP = (float)my_atof(word);
      pdop_gps = gps.PDOP;
    }

    // ˮƽ����ֵ
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPGSA4:%s\n\r",word);
    if (word != NULL) {
      gps.HDOP = (float)my_atof(word);
    }

    // �߶Ⱦ���ֵ
    word = split(left, SEMICOLON, &left);
    //        AT_PRINTF("GPGSA4:%s\n\r",word);
    if (word != NULL) {
      gps.VDOP = (float)my_atof(word);
    }
  }

  if (gps.latitude > 90.0)
    gps.latitude = 0.0;

  if (gps.longitude > 180.0)
    gps.longitude = 0.0;

  return 1;
}

#define NEMA_NUM_MAX 6    // �����NEMA�������
#define NEMA_CHAR_MAX 255 // �����NEMA����ַ�����

struct {
  uint8_t isupdated;          // NEMA ���������±�־
  char buffer[NEMA_CHAR_MAX]; // NEMA ������
} GPS_NEMA[NEMA_NUM_MAX];

void GPS_usart(uint8_t buffer) {

  static uint8_t NEMA_count = 0;
  static uint8_t char_count = 0;
  //		 uint8_t Empty = 0;
  if (isFirmwareUpdate == 1) // ���¹̼�
    return;
  if (buffer == '$') {
    GPS_NEMA[NEMA_count].isupdated = 1; // ������һ�������ϸ��±�־
    NEMA_count++;
    if (NEMA_count > (NEMA_NUM_MAX - 1)) {
      NEMA_count = 0;
    }
    GPS_NEMA[NEMA_count].isupdated = 0; // ������������δ���±�־
    GPS_NEMA[NEMA_count].buffer[0] = '$';
    char_count = 1;

  } else {
    if (char_count < NEMA_CHAR_MAX - 1)
      GPS_NEMA[NEMA_count].buffer[char_count++] = buffer;
  }
  count++;
  if (count == 255) {
    GPS_NEMA[NEMA_count].isupdated = 0;
    GPS_NEMA[NEMA_count].buffer[char_count++] = 0;
    count = 0;
  }
}
uint8_t GPS_INFO_update(void) {
  uint8_t i;
  uint8_t temp = 0;
  for (i = 0; i < NEMA_NUM_MAX; i++) {
    if (GPS_NEMA[i].isupdated == 1) {
      temp = GPS_parse(GPS_NEMA[i].buffer);
      GPS_NEMA[i].isupdated = 0;
    }
  }

  return temp;
}

//======================================================================
// �� �� ��: GPS_init()
// ��    ��: ����LED�ܽ�
// ��ڲ���: ��
// ���ڲ���: ��
// �� �� ֵ: ��
//======================================================================
void GPS_init(void) {
  GPIO_InitTypeDef GPIO_InitStructure;

  /* ���� PC.0 GPS_BOOT -PC.1 GPS_EN Ϊ���ģʽ*/

  __GPIOB_CLK_ENABLE();

  GPIO_InitStructure.Pin = GPIO_PIN_3;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull = GPIO_PULLUP;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.Pin = GPIO_PIN_4;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull = GPIO_PULLUP;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.Pin = GPIO_PIN_5;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull = GPIO_PULLUP;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPS_Stop();
}

void GPS_doinit(void) {
  GPIO_InitTypeDef GPIO_InitStructure;

  __GPIOB_CLK_ENABLE();

  GPIO_InitStructure.Pin = GPIO_PIN_3;
  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.Pin = GPIO_PIN_4;
  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void GPS_INPUT(void) {
  int dd, mm;
  FP32 ss;
  //
  GPS_INFO_update();
  if (loggps == 1) {
    GPS_DegreeToDMS(gps.latitude, &dd, &mm, &ss);

    AT_PRINTF("%s:%3d %2d'%5.2f ", (gps.latNS == 'N') ? "North" : "South", dd,
              mm, ss);
    AT_PRINTF("%s: %.6f\n\r", (gps.latNS == 'N') ? "North" : "South",
              gps.latitude);

    GPS_DegreeToDMS(gps.longitude, &dd, &mm, &ss);
    AT_PRINTF("%s:%3d %2d'%05.2f", (gps.lgtEW == 'E') ? "East" : "West", dd, mm,
              ss);
    AT_PRINTF("%s: %.6f\n\r ", (gps.lgtEW == 'E') ? "East" : "West",
              gps.longitude);
    AT_PRINTF("Altitude:%.1f%c ", gps.altitude, gps.altitudeunit);
    AT_PRINTF("Speed:%.1f km/h ", gps.speed);
    AT_PRINTF("Course:%.1f ", gps.direction);
    AT_PRINTF("Time:%2d:%02d:%02d ", (gps.hh < 16) ? gps.hh + 8 : gps.hh - 16,
              gps.mm, gps.ss);
    AT_PRINTF("Date:20%02d-%d-%d ", gps.YY, gps.MM, gps.DD);
    AT_PRINTF("Satellite:%2d/%2d", gps.usedsatnum, gps.allsatnum);
    AT_PRINTF("Mode:%2d\n\r", gps.GSA_mode2);
    AT_PRINTF("PDOP:%.1f\n\r", pdop_gps);
  }

  // TODO: What's really the point here? Better comment-out everything. 
  switch (gps.FixMode) {
  case 0:
    //					AT_PRINTF("GPS״̬:δ��λ   \n\r");
    break;
  case 1:
    //					AT_PRINTF("GPS״̬:%dD SPS  \n\r
    //",gps.GSA_mode2);
    break;
  case 2:
    //					AT_PRINTF("GPS״̬:%dD DGPS
    //\n\r",gps.GSA_mode2);
    break;
  case 6:
    //					AT_PRINTF("GPS״̬:������    \n\r");
    break;
  default:
    break;
  }
  if (gps.GSA_mode2 == 2 || gps.GSA_mode2 == 3) {
    gps.flag = 0;
  }
}

void POWER_ON() {
  GPS_init();
  GPS_POWER_ON();
  if ((ic_version <= 1) && ((se_mode != 0) || (fr_mode != 0))) {
    if (gps_setflags == 0) {
      send_setting();
      gpspower_flag++;
      if (gpspower_flag == 30) {
        gps_setflags = 1;
        gpspower_flag = 0;
      }
    }
  }
}
void POWER_OFF() { GPS_POWER_OFF(); }

void send_setting(void) {
  uint8_t txdata1[25];
  uint8_t txdata2[17];

  if ((ic_version == 1) && (se_mode != 0)) {
    PMTK353();
    copytxdata(txdata1, txdata353);
    HAL_UART_Transmit(&uart1, txdata1, 25, 0xFFFF); // Set search mode
    DelayMs(50);
  }

  if (fr_mode != 0) {
    PMTK886();
    copytxdata(txdata2, txdata886);
    HAL_UART_Transmit(&uart1, txdata2, 17, 0xFFFF); // Set fr mode
    DelayMs(50);
  }
}

void PMTK353(void) {
  if (se_mode == 1) {
    txdata353 = "$PMTK353,1,1,0,0,0*2B\r\n";
  } else if (se_mode == 2) {
    txdata353 = "$PMTK353,1,0,0,0,1*2B\r\n";
  } else if (se_mode == 3) {
    txdata353 = "$PMTK353,1,0,1,0,0*2B\r\n";
  } else if (se_mode == 4) {
    txdata353 = "$PMTK353,1,1,1,0,0*2A\r\n";
  }
}

void PMTK886(void) {
  if (fr_mode == 1) {
    txdata886 = "$PMTK886,0*28\r\n";
  } else if (fr_mode == 2) {
    txdata886 = "$PMTK886,1*29\r\n";
  } else if (fr_mode == 3) {
    txdata886 = "$PMTK886,2*2A\r\n";
  } else if (fr_mode == 4) {
    txdata886 = "$PMTK886,3*2B\r\n";
  } else if (fr_mode == 5) {
    txdata886 = "$PMTK886,4*2C\r\n";
  }
}

void copytxdata(uint8_t data1[], char *data2) {
  char data3[40];
  strcpy(data3, data2);
  for (int i = 0; i < (strlen(data3)); i++) {
    data1[i] = data3[i];
  }
  data1[strlen(data3)] = '\\';
  data1[strlen(data3) + 1] = 'n';
}
