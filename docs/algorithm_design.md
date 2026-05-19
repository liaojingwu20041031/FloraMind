# 算法设计详解

## 1. 卡尔曼滤波器

### 1.1 数学模型

卡尔曼滤波是一种基于预测-更新的递推最优估计算法，用于从含噪声的传感器数据中提取真实信号。

**预测阶段:**
```
x̂(k|k-1) = x̂(k-1)           -- 状态预测 (假设状态不变)
P(k|k-1) = P(k-1) + Q        -- 协方差预测
```

**更新阶段:**
```
K = P(k|k-1) / (P(k|k-1) + R)    -- 卡尔曼增益
x̂(k) = x̂(k|k-1) + K * (z - x̂(k|k-1))  -- 状态更新
P(k) = (1 - K) * P(k|k-1)        -- 协方差更新
```

其中:
- `Q` -- 过程噪声协方差, 越大表示系统状态变化越快
- `R` -- 测量噪声协方差, 越大表示传感器越不精确
- `K` -- 卡尔曼增益, 自动平衡预测与测量的信任度

### 1.2 实现代码

```c
void KalmanFilter_Init(KalmanFilter *kf, float q, float r, float initial_value, float initial_p) {
    kf->q = q;
    kf->r = r;
    kf->x = initial_value;
    kf->p = initial_p;
}

float KalmanFilter_Update(KalmanFilter *kf, float measurement) {
    kf->p += kf->q;                          // 预测协方差
    kf->k = kf->p / (kf->p + kf->r);        // 卡尔曼增益
    kf->x += kf->k * (measurement - kf->x);  // 状态更新
    kf->p *= (1 - kf->k);                    // 协方差更新
    return kf->x;
}
```

### 1.3 各传感器参数

| 传感器 | Q (过程噪声) | R (测量噪声) | 说明 |
|--------|-------------|-------------|------|
| DHT11 温度 | 0.1 | 1.3 | 温度变化缓慢, 传感器精度一般 |
| DHT11 湿度 | 0.1 | 1.3 | 同上 |
| 土壤湿度 | 0.1 | 1.3 | ADC 采样, 中等噪声 |
| 光照强度 | 0.1 | 4.0 | 光敏电阻噪声较大 |
| CO2 (SGP30) | 动态 | 动态 | 见下方特殊处理 |

### 1.4 CO2 传感器动态噪声处理

SGP30 CO2 传感器具有非线性误差特性和较长的预热时间，需要特殊处理。

**预热保护:**
```c
if (CO2 > 480) {
    CO2_NO = 1;  // 预热完成标志
    // 进入正常滤波模式
} else if (CO2_NO == 0) {
    CO2_2 = CO2;  // 预热阶段直接输出原始值
}
```

**动态测量噪声 R:**
```c
CO2_kal.r = 2500.0f + 0.0025f * CO2_kal.x * CO2_kal.x;
// CO2=500ppm 时, R=2500+625=3125
// CO2=1000ppm 时, R=2500+2500=5000
// 物理意义: 浓度越高, 绝对误差越大, 降低测量权重
```

**动态过程噪声 Q:**
```c
float change_rate = fabs(CO2_kal.x - last_CO2) / 1.0f;
CO2_kal.q = 5.0f + change_rate * 2.0f;
// 变化率越大, Q越大, 滤波器响应越快
```

**数据下限保护:**
```c
float raw_CO2 = (CO2 < 400) ? 400.0f : (float)CO2;
// 强制 CO2 >= 400ppm, 对抗预热异常低值
```

**效果对比:**
| 时间 | 原始 CO2 | 滤波后 CO2 | 策略 |
|------|----------|------------|------|
| 0 min (冷启动) | 380 (异常) | 400 (强制下限) | 预热保护 |
| 1 min | 420 | 415 | 动态 R ≈ 2934 |
| 3 min | 600 | 595 | Q 噪声自适应 |
| 5 min (稳定) | 800 | 795 | R ≈ 4075 |

---

## 2. 动态迟滞控制

### 2.1 算法原理

迟滞控制通过引入 "死区" 避免设备频繁启停。动态迟滞根据环境变量的变化速率自动调整死区宽度。

**迟滞宽度公式:**
```
H = (|dx/dt| > threshold) ? H_wide : H_narrow
```

**控制逻辑 (以补光灯为例):**
```c
if (gq < (threshold - hysteresis))
    bg_num = 1;  // 开启补光灯
else if (gq > (threshold + hysteresis))
    bg_num = 0;  // 关闭补光灯
// 在 [threshold-H, threshold+H] 区间内保持当前状态
```

### 2.2 应用场景

| 控制对象 | 迟滞窄 | 迟滞宽 | 切换条件 |
|----------|--------|--------|----------|
| 补光灯 | 5 单位 | 10 单位 | 光照变化速率 > 5 |
| 水泵 | 3 单位 | 8 单位 | 土壤湿度预测干燥 |

---

## 3. 预测性灌溉

### 3.1 线性外推预测

基于最近 3 次土壤湿度数据，使用线性外推预测下一时刻的湿度值:

```
X(n+1) = 3X(n) - 3X(n-1) + X(n-2)
```

**实现代码:**
```c
static float soil_history[3] = {0};
soil_history[2] = soil_history[1];
soil_history[1] = soil_history[0];
soil_history[0] = tr_sd_2;
float soil_pred = 3*soil_history[0] - 3*soil_history[1] + soil_history[2];
```

### 3.2 动态迟滞调整

- 当预测值显示干燥趋势 (`soil_pred < soil_history[0]`): 迟滞宽度扩大为 8
- 否则: 迟滞宽度保持为 3

这种机制确保:
- 湿度下降趋势时提前灌溉, 防止土壤过干
- 湿度上升趋势时延迟关闭, 避免水泵频繁启停

---

## 4. 多因素风扇控制

### 4.1 复合评分机制

风扇控制采用温度和 CO2 浓度的加权复合评分:

```
score = 0.6 × (temp / temp_high) + 0.4 × (CO2 / CO2_high)
```

**决策逻辑:**
```c
if (score > 1.0)
    fs_num = 1;  // 启动风扇
else
    fs_num = 0;  // 关闭风扇
```

**权重分配:**
- 温度权重 0.6: 温度是影响植物生长的主要因素
- CO2 权重 0.4: CO2 超标影响空气质量, 但不如温度紧急

---

## 5. 自适应阈值

### 5.1 环形缓冲区

温度阈值基于历史数据动态调整，使用 30 个采样点的环形缓冲区:

```c
static float temp_history[30] = {0};
static uint8_t history_index = 0;

// 每 3 秒更新一次
if (HAL_GetTick() - last_update > 3000) {
    temp_history[history_index] = wd_2;
    history_index = (history_index + 1) % 30;

    float temp_sum = 0;
    uint8_t n = (history_index > 30) ? 30 : history_index;
    for (int i = 0; i < n; i++) temp_sum += temp_history[i];
    float temp_avg = temp_sum / n;

    env_threshold.temp_high = temp_avg + 3.0f;
    env_threshold.temp_low = temp_avg - 3.0f;
    last_update = HAL_GetTick();
}
```

### 5.2 优势

- **实时性**: 周期性更新保证对环境变化的快速响应
- **鲁棒性**: 平滑化短期波动, 减少误触发
- **自适应性**: 无需人工干预, 自动适应不同环境

---

## 6. 拟人表情决策系统

### 6.1 状态权重计算

系统根据环境数据计算 6 种状态的权重:

| 状态 | 权重计算 | 条件 |
|------|----------|------|
| 高温 | `is_active × (1 + 0.001 × duration)` | 温度 > temp_high |
| 干燥 | `is_active × (1 + 0.002 × duration)` | 土壤湿度 < 干燥阈值 |
| CO2 超标 | `is_active × (1 + 0.001 × duration)` | CO2 > co2_high |
| 低温 | `1` | 温度 < temp_low |
| 低光 | `1` | 光照 < 光照阈值 |
| 正常 | `1` | 默认状态 |

### 6.2 最大优先级决策

```c
uint8_t max_index = 5; // 默认正常状态
for (int i = 0; i < 5; i++) {
    if (state_weights[i] > state_weights[max_index]) {
        max_index = i;
    }
}
```

### 6.3 表情映射

| 状态 | 持续时间 | 表情 ID | 表情 |
|------|----------|---------|------|
| 高温 | > 1000 | 4 | 生气 |
| 高温 | ≤ 1000 | 3 | 眩晕 |
| 干燥 | > 500 | 2 | 哭泣 |
| 干燥 | ≤ 500 | - | 语音提示 |
| CO2 超标 | > 1500ppm | 3 | 眩晕 |
| CO2 超标 | ≤ 1500ppm | 1 | 难过 |
| 低温 | - | 5 | 害怕 |
| 低光 | - | 6 | 休眠 |
| 正常 | 无人 | 0 | 开心 |
| 正常 | 有人 (PIR) | 7/8 | 戳脸/害羞 |

### 6.4 防抖机制

```c
if (current_expression != last_expression) {
    static uint32_t last_change = 0;
    if (HAL_GetTick() - last_change > 3000) {  // 3 秒防抖
        xsp_fs("num", current_expression);
        last_expression = current_expression;
        last_change = HAL_GetTick();
    }
}
```

---

## 7. 优先级仲裁

### 7.1 控制源优先级

```
远程控制 (WiFi/语音) > 本地自动控制
```

### 7.2 仲裁逻辑

```c
void jdq_pos_wifi(void) {
    if (yy_num2 == 1) {
        // 远程控制优先
        jdq_pos(0, ESP8266_bg);  // 补光灯
        jdq_pos(1, ESP8266_sb);  // 水泵
        jdq_pos(3, ESP8266_fs);  // 风扇
    } else {
        // 本地自动控制
        jdq_pos(0, zt_bg);
        jdq_pos(1, zt_sb);
        jdq_pos(3, zt_fs);
    }
}
```

### 7.3 超时回退

- 远程控制激活时设置 `yy_num2 = 1`
- 每次收到远程指令重置计时器
- 5 分钟无远程指令后 `yy_num2 = 0`, 回退到本地控制
- 互斥锁 `esp8266_x_01` 保护 UART2 防止并发访问
