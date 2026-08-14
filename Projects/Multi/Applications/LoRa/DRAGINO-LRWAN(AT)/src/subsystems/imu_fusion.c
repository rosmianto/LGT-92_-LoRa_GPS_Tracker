#include "imu_fusion.h"

static float pitchoffset, rolloffset, yawoffset;

/*
 *@Features: Quickly get an open countdown
 *
 *
 */
float invSqrt(float number) {
	long i;
	float x, y;
	const float f = 1.5f;

	x = number * 0.5f;
	y = number;
	i = *((long *)&y);
	i = 0x5f375a86 - (i >> 1);
	y = *((float *)&i);
	y = y * (f - (x * y * y));
	return y;
}

/*
 *@Features: Fusion accelerometer and magnetometer for attitude adjustment
 *
 *
 */
void AHRSupdate(float gx, float gy, float gz, float ax, float ay, float az,
				float mx, float my, float mz, float *roll, float *pitch,
				float *yaw) {
	if (flag_2 == 1) {
		flag_2 = 0;
		ax_old = ax;
		ay_old = ay;
		az_old = az;

		gx_old = gx;
		gy_old = gy;
		gz_old = gz;

		mx_old = mx;
		my_old = my;
		mz_old = mz;
	}

	ax = (ax + ax_old) / 2.0;
	ay = (ay + ay_old) / 2.0;
	az = (az + az_old) / 2.0;

	gx = (gx + gx_old) / 2.0;
	gx = (gx + gx_old) / 2.0;
	gx = (gx + gx_old) / 2.0;

	mx = (mx + mx_old) / 2.0;
	mx = (mx + mx_old) / 2.0;
	mx = (mx + mx_old) / 2.0;

	ax_old = ax;
	ay_old = ay;
	az_old = az;

	gx_old = gx;
	gy_old = gy;
	gz_old = gz;

	mx_old = mx;
	my_old = my;
	mz_old = mz;

	float norm;				  // For unitization
	float hx, hy, hz, bx, bz; //
	float vx, vy, vz, wx, wy, wz;
	float ex, ey, ez;
	//					 float tmp0,tmp1,tmp2,tmp3;

	// auxiliary variables to reduce number of repeated operations
	float q0q0 = q0 * q0;
	float q0q1 = q0 * q1;
	float q0q2 = q0 * q2;
	float q0q3 = q0 * q3;
	float q1q1 = q1 * q1;
	float q1q2 = q1 * q2;
	float q1q3 = q1 * q3;
	float q2q2 = q2 * q2;
	float q2q3 = q2 * q3;
	float q3q3 = q3 * q3;

	// normalise the measurements
	norm = invSqrt(ax * ax + ay * ay + az * az);
	ax = ax * norm;
	ay = ay * norm;
	az = az * norm;
	norm = invSqrt(mx * mx + my * my + mz * mz);
	mx = mx * norm;
	my = my * norm;
	mz = mz * norm;

	// compute reference direction of magnetic field
	// hx,hy,hz is mx,my,mz Representation in the reference coordinate system
	hx = 2 * mx * (0.50 - q2q2 - q3q3) + 2 * my * (q1q2 - q0q3) +
		 2 * mz * (q1q3 + q0q2);
	hy = 2 * mx * (q1q2 + q0q3) + 2 * my * (0.50 - q1q1 - q3q3) +
		 2 * mz * (q2q3 - q0q1);
	hz = 2 * mx * (q1q3 - q0q2) + 2 * my * (q2q3 + q0q1) +
		 2 * mz * (0.50 - q1q1 - q2q2);
	// bx,by,bz Is the representation of the Earth's magnetic field in the
	// reference coordinate system
	bx = sqrt((hx * hx) + (hy * hy));
	bz = hz;

	// estimated direction of gravity and magnetic field (v and w)
	// vx,vy,vz Is the representation of gravity acceleration in the object
	// coordinate system
	vx = 2 * (q1q3 - q0q2);
	vy = 2 * (q0q1 + q2q3);
	vz = q0q0 - q1q1 - q2q2 + q3q3;
	// wx,wy,wz Is the representation of the earth's magnetic field in the
	// object coordinate system
	wx = 2 * bx * (0.5 - q2q2 - q3q3) + 2 * bz * (q1q3 - q0q2);
	wy = 2 * bx * (q1q2 - q0q3) + 2 * bz * (q0q1 + q2q3);
	wz = 2 * bx * (q0q2 + q1q3) + 2 * bz * (0.5 - q1q1 - q2q2);

	// error is sum ofcross product between reference direction of fields and
	// directionmeasured by sensors
	// ex,ey,ezIt is the error between the acceleration and the measured
	// direction of the magnetometer and the actual gravity acceleration and the
	// geomagnetic direction.
	// The error is expressed by the cross product, and the weight of the
	// accelerometer and the magnetometer are the same.
	ex = (ay * vz - az * vy) + (my * wz - mz * wy);
	ey = (az * vx - ax * vz) + (mz * wx - mx * wz);
	ez = (ax * vy - ay * vx) + (mx * wy - my * wx);

	// integral error scaled integral gain
	exInt = exInt + ex * Ki * dt;
	eyInt = eyInt + ey * Ki * dt;
	ezInt = ezInt + ez * Ki * dt;
	// printf("exInt=%0.1f eyInt=%0.1f ezInt=%0.1f ",exInt,eyInt,ezInt);
	// adjusted gyroscope measurements
	gx = gx + Kp * ex + exInt;
	gy = gy + Kp * ey + eyInt;
	gz = gz + Kp * ez + ezInt;
	// printf("gx=%0.1f gy=%0.1f gz=%0.1f",gx,gy,gz);

	// integrate quaernion rate aafnd normalaizle

	//           tmp0 = q0 + (-q1*gx - q2*gy - q3*gz)*halfT;
	//           tmp1 = q1 + ( q0*gx + q2*gz - q3*gy)*halfT;
	//           tmp2 = q2 + ( q0*gy - q1*gz + q3*gx)*halfT;
	//           tmp3 = q3 + ( q0*gz + q1*gy - q2*gx)*halfT;
	//					 q0=tmp0;
	//					 q1=tmp1;
	//					 q2=tmp2;
	//					 q3=tmp3;
	// printf("q0=%0.1f q1=%0.1f q2=%0.1f q3=%0.1f",q0,q1,q2,q3);
	////RUNGE_KUTTA Solving differential equation
	k10 = 0.5 * (-gx * q1 - gy * q2 - gz * q3);
	k11 = 0.5 * (gx * q0 + gz * q2 - gy * q3);
	k12 = 0.5 * (gy * q0 - gz * q1 + gx * q3);
	k13 = 0.5 * (gz * q0 + gy * q1 - gx * q2);

	k20 =
		0.5 *
		(halfT * (q0 + halfT * k10) + (halfT - gx) * (q1 + halfT * k11) +
		 (halfT - gy) * (q2 + halfT * k12) + (halfT - gz) * (q3 + halfT * k13));
	k21 =
		0.5 *
		((halfT + gx) * (q0 + halfT * k10) + halfT * (q1 + halfT * k11) +
		 (halfT + gz) * (q2 + halfT * k12) + (halfT - gy) * (q3 + halfT * k13));
	k22 =
		0.5 *
		((halfT + gy) * (q0 + halfT * k10) + (halfT - gz) * (q1 + halfT * k11) +
		 halfT * (q2 + halfT * k12) + (halfT + gx) * (q3 + halfT * k13));
	k23 =
		0.5 *
		((halfT + gz) * (q0 + halfT * k10) + (halfT + gy) * (q1 + halfT * k11) +
		 (halfT - gx) * (q2 + halfT * k12) + halfT * (q3 + halfT * k13));

	k30 =
		0.5 *
		(halfT * (q0 + halfT * k20) + (halfT - gx) * (q1 + halfT * k21) +
		 (halfT - gy) * (q2 + halfT * k22) + (halfT - gz) * (q3 + halfT * k23));
	k31 =
		0.5 *
		((halfT + gx) * (q0 + halfT * k20) + halfT * (q1 + halfT * k21) +
		 (halfT + gz) * (q2 + halfT * k22) + (halfT - gy) * (q3 + halfT * k23));
	k32 =
		0.5 *
		((halfT + gy) * (q0 + halfT * k20) + (halfT - gz) * (q1 + halfT * k21) +
		 halfT * (q2 + halfT * k22) + (halfT + gx) * (q3 + halfT * k23));
	k33 =
		0.5 *
		((halfT + gz) * (q0 + halfT * k20) + (halfT + gy) * (q1 + halfT * k21) +
		 (halfT - gx) * (q2 + halfT * k22) + halfT * (q3 + halfT * k23));

	k40 = 0.5 * (dt * (q0 + dt * k30) + (dt - gx) * (q1 + dt * k31) +
				 (dt - gy) * (q2 + dt * k32) + (dt - gz) * (q3 + dt * k33));
	k41 = 0.5 * ((dt + gx) * (q0 + dt * k30) + dt * (q1 + dt * k31) +
				 (dt + gz) * (q2 + dt * k32) + (dt - gy) * (q3 + dt * k33));
	k42 = 0.5 * ((dt + gy) * (q0 + dt * k30) + (dt - gz) * (q1 + dt * k31) +
				 dt * (q2 + dt * k32) + (dt + gx) * (q3 + dt * k33));
	k43 = 0.5 * ((dt + gz) * (q0 + dt * k30) + (dt + gy) * (q1 + dt * k31) +
				 (dt - gx) * (q2 + dt * k32) + dt * (q3 + dt * k33));

	q0 = q0 + dt / 6.0 * (k10 + 2 * k20 + 2 * k30 + k40);
	q1 = q1 + dt / 6.0 * (k11 + 2 * k21 + 2 * k31 + k41);
	q2 = q2 + dt / 6.0 * (k12 + 2 * k22 + 2 * k32 + k42);
	q3 = q3 + dt / 6.0 * (k13 + 2 * k23 + 2 * k33 + k43);

	// normalise quaternion
	norm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 = q0 * norm;
	q1 = q1 * norm;
	q2 = q2 * norm;
	q3 = q3 * norm;

	*pitch = -asin(-2 * q1 * q3 + 2 * q0 * q2) * 57.3; // pitch
	*roll = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) *
			57.3; // roll
	*yaw =
		atan2(2 * (q1 * q2 + q0 * q3), q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) *
		57.3; // yaw
}

/*
 *@Features: Calculate the number of turns in the horizontal direction
 *
 *
 */
void CountTurns(float *newdata, float *olddata, short *turns) {
	if (*newdata < -170.0f && *olddata > 170.0f)
		(*turns)++;
	if (*newdata > 170.0f && *olddata < -170.0f)
		(*turns)--;
}

/*
 *@Features: Calculate the yaw angle
 *
 *
 */
void CalYaw(float *yaw, short *turns) { *yaw = 360.0 * *turns + *yaw; }

/*
 *@Features: Compensate for Euler angle offset, mainly to compensate for yaw
 * angle
 *
 *
 */
void CalibrateToZero(void) {
	uint8_t t = 0;
	float sumpitch = 0, sumroll = 0, sumyaw = 0;
	float pitch, roll, yaw;
	short igx, igy, igz;
	short iax, iay, iaz;
	short imx, imy, imz;
	float gx, gy, gz;
	float ax, ay, az;
	float mx, my, mz;
	for (t = 0; t < 150; t++) {
		MPU_Get_Gyro(&igx, &igy, &igz, &gx, &gy, &gz);
		MPU_Get_Accel(&iax, &iay, &iaz, &ax, &ay, &az);
		MPU_Get_Mag(&imx, &imy, &imz, &mx, &my, &mz);
		AHRSupdate(gx, gy, gz, ax, ay, az, mx, my, mz, &roll, &pitch, &yaw);
		//			delay_us(6430);
		DelayMs(7);
		if (t >= 100) {
			sumpitch += pitch;
			sumroll += roll;
			sumyaw += yaw;
		}
	}
	pitchoffset = -sumpitch / 150.0f;
	rolloffset = -sumroll / 150.0f;
	yawoffset = -sumyaw / 150.0f;

	//			PRINTF("offset %0.1f %0.1f\n\r",rolloffset,pitchoffset);
	pitchoffset = 0;
	rolloffset = 0;
	yawoffset = 0;
}
