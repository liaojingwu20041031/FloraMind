/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "wifi.h"
#include "tjc_usart_hmi.h"
#include "stdio.h"
#include "dht11.h"
#include "adc_2.h"
#include "sgp30.h"
#include "jdq.h"
#include "string.h"
#include "stm32f1xx_hal_pwr.h"
#include "stm32f1xx_hal_rtc.h"
#include  <stdlib.h>
#include <Kal_lb.h>
#include <math.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint8_t wifi_string;// WiFi模块串口接收数据
uint8_t tw_fs[10];
char* wifi_string2 = NULL; 
uint8_t tw_num[4];
uint8_t sb_num2=0,bg_num2=0,fs_num2=0;//水泵，补光灯，风扇标志位
uint8_t sb_num_yy=0,bg_num_yy=0,fs_num_yy=0,sb_num_yy2=0,bg_num_yy2=0,fs_num_yy2=0,yy_num2=0;//远端、语音控制标志位

KalmanFilter wd_kal,sd_kal,tr_sd_kal,CO2_kal,gq_kal;//卡尔曼滤波参�?

float wd_2,sd_2,tr_sd_2,gq_2;//卡尔曼滤波后的�??
uint16_t CO2_2;//卡尔曼滤波后的�??

uint8_t CO2_NO=0;//预热完成标准�?
uint8_t yy_num=0; //AI语音聊天模式标志�?


uint8_t esp8266_key=0;//WiFi连接状�?�标志位

// 环形缓冲区结构定�?
#define UART_BUFFER_SIZE 512
typedef struct {
    uint8_t buffer[UART_BUFFER_SIZE];// 缓冲�?
    volatile uint16_t head;// 写指�?
    volatile uint16_t tail;// 读指�?
    volatile uint8_t overflow;// 溢出标志
    volatile bool message_ready;// 消息就绪标志
} ESP8266_RingBuffer;
static ESP8266_RingBuffer esp8266_rx_buffer = {0};// 初始化缓冲区
static bool json_processing_busy = false;// JSON处理忙标�?


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
void extract_switch_value(const char *json_str, const char *field,uint8_t*num);
void tw_cl(char *tw);//天问串口处理

void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
bool gy_pos(void);//人体感应模块
void cgq_sz(void);//传感器读数�??
void OLED_UI(void);//UI界面函数
bool wifi_cl(char *json_str);//WiFi数据处理函数
void ck_fs(uint8_t num);//发�?�串口数�?
bool zd_pos(void);//震动感应模块
void tw_kz(uint8_t*tw);//天问控制函数
void tw_cl2(void);//天问模块字符串发送处�?
void zw_zt_cl(float wd_2_z,float tr_sd_2_z,float gq_2_z,uint16_t CO2_2_z,uint8_t zt_fs);//植物状�?�检测函数（�?测环境数据）
void Kal_Init(uint16_t wd,uint16_t sd,uint16_t TVOC,uint16_t CO2);//卡尔曼滤波初始化
void Kal_pos(uint16_t wd,uint16_t sd,uint16_t TVOC,uint16_t CO2);//卡尔曼滤�?
bool extract_json_from_buffer(char *buffer, size_t buffer_size);

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)//wifi发�??
    {
      // 发�?�完成，可以在这里处理后续�?�辑
    }
		if(huart->Instance == USART1)//显示屏发�?
		{
			// 发�?�完成，可以在这里处理后续�?�辑
		}
		if(huart->Instance == USART3)//天问语言模块
		{
		
		}
}
// 接收完成回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
     if (huart->Instance == USART2) // ESP8266串口
    {
        // 保存接收到的�?个字节到环形缓冲�?
        uint16_t next_head = (esp8266_rx_buffer.head + 1) % UART_BUFFER_SIZE;
        
        if (next_head != esp8266_rx_buffer.tail) { // 缓冲区未�?
            esp8266_rx_buffer.buffer[esp8266_rx_buffer.head] = wifi_string;
            esp8266_rx_buffer.head = next_head;
            
            // �?测JSON消息结束�? ('}')
            if (esp8266_rx_buffer.buffer[esp8266_rx_buffer.head - 1] == '}') {
                // 标记有完整消息可以处�?
                esp8266_rx_buffer.message_ready = true;// 设置消息就绪标志
            }
        } else {
            // 缓冲区溢�?
            esp8266_rx_buffer.overflow = 1;
        }
        
        // 继续接收下一个字�?
        HAL_UART_Receive_IT(&huart2, &wifi_string, 1); // 改为单字节接�?
    }
		if(huart->Instance == USART3)//接受天问模块的信�?
		{ 
				
	    tw_cl((char*)tw_num);//信息处理
			HAL_UART_Receive_IT(&huart3, tw_num, 3);//初始化接�?
		}
}

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for cgq_01 */
osThreadId_t cgq_01Handle;
const osThreadAttr_t cgq_01_attributes = {
  .name = "cgq_01",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for ztcl_02 */
osThreadId_t ztcl_02Handle;
const osThreadAttr_t ztcl_02_attributes = {
  .name = "ztcl_02",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for usart_03 */
osThreadId_t usart_03Handle;
const osThreadAttr_t usart_03_attributes = {
  .name = "usart_03",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for twkz_04 */
osThreadId_t twkz_04Handle;
const osThreadAttr_t twkz_04_attributes = {
  .name = "twkz_04",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for ESP8266_05 */
osThreadId_t ESP8266_05Handle;
const osThreadAttr_t ESP8266_05_attributes = {
  .name = "ESP8266_05",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityHigh1,
};
/* Definitions for jdq_06 */
osThreadId_t jdq_06Handle;
const osThreadAttr_t jdq_06_attributes = {
  .name = "jdq_06",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for UI_OLED_07 */
osThreadId_t UI_OLED_07Handle;
const osThreadAttr_t UI_OLED_07_attributes = {
  .name = "UI_OLED_07",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Kal_wd */
osMessageQueueId_t Kal_wdHandle;
const osMessageQueueAttr_t Kal_wd_attributes = {
  .name = "Kal_wd"
};
/* Definitions for Kal_sd */
osMessageQueueId_t Kal_sdHandle;
const osMessageQueueAttr_t Kal_sd_attributes = {
  .name = "Kal_sd"
};
/* Definitions for Kal_trsd */
osMessageQueueId_t Kal_trsdHandle;
const osMessageQueueAttr_t Kal_trsd_attributes = {
  .name = "Kal_trsd"
};
/* Definitions for Kal_gq */
osMessageQueueId_t Kal_gqHandle;
const osMessageQueueAttr_t Kal_gq_attributes = {
  .name = "Kal_gq"
};
/* Definitions for Kal_co2 */
osMessageQueueId_t Kal_co2Handle;
const osMessageQueueAttr_t Kal_co2_attributes = {
  .name = "Kal_co2"
};
/* Definitions for zw_key */
osMessageQueueId_t zw_keyHandle;
const osMessageQueueAttr_t zw_key_attributes = {
  .name = "zw_key"
};
/* Definitions for tw_key */
osMessageQueueId_t tw_keyHandle;
const osMessageQueueAttr_t tw_key_attributes = {
  .name = "tw_key"
};
/* Definitions for zt_fs */
osMessageQueueId_t zt_fsHandle;
const osMessageQueueAttr_t zt_fs_attributes = {
  .name = "zt_fs"
};
/* Definitions for zt_bg */
osMessageQueueId_t zt_bgHandle;
const osMessageQueueAttr_t zt_bg_attributes = {
  .name = "zt_bg"
};
/* Definitions for zt_sb */
osMessageQueueId_t zt_sbHandle;
const osMessageQueueAttr_t zt_sb_attributes = {
  .name = "zt_sb"
};
/* Definitions for tw_fs */
osMessageQueueId_t tw_fsHandle;
const osMessageQueueAttr_t tw_fs_attributes = {
  .name = "tw_fs"
};
/* Definitions for tw_bg */
osMessageQueueId_t tw_bgHandle;
const osMessageQueueAttr_t tw_bg_attributes = {
  .name = "tw_bg"
};
/* Definitions for tw_sb */
osMessageQueueId_t tw_sbHandle;
const osMessageQueueAttr_t tw_sb_attributes = {
  .name = "tw_sb"
};
/* Definitions for ESP8266_fs */
osMessageQueueId_t ESP8266_fsHandle;
const osMessageQueueAttr_t ESP8266_fs_attributes = {
  .name = "ESP8266_fs"
};
/* Definitions for ESP8266_bg */
osMessageQueueId_t ESP8266_bgHandle;
const osMessageQueueAttr_t ESP8266_bg_attributes = {
  .name = "ESP8266_bg"
};
/* Definitions for ESP8266_sb */
osMessageQueueId_t ESP8266_sbHandle;
const osMessageQueueAttr_t ESP8266_sb_attributes = {
  .name = "ESP8266_sb"
};
/* Definitions for esp8266_x_01 */
osMutexId_t esp8266_x_01Handle;
const osMutexAttr_t esp8266_x_01_attributes = {
  .name = "esp8266_x_01"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTask02(void *argument);
void StartTask03(void *argument);
void StartTask04(void *argument);
void StartTask05(void *argument);
void StartTask06(void *argument);
void StartTask07(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of esp8266_x_01 */
  esp8266_x_01Handle = osMutexNew(&esp8266_x_01_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of Kal_wd */
  Kal_wdHandle = osMessageQueueNew (16, sizeof(float), &Kal_wd_attributes);

  /* creation of Kal_sd */
  Kal_sdHandle = osMessageQueueNew (16, sizeof(float), &Kal_sd_attributes);

  /* creation of Kal_trsd */
  Kal_trsdHandle = osMessageQueueNew (16, sizeof(float), &Kal_trsd_attributes);

  /* creation of Kal_gq */
  Kal_gqHandle = osMessageQueueNew (16, sizeof(float), &Kal_gq_attributes);

  /* creation of Kal_co2 */
  Kal_co2Handle = osMessageQueueNew (16, sizeof(uint16_t), &Kal_co2_attributes);

  /* creation of zw_key */
  zw_keyHandle = osMessageQueueNew (16, sizeof(uint8_t), &zw_key_attributes);

  /* creation of tw_key */
  tw_keyHandle = osMessageQueueNew (16, sizeof(uint8_t), &tw_key_attributes);

  /* creation of zt_fs */
  zt_fsHandle = osMessageQueueNew (16, sizeof(uint8_t), &zt_fs_attributes);

  /* creation of zt_bg */
  zt_bgHandle = osMessageQueueNew (16, sizeof(uint8_t), &zt_bg_attributes);

  /* creation of zt_sb */
  zt_sbHandle = osMessageQueueNew (16, sizeof(uint8_t), &zt_sb_attributes);

  /* creation of tw_fs */
  tw_fsHandle = osMessageQueueNew (16, sizeof(uint8_t), &tw_fs_attributes);

  /* creation of tw_bg */
  tw_bgHandle = osMessageQueueNew (16, sizeof(uint8_t), &tw_bg_attributes);

  /* creation of tw_sb */
  tw_sbHandle = osMessageQueueNew (16, sizeof(uint8_t), &tw_sb_attributes);

  /* creation of ESP8266_fs */
  ESP8266_fsHandle = osMessageQueueNew (16, sizeof(uint8_t), &ESP8266_fs_attributes);

  /* creation of ESP8266_bg */
  ESP8266_bgHandle = osMessageQueueNew (16, sizeof(uint8_t), &ESP8266_bg_attributes);

  /* creation of ESP8266_sb */
  ESP8266_sbHandle = osMessageQueueNew (16, sizeof(uint8_t), &ESP8266_sb_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of cgq_01 */
  cgq_01Handle = osThreadNew(StartDefaultTask, NULL, &cgq_01_attributes);

  /* creation of ztcl_02 */
  ztcl_02Handle = osThreadNew(StartTask02, NULL, &ztcl_02_attributes);

  /* creation of usart_03 */
  usart_03Handle = osThreadNew(StartTask03, NULL, &usart_03_attributes);

  /* creation of twkz_04 */
  twkz_04Handle = osThreadNew(StartTask04, NULL, &twkz_04_attributes);

  /* creation of ESP8266_05 */
  ESP8266_05Handle = osThreadNew(StartTask05, NULL, &ESP8266_05_attributes);

  /* creation of jdq_06 */
  jdq_06Handle = osThreadNew(StartTask06, NULL, &jdq_06_attributes);

  /* creation of UI_OLED_07 */
  UI_OLED_07Handle = osThreadNew(StartTask07, NULL, &UI_OLED_07_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  任务1：传感器处理（读�?+卡尔曼滤波）-------------------------------------------------------任务1-----------------------------------------------------------------
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */

   DHT11_Init();//初始化温湿度传感�?
	 HAL_ADCEx_Calibration_Start(&hadc1 );//adc校准
   HAL_ADC_Start_DMA(&hadc1, (uint32_t *)My_adcData, adc_max);//光敏和土壤湿度传感器
	 sgp30_init();//sgp30初始�?
   Kal_Init(25, 60, 200, 500);//卡尔曼滤波初始化( 初始值：温度25度，湿度60%，TVOC200ppb，CO2 500ppm )

  /* Infinite loop */
  for(;;)
  {
    cgq_sz();//传感器读取函�?(处理分析+卡尔曼滤�?)
    osDelay(5);
  } 
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief 任务2：植物状态处理（环境数据+自动控制+串口屏控制）------------------------------------------任务2-------------------------------------------------------------------
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */
  static float kal_wd_2=0,kal_sd_2=0,kal_tr_sd_2=0,kal_gq_2=0;//卡尔曼滤波后的�??
  static uint16_t kal_CO2_2=0;//卡尔曼滤波后的�??
  static uint8_t zt_fs=0XFF;//植物状�?�码
  HAL_UART_Init(&huart1);  
  /* Infinite loop */
  for(;;)
  {
    if(osMessageQueueGet(Kal_wdHandle, &kal_wd_2, NULL, 100 ) == osOK && (osMessageQueueGet(Kal_sdHandle, &kal_sd_2, NULL, 100 ) == osOK)
       && (osMessageQueueGet(Kal_trsdHandle, &kal_tr_sd_2, NULL, 100 ) == osOK) && (osMessageQueueGet(Kal_gqHandle, &kal_gq_2, NULL, 100 ) == osOK)
       && (osMessageQueueGet(Kal_co2Handle, &kal_CO2_2, NULL, 100 ) == osOK)){
      zw_zt_cl(kal_wd_2,kal_sd_2,kal_gq_2,kal_CO2_2,zt_fs);//植物状�?�检测函数（�?测环境数据）
    }
    osDelay(5);
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief 发�?�串口信息（ESP8266和天问模块）------------------------------------------------任务3-------------------------------------------------------------------
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void *argument)
{
  /* USER CODE BEGIN StartTask03 */
   static uint8_t zt_fs=0;//植物状�?�码
	 
  /* Infinite loop */
  for(;;)
  {
   osMessageQueueGet(zw_keyHandle, &zt_fs, NULL, 10 );//----获取植物状�?�码
   ck_fs(zt_fs);//发�?�串口消息（ESP8266和天问模块）
   osDelay(10);
  }
  /* USER CODE END StartTask03 */
}

/* USER CODE BEGIN Header_StartTask04 */
/**
* @brief 天问模块语言控制（同时更新后端数据）----------------------------------------------------任务4-------------------------------------------------------------------
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask04 */
void StartTask04(void *argument)
{
  /* USER CODE BEGIN StartTask04 */
  static uint8_t tw_num2=0;//天问控制�?
  HAL_UART_Init(&huart3);  //天问模块
	HAL_UART_Receive_IT(&huart3, tw_num, 3);//初始化接�?

  /* Infinite loop */
  for(;;)
  {
    if(osMessageQueueGet(tw_keyHandle, &tw_num2, NULL, 0) == osOK){//----获取天问控制�?
      tw_kz(&tw_num2);//天问模块控制
    }
    osDelay(5);
  }
  /* USER CODE END StartTask04 */
}

/* USER CODE BEGIN Header_StartTask05 */
/**
* @brief esp8266模块物联网json信息处理--------------------------------------------------------------任务5-------------------------------------------------------------------
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask05 */
void StartTask05(void *argument)
{
  /* USER CODE BEGIN StartTask05 */
   
  osThreadSuspend(UI_OLED_07Handle);// 暂停OLED显示任务---------------------------------------------调试�?-------------------------------------------------------

  HAL_UART_Init(&huart2);  // WiFi模块
  HAL_UART_Receive_IT(&huart2, &wifi_string, 1);//初始化接�?---待修�?  
  osMutexAcquire(esp8266_x_01Handle, osWaitForever); // 上锁，确保物联网先连�?
  ESP8266_Init();//热点连接
  osMutexRelease(esp8266_x_01Handle);// 解锁
  
  esp8266_rx_buffer.head = 0;// 初始化缓冲区指针
  esp8266_rx_buffer.tail = 0;// 初始化缓冲区指针
  esp8266_rx_buffer.overflow = 0;// 初始化缓冲区状�??
  esp8266_rx_buffer.message_ready = false;// 初始化缓冲区状�??

  esp8266_key=1;//WiFi连接成功标志�?

  // JSON解析用临时缓冲区
   static char json_buffer[384] = {0};

  /* Infinite loop */
  for(;;)
  {
    // �?查是否有完整JSON�?要处�?
        if (esp8266_rx_buffer.message_ready && !json_processing_busy) {
            json_processing_busy = true;
            
            if (extract_json_from_buffer(json_buffer, sizeof(json_buffer))) {
                wifi_cl(json_buffer);
            }
            
            json_processing_busy = false;
        }
        
        // �?查缓冲区溢出
        if (esp8266_rx_buffer.overflow) {
            // 清空缓冲区，重新�?�?
            esp8266_rx_buffer.head = 0;
            esp8266_rx_buffer.tail = 0;
            esp8266_rx_buffer.overflow = 0;
            esp8266_rx_buffer.message_ready = false;
        }
    osDelay(5);
  }
  /* USER CODE END StartTask05 */
}

/* USER CODE BEGIN Header_StartTask06 */
/**
* @brief 继电器控制任务（风扇、水泵�?�补光灯�?----------------------------------------------------任务6-------------------------------------------------------------------
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask06 */
void StartTask06(void *argument)
{
  /* USER CODE BEGIN StartTask06 */
  static uint8_t sb_num=0,bg_num=0,fs_num=0;//水泵，补光灯，风扇标志位
  static uint8_t sb_num_yy_2=0,bg_num_yy_2=0,fs_num_yy_2=0;//远端、语音控制标志位(获取�?)
  static bool yy_key=false;//语音控制标志�?(变化�?)
  
  /* Infinite loop */
  for(;;)
  {
   if(yy_num2==1){//远程端高于本地端
    if(osMessageQueueGet(ESP8266_sbHandle, &sb_num_yy_2, NULL, 0) == osOK){
      yy_key=true;
    }
    if(osMessageQueueGet(ESP8266_bgHandle, &bg_num_yy_2, NULL, 0) == osOK){
      yy_key=true;
    }
    if(osMessageQueueGet(ESP8266_fsHandle, &fs_num_yy_2, NULL, 0) == osOK){
      yy_key=true;
    }
    if(osMessageQueueGet(tw_sbHandle, &sb_num_yy_2, NULL, 0) == osOK){
      yy_key=true;
    }
    if(osMessageQueueGet(tw_bgHandle, &bg_num_yy_2, NULL, 0) == osOK){
      yy_key=true;
    }
    if(osMessageQueueGet(tw_fsHandle, &fs_num_yy_2, NULL, 0) == osOK){
      yy_key=true;
    }
   }
   else{//本地端高于远程端
    if(osMessageQueueGet(zt_sbHandle, &sb_num, NULL, 0) == osOK){
      yy_key=true;
    }
    if(osMessageQueueGet(zt_bgHandle, &bg_num, NULL, 0) == osOK){
      yy_key=true;
    }
    if(osMessageQueueGet(zt_fsHandle, &fs_num, NULL, 0) == osOK){
      yy_key=true;
    }
   }
  
  if(yy_key==true){//有变化的时�?�才执行
    yy_key=false;
    jdq_pos_wifi(sb_num,bg_num,fs_num,&yy_num2,sb_num_yy,bg_num_yy,fs_num_yy);//继电器控制开�?
    }

    osDelay(5);
  }
  /* USER CODE END StartTask06 */
}

/* USER CODE BEGIN Header_StartTask07 */
/**
* @brief UI_OLED显示任务（OLED屏幕显示�?------------------------------------------------------------任务7-------------------------------------------------------------------
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask07 */
void StartTask07(void *argument)
{
  /* USER CODE BEGIN StartTask07 */

   OLED_Init();
	 OLED_Clear();
  /* Infinite loop */
  for(;;)
  {
    OLED_UI();//UI界面函数
    osDelay(5);
  }
  /* USER CODE END StartTask07 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
//--------------------------------------------------------------------------------------------------------------------------------------------↓函数区-------------------------------------------------------------------------------------------------------------------------------
/**
 * @description:读取传感器数值函数（读取、滤波�?�调节感应）
 */
void cgq_sz(void)
{
    static uint16_t wd,sd,TVOC,CO2;
  	DHT11_Read_Data(&wd,&sd);//温湿度检�?
		ADC_dispose();//adc处理
		sgp30_read(&CO2, &TVOC);//TVOC和CO2浓度�?�?
	  Kal_pos(wd,sd,TVOC,CO2);//卡尔曼滤�?
}



/**
 * @description:卡尔曼滤波初始化函数(�?有传感器)
 */
void Kal_Init(uint16_t wd,uint16_t sd,uint16_t TVOC,uint16_t CO2)
{
  KalmanFilter_Init(&wd_kal,0.1,1.3,(float)(wd >> 8) + (float)(wd & 0xFF) / 256.0f,1);
	KalmanFilter_Init(&sd_kal,0.1,1.3,(float)(sd >> 8) + (float)(sd & 0xFF) / 256.0f,1);
  KalmanFilter_Init(&tr_sd_kal,0.1,1.3,tr_sd,1);
	KalmanFilter_Init(&gq_kal,0.1,4,gq,1);
	
	 // CO2传感器专用参数（动�?�噪声版本）
  KalmanFilter_Init(&CO2_kal, 
                    10.0f,    // 初始q较大以�?�应上升阶段
                    2500.0f,  // r=502（基�?噪声�?
                    (CO2 < 400) ? 400.0f : (float)CO2, // 强制不低�?400ppm
                    10000.0f); // 初始高不确定�?
}
/**
 * @description:卡尔曼滤波函�?(�?有传感器)
 */
void Kal_pos(uint16_t wd,uint16_t sd,uint16_t TVOC,uint16_t CO2)
{

  wd_2 = KalmanFilter_Update(&wd_kal, (float)(wd >> 8) + (float)(wd & 0xFF) / 256.0f);
  osMessageQueuePut(Kal_wdHandle, &wd_2, 0, 0);//放入消息队列
	sd_2=KalmanFilter_Update(&sd_kal,(float)(sd >> 8) + (float)(sd & 0xFF) / 256.0f);
  osMessageQueuePut(Kal_sdHandle, &sd_2, 0, 0);//放入消息队列
	tr_sd_2=KalmanFilter_Update(&tr_sd_kal,tr_sd);
  osMessageQueuePut(Kal_trsdHandle, &tr_sd_2, 0, 0);//放入消息队列
	gq_2=KalmanFilter_Update(&gq_kal,gq);
  osMessageQueuePut(Kal_gqHandle, &gq_2, 0, 0);//放入消息队列

	if(CO2>480)//等待预热完成
  {
		CO2_NO=1;
		/* CO2特殊处理 */
    // 1. 动�?�调整测量噪声（误差±5%读数�?
    CO2_kal.r = 2500.0f + 0.0025f * CO2_kal.x * CO2_kal.x;
    
    // 2. 强制不低�?400ppm
    float raw_CO2 = (CO2 < 400) ? 400.0f : (float)CO2;
    
    // 3. 动�?�过程噪声（根据变化率调整）
    static float last_CO2 = 400.0f;
    float change_rate = (float)fabs((double)(CO2_kal.x - last_CO2)) / 1.0f; // 假设1秒采样间�?
    CO2_kal.q = 5.0f + change_rate * 2.0f;
    last_CO2 = CO2_kal.x;
    
    // 4. 执行滤波
    CO2_2 = KalmanFilter_Update(&CO2_kal, raw_CO2);
    osMessageQueuePut(Kal_co2Handle, &CO2_2, 0, 0);//放入消息队列
	}
	else if(CO2_NO==0)
	{
	  CO2_2=CO2;
    osMessageQueuePut(Kal_co2Handle, &CO2_2, 0, 0);//放入消息队列
	}

}

/**
 * @description: 植物状�?�控制核心函�?
 * @note: 综合环境数据与互动状态，实现智能设备控制与拟人表情管�?
 * 算法特点�?
 * 1. 自�?�应阈�?�调整（基于历史数据�?
 * 2. 预测性控制（土壤湿度趋势预测�?
 * 3. 多因素协同决策（温度-CO2综合评分�?
 * 4. 状�?�持续时间加�?
 * 5. 表情平滑过渡管理
 */
void zw_zt_cl(float wd_2_z,float tr_sd_2_z,float gq_2_z,uint16_t CO2_2_z,uint8_t zt_fs)
{
    /*----------------------- 环境参数结构�? -----------------------*/
    // 使用结构体封装所有阈值参数和状�?�计时器，增强代码可维护�?
    static struct {
        float temp_high;        // 动�?�高温阈值（基于24小时历史数据�?
        float temp_low;         // 动�?�低温阈�?
        float soil_dry;         // 土壤干燥阈�?�（%�?
        float soil_wet;         // 土壤湿润阈�?�（%�?
        float light_low;        // 光照不足阈�?�（%�?
        float co2_high;         // CO2超标阈�?�（ppm�?
        uint32_t state_timer[6]; // 状�?�持续时间计数器[高温,干燥,CO2,低温,低光,正常]
    } env_threshold = {
        .temp_high = 32.0f,     // 初始默认�?(可修�?))
        .temp_low = 20.0f,
        .soil_dry = 25.0f,
        .soil_wet = 65.0f,
        .light_low = 55.0f,
        .co2_high = 1000.0f
    };

    /*----------------------- 状�?�跟踪变�? -----------------------*/
    static uint8_t last_expression = 0; // 上一次发送的表情ID(0表示初始状�??)
    uint8_t current_expression = 0;        // 当前计算的表情ID（默认待机表情）
    static uint32_t last_update = 0;       // 上次阈�?�更新时间戳（ms�?

    static uint8_t sb_num=0,bg_num=0,fs_num=0;
    /*----------------------- 自�?�应阈�?�系�? -----------------------*/
    // 温度历史数据窗口(30个周期（假设每隔3秒更新一次）
    static float temp_history[30] = {0};   // 环形缓冲区存储温度历�?
    static uint8_t history_index = 0;      // 当前写入位置索引

    // �?3秒更新一次温度历史并重新计算阈�??
    if(HAL_GetTick() - last_update > 3000) { // 3000ms = 3�?
			// 更新环形缓冲�?
        temp_history[history_index] = wd_2_z;
        history_index = (history_index + 1) % 24;
        
        // 计算目前位置索引+1(不超�?30个周�?)的平均温�?
        float temp_sum = 0;
			  uint8_t n=(history_index+1>60)?60:history_index+1;
			  for(int i=0; i<n;i++) temp_sum += temp_history[i];
        float temp_avg = temp_sum /n;
        
        // 动�?�调整阈值：平均值�?3�?
        env_threshold.temp_high = temp_avg + 3.0f;
        env_threshold.temp_low = temp_avg - 3.0f;
        
        last_update = HAL_GetTick(); // 更新时间�?
    }

    /*====================== 智能设备控制算法 ======================*/
    // 补光灯控制策略：动�?�迟滞光照控�?
    static float light_last = 0;           // 上一次光照�??
    float light_change_rate = fabs(gq_2_z - light_last); // 光照变化�?
    float light_hysteresis = light_change_rate > 5 ? 10:5; // 动�?�迟滞范�?
		
    // 控制逻辑：光照低于阈�?-迟滞时开启，高于阈�??+迟滞时关�?
    if(gq_2_z < (env_threshold.light_low - light_hysteresis)) 
		{
      bg_num = 1; // �?启补光灯
      osMessageQueuePut(zt_bgHandle, &bg_num, 0, 0);//放入消息队列--用于继电器控�?
    } else if(gq_2_z > (env_threshold.light_low + light_hysteresis)) 
		{
      bg_num = 0; // 关闭补光�?
      osMessageQueuePut(zt_bgHandle, &bg_num, 0, 0);//放入消息队列--用于继电器控�?
    }
    light_last = gq_2_z; // 保存当前光照�?

		
    //抽水机控制策略：带趋势预测的土壤湿度控制
    static float soil_history[3] = {0};    // �?�?3次土壤湿度记�?
    soil_history[2] = soil_history[1];     // 滚动更新历史数据
    soil_history[1] = soil_history[0];
    soil_history[0] = tr_sd_2_z;
    
    // 线�?�外推预测公式：X(n+1) = 3X(n) - 3X(n-1) + X(n-2)
    float soil_pred = 3*soil_history[0] - 3*soil_history[1] + soil_history[2];
    // 动�?�迟滞：预测干燥时扩大迟滞范围（防止频繁�?关）
    float soil_hysteresis = (soil_pred < env_threshold.soil_dry) ? 8.0f : 3.0f;
    
    if(tr_sd_2_z < (env_threshold.soil_dry - soil_hysteresis)) {
        sb_num = 1; // �?启抽水机
        osMessageQueuePut(zt_sbHandle, &sb_num, 0, 0);//放入消息队列--用于继电器控�?
        env_threshold.state_timer[1]++; // 干燥状�?�持续时�?+1
    } else if(tr_sd_2_z > (env_threshold.soil_dry + soil_hysteresis)) {
        sb_num = 0; // 关闭抽水�?
        osMessageQueuePut(zt_sbHandle, &sb_num, 0, 0);//放入消息队列--用于继电器控�?
        env_threshold.state_timer[1] = 0; // 重置干燥计时�?
    }

    // 风扇控制策略：温�?-CO2综合评分系统
    // 评分公式:0.6*(当前温度/高温阈�??) + 0.4*(当前CO2/CO2阈�??)
    float temp_co2_score = 0.6f*(wd_2_z/env_threshold.temp_high) 
                         + 0.4f*(CO2_2_z/env_threshold.co2_high);
		
    if(temp_co2_score > 1.0f) { // 综合评分超过100%
        fs_num = 1; // �?启风�?
        osMessageQueuePut(zt_fsHandle, &fs_num, 0, 0);//放入消息队列--用于继电器控�?
        // 更新状�?�持续时间计数器
        env_threshold.state_timer[0]++; // 高温计时+1
        env_threshold.state_timer[2]++; // CO2计时+1
    } else {
        fs_num = 0; // 关闭风扇
        osMessageQueuePut(zt_fsHandle, &fs_num, 0, 0);//放入消息队列--用于继电器控�?
        env_threshold.state_timer[0] = 0; // 重置高温计时�?
        env_threshold.state_timer[2] = 0; // 重置CO2计时�?
    }

    /*====================== 智能表情决策系统 ======================*/
    // 状�?�权重计算（考虑持续时间和严重程度）
    float state_weights[6] = {0}; //------------------------------------------各状态权重[高温,干燥,CO2,低温,低光,正常] ------------------------------------------ //
    
    // 高温权重 = 是否超标 * (1 + 0.0001*持续时间)
    state_weights[0] = (wd_2_z > env_threshold.temp_high) 
                     * (1 + 0.0001f*env_threshold.state_timer[0]);
    
    // 干燥权重 = 是否干燥 * (1 + 0.0005*持续时间)
    state_weights[1] = (tr_sd_2_z < env_threshold.soil_dry)
                     * (1 + 0.0005f*env_threshold.state_timer[1]);
    
    // CO2权重 = 是否超标 * (1 + 0.0008*持续时间)
    state_weights[2] = (CO2_2_z > env_threshold.co2_high)
                     * (1 + 0.0008f*env_threshold.state_timer[2]);
    
    // 低温权重（简单布尔�?�）
    state_weights[3] = (wd_2_z < env_threshold.temp_low)+0.01;
    
    // 低光权重（简单布尔�?�）
    state_weights[4] = (gq_2_z < env_threshold.light_low)+0.01;
    
    // 正常状�?�基�?权重
    state_weights[5] = 1.0f;


    // 确定�?高优先级状�??
    uint8_t max_index = 0; // 默认正常待机状�??
    for(int i=0; i<=5; i++) {
        if(state_weights[i] >= state_weights[max_index]) {
            max_index = i;
        }
    }
    /*----------------------- 表情映射逻辑 -----------------------*/
    switch(max_index) {
        case 0: // 高温状�??
            // 持续时间>10单位：生气，否则眩晕
            current_expression = (env_threshold.state_timer[0] > 30) ? 8 : 12;
            break;
            
        case 1: // 干燥状�??
            // 持续时间>5单位：哭泣，否则眩晕
            current_expression = (env_threshold.state_timer[1] > 30) ? 6 : 12;
            break;
            
        case 2: // CO2超标
            // CO2>1500ppm：眩晕，否则难过
            current_expression = (CO2_2 > 1500) ? 12 : 7;
            break;
            
        case 3: // 低温状�??
            current_expression = 4; // 害羞
            break;
            
        case 4: // 低光状�??
            current_expression = 11; // 休眠
            break;
            
        default: // 正常状�??
            if(gy_pos() || current_expression!=0) { // �?测到人体
                // 互动超时处理�??5秒切换表情）
                static uint32_t interact_timer = 0;
                if(HAL_GetTick() - interact_timer > 5000) {
									  xsp_fs("sleep", 0); // 保证屏幕为非睡眠状�??
                    current_expression = (current_expression == 0) ? 2:0; // 切换得意
                    interact_timer = HAL_GetTick();
                }
            }
    }

    /*----------------------- 表情输出管理和RGB灯控�?? -----------------------*/
    if(current_expression != last_expression) {
        // �??小间隔保护（3秒防抖动�??
        static uint32_t last_change = 0;
        if(HAL_GetTick() - last_change > 3000) {
					 if(yy_num==1)//语音模式下强制为识别语音的状�??
					 {
					  xsp_fs("num", 9); // 发�?�表情指�??
						current_expression=0;//默认语音下为白色灯效
					 }
					 else
					 {
            xsp_fs("num", current_expression); // 发�?�表情指�??
					 }
					 
					 zt_fs=current_expression;//植物状�?�RGB灯映�??
				   osMessageQueuePut(zw_keyHandle, &zt_fs, 0, 0);//放入消息队列(传给发�?�串口函�??)

           last_expression = current_expression;
           last_change = HAL_GetTick(); // 记录�??后变更时�??
        }
    }
}

/**
 * @description:OLED显示界面函数
 */
void OLED_UI(void)
{
  
    OLED_Showdecimal(0,0, wd_2,2,2,16,0);
		OLED_Showdecimal(40,0, sd_2,2,2,16,0);
		OLED_Showdecimal(0,2,gq_2,2,2,16,0);
		OLED_Showdecimal(40,2,tr_sd_2,2,2,16,0);
		OLED_ShowNum(0,4,CO2_2,5,16,0);		
	
		//OLED_ShowNum(0,6,tw_num[0],3,16,0);
		//OLED_ShowNum(30,6,tw_num[1],3,16,0);
	  OLED_ShowNum(0,6,sb_num_yy,3,16,0);
		OLED_ShowNum(30,6,fs_num_yy,3,16,0);
	  OLED_ShowNum(60,6,bg_num_yy,3,16,0);

    OLED_ShowNum(85,6,esp8266_key,1,16,0); //ESP8266连接标志�??
	  //OLED_ShowNum(0,6,HAL_GetTick(),8,16,0);
	
	  //OLED_ShowNum(64,6,time,8,16,0);


	  if(gy_pos())
		{
		 OLED_ShowNum(50,4,1,1,16,0);
		}
		else
		{
		 OLED_ShowNum(50,4,0,1,16,0 );
		}
		
		if(zd_pos())
		{
	 OLED_ShowNum(60,4,1,1,16,0);
		}
		else
		{
		 OLED_ShowNum(60,4,0,1,16,0 );
		}		
}

/**
 * @description: 人体感应模块函数（返�?? 1有人�??0无人�??
 */
bool gy_pos(void)
{
  if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_0))
	{
		return true;
	}
	else
	{
	  return false;
	}
}

/**
 * @description: 震动感应模块函数（返�?? 1有人�??0无人�??
 */
bool zd_pos(void)
{
  if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_1))
	{
		return true;
	}
	else
	{
	  return false;
	}
}


/**
 * @description: 直接从字符串中提取字段的整数值（1�??0�??
 */
void extract_switch_value(const char *json_str, const char *field,uint8_t*num) {
    // 构�?�搜索模式（�?? "FanSwitch":"�??
    char pattern[32];
    snprintf(pattern, sizeof(pattern), "\"%s\":\"", field);

    // 查找字段位置
    const char *pos = strstr(json_str, pattern);
    if (!pos) // 字段不存�??
		{
		  return;
		} 
    else
		{
			// 跳转到�?�的位置（跳�?? "FanSwitch":"�??
			pos += strlen(pattern);
			// 直接返回字符 '0' �?? '1' 对应的整数�??
			*num = ((*pos == '1') ? 1 : 0);
		}
}


/**
 * @description: 从环形缓冲区中提取完整JSON
 * @param buffer: 输出JSON的缓冲区
 * @param buffer_size: 缓冲区大�?
 * @return: 是否提取到完整JSON
 */
bool extract_json_from_buffer(char *buffer, size_t buffer_size)
{
    if (!esp8266_rx_buffer.message_ready) {
        return false;
    }
    
    // 找到�?近的JSON�?始位�? '{'
    uint16_t read_pos = esp8266_rx_buffer.head;
    bool found_start = false;
    uint16_t json_length = 0;
    uint16_t json_start = 0;
    uint8_t brace_count = 0;
    
    // 从最近接收的字符向前搜索
    while (read_pos != esp8266_rx_buffer.tail) {
        read_pos = (read_pos == 0) ? (UART_BUFFER_SIZE - 1) : (read_pos - 1);
        
        if (esp8266_rx_buffer.buffer[read_pos] == '}') {
            brace_count++;
        } else if (esp8266_rx_buffer.buffer[read_pos] == '{') {
            brace_count--;
            if (brace_count == 0) {
                // 找到完整的JSON�?始位�?
                json_start = read_pos;
                found_start = true;
                break;
            }
        }
    }
    
    if (!found_start) {
        return false;
    }
    
    // 复制JSON到输出缓冲区
    uint16_t read_idx = json_start;
    json_length = 0;
    
    while (read_idx != esp8266_rx_buffer.head && json_length < buffer_size - 1) {
        buffer[json_length++] = esp8266_rx_buffer.buffer[read_idx];
        read_idx = (read_idx + 1) % UART_BUFFER_SIZE;
    }
    
    // 添加结束�?
    buffer[json_length] = '\0';
    
    // 重置message_ready标志
    esp8266_rx_buffer.message_ready = false;
    
    // 更新tail指针，释放已处理数据
    esp8266_rx_buffer.tail = (esp8266_rx_buffer.head + 1) % UART_BUFFER_SIZE;
    
    return true;
}

/**
 * @description: 高效处理WiFi JSON数据
 * @param json_str: JSON数据字符�?
 * @return: 解析是否成功
 */
bool wifi_cl(char *json_str)
{
    // 1. �?查是否为包含有效控制命令的消�?
    if (!strstr(json_str, "params")) {
        esp8266_rx_buffer.head = 0;
        esp8266_rx_buffer.tail = 0;
        esp8266_rx_buffer.overflow = 0;
        esp8266_rx_buffer.message_ready = false;// 清空缓冲区状态（为新的状态做准备�?
        return false; // 不含params，可能是返回码消息，不处�?
    }
    

    bool value_changed = false;
    
    // 2. �?次扫描提取所有设备状�?
    
    // 查找水泵状�?? (IrrigationPumpStatus)
    char *pos = strstr(json_str, "\"IrrigationPumpStatus\":\"");
    if (pos != NULL) {
        pos += 24; // 跳过字段名和引号
        uint8_t new_value = (*pos == '1') ? 1 : 0;
        if (sb_num_yy != new_value) {
            sb_num_yy = new_value;
            value_changed = true;
        }
    }
    
    // 查找补光灯状�? (LightStatus)
    pos = strstr(json_str, "\"LightStatus\":\"");
    if (pos != NULL) {
        pos += 15; // 跳过字段名和引号
        uint8_t new_value = (*pos == '1') ? 1 : 0;
        if (bg_num_yy != new_value) {
            bg_num_yy = new_value;
            value_changed = true;
        }
    }
    
    // 查找风扇状�?? (FanSwitch)
    pos = strstr(json_str, "\"FanSwitch\":\"");
    if (pos != NULL) {
        pos += 13; // 跳过字段名和引号
        uint8_t new_value = (*pos == '1') ? 1 : 0;
        if (fs_num_yy != new_value) {
            fs_num_yy = new_value;
            value_changed = true;
        }
    }
    
    // 3. 只在值发生变化时执行后续操作
    if (value_changed) {
        yy_num2 = 1; // 远端控制优先级调�?
        
        // 将新状�?�写入消息队�?
        osMessageQueuePut(ESP8266_bgHandle, &bg_num_yy, 0, 0);
        osMessageQueuePut(ESP8266_sbHandle, &sb_num_yy, 0, 0);
        osMessageQueuePut(ESP8266_fsHandle, &fs_num_yy, 0, 0);
        
        esp8266_rx_buffer.head = 0;
        esp8266_rx_buffer.tail = 0;
        esp8266_rx_buffer.overflow = 0;
        esp8266_rx_buffer.message_ready = false;// 清空缓冲区状态（为新的状态做准备�?

        // 同步到云�?
        osMutexAcquire(esp8266_x_01Handle, osWaitForever);
        ESP8266_fs_Data2(sb_num_yy, bg_num_yy, fs_num_yy);
        osMutexRelease(esp8266_x_01Handle);
    }
    
    return value_changed;
}




/**
 * @description: 发�?�串口消�??(核心模块)
 * @param num:定时器计数器标志位（也是植物状�?�标志位�??
 * @param num2:植物状�?�的zhuangtai�??
 */
void ck_fs(uint8_t num)
{
    static uint8_t zt_fs2=0XFF;//植物状�?�码
    static uint32_t time_ST=0;//�?始定时器
    static bool esp_k=true;//ESP8266连接标志�?

    if(HAL_GetTick() - time_ST > 5000 && esp_k){//超过5秒未发�?�数�?
      osMutexAcquire(esp8266_x_01Handle, osWaitForever); // 上锁，确保串口发送互�?
      ESP8266_fs_Data(wd_2,(int)tr_sd_2,sd_2,CO2_2,(int)gq_2);
      osMutexRelease(esp8266_x_01Handle);// 解锁
      esp_k=false;//关闭esp8266发�??
    }
    if(HAL_GetTick() - time_ST > 10000) // 超过10秒发送一次数�?
    {
      tw_cl2();
      esp_k=true;//允许esp8266发�??
      time_ST = HAL_GetTick();//重新计时
    }
		

		if(num!=zt_fs2)
		{
			    zt_fs2=num;//判断命令有否改变
		      tw_fs[0]=0x00;
			    tw_fs[1]=0x00;
			    tw_fs[2]=0x00;
					tw_fs[3]=0x00;
					tw_fs[4]=0x00;
					tw_fs[5]=0x00;
					tw_fs[6]=0x00;
					tw_fs[7]=0x00;
				  tw_fs[8]=0xFA;
					tw_fs[9]=num;//植物状�?�码
					HAL_UART_Transmit_IT(&huart3, tw_fs, 10);	  
		}
}



/**
 * @description: 天问模块字符串接受处�??
 */
void tw_cl(char*tw)
{
   static uint8_t tw_bz=0;//天问模块标志�??
   if((uint8_t)tw[0]==0xFF && (uint8_t)tw[2]==0x00)
	 {
		 if((uint8_t)tw[1]<0x07)//指令集判�??
	   {
		   tw_bz=(uint8_t)tw[1];
       osMessageQueuePut(tw_keyHandle, &tw_bz, 0, 0);//放入消息队列----用于天问模块控制
		 }
		 else
		 {
			 yy_num=(uint8_t)tw[1]==0x07?1:0;
		 }
	 }
	 else
	 {
	   tw_bz=0;
     osMessageQueuePut(tw_keyHandle, &tw_bz, 0, 0);//放入消息队列----用于天问模块控制
	 }
	  memset(tw_num, 0, sizeof(tw_num));
}
/**
 * @description: 天问模块字符串发送处�??
 */
void tw_cl2(void)
{
	  tw_fs[0]=0xFE;
	  tw_fs[1]=((uint16_t)wd_2 >> 8) & 0xFF;
	  tw_fs[2]=0xFD;
	  tw_fs[3]=((uint16_t)sd_2 >> 8) & 0xFF;
	  tw_fs[4]=0xFC;
	  tw_fs[5]=(int)gq_2 & 0xFF;
	  tw_fs[6]=0xFB;
	  tw_fs[7]=(int)tr_sd_2 & 0xFF;
	  tw_fs[8]=0x00;
	  tw_fs[9]=0x00;
    HAL_UART_Transmit_IT(&huart3, tw_fs, 10);
}
/**
 * @description: 天问模块控制函数(核心模块)
 */
void tw_kz(uint8_t* tw)
{
   switch(*tw)
	 {
	   case 1:
		 {
		   bg_num_yy=1;
			 if(sb_num_yy!=sb_num_yy2 || bg_num_yy!=bg_num_yy2 || fs_num_yy!=fs_num_yy2)//判断标志位是否改�??
			{
			 yy_num2=1;//优先级更�??
       osMessageQueuePut(tw_bgHandle, &bg_num_yy, 0, 0);//放入消息队列----用于继电器控制（天问模块控制�??

       osMutexAcquire(esp8266_x_01Handle, osWaitForever); // 上锁，确保串口发送互�?
			 ESP8266_fs_Data2(sb_num_yy,bg_num_yy,fs_num_yy);
       osMutexRelease(esp8266_x_01Handle);// 解锁
			 *tw=0;
			}
			 break;
		 }
		 case 2:
		 {
		   bg_num_yy=0;
			 if(sb_num_yy!=sb_num_yy2 || bg_num_yy!=bg_num_yy2 || fs_num_yy!=fs_num_yy2)//判断标志位是否改�??
			{
			 yy_num2=1;//优先级更�??
       osMessageQueuePut(tw_bgHandle, &bg_num_yy, 0, 0);//放入消息队列----用于继电器控制（天问模块控制�??

       osMutexAcquire(esp8266_x_01Handle, osWaitForever); // 上锁，确保串口发送互�?
			 ESP8266_fs_Data2(sb_num_yy,bg_num_yy,fs_num_yy);
       osMutexRelease(esp8266_x_01Handle);// 解锁

			 *tw=0;
			}
			 break;
		 }
		 case 3:
		 { 
		   fs_num_yy=1;
			 if(sb_num_yy!=sb_num_yy2 || bg_num_yy!=bg_num_yy2 || fs_num_yy!=fs_num_yy2)//判断标志位是否改�??
			{
			 yy_num2=1;//优先级更�??
       osMessageQueuePut(tw_fsHandle, &fs_num_yy, 0, 0);//放入消息队列----用于继电器控制（天问模块控制�??

       osMutexAcquire(esp8266_x_01Handle, osWaitForever); // 上锁，确保串口发送互�?
			 ESP8266_fs_Data2(sb_num_yy,bg_num_yy,fs_num_yy);
        osMutexRelease(esp8266_x_01Handle);// 解锁

			 *tw=0;
			}
			 break;
		 }
		 case 4:
		 {
		   fs_num_yy=0;
			  if(sb_num_yy!=sb_num_yy2 || bg_num_yy!=bg_num_yy2 || fs_num_yy!=fs_num_yy2)//判断标志位是否改�??
			{
			 yy_num2=1;//优先级更�??
       osMessageQueuePut(tw_fsHandle, &fs_num_yy, 0, 0);//放入消息队列----用于继电器控制（天问模块控制�??

       osMutexAcquire(esp8266_x_01Handle, osWaitForever); // 上锁，确保串口发送互�?
			 ESP8266_fs_Data2(sb_num_yy,bg_num_yy,fs_num_yy);
       osMutexRelease(esp8266_x_01Handle);// 解锁

			 *tw=0;
			}
			 break;
		 }
		 case 5:
		 {
		   sb_num_yy=1;
			 if(sb_num_yy!=sb_num_yy2 || bg_num_yy!=bg_num_yy2 || fs_num_yy!=fs_num_yy2)//判断标志位是否改�??
			{
			yy_num2=1;//优先级更�??
       osMessageQueuePut(tw_sbHandle, &sb_num_yy, 0, 0);//放入消息队列----用于继电器控制（天问模块控制�??
       osMutexAcquire(esp8266_x_01Handle, osWaitForever); // 上锁，确保串口发送互�?
			 ESP8266_fs_Data2(sb_num_yy,bg_num_yy,fs_num_yy);
       osMutexRelease(esp8266_x_01Handle);// 解锁
			 *tw=0;
			}
			 break;
		 }
		 case 6:
		 {
		   sb_num_yy=0;
			 if(sb_num_yy!=sb_num_yy2 || bg_num_yy!=bg_num_yy2 || fs_num_yy!=fs_num_yy2)//判断标志位是否改�??
			{
			 yy_num2=1;//优先级更�??
       osMessageQueuePut(tw_sbHandle, &sb_num_yy, 0, 0);//放入消息队列----用于继电器控制（天问模块控制�??
       osMutexAcquire(esp8266_x_01Handle, osWaitForever); // 上锁，确保串口发送互�?
			 ESP8266_fs_Data2(sb_num_yy,bg_num_yy,fs_num_yy);
       osMutexRelease(esp8266_x_01Handle);// 解锁
			 *tw=0;
			}
			 break;
		 }
	 }
}


//----------------------------------------------------------------------------------------------------------------↑函数区�??------------------------------------------------------------------------------------------------------------------

/* USER CODE END Application */

