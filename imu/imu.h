// imu.h
#ifndef IMU_H
#define IMU_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

#define SDA               10
#define SCL               11
#define I2C               i2c1
#define BAUDRATE          400000

#define MPU6050_ADDR      0x68
#define PWR_MGMT_1        0x6B
#define GYRO_CONF_ADDR    0x1B
#define ACC_CONF_ADDR     0x1C
#define DLPF_CONFIG_ADDR  0x1A
#define SMPLRT_DIV        0x19
#define ACCEL_XOUT_H      0x3B

void MPU6050init();
void MPU6050read(float gyro[3], float accel[3]);

#endif // IMU_H