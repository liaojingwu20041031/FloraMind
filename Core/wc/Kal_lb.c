#include <Kal_lb.h> 

// 初始化卡尔曼滤波器
void KalmanFilter_Init(KalmanFilter *kf, float q, float r, float initial_value, float initial_p)
	{  
		// q: 过程噪声(系统不确定性)，值越大滤波器响应越快但可能不稳定
    // r: 测量噪声(传感器噪声)，值越大滤波器越不信任测量值
    kf->q = q;
    kf->r = r;
    kf->x = initial_value;
    kf->p = initial_p;
    kf->k = 0;
}

// 卡尔曼滤波处理
float KalmanFilter_Update(KalmanFilter *kf, float measurement) 
	{
    /* 预测阶段 */
    // 状态预测：x_k = A * x_{k-1} (这里A=1，因为是单变量)
    // 误差协方差预测：P_k = A * P_{k-1} * A' + Q (这里A=1)
    kf->p = kf->p + kf->q;
		
    /* 更新阶段 */
    // 卡尔曼增益计算：K = P * H' / (H * P * H' + R) (这里H=1)
    kf->k = kf->p / (kf->p + kf->r);
    // 状态更新：x = x + K * (z - H * x) (这里H=1)
    kf->x = kf->x + kf->k * (measurement - kf->x);
    // 误差协方差更新：P = (I - K * H) * P (这里H=1)
    kf->p = (1 - kf->k) * kf->p;
    
    return kf->x;
}
