#ifndef Kal_lb_H_
#define Kal_lb_H_

#include "stm32f1xx_hal.h"
#include "bool.h"
#include <stdio.h>

typedef struct {
    float q;    // 过程噪声协方差
    float r;    // 测量噪声协方差
    float x;    // 估计值
    float p;    // 估计误差协方差
    float k;    // 卡尔曼增益
} KalmanFilter;
void KalmanFilter_Init(KalmanFilter *kf, float q, float r, float initial_value, float initial_p);
float KalmanFilter_Update(KalmanFilter *kf, float measurement);


#endif
