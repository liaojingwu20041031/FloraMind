#include "wifi.h"
#include <stdarg.h>

#define UART_TX_BUFFER_SIZE 512
static uint8_t uart_tx_buffer[UART_TX_BUFFER_SIZE];

void HAL_UART_Transmit_IT_printf( const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int len = vsnprintf((char *)uart_tx_buffer, UART_TX_BUFFER_SIZE, format, args);
    va_end(args);

    if (len > 0)
    {
        HAL_UART_Transmit_IT(&huart2, uart_tx_buffer, len);
    }
}

void ESP8266_Init()
{
    HAL_UART_Transmit_IT_printf("AT+CWJAP=\"%s\",\"%s\"\r\n", wifi_num, wifi_key);
	  HAL_Delay(200);
    HAL_UART_Transmit_IT_printf("AT+CIPMODE=1\r\n"); // 开启透传模式
	  HAL_Delay(200);
	  HAL_UART_Transmit_IT_printf("AT+CIPSNTPCFG=1,8,\"ntp1.aliyun.com\"\r\n");		//第三步
	  HAL_Delay(4000);					//延迟
	  HAL_UART_Transmit_IT_printf("AT+MQTTUSERCFG=0,1,\"NULL\",\"ESP8266&k1oaeW0E3IM\",\"6eb7724829702a8673e7019dc54476032874f321d3f34e43e15fec5203971a60\",0,0,\"\"\r\n");		//第五步
	  HAL_Delay(4000);					//延迟			
	  HAL_UART_Transmit_IT_printf("AT+MQTTCLIENTID=0,\"k1oaeW0E3IM.ESP8266|securemode=2\\,signmethod=hmacsha256\\,timestamp=1743827052139|\"\r\n");		//第六步
	  HAL_Delay(4000);					//延迟	
	  HAL_UART_Transmit_IT_printf("AT+MQTTCONN=0,\"iot-06z00c2ical049c.mqtt.iothub.aliyuncs.com\",1883,1\r\n");		//第七步
	  HAL_Delay(4000);	
	  HAL_UART_Transmit_IT_printf("AT+MQTTSUB=0,\"/sys/k1oaeW0E3IM/ESP8266/thing/service/property/set\",1\r\n");		//订阅指令
	  HAL_Delay(2000);
}


/**
 * @description: ESP8266发送数据到服务器
 * @param temperature：温度
 * @param soil_moisture：土壤湿度
 * @param humidity：室内湿度
 * @param co2：二氧化碳浓度
 * @param light_lux：光照强度
 */
void ESP8266_fs_Data(float temperature, int soil_moisture, float humidity, int co2, int light_lux) {
       HAL_UART_Transmit_IT_printf(
        "AT+MQTTPUB=0,"
        "\"/sys/k1oaeW0E3IM/ESP8266/thing/event/property/post\","
        "\"{\\\"id\\\":8\\,\\\"params\\\":{"
        "\\\"Temperature\\\":%.1f\\,"       // 温度(浮点)
        "\\\"SoilMoisture\\\":%d\\,"       // 土壤湿度(整型)
        "\\\"EnvironmentHumidity\\\":%.1f\\," // 环境湿度(浮点)
        "\\\"CO2Value\\\":%d\\,"           // CO2(整型)
        "\\\"LightLux\\\":%d"              // 光照(整型)
        "}\\,"
        "\\\"method\\\":\\\"thing.event.property.post\\\""
        "}\","
        "1,0\r\n",  // QoS=1, Retain=0
        temperature, soil_moisture, humidity, co2, light_lux
    );
}

/**
 * @description: 继电器控制状态标志发送
 */
void ESP8266_fs_Data2(uint8_t fs,uint8_t sb,uint8_t bg) {
     HAL_UART_Transmit_IT_printf(
    "AT+MQTTPUB=0,"
    "\"/sys/k1oaeW0E3IM/ESP8266/thing/event/property/post\","
    "\"{\\\"id\\\":8\\,"
    "\\\"params\\\":{"
        "\\\"FanSwitch\\\":\\\"%d\\\"\\,"       // 风扇状态 (1/0)
        "\\\"IrrigationPumpStatus\\\":\\\"%d\\\"\\," // 水泵状态 (1/0)
        "\\\"LightStatus\\\":\\\"%d\\\""        // 灯光状态 (1/0)
    "}\\,"
    "\\\"version\\\":\\\"1.0\\\"\\,"
    "\\\"method\\\":\\\"thing.event.property.post\\\""
    "}\","
    "1,0\r\n", 
    fs,   // 替换为风扇状态变量 (1或0)
    sb,   // 替换为水泵状态变量 (1或0)
    bg   // 替换为灯光状态变量 (1或0)
);
}

