#ifndef HMC5883L_H
#define HMC5883L_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define HMC5883L_ADDR     0x1E
#define HMC_CONFIG_A      0x00
#define HMC_CONFIG_B      0x01
#define HMC_MODE          0x02
#define HMC_DATA_START    0x03
#define HMC_ID_REG        0x0A

// Config A options
#define HMC_SAMPLES_1     0x00
#define HMC_SAMPLES_2     0x20
#define HMC_SAMPLES_4     0x40
#define HMC_SAMPLES_8     0x60

#define HMC_ODR_0_75      0x00
#define HMC_ODR_1_5       0x04
#define HMC_ODR_3         0x08
#define HMC_ODR_7_5       0x0C
#define HMC_ODR_15        0x10
#define HMC_ODR_30        0x14
#define HMC_ODR_75        0x18

// Config B gain options
#define HMC_GAIN_1_3      0x20   // ±1.3 Ga — 1090 LSB/Ga
#define HMC_GAIN_1_9      0x40
#define HMC_GAIN_2_5      0x60
#define HMC_GAIN_4_0      0x80
#define HMC_GAIN_4_7      0xA0
#define HMC_GAIN_5_6      0xC0
#define HMC_GAIN_8_1      0xE0

typedef struct {
    i2c_inst_t *i2c;
    float       gain_lsb;    // LSB per Gauss, set by gain config
    float       x;
    float       y;
    float       z;
    float       heading;
} HMC5883L;

bool    HMC5883L_init(HMC5883L *dev, i2c_inst_t *i2c, uint8_t config_a, uint8_t config_b);
bool    HMC5883L_read(HMC5883L *dev);
float   HMC5883L_heading(HMC5883L *dev);
bool    HMC5883L_heading_tilt_compensated(HMC5883L *dev, float roll_rad, float pitch_rad, float *heading_out);

#endif // HMC5883L_H