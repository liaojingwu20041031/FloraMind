# 引脚映射详解

## GPIO 输出引脚 (执行机构)

| 功能 | 引脚 | 端口 | 定义宏 | 有效电平 | 说明 |
|------|------|------|--------|----------|------|
| 补光灯继电器 | PD0 | GPIOD | `LED_GPIO_Pin` / `LED_GPIO_GPIO_Port` | 低电平有效 | 5V LED 补光灯 |
| 水泵继电器 | PD1 | GPIOD | `csj_GPIO_Pin` / `csj_GPIO_GPIO_Port` | 低电平有效 | 5V 直流电机微型水泵 |
| 风扇继电器 | PD3 | GPIOD | `fs_GPIO_Pin` / `fs_GPIO_GPIO_Port` | 低电平有效 | 5V 轴流风机 |

## GPIO 输入引脚 (传感器)

| 功能 | 引脚 | 端口 | 定义宏 | 说明 |
|------|------|------|--------|------|
| PIR 人体感应 | PE0 | GPIOE | `gymk_GPIO_Pin` / `gymk_GPIO_GPIO_Port` | 高电平检测到人体 |
| 振动传感器 | PE1 | GPIOE | `zdmk_GPIO_Pin` / `zdmk_GPIO_GPIO_Port` | 高电平检测到振动 |
| DHT11 温湿度 | PA1 | GPIOA | `DHT11_Pin` / `DHT11_GPIO_Port` | 单总线协议, 40-bit 数据 |

## ADC 模拟采样引脚

| 功能 | 引脚 | 端口 | ADC 通道 | 定义宏 | 采样方式 |
|------|------|------|----------|--------|----------|
| 土壤湿度 | PA2 | GPIOA | ADC1_CH2 | `tr_sd_adc_IN2_Pin` / `tr_sd_adc_IN2_GPIO_Port` | DMA, 10 次采样取中间 8 次平均 |
| 光照强度 | PA3 | GPIOA | ADC1_CH3 | `gq_adc_IN3_Pin` / `gq_adc_IN3_GPIO_Port` | DMA, 10 次采样取中间 8 次平均 |

## I2C 引脚

| 功能 | SCL 引脚 | SDA 引脚 | 类型 | 定义宏 | 说明 |
|------|----------|----------|------|--------|------|
| OLED SSD1306 | PB6 | PB7 | 硬件 I2C1 | - | 0.96寸 128x64 显示屏 |
| SGP30 CO2/TVOC | PB10 | PB11 | 软件 I2C | `SGP30_SCL_Pin` / `SGP30_SDA_Pin` | PB10=SCL, PB11=SDA |

## UART 引脚

| 功能 | TX 引脚 | RX 引脚 | 外设 | 波特率 | 定义宏 | 说明 |
|------|---------|---------|------|--------|--------|------|
| TJC 串口屏 | PA9 | PA10 | USART1 | 9600 | `xsp_TX_Pin` / `xsp_RX_Pin` | 4.3寸 HMI 触摸屏 |
| ESP8266 WiFi | PD5 | PD6 | USART2 (Remap) | 115200 | `wifi_TX_Pin` / `wifi_RX_Pin` | AT 指令集, MQTT |
| 天问语音 | PD8 | PD9 | USART3 (Remap) | 9600 | `tw_TX_Pin` / `tw_RX_Pin` | ASRPRO 离线语音 |
| Feetech 舵机 | PC12 | PD2 | UART5 | 9600 | `ly_TX_Pin` / `ly_RX_Pin` | 串口总线舵机 |

## 引脚电气特性

| 参数 | 值 |
|------|------|
| MCU 供电电压 | 3.3V |
| GPIO 输出电流 | 最大 25mA |
| ADC 参考电压 | 3.3V |
| ADC 分辨率 | 12-bit (0-4095) |
| 继电器驱动 | 通过三极管/MOS管驱动，GPIO 低电平导通 |

## 注意事项

1. **USART2/USART3 使用引脚重映射** -- 默认引脚被其他外设占用，需在 CubeMX 中配置 Remap
2. **SGP30 使用软件 I2C** -- 因硬件 I2C2 已被 OLED 占用，SGP30 通过 GPIO 模拟 I2C 时序
3. **DHT11 时序要求严格** -- 单总线协议需要精确的微秒级延时，使用 HAL_Delay 和 SysTick 实现
4. **ADC 使用 DMA 传输** -- 两个通道各采集 10 个样本，去除最大最小值后取平均
