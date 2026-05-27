#include "mpu6050.h"

//MPU
void MPU6050init(){
    i2c_init(I2C,BAUDRATE);

    gpio_set_function(SDA,GPIO_FUNC_I2C);
    gpio_set_function(SCL,GPIO_FUNC_I2C);
    
    gpio_pull_up(SDA);
    gpio_pull_up(SCL);

    // Wake up +  X gyro PLL
    uint8_t pwrbyte[2] = {PWR_MGMT_1, 0x01};
    int bytes = i2c_write_blocking(I2C,MPU6050_ADDR,pwrbyte,2,false);
    sleep_ms(10); 

    //Accl configuration
    uint8_t acclConfig[] = {ACC_CONF_ADDR, 0x10};  //     ±8g 
    int acclWritebytes = i2c_write_blocking(I2C,MPU6050_ADDR,acclConfig,2,false);

    //Gyro configuration
    uint8_t gyroConfig[] = {GYRO_CONF_ADDR, 0x08};  //   ±500°/s:
    int gyroWritebytes = i2c_write_blocking(I2C,MPU6050_ADDR,gyroConfig,2,false);

    //DLPF configuration
    uint8_t dlpfConfig[] = {DLPF_CONFIG_ADDR, 0x00};  // 280Hz
    i2c_write_blocking(I2C, MPU6050_ADDR, dlpfConfig, 2,false);

    // SAMPLE RATE 1kHz
    uint8_t sampleRate[] = {0x19, 0x07}; 
    i2c_write_blocking(I2C, MPU6050_ADDR, sampleRate, 2, false);
}

void MPU6050read(float gyro[3], float accel[3]){
    uint8_t reg = 0x3B; // ACCEL_XOUT_H
    uint8_t raw[14];
    
    i2c_write_blocking(I2C, MPU6050_ADDR, &reg, 1, true); // true = repeated start
    i2c_read_blocking(I2C, MPU6050_ADDR, raw, 14, false);

    // accel
    accel[0] = (int16_t)(raw[0]  << 8 | raw[1])  / 4096.0f; // ±8g → 4096 LSB/g
    accel[1] = (int16_t)(raw[2]  << 8 | raw[3])  / 4096.0f;
    accel[2] = (int16_t)(raw[4]  << 8 | raw[5])  / 4096.0f;

    // raw[6] raw[7] = temperature, skip

    // gyro
    gyro[0]  = (int16_t)(raw[8]  << 8 | raw[9])  / 65.5f;  // ±500°/s → 65.5 LSB/°/s
    gyro[1]  = (int16_t)(raw[10] << 8 | raw[11]) / 65.5f;
    gyro[2]  = (int16_t)(raw[12] << 8 | raw[13]) / 65.5f;
}
