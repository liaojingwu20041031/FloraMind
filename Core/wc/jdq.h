#ifndef jdq_H_
#define jdq_H_

#include "gpio.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h> 
#include "rtc.h"

void jdq_pos(int x,bool key);
void jdq_pos_wifi(uint8_t sb,uint8_t bg,uint8_t fs);
#endif

