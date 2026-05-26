#include "ibus.h"

#define IBUS_FRAME_SIZE 32
#define IBUS_HEADER_0 0x20
#define IBUS_HEADER_1 0x40
#define IBUS_CHANNELS 14

bool readIBus(int data[IBUS_CHANNELS]){

    uint8_t buff[30];
    uint8_t byte0 ;
    uart_read_blocking(uart1 ,&byte0,1);
    if(byte0 == IBUS_HEADER_0){
        uint8_t byte1 ;
        uart_read_blocking(uart1 ,&byte1,1);
        if(byte1 == IBUS_HEADER_1){
            uart_read_blocking(uart1 ,&buff[0],30);
            for(int i=0;i<IBUS_CHANNELS;i++){
                data[i] = (buff[i*2 +1] << 8)| buff[i*2];
            }
            return true;
        }
    }
    return false;

}