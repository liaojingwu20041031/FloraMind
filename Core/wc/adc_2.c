#include "adc_2.h"

uint16_t My_adcData [adc_max]={0};
adcValue_type adcValue ;
float gq; 
float tr_sd; 
/* USER CODE BEGIN 1 */
	/*
	*adc数据处理
	*每通道的数据进行10次获取，数据的每一组的第一个和最后一个不要，并且将剩下的进行取平均值
	*
	*/
void ADC_dispose (void)
{
	adcValue .value1=adcValue .value2=0;
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)My_adcData,adc_max);//因为你选择的软件触发，所以每次采集都需要开启一次
	static   uint8_t i;
	for(i=1;i<=8;i++){                       							 //遍历10次，进行滤波
		adcValue .value1 += My_adcData[0+2*i]*330/4096;					
		adcValue .value2 += My_adcData[1+2*i]*330/4096;
	}
	adcValue .value1 = adcValue .value1/8 ;//土壤湿度
	adcValue .value2 = adcValue .value2/8 ;//光照强度
	
	///比值转换////
	gq=100-((float)(adcValue.value2-gq_min)/(gq_max-gq_min))*100;
	tr_sd=100-((float)(adcValue.value1-tr_sd_min)/(tr_sd_max-tr_sd_min))*100;
}
/* USER CODE END 1 */
