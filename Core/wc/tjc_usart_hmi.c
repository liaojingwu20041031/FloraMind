#include "tjc_usart_hmi.h"            // Device header
uint8_t dynamic_buffer[32];



//显示屏表情状态
void xsp_fs(const char* str, int num) {
    // 预留3字节给0xFF
    int len = snprintf((char*)dynamic_buffer,32,"%s=%d",str,num);
    // 安全追加结束符
    for(int i=0; i<3; i++) {
        dynamic_buffer[len++] = 0xFF;
    }
    
    HAL_UART_Transmit_IT(&huart1, dynamic_buffer, len);
}
