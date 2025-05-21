

#ifndef wifi_H_
#define wifi_H_

#include "stm32f1xx_hal.h"
#include "bool.h"
#include <stdio.h>
#include "usart.h"


#define wifi_num "xxfq1"             //wifi名称
#define wifi_key "123456789"         //wifi密码
//#define ID_dz    "192.168.146.225"                 //ID地址
//#define ID_dk    "8080"                 //ID端口
#define UART_TX_BUFFER_SIZE 512
static uint8_t uart_tx_buffer[UART_TX_BUFFER_SIZE];
extern UART_HandleTypeDef huart2;      // 声明 huart2 句柄

void HAL_UART_Transmit_IT_printf( const char *format, ...);
void ESP8266_Init(void);
void ESP8266_fs_Data(float temperature, int soil_moisture, float humidity, int co2, int light_lux);
void ESP8266_fs_Data2(uint8_t fs,uint8_t sb,uint8_t bg);
#endif
