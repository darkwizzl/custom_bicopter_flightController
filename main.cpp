#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"

#include "ibus.h"
#include "mpu6050.h"
#include "hmc5883l.h"

#define PI 3.14159265358979323846f
#define MAX_DEFLECTION 90

#define servo1 0    // slice 0    
#define servo2 1    // slice 0    
#define servo3 2    // slice 1    
#define servo4 5    // slice 2    

#define bldcTop 3   // slice 1
#define bldcBot 4   // slice 2

#define RED 17
#define BLUE 13
#define GREEN 16

int finAngles[4] = {45, 135,225, 315};
int servoList[4] = {servo1,servo2,servo3,servo4};
int servoNeutral[4] = {90,90,90,90};

float toRadians(float deg){
    return (deg*PI/180.0f);
}

float toDegree(float rad){
    return(rad*180/PI);
}

void setServo(int servoNo, int angle){
    uint16_t pulse = 1000.0f + (angle / 180.0f) * 1000.0f;
    pwm_set_gpio_level(servoNo, pulse);
}

void callibrateEsc(){
    gpio_put(BLUE,0);
    setServo(bldcTop,180);setServo(bldcBot,180);
    sleep_us(3*1000000);

    setServo(bldcTop,0);setServo(bldcBot,0);
    sleep_us(3*1000000);
    gpio_put(BLUE,1);
    
}

void resetAllMotors(){
    for (int i =0; i<4; i++){
        setServo(servoList[i], servoNeutral[i]);
    }

    setServo(bldcTop,0);
    setServo(bldcBot,0);
}

void fin_mixing(int data[]) {
    float x = (data[1] - 1500) / 500.0f;
    float y = -(data[0] - 1500) / 500.0f;

    float A = sqrtf(x*x + y*y);
    if (A > 1.0f) A = 1.0f;

    float directionAngle = atan2f(y, x);   // radians, atan2 already handles all quadrants

    for (int i = 0; i < 4; i++) {
        float delta      = cosf(toRadians(finAngles[i]) - directionAngle);
        float servoAngle =  servoNeutral[i]+ MAX_DEFLECTION * A * delta;
        setServo(servoList[i], (int)servoAngle);
    }
}

void toggleLed(int led, int time_ms){

    static bool initialized = false;
    static absolute_time_t timeStart;

    if(!initialized){
        timeStart = get_absolute_time();
        initialized = true;
    }

    int64_t timeDiff =
        absolute_time_diff_us(timeStart, get_absolute_time());

    // Convert ms -> us
    if(timeDiff >= time_ms * 1000){
        gpio_put(led, !gpio_get(led));
        timeStart = get_absolute_time();
    }
}

int main(){
    
    //gp16-Green
    gpio_init(GREEN);
    gpio_set_dir(GREEN, GPIO_OUT);

    //gp13 - Blue
    gpio_init(BLUE);
    gpio_set_dir(BLUE, GPIO_OUT);

    //gp17-Red
    gpio_init(RED);
    gpio_set_dir(RED, GPIO_OUT);

    gpio_put(RED,1);
    gpio_put(BLUE,1);
    gpio_put(GREEN,1);

    // for printf
    stdio_init_all();

    uint8_t buff[30];
    int IBusData[14];
    float IMUdata[3];


    //uart (IBUS)
    gpio_set_function(9, UART_FUNCSEL_NUM(uart1,9));
    gpio_set_function(8, UART_FUNCSEL_NUM(uart1,8));
    
    uart_init(uart1,115200);

    //configure pwm 
    gpio_set_function(servo1, GPIO_FUNC_PWM);
    gpio_set_function(servo2, GPIO_FUNC_PWM) ;
    gpio_set_function(servo3, GPIO_FUNC_PWM);
    gpio_set_function(servo4, GPIO_FUNC_PWM);

    gpio_set_function(bldcTop, GPIO_FUNC_PWM);
    gpio_set_function(bldcBot, GPIO_FUNC_PWM);

    pwm_config pwmConfig = pwm_get_default_config();
    pwm_config_set_clkdiv(&pwmConfig, 133);
    pwm_config_set_wrap(&pwmConfig, 20000);

    pwm_init(0,&pwmConfig,true); // slice 0
    pwm_init(1,&pwmConfig,true); // slice 1
    pwm_init(2,&pwmConfig,true); // slice 2

    setServo(bldcBot,0);
    setServo(bldcTop,0);

    //callibrateEsc();
    resetAllMotors();
    MPU6050init();

    HMC5883L mag;
    if (!HMC5883L_init(&mag, i2c1, HMC_SAMPLES_8 | HMC_ODR_15, HMC_GAIN_1_3)) {
        printf("HMC5883L not found\n");
    }

    float gyro[3], accel[3];
    float roll_rad  = 0.0f;
    float pitch_rad = 0.0f;

    absolute_time_t prevTime = get_absolute_time();

    while (true) {
        absolute_time_t now = get_absolute_time();
        float dt = absolute_time_diff_us(prevTime, now) / 1000000.0f;
        prevTime = now;

        MPU6050read(gyro, accel);
        HMC5883L_read(&mag);

        float accel_roll  = atan2f(accel[1], accel[2])                                       * (180.0f / PI);
        float accel_pitch = atan2f(-accel[0], sqrtf(accel[1]*accel[1] + accel[2]*accel[2])) * (180.0f / PI);

        float roll_deg  = 0.98f * (roll_rad  * (180.0f/PI) + gyro[0]*dt) + 0.02f * accel_roll;
        float pitch_deg = 0.98f * (pitch_rad * (180.0f/PI) + gyro[1]*dt) + 0.02f * accel_pitch;

        roll_rad  = roll_deg  * (PI / 180.0f);
        pitch_rad = pitch_deg * (PI / 180.0f);

        float heading;
        HMC5883L_heading_tilt_compensated(&mag, roll_rad, pitch_rad, &heading);

        printf("roll: %.2f  pitch: %.2f  heading: %.1f\n", roll_deg, pitch_deg, heading);
    }
}