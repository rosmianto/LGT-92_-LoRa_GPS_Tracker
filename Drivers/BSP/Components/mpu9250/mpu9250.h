/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: contains all hardware driver

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
 /******************************************************************************
  * @file    IIC.h
  * @author  MCD Application Team
  * @version V1.1.2
  * @date    01-June-2017
  * @brief   contains all hardware driver
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MPU9250_H__
#define __MPU9250_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

 /* Includes
  * ------------------------------------------------------------------*/
 /* Exported types
  * ------------------------------------------------------------*/

 /* Exported constants
  * --------------------------------------------------------*/
 /* External variables
  * --------------------------------------------------------*/
 /* Exported macros
  * -----------------------------------------------------------*/
 /* Exported functions -------------------------------------------------------
  */
 // See also MPU-9250 Register Map and Descriptions, Revision 4.0,
 // RM-MPU-9250A-00, Rev. 1.4, 9/9/2013 for registers not listed in above
 // document; the MPU9250 and MPU9150 are virtually identical but the latter has
 // a different register map
 //
#define IMU_SAMPLE_RATE 200.0f
#define IMU_FILTER_CUTOFF_FREQ 30.0f

#define MPU9250_ADDR 0X68 // MPU6500������IIC��ַ
#define MPU6500_ID1 0X71  // MPU6500������ID1
#define MPU6500_ID2 0X73  // MPU6500������ID2

 // MPU9250�ڲ���װ��һ��AK8963������,��ַ��ID����:
#define AK8963_ADDR 0X0C // AK8963��I2C��ַ
#define AK8963_ID 0X48   // AK8963������ID

 // AK8963���ڲ��Ĵ���
#define MAG_WIA 0x00 // AK8963������ID�Ĵ�����ַ
#define MAG_CNTL1 0X0A
#define MAG_CNTL2 0X0B

#define MAG_XOUT_L 0X03
#define MAG_XOUT_H 0X04
#define MAG_YOUT_L 0X05
#define MAG_YOUT_H 0X06
#define MAG_ZOUT_L 0X07
#define MAG_ZOUT_H 0X08

 // MPU6500���ڲ��Ĵ���
#define MPU_SELF_TESTX_REG 0X0D   // �Լ�Ĵ���X
#define MPU_SELF_TESTY_REG 0X0E   // �Լ�Ĵ���Y
#define MPU_SELF_TESTZ_REG 0X0F   // �Լ�Ĵ���Z
#define MPU_SELF_TESTA_REG 0X10   // �Լ�Ĵ���A
#define MPU_SAMPLE_RATE_REG 0X19  // ����Ƶ�ʷ�Ƶ��
#define MPU_CFG_REG 0X1A          // ���üĴ���
#define MPU_GYRO_CFG_REG 0X1B     // ���������üĴ���
#define MPU_ACCEL_CFG_REG 0X1C    // ���ٶȼ����üĴ���
#define MPU_ACCEL_CFG2_REG 0X1D   // ���ٶȼ����üĴ���2
#define MPU_LP_ACCEL_ODR_REG 0X1E // ����Ƶ�ʼĴ���
#define MPU_MOTION_DET_REG 0X1F   // �˶���ֵⷧ���üĴ���
#define MPU_FIFO_EN_REG 0X23      // FIFOʹ�ܼĴ���
#define MPU_I2CMST_CTRL_REG 0X24  // IIC�������ƼĴ���
#define MPU_I2CSLV0_ADDR_REG 0X25 // IIC�ӻ�0������ַ�Ĵ���
#define MPU_I2CSLV0_REG 0X26      // IIC�ӻ�0���ݵ�ַ�Ĵ���
#define MPU_I2CSLV0_CTRL_REG 0X27 // IIC�ӻ�0���ƼĴ���
#define MPU_I2CSLV1_ADDR_REG 0X28 // IIC�ӻ�1������ַ�Ĵ���
#define MPU_I2CSLV1_REG 0X29      // IIC�ӻ�1���ݵ�ַ�Ĵ���
#define MPU_I2CSLV1_CTRL_REG 0X2A // IIC�ӻ�1���ƼĴ���
#define MPU_I2CSLV2_ADDR_REG 0X2B // IIC�ӻ�2������ַ�Ĵ���
#define MPU_I2CSLV2_REG 0X2C      // IIC�ӻ�2���ݵ�ַ�Ĵ���
#define MPU_I2CSLV2_CTRL_REG 0X2D // IIC�ӻ�2���ƼĴ���
#define MPU_I2CSLV3_ADDR_REG 0X2E // IIC�ӻ�3������ַ�Ĵ���
#define MPU_I2CSLV3_REG 0X2F      // IIC�ӻ�3���ݵ�ַ�Ĵ���
#define MPU_I2CSLV3_CTRL_REG 0X30 // IIC�ӻ�3���ƼĴ���
#define MPU_I2CSLV4_ADDR_REG 0X31 // IIC�ӻ�4������ַ�Ĵ���
#define MPU_I2CSLV4_REG 0X32      // IIC�ӻ�4���ݵ�ַ�Ĵ���
#define MPU_I2CSLV4_DO_REG 0X33   // IIC�ӻ�4д���ݼĴ���
#define MPU_I2CSLV4_CTRL_REG 0X34 // IIC�ӻ�4���ƼĴ���
#define MPU_I2CSLV4_DI_REG 0X35   // IIC�ӻ�4�����ݼĴ���

#define MPU_I2CMST_STA_REG 0X36 // IIC����״̬�Ĵ���
#define MPU_INTBP_CFG_REG 0X37  // �ж�/��·���üĴ���
#define MPU_INT_EN_REG 0X38     // �ж�ʹ�ܼĴ���
#define MPU_INT_STA_REG 0X3A    // �ж�״̬�Ĵ���

#define MPU_ACCEL_XOUTH_REG 0X3B // ���ٶ�ֵ,X���8λ�Ĵ���
#define MPU_ACCEL_XOUTL_REG 0X3C // ���ٶ�ֵ,X���8λ�Ĵ���
#define MPU_ACCEL_YOUTH_REG 0X3D // ���ٶ�ֵ,Y���8λ�Ĵ���
#define MPU_ACCEL_YOUTL_REG 0X3E // ���ٶ�ֵ,Y���8λ�Ĵ���
#define MPU_ACCEL_ZOUTH_REG 0X3F // ���ٶ�ֵ,Z���8λ�Ĵ���
#define MPU_ACCEL_ZOUTL_REG 0X40 // ���ٶ�ֵ,Z���8λ�Ĵ���

#define MPU_TEMP_OUTH_REG 0X41 // �¶�ֵ�߰�λ�Ĵ���
#define MPU_TEMP_OUTL_REG 0X42 // �¶�ֵ��8λ�Ĵ���

#define MPU_GYRO_XOUTH_REG 0X43 // ������ֵ,X���8λ�Ĵ���
#define MPU_GYRO_XOUTL_REG 0X44 // ������ֵ,X���8λ�Ĵ���
#define MPU_GYRO_YOUTH_REG 0X45 // ������ֵ,Y���8λ�Ĵ���
#define MPU_GYRO_YOUTL_REG 0X46 // ������ֵ,Y���8λ�Ĵ���
#define MPU_GYRO_ZOUTH_REG 0X47 // ������ֵ,Z���8λ�Ĵ���
#define MPU_GYRO_ZOUTL_REG 0X48 // ������ֵ,Z���8λ�Ĵ���

#define MPU_I2CSLV0_DO_REG 0X63 // IIC�ӻ�0���ݼĴ���
#define MPU_I2CSLV1_DO_REG 0X64 // IIC�ӻ�1���ݼĴ���
#define MPU_I2CSLV2_DO_REG 0X65 // IIC�ӻ�2���ݼĴ���
#define MPU_I2CSLV3_DO_REG 0X66 // IIC�ӻ�3���ݼĴ���

#define MPU_I2CMST_DELAY_REG 0X67 // IIC������ʱ�����Ĵ���
#define MPU_SIGPATH_RST_REG 0X68  // �ź�ͨ����λ�Ĵ���
#define MPU_MDETECT_CTRL_REG 0X69 // �˶������ƼĴ���
#define MPU_USER_CTRL_REG 0X6A    // �û����ƼĴ���
#define MPU_PWR_MGMT1_REG 0X6B    // ��Դ�����Ĵ���1
#define MPU_PWR_MGMT2_REG 0X6C    // ��Դ�����Ĵ���2
#define MPU_FIFO_CNTH_REG 0X72    // FIFO�����Ĵ����߰�λ
#define MPU_FIFO_CNTL_REG 0X73    // FIFO�����Ĵ����Ͱ�λ
#define MPU_FIFO_RW_REG 0X74      // FIFO��д�Ĵ���
#define MPU_DEVICE_ID_REG 0X75    // ����ID�Ĵ���

#define gryo_scale (4000.0 / 65536.0 * 3.14159 / 180.0) // ������
#define accel_scale (16.0 / 65536.0)                    // G��λ
#define mag_scale (9600.0 / 16384.0 / 100.0)            // ��˹��λ

 uint8_t MPU_Init(void);
 void MPU_INT_Init(void);
 uint8_t MPU_WaitForReady(uint8_t devaddr);
 uint8_t MPU_Write_Byte(uint8_t devaddr, uint8_t reg, uint8_t data);
 uint8_t MPU_Read_Byte(uint8_t devaddr, uint8_t reg);
 uint8_t MPU_Set_Gyro_Fsr(uint8_t fsr);
 uint8_t MPU_Set_Accel_Fsr(uint8_t fsr);
 uint8_t MPU_Set_Rate(uint16_t rate);
 uint8_t MPU_Write_Len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf);
 uint8_t MPU_Read_Len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf);
 short MPU_Get_Temperature(void);
 uint8_t MPU_Get_Gyroscope(short *gx, short *gy, short *gz);
 uint8_t MPU_Get_Accelerometer(short *ax, short *ay, short *az);
 uint8_t MPU_Get_Magnetometer(short *mx, short *my, short *mz);

 uint8_t MPU_Get_Gyro(short *igx, short *igy, short *igz, float *gx, float *gy,
                      float *gz);
 uint8_t MPU_Get_Accel(short *iax, short *iay, short *iaz, float *ax, float *ay,
                       float *az);
 uint8_t MPU_Get_Mag(short *imx, short *imy, short *imz, float *mx, float *my,
                     float *mz);
 void calibrate1(void);
 void calibrate2(void);
 void get_calibrated_data(void);
#ifdef __cplusplus
 }
#endif

#endif /* __MPU9250_H__ */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
