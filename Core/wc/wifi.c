#include "wifi.h"
#include "cmsis_os.h"
#include <stdarg.h>

#define UART_TX_BUFFER_SIZE 512
#define HAL_Delay_MAX 2000 // 最大延迟2S
static uint8_t uart_tx_buffer[UART_TX_BUFFER_SIZE];

void HAL_UART_Transmit_IT_printf( const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int len = vsnprintf((char *)uart_tx_buffer, UART_TX_BUFFER_SIZE, format, args);
    va_end(args);

    if (len > 0)
    {
        HAL_UART_Transmit(&huart2, uart_tx_buffer, len, HAL_Delay_MAX);
    }
}

void ESP8266_Init(void)
{

      HAL_UART_Transmit_IT_printf("AT+CWJAP=\"%s\",\"%s\"\r\n", wifi_num, wifi_key);
	  osDelay(200);
      HAL_UART_Transmit_IT_printf("AT+CIPMODE=1\r\n"); // 开启透传模式
	  osDelay(200);
	  HAL_UART_Transmit_IT_printf("AT+CIPSNTPCFG=1,8,\"ntp1.aliyun.com\"\r\n");		//第三步
	  osDelay(3000);					//延迟
	  HAL_UART_Transmit_IT_printf("AT+MQTTUSERCFG=0,1,\"NULL\",\"ESP8266&k1oaeW0E3IM\",");		
      HAL_UART_Transmit_IT_printf("\"f8a3450dd1fb77a865bfeef5bca3572dc6ffbde7cfeca5531fda8aa78ee20b72\",0,0,\"\"\r\n");//第五步(因为指令过长，分两次发送)
	  osDelay(3000);					//延迟			
	  HAL_UART_Transmit_IT_printf("AT+MQTTCLIENTID=0,\"k1oaeW0E3IM.ESP8266|securemode=2\\,signmethod=hmacsha256\\,timestamp=1757828500940|\"\r\n");		//第六步
	  osDelay(3000);					//延迟	
	  HAL_UART_Transmit_IT_printf("AT+MQTTCONN=0,\"iot-06z00c2ical049c.mqtt.iothub.aliyuncs.com\",1883,1\r\n");		//第七步
	  osDelay(3000);	
	  HAL_UART_Transmit_IT_printf("AT+MQTTSUB=0,\"/sys/k1oaeW0E3IM/ESP8266/thing/service/property/set\",0\r\n");//订阅指令(后面的参数0表示QoS等级为0)
	  osDelay(1000);
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

