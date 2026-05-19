/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

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
//#include <algorithm>
#define FRAME_LENGTH 7
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint8_t wifi_string[256];
uint8_t tw_fs[10];
uint8_t ly_js[4];//蓝牙模块接受缓冲区
char* wifi_string2 = NULL; 
uint8_t tw_num[4];
uint16_t wd;//温度
uint16_t sd;//湿度
uint16_t TVOC = 0, CO2 = 0;//TVOC和CO2浓度
uint8_t sb_num=0,bg_num=0,fs_num=0,sb_num2=0,bg_num2=0,fs_num2=0;//水泵，补光灯，风扇标志位
uint8_t sb_num_yy=0,bg_num_yy=0,fs_num_yy=0,sb_num_yy2=0,bg_num_yy2=0,fs_num_yy2=0,yy_num2=0;//远端、语音控制标志位
uint8_t dsj_tim3=0,dsq_fs=0;//计时标准位（1秒单位）
uint8_t zt_fs=0XFF,zt_fs2=0XFF;//植物状态码
uint8_t ly_num=0x00;//判断蓝牙信号标志位0x00:正常无事，后面再定。。。。）
uint8_t tw_bz=0;
KalmanFilter wd_kal,sd_kal,tr_sd_kal,CO2_kal,gq_kal;//卡尔曼滤波参数
float wd_2,sd_2,tr_sd_2,gq_2;//卡尔曼滤波后的值
uint16_t CO2_2;//卡尔曼滤波后的值
uint8_t CO2_NO=0;//预热完成标准位
uint64_t time=0;//用于程序循环计数器
uint8_t yy_num=0; //AI语音聊天模式标志位
#define MAX_REBOOTS 1
#define BKP_DR1     0x00  // 使用DR1作为计数器存储位置
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
void extract_switch_value(const char *json_str, const char *field,uint8_t*num);
void tw_cl(char *tw);//天问串口处理
void ly_cl(uint8_t *ly);//蓝牙串口处理
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)//wifi发送
    {
			
        // 发送完成，可以在这里处理后续逻辑
    }
		if(huart->Instance == USART1)//显示屏发送
		{
		
		}
		if(huart->Instance == USART3)//天问语言模块
		{
		
		}
}

// 接收完成回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
			
      HAL_UART_Receive_IT(&huart2, wifi_string, 256);
    }
		if(huart->Instance == USART3)//接受天问模块的信息
		{ 
				
	    tw_cl((char*)tw_num);//信息处理
			HAL_UART_Receive_IT(&huart3, tw_num, 3);//初始化接受
		}
		if(huart->Instance == UART5)//接受蓝牙模块的信息
		{
			
      ly_cl(ly_js);//信息处理
      HAL_UART_Receive_IT(&huart5, ly_js, 4);
		}
}



void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)//定时发送功能实现
{
	if(htim == &htim3)
	{
		dsj_tim3++;
		if(dsj_tim3>50)
		{
		 dsq_fs=1;
		 dsj_tim3=0;	 
		}
		else if(dsj_tim3==30)
		{
		 dsq_fs=2;
		}
	}
}

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
bool gy_pos(void);//人体感应模块
void check_and_reboot(void);//重启函数
void cgq_sz(void);//传感器读数值
void OLED_UI(void);//UI界面函数
void wifi_cl(char*json_str);
void ck_fs(uint8_t num,uint8_t num2,uint8_t ly_num3);//发送串口数据
bool zd_pos(void);//震动感应模块
void tw_kz(uint8_t tw);//天问控制函数
void tw_cl2(void);//天问模块字符串发送处理
void zw_zt_cl(void);//植物状态码（检测环境数据）
void Kal_Init(void);//卡尔曼滤波初始化
void Kal_pos(void);//卡尔曼滤波
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_ADC1_Init();
  MX_I2C2_Init();
  MX_USART3_UART_Init();
  MX_TIM3_Init();
  MX_RTC_Init();
  MX_UART5_Init();
  /* USER CODE BEGIN 2 */
   HAL_TIM_Base_Start_IT(&htim3);//打开定时器3中断
	 
	 
   OLED_Init();
	 OLED_Clear();
	 DHT11_Init();//初始化温湿度传感器
	 
	 HAL_ADCEx_Calibration_Start(&hadc1 );//adc校准
   HAL_ADC_Start_DMA(&hadc1, (uint32_t *)My_adcData, adc_max);//光敏和土壤湿度传感器
	 
	 sgp30_init();//sgp30初始化
	 

	 HAL_UART_Init(&huart2);  
	 HAL_UART_Receive_IT(&huart2, wifi_string, 256);//初始化接受
	 
	 HAL_UART_Init(&huart3);  //天问模块
	 HAL_UART_Receive_IT(&huart3, tw_num, 3);//初始化接受

	 HAL_UART_Init(&huart5);  //蓝牙模块
	 HAL_UART_Receive_IT(&huart5, ly_js, 4);//初始化接受
   
	 HAL_UART_Init(&huart1);  
	
	 ESP8266_Init();//热点连接
	  	
	 check_and_reboot();//重启函数(稳定连接服务器（2次重新连接）)

	 Kal_Init();//初始化卡尔曼滤波器(所有传感器)
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//		time++;
	  cgq_sz();//传感器读取函数(处理分析)	
		
		ck_fs(dsq_fs,zt_fs,ly_num);//发送串口消息（ESP8266和天问模块）
		
		tw_kz(tw_bz);//天问模块控制
		
    wifi_cl((char*)wifi_string);//物联网WiFi数据处理
		
		jdq_pos_wifi(sb_num,bg_num,fs_num);//继电器控制开关
		
		OLED_UI();//OLED屏显
		

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
//--------------------------------------------------------------------------------------------------------------------------------------------↓函数区↓-------------------------------------------------------------------------------------------------------------------------------
/**
 * @description:读取传感器数值函数（读取、滤波、调节感应）
 */
void cgq_sz(void)
{
  	DHT11_Read_Data(&wd,&sd);//温湿度检测
		ADC_dispose();//adc检测
		sgp30_read(&CO2, &TVOC);//TVOC和CO2浓度检测
	  Kal_pos();//卡尔曼滤波
	  zw_zt_cl();//状态感应、处理
}



/**
 * @description:卡尔曼滤波初始化函数(所有传感器)
 */
void Kal_Init(void)
{
  KalmanFilter_Init(&wd_kal,0.1,1.3,(float)(wd >> 8) + (float)(wd & 0xFF) / 256.0f,1);
	KalmanFilter_Init(&sd_kal,0.1,1.3,(float)(sd >> 8) + (float)(sd & 0xFF) / 256.0f,1);
  KalmanFilter_Init(&tr_sd_kal,0.1,1.3,tr_sd,1);
	KalmanFilter_Init(&gq_kal,0.1,4,gq,1);
	
	 // CO?传感器专用参数（动态噪声版本）
  KalmanFilter_Init(&CO2_kal, 
                    10.0f,    // 初始q较大以适应上升阶段
                    2500.0f,  // r=502（基础噪声）
                    (CO2 < 400) ? 400.0f : (float)CO2, // 强制不低于400ppm
                    10000.0f); // 初始高不确定度
}
/**
 * @description:卡尔曼滤波函数(所有传感器)
 */
void Kal_pos(void)
{
  wd_2=KalmanFilter_Update(&wd_kal,(float)(wd >> 8) + (float)(wd & 0xFF) / 256.0f);
	sd_2=KalmanFilter_Update(&sd_kal,(float)(sd >> 8) + (float)(sd & 0xFF) / 256.0f);
	tr_sd_2=KalmanFilter_Update(&tr_sd_kal,tr_sd);
	gq_2=KalmanFilter_Update(&gq_kal,gq);

	if(CO2>480)//等待预热完成
  {
		CO2_NO=1;
		/* CO?特殊处理 */
    // 1. 动态调整测量噪声（误差±5%读数）
    CO2_kal.r = 2500.0f + 0.0025f * CO2_kal.x * CO2_kal.x;
    
    // 2. 强制不低于400ppm
    float raw_CO2 = (CO2 < 400) ? 400.0f : (float)CO2;
    
    // 3. 动态过程噪声（根据变化率调整）
    static float last_CO2 = 400.0f;
    float change_rate = (float)fabs((double)(CO2_kal.x - last_CO2)) / 1.0f; // 假设1秒采样间隔
    CO2_kal.q = 5.0f + change_rate * 2.0f;
    last_CO2 = CO2_kal.x;
    
    // 4. 执行滤波
    CO2_2 = KalmanFilter_Update(&CO2_kal, raw_CO2);
	}
	else if(CO2_NO==0)
	{
	  CO2_2=CO2;
	}
  

}

/**
 * @description: 植物状态控制核心函数
 * @note: 综合环境数据与互动状态，实现智能设备控制与拟人表情管理
 * 算法特性：
 * 1. 自适应阈值调整（基于历史数据）
 * 2. 预测性控制（土壤湿度趋势预测）
 * 3. 多因素协同决策（温度-CO2综合评分）
 * 4. 状态持续时间加权
 * 5. 表情平滑过渡管理
 */
void zw_zt_cl(void)
{
    /*----------------------- 环境参数结构体 -----------------------*/
    // 使用结构体封装所有阈值参数和状态计时器，增强代码可维护性
    static struct {
        float temp_high;        // 动态高温阈值（基于24小时历史数据）
        float temp_low;         // 动态低温阈值
        float soil_dry;         // 土壤干燥阈值（%）
        float soil_wet;         // 土壤湿润阈值（%）
        float light_low;        // 光照不足阈值（%）
        float co2_high;         // CO2超标阈值（ppm）
        uint32_t state_timer[6]; // 状态持续时间计数器[高温,干燥,CO2,低温,低光,正常]
    } env_threshold = {
        .temp_high = 32.0f,     // 初始默认值(可修改)
        .temp_low = 20.0f,
        .soil_dry = 25.0f,
        .soil_wet = 65.0f,
        .light_low = 55.0f,
        .co2_high = 1000.0f
    };

    /*----------------------- 状态跟踪变量 -----------------------*/
    static uint8_t last_expression = 0; // 上一次发送的表情ID（0表示初始状态）
    uint8_t current_expression = 0;        // 当前计算的表情ID（默认待机表情）
    static uint32_t last_update = 0;       // 上次阈值更新时间戳（ms）

    /*----------------------- 自适应阈值系统 -----------------------*/
    // 温度历史数据窗口：30个周期（假设每隔3秒更新一次）
    static float temp_history[30] = {0};   // 环形缓冲区存储温度历史
    static uint8_t history_index = 0;      // 当前写入位置索引

    // 每3秒更新一次温度历史并重新计算阈值
    if(HAL_GetTick() - last_update > 3000) { // 3000ms = 3秒
			// 更新环形缓冲区
        temp_history[history_index] = wd_2;
        history_index = (history_index + 1) % 24;
        
        // 计算目前位置索引+1(不超过30个周期)的平均温度
        float temp_sum = 0;
			  uint8_t n=(history_index+1>60)?60:history_index+1;
			  for(int i=0; i<n;i++) temp_sum += temp_history[i];
        float temp_avg = temp_sum /n;
        
        // 动态调整阈值：平均值±3℃
        env_threshold.temp_high = temp_avg + 3.0f;
        env_threshold.temp_low = temp_avg - 3.0f;
        
        last_update = HAL_GetTick(); // 更新时间戳
    }

    /*====================== 智能设备控制算法 ======================*/
    // 补光灯控制策略：动态迟滞光照控制
    static float light_last = 0;           // 上一次光照值
    float light_change_rate = fabs(gq_2 - light_last); // 光照变化率
    float light_hysteresis = light_change_rate > 5 ? 10:5; // 动态迟滞范围
		
    // 控制逻辑：光照低于阈值-迟滞时开启，高于阈值+迟滞时关闭
    if(gq_2 < (env_threshold.light_low - light_hysteresis)) 
		{
      bg_num = 1; // 开启补光灯
    } else if(gq_2 > (env_threshold.light_low + light_hysteresis)) 
		{
      bg_num = 0; // 关闭补光灯
    }
    light_last = gq_2; // 保存当前光照值

		
    // 抽水机控制策略：带趋势预测的土壤湿度控制
    static float soil_history[3] = {0};    // 最近3次土壤湿度记录
    soil_history[2] = soil_history[1];     // 滚动更新历史数据
    soil_history[1] = soil_history[0];
    soil_history[0] = tr_sd_2;
    
    // 线性外推预测公式：X(n+1) = 3X(n) - 3X(n-1) + X(n-2)
    float soil_pred = 3*soil_history[0] - 3*soil_history[1] + soil_history[2];
    // 动态迟滞：预测干燥时扩大迟滞范围（防止频繁开关）
    float soil_hysteresis = (soil_pred < env_threshold.soil_dry) ? 8.0f : 3.0f;
    
    if(tr_sd_2 < (env_threshold.soil_dry - soil_hysteresis)) {
        sb_num = 1; // 开启抽水机
        env_threshold.state_timer[1]++; // 干燥状态持续时间+1
    } else if(tr_sd_2 > (env_threshold.soil_dry + soil_hysteresis)) {
        sb_num = 0; // 关闭抽水机
        env_threshold.state_timer[1] = 0; // 重置干燥计时器
    }

    // 风扇控制策略：温度-CO2综合评分系统
    // 评分公式：0.6*（当前温度/高温阈值） + 0.4*（当前CO2/CO2阈值）
    float temp_co2_score = 0.6f*(wd_2/env_threshold.temp_high) 
                         + 0.4f*(CO2_2/env_threshold.co2_high);
		
    if(temp_co2_score > 1.0f) { // 综合评分超过100%
        fs_num = 1; // 开启风扇
        // 更新状态持续时间计数器
        env_threshold.state_timer[0]++; // 高温计时+1
        env_threshold.state_timer[2]++; // CO2计时+1
    } else {
        fs_num = 0; // 关闭风扇
        env_threshold.state_timer[0] = 0; // 重置高温计时器
        env_threshold.state_timer[2] = 0; // 重置CO2计时器
    }

    /*====================== 智能表情决策系统 ======================*/
    // 状态权重计算（考虑持续时间和严重程度）
    float state_weights[6] = {0}; // ------------------------------------------各状态权重值[高温,干燥,CO2,低温,低光,正常] ------------------------------------------ //
    
    // 高温权重 = 是否超标 * (1 + 0.0001*持续时间)
    state_weights[0] = (wd_2 > env_threshold.temp_high) 
                     * (1 + 0.0001f*env_threshold.state_timer[0]);
    
    // 干燥权重 = 是否干燥 * (1 + 0.0005*持续时间)
    state_weights[1] = (tr_sd_2 < env_threshold.soil_dry)
                     * (1 + 0.0005f*env_threshold.state_timer[1]);
    
    // CO2权重 = 是否超标 * (1 + 0.0008*持续时间)
    state_weights[2] = (CO2_2 > env_threshold.co2_high)
                     * (1 + 0.0008f*env_threshold.state_timer[2]);
    
    // 低温权重（简单布尔值）
    state_weights[3] = (wd_2 < env_threshold.temp_low)+0.01;
    
    // 低光权重（简单布尔值）
    state_weights[4] = (gq_2 < env_threshold.light_low)+0.01;
    
    // 正常状态基础权重
    state_weights[5] = 1.0f;


    // 确定最高优先级状态
    uint8_t max_index = 0; // 默认正常待机状态
    for(int i=0; i<=5; i++) {
        if(state_weights[i] >= state_weights[max_index]) {
            max_index = i;
        }
    }
    /*----------------------- 表情映射逻辑 -----------------------*/
    switch(max_index) {
        case 0: // 高温状态
            // 持续时间>10单位：生气，否则眩晕
            current_expression = (env_threshold.state_timer[0] > 30) ? 8 : 12;
            break;
            
        case 1: // 干燥状态
            // 持续时间>5单位：哭泣，否则眩晕
            current_expression = (env_threshold.state_timer[1] > 30) ? 6 : 12;
            break;
            
        case 2: // CO2超标
            // CO2>1500ppm：眩晕，否则难过
            current_expression = (CO2_2 > 1500) ? 12 : 7;
            break;
            
        case 3: // 低温状态
            current_expression = 4; // 害怕
            break;
            
        case 4: // 低光状态
            current_expression = 11; // 休眠
            break;
            
        default: // 正常状态
            if(gy_pos() || current_expression!=0) { // 检测到人体
                // 互动超时处理（5秒切换表情）
                static uint32_t interact_timer = 0;
                if(HAL_GetTick() - interact_timer > 5000) {
									  xsp_fs("sleep", 0); // 保证屏幕为非睡眠状态
                    current_expression = (current_expression == 0) ? 2:0; // 切换得意
                    interact_timer = HAL_GetTick();
                }
            }
    }

    /*----------------------- 表情输出管理和RGB灯控制 -----------------------*/
    if(current_expression != last_expression) {
        // 最小间隔保护（3秒防抖动）
        static uint32_t last_change = 0;
        if(HAL_GetTick() - last_change > 3000) {
					 if(yy_num==1)//语音模式下强制为识别语音的状态
					 {
					  xsp_fs("num", 9); // 发送表情指令
						current_expression=0;//默认语音下为白色灯效
					 }
					 else
					 {
            xsp_fs("num", current_expression); // 发送表情指令
					 }
					 
					 zt_fs=current_expression;//植物状态RGB灯映射
				
  						
					
            last_expression = current_expression;
            last_change = HAL_GetTick(); // 记录最后变更时间
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
		OLED_ShowNum(0,4,CO2,5,16,0);		
		//OLED_ShowNum(0,6,tw_num[0],3,16,0);
		//OLED_ShowNum(30,6,tw_num[1],3,16,0);
//	  OLED_ShowNum(0,6,sb_num,3,16,0);
//		OLED_ShowNum(30,6,fs_num,3,16,0);
//	  OLED_ShowNum(60,6,bg_num,3,16,0);
	  OLED_ShowNum(0,6,HAL_GetTick(),8,16,0);
	  OLED_ShowNum(64,6,time,8,16,0);
	
	
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
 * @description: 人体感应模块函数（返还 1有人，0无人）
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
 * @description: 震动感应模块函数（返还 1有人，0无人）
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
 * @description: 重启函数
 */
void check_and_reboot(void)
{
    // 1. 启用PWR和BKP时钟
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_BKP_CLK_ENABLE();
    
    // 2. 允许访问备份域
    HAL_PWR_EnableBkUpAccess();
    
    // 3. 直接操作备份寄存器（不需要RTC句柄）
    uint32_t reboot_count = BKP->DR1;
    
    if(reboot_count < MAX_REBOOTS)
    {
        // 4. 增加计数并写入
        BKP->DR1 = reboot_count + 1;
        
        // 5. 确保写入完成
        HAL_Delay(10);
        
        // 6. 执行重启
        HAL_NVIC_SystemReset();
    }
    else
    {
        // 7. 重置计数器
        BKP->DR1 = 0;
    }
}

/**
 * @description: 直接从字符串中提取字段的整数值（1或0）
 */
void extract_switch_value(const char *json_str, const char *field,uint8_t*num) {
    // 构造搜索模式（如 "FanSwitch":"）
    char pattern[32];
    snprintf(pattern, sizeof(pattern), "\"%s\":\"", field);

    // 查找字段位置
    const char *pos = strstr(json_str, pattern);
    if (!pos) // 字段不存在
		{
		  return;
		} 
    else
		{
			// 跳转到值的位置（跳过 "FanSwitch":"）
			pos += strlen(pattern);
			// 直接返回字符 '0' 或 '1' 对应的整数值
			*num = ((*pos == '1') ? 1 : 0);
		}
}
/**
 * @description: wifi字符串接受处理
 */
void wifi_cl(char*json_str)
{
  extract_switch_value(json_str, "IrrigationPumpStatus",&sb_num_yy);
  extract_switch_value(json_str, "LightStatus",&bg_num_yy);
  extract_switch_value(json_str, "FanSwitch",&fs_num_yy);
	if(sb_num_yy!=sb_num_yy2 || bg_num_yy!=bg_num_yy2 || fs_num_yy!=fs_num_yy2)//判断标志位是否改变
	{
	 yy_num2=1;//远端、语音优先级调高
	 memset(wifi_string, 0, sizeof(wifi_string));
	 ESP8266_fs_Data2(sb_num_yy,bg_num_yy,fs_num);
	}
	
}
/**
 * @description: 蓝牙模块字符串接受处理
 */
void ly_cl(uint8_t*ly)
{
    // 检查帧头和帧尾是否符合要求
    if ((uint8_t)ly[0] == 0xFF && (uint8_t)ly[3] == 0x00)
    {
         switch ((uint8_t)ly[1])
         {
         case 0x00:
          { 
            if((uint8_t)ly[2]==0x00)
            {
              ly_num=1;//殴打
            }
            break;
          }
         case 0x01:
          {
            if((uint8_t)ly[2]==0x00)
            {
              ly_num=2;//纠正坐姿
            }
            break;
          }
         case 0x02:
          {
            if((uint8_t)ly[2]==0x00)
            {
              ly_num=3;//检测是否认真写作业
            }
            break;
          }
         case 0x03:
          {
            if((uint8_t)ly[2]==0x00)
            {
              ly_num=4;//休息
            }
            break;
          }
         case 0x04:
          {
            if((uint8_t)ly[2]==0x00)//手势识别
            {
              ly_num=5;//
            }
          }
         }
    }
}


/**
 * @description: 发送串口消息(核心模块)
 * @param num:定时器计数器标志位（也是植物状态标志位）
 * @param num2:植物状态的zhuangtai码
 * @param ly_num3:蓝牙模块状态码
 */
void ck_fs(uint8_t num,uint8_t num2,uint8_t ly_num3)
{
    if(num==1)//向阿里云发送检测数据
		{
			ESP8266_fs_Data((float)(wd >> 8) + (float)(wd & 0xFF) / 256.0f,tr_sd,(float)(sd >> 8) + (float)(sd & 0xFF) / 256.0f,CO2,gq);
		  dsq_fs=0;
		}
		else if(num==2)//向天问模块发送检测模块数据
		{
		  tw_cl2();
			dsq_fs=0;
		}

		if(num2!=zt_fs2)
		{
			    zt_fs2=num2;//判断命令有否改变
			
		      tw_fs[0]=0x00;
			    tw_fs[1]=0x00;
			    tw_fs[2]=0x00;
					tw_fs[3]=0x00;
					tw_fs[4]=0x00;
					tw_fs[5]=0x00;
					tw_fs[6]=0x00;
					tw_fs[7]=0x00;
				  tw_fs[8]=0xFA;
					tw_fs[9]=num2;
					HAL_UART_Transmit_IT(&huart3, tw_fs, 10);	  
		}
    else if(ly_num3!=0)
    {
        ly_num3=0;// 重置状态

        tw_fs[0]=0x00;
        tw_fs[1]=0x00;
        tw_fs[2]=0x00;
        tw_fs[3]=0x00;
        tw_fs[4]=0x00;
        tw_fs[5]=0x00;
        tw_fs[6]=0x00;
        tw_fs[7]=0x00;
        tw_fs[8]=0xF9;
        tw_fs[9]=ly_num3;
        HAL_UART_Transmit_IT(&huart3, tw_fs, 10);//发送蓝牙模块状态
    }
}

/**
 * @description: 天问模块字符串接受处理
 */
void tw_cl(char*tw)
{
   if((uint8_t)tw[0]==0xFF && (uint8_t)tw[2]==0x00)
	 {
		 if((uint8_t)tw[1]<0x07)//指令集判断
	   {
		   tw_bz=(uint8_t)tw[1];
		 }
		 else
		 {
			 yy_num=(uint8_t)tw[1]==0x07?1:0;
		 }
	 }
	 else
	 {
	   tw_bz=0;
	 }
	  memset(tw_num, 0, sizeof(tw_num));
}
/**
 * @description: 天问模块字符串发送处理
 */
void tw_cl2(void)
{
	  tw_fs[0]=0xFE;
	  tw_fs[1]=(wd >> 8) & 0xFF;
	  tw_fs[2]=0xFD;
	  tw_fs[3]=(sd >> 8) & 0xFF;
	  tw_fs[4]=0xFC;
	  tw_fs[5]=(int)gq & 0xFF;
	  tw_fs[6]=0xFB;
	  tw_fs[7]=(int)tr_sd & 0xFF;
	  tw_fs[8]=0x00;
	  tw_fs[9]=0x00;
    HAL_UART_Transmit_IT(&huart3, tw_fs, 10);
}
/**
 * @description: 天问模块控制函数(核心模块)
 */
void tw_kz(uint8_t tw)
{
   switch(tw)
	 {
	   case 1:
		 {
		   bg_num_yy=1;
			 if(sb_num_yy!=sb_num_yy2 || bg_num_yy!=bg_num_yy2 || fs_num_yy!=fs_num_yy2)//判断标志位是否改变
			{
			 yy_num2=1;//优先级更替
			 ESP8266_fs_Data2(sb_num_yy,bg_num_yy,fs_num_yy);
			 tw_bz=0;
			}
			 break;
		 }
		 case 2:
		 {
		   bg_num_yy=0;
			 if(sb_num_yy!=sb_num_yy2 || bg_num_yy!=bg_num_yy2 || fs_num_yy!=fs_num_yy2)//判断标志位是否改变
			{
			 yy_num2=1;//优先级更替
			 ESP8266_fs_Data2(sb_num_yy,bg_num_yy,fs_num_yy);
			 tw_bz=0;
			}
			 break;
		 }
		 case 3:
		 { 
		   fs_num_yy=1;
			 if(sb_num_yy!=sb_num_yy2 || bg_num_yy!=bg_num_yy2 || fs_num_yy!=fs_num_yy2)//判断标志位是否改变
			{
			yy_num2=1;//优先级更替
			 ESP8266_fs_Data2(sb_num_yy,bg_num_yy,fs_num_yy);
			 tw_bz=0;
			}
			 break;
		 }
		 case 4:
		 {
		   fs_num_yy=0;
			  if(sb_num_yy!=sb_num_yy2 || bg_num_yy!=bg_num_yy2 || fs_num_yy!=fs_num_yy2)//判断标志位是否改变
			{
			yy_num2=1;//优先级更替
			 ESP8266_fs_Data2(sb_num_yy,bg_num_yy,fs_num_yy);
			 tw_bz=0;
			}
			 break;
		 }
		 case 5:
		 {
		   sb_num_yy=1;
			 if(sb_num_yy!=sb_num_yy2 || bg_num_yy!=bg_num_yy2 || fs_num_yy!=fs_num_yy2)//判断标志位是否改变
			{
			yy_num2=1;//优先级更替
			 ESP8266_fs_Data2(sb_num_yy,bg_num_yy,fs_num_yy);
			 tw_bz=0;
			}
			 break;
		 }
		 case 6:
		 {
		   sb_num_yy=0;
			 if(sb_num_yy!=sb_num_yy2 || bg_num_yy!=bg_num_yy2 || fs_num_yy!=fs_num_yy2)//判断标志位是否改变
			{
			 yy_num2=1;//优先级更替
			 ESP8266_fs_Data2(sb_num_yy,bg_num_yy,fs_num_yy);
			 tw_bz=0;
			}
			 break;
		 }
	 }
}


//----------------------------------------------------------------------------------------------------------------↑函数区↑------------------------------------------------------------------------------------------------------------------
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
