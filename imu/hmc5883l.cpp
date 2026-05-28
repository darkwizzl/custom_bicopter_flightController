#include "hmc5883l.h"
#include <math.h>


static bool hmc_write_reg(i2c_inst_t *i2c, uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    return i2c_write_blocking(i2c, HMC5883L_ADDR, buf, 2, false) == 2;
}

static float gain_to_lsb(uint8_t config_b) {
    switch (config_b & 0xE0) {
        case HMC_GAIN_1_3: return 1090.0f;
        case HMC_GAIN_1_9: return 820.0f;
        case HMC_GAIN_2_5: return 660.0f;
        case HMC_GAIN_4_0: return 440.0f;
        case HMC_GAIN_4_7: return 390.0f;
        case HMC_GAIN_5_6: return 330.0f;
        case HMC_GAIN_8_1: return 230.0f;
        default:           return 1090.0f;
    }
}

bool HMC5883L_init(HMC5883L *dev, i2c_inst_t *i2c, uint8_t config_a, uint8_t config_b) {
    dev->i2c      = i2c;
    dev->gain_lsb = gain_to_lsb(config_b);
    dev->x = dev->y = dev->z = dev->heading = 0.0f;

    // sanity check — ID reg should return 0x48
    uint8_t reg = HMC_ID_REG;
    uint8_t id  = 0;
    i2c_write_blocking(i2c, HMC5883L_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c,  HMC5883L_ADDR, &id,  1, false);
    if (id != 0x48) return false;

    if (!hmc_write_reg(i2c, HMC_CONFIG_A, config_a)) return false;
    if (!hmc_write_reg(i2c, HMC_CONFIG_B, config_b)) return false;
    if (!hmc_write_reg(i2c, HMC_MODE,     0x00))     return false; // continuous

    sleep_ms(10);
    return true;
}

bool HMC5883L_read(HMC5883L *dev) {
    uint8_t reg = HMC_DATA_START;
    uint8_t buf[6];

    i2c_write_blocking(dev->i2c, HMC5883L_ADDR, &reg, 1, true);
    int rc = i2c_read_blocking(dev->i2c, HMC5883L_ADDR, buf, 6, false);
    if (rc != 6) return false;

    // register order: X H/L, Z H/L, Y H/L  ← Z before Y, not a typo
    int16_t raw_x = (int16_t)(buf[0] << 8 | buf[1]);
    int16_t raw_z = (int16_t)(buf[2] << 8 | buf[3]);
    int16_t raw_y = (int16_t)(buf[4] << 8 | buf[5]);

    dev->x = raw_x / dev->gain_lsb;
    dev->y = raw_y / dev->gain_lsb;
    dev->z = raw_z / dev->gain_lsb;

    dev->heading = HMC5883L_heading(dev);
    return true;
}

float HMC5883L_heading(HMC5883L *dev) {
    float h = atan2f(dev->y, dev->x) * (180.0f / (float)M_PI);
    if (h < 0.0f) h += 360.0f;
    return h;
}

bool HMC5883L_heading_tilt_compensated(HMC5883L *dev, float roll_rad, float pitch_rad, float *heading_out) {
    if (!heading_out) return false;

    float cos_r = cosf(roll_rad);
    float sin_r = sinf(roll_rad);
    float cos_p = cosf(pitch_rad);
    float sin_p = sinf(pitch_rad);

    // rotate mag vector back to horizontal plane
    float xh = dev->x * cos_p + dev->z * sin_p;
    float yh = dev->x * sin_r * sin_p + dev->y * cos_r - dev->z * sin_r * cos_p;

    float h = atan2f(yh, xh) * (180.0f / (float)M_PI);
    if (h < 0.0f) h += 360.0f;

    *heading_out = h;
    return true;
}