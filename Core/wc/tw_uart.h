#ifndef _MY_UART_
#define _MY_UART_

#include "stm32f1xx_hal.h"
#include "stdio.h"	
#include "usart.h"
extern float pos_x;
extern float pos_y;
extern float zangle;
void Data_Analyse(uint8_t rec);
void Data_stop(double*pos_X,double*pos_Y);
#endif
