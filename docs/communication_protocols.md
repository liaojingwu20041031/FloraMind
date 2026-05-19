# 通信协议详解

## 1. ESP8266 WiFi / MQTT 协议 (USART2, 115200 baud)

### 1.1 连接流程

```
AT+RST                                    -- 复位模块
AT+CWMODE=1                               -- 设置 Station 模式
AT+CWJAP="ssid","password"                -- 连接 WiFi 热点
AT+CIPMODE=1                              -- 设置透传模式
AT+CIPSNTPCFG=1,8,"ntp1.aliyun.com"      -- 配置 SNTP (阿里云 NTP)
AT+MQTTUSERCFG=0,1,"device","product","secret",0,0,""  -- MQTT 用户配置
AT+MQTTCLIENTID=0,"clientId"              -- MQTT 客户端 ID
AT+MQTTCONN=0,"broker",1883,1             -- 连接 MQTT Broker
AT+MQTTSUB=0,"topic",1                    -- 订阅控制主题
```

### 1.2 数据上报 (JSON)

**发布主题:** `/sys/{productKey}/{deviceName}/thing/event/property/post`

**传感器数据格式:**
```json
{
  "id": 9,
  "params": {
    "Temperature": 26,
    "SoilMoisture": 65,
    "EnvironmentHumidity": 80,
    "CO2Value": 560,
    "LightLux": 550
  },
  "version": "1.0",
  "method": "thing.event.property.post"
}
```

**继电器状态格式:**
```json
{
  "id": 10,
  "params": {
    "FanSwitch": 0,
    "IrrigationPumpStatus": 0,
    "LightStatus": 1
  },
  "version": "1.0",
  "method": "thing.event.property.post"
}
```

### 1.3 远程控制指令

**订阅主题:** `/sys/{productKey}/{deviceName}/thing/service/property/set`

**控制字段:**
| 字段 | 类型 | 说明 |
|------|------|------|
| `FanSwitch` | int | 0=关, 1=开 |
| `IrrigationPumpStatus` | int | 0=关, 1=开 |
| `LightStatus` | int | 0=关, 1=开 |

### 1.4 上传周期

- 传感器数据: 每 10 秒上传一次
- 继电器状态: 每 10 秒上传一次 (与传感器数据交替发送, 间隔 5 秒)

### 1.5 接收处理

- 使用 512 字节环形缓冲区
- 中断逐字节接收
- 通过 JSON 花括号匹配实现消息帧界定
- 使用 `strstr` 手动解析 JSON 字段 (非 cJSON 库)

---

## 2. 天问 AI 语音模块协议 (USART3, 9600 baud)

### 2.1 下行指令 (ASRPRO → STM32)

**帧格式:** `0xFF {command} 0x00` (3 字节)

| 指令码 | 功能 | 说明 |
|--------|------|------|
| 0x01 | 开启补光灯 | 控制 PD0 继电器 |
| 0x02 | 关闭补光灯 | 控制 PD0 继电器 |
| 0x03 | 开启风扇 | 控制 PD3 继电器 |
| 0x04 | 关闭风扇 | 控制 PD3 继电器 |
| 0x05 | 开启水泵 | 控制 PD1 继电器 |
| 0x06 | 关闭水泵 | 控制 PD1 继电器 |
| 0x07 | 进入 AI 对话模式 | 切换到 AI 语音交互 |
| 0x08 | 退出 AI 对话模式 | 返回普通模式 |

### 2.2 上行数据 (STM32 → ASRPRO)

**帧格式:** `{标识符} {数据高字节} {标识符} {数据低字节} ...` (10 字节)

| 标识符 | 数据 | 说明 |
|--------|------|------|
| 0xFE | 温度值 (8-bit) | 如 `0xFE 0x1A` = 26°C |
| 0xFD | 湿度值 (8-bit) | 如 `0xFD 0x3E` = 62%RH |
| 0xFC | 光照值 (8-bit) | 0-255 对应 0-65535 Lux |
| 0xFB | 土壤湿度 (8-bit) | 0-100% 线性映射 |
| 0xFA | 植物状态码 (8-bit) | 0-255 对应多种状态, 用于 RGB LED 颜色映射 |

### 2.3 发送时机

- 定时上报: 每 10 秒发送一次环境数据
- 状态突变: CO2 浓度超限时即时告警
- 语音查询: 用户询问 "当前温度" 时回传数据

---

## 3. TJC 串口屏协议 (USART1, 9600 baud)

### 3.1 指令格式

```
{属性名} {值} 0xFF 0xFF 0xFF
```

通过 `xsp_fs(str, num)` 函数发送，其中:
- `str`: 属性名称 (如 "num", "txt")
- `num`: 属性值
- 结尾固定 3 个 `0xFF` 作为结束符

### 3.2 表情 ID 映射

| ID | 表情 | 触发条件 |
|----|------|----------|
| 0 | 开心 (Happy) | 正常状态, 无人 |
| 1 | 难过 (Sad) | CO2 轻微超标 |
| 2 | 哭泣 (Crying) | 干燥持续 > 500 |
| 3 | 眩晕 (Dizzy) | 高温或 CO2 中度超标 |
| 4 | 生气 (Angry) | 高温持续 > 1000 |
| 5 | 害怕 (Scared) | 低温 |
| 6 | 休眠 (Sleepy) | 低光 |
| 7 | 戳脸互动 (Poke) | PIR 检测到人体 |
| 8 | 害羞 (Shy) | PIR 互动 + 振动 |
| 9-14 | 其他表情 | 特定环境组合 |

---

## 4. Feetech 串口舵机协议 (UART5, 9600 baud)

### 4.1 帧格式

```
0x55 {length} {command} {parameters...}
```

### 4.2 指令集

| 指令码 | 功能 | 参数 | 说明 |
|--------|------|------|------|
| 0x03 | 舵机移动 | ID, Position_L, Position_H, Time_L, Time_H | 单舵机位置控制 |
| 0x06 | 运行动作组 | Group_ID, Times_L, Times_H | 执行预设动作序列 |
| 0x07 | 停止动作组 | - | 停止当前动作 |
| 0x0B | 设置速度 | ID, Speed_L, Speed_H | 单舵机速度控制 |
| 0x0F | 读取电池电压 | ID | 返回电池电压值 |

### 4.3 应用场景

- 物理姿态交互: 植物 "点头"、"摇头" 等动作
- 情感表达: 配合表情系统同步物理动作
- 动作组: 预设多舵机联动序列
