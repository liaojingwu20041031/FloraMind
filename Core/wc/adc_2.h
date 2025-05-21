#ifndef adc_2_H_
#define adc_2_H_
#include "adc.h"
#include "stdint.h"
 


#define adc_max 20
#define gq_max 375
#define gq_min 0
#define tr_sd_max 330
#define tr_sd_min 150

extern  uint16_t My_adcData [adc_max];

typedef struct {
	uint16_t value1;
	uint16_t value2;
}adcValue_type;
 
extern  adcValue_type adcValue ;
extern float gq;
extern float tr_sd;
void ADC_dispose (void);
 
#endif

