#pragma once

float invSqrt(float number);
void AHRSupdate(float gx, float gy, float gz, float ax, float ay, float az,
				float mx, float my, float mz, float *roll, float *pitch,
				float *yaw);
void CountTurns(float *newdata, float *olddata, short *turns);
void CalYaw(float *yaw, short *turns);
void CalibrateToZero(void);