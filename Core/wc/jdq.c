#include "jdq.h"
#include "main.h"
/**
 * @description: 控制继电器开关
 * @param x为继电器的ID 1:控制补光 2:控制抽水机 3:风扇
 * @param y为是否打开的标志位(true: 开,false: 关)
 */
void jdq_pos(int x,bool key)
{
  switch(x)
	{
		case 1://控制补光
		{
		  if(key)
			{
			  HAL_GPIO_WritePin( GPIOD, GPIO_PIN_0,0);
			}
			else
			{
			  HAL_GPIO_WritePin( GPIOD, GPIO_PIN_0,1);
			}
			break;
		}
		case 2://控制抽水机
		{
		  if(key)
			{
			  HAL_GPIO_WritePin( GPIOD, GPIO_PIN_1,0);
			}
			else
			{
			  HAL_GPIO_WritePin( GPIOD, GPIO_PIN_1,1);
			}
			break;
		}
		case 3:
		{
		  if(key)
			{
			   HAL_GPIO_WritePin( GPIOD, GPIO_PIN_3,0);
			}
			else
			{
			   HAL_GPIO_WritePin( GPIOD, GPIO_PIN_3,1);
			}
			break;
		}
	}
}

/**
 * @description: 服务器端控制继电器开关 
 * @param sb:控制抽水机 bg;控制补光 fs:风扇
 */

void jdq_pos_wifi(uint8_t sb,uint8_t bg,uint8_t fs)
{
	if(yy_num2==1)
	{
		static uint32_t yy_time=0;//记录远端控制和语音控制的开始控制时间
		
		if(yy_time!=0 && HAL_GetTick()-yy_time>60000)//不操作60秒优先级更替
		{
		  yy_num2=0;
		}
	   if(sb_num_yy!=sb_num_yy2 || bg_num_yy!=bg_num_yy2 || fs_num_yy != fs_num_yy2)
		 {
		    sb_num_yy2=sb_num_yy;
			  bg_num_yy2=bg_num_yy;
			  fs_num_yy2=fs_num_yy;//刷新标志位
			   if(sb_num_yy==1)
				 {
					jdq_pos(2,1);
				 }
				 else
				 {
					jdq_pos(2,0);
				 }
				 if(bg_num_yy==1)
				 {
					 jdq_pos(1,1);
				 }
				 else
				 {
					jdq_pos(1,0);
				 }
				 if(fs_num_yy==1)
				 {
					jdq_pos(3,1);
				 }
				 else
				 {
					jdq_pos(3,0);
				 }
				  yy_time = HAL_GetTick();//记录最后远端、语音控制变更时间
		 }
	}
  else
	{	
		if(sb==1)
		 {
			jdq_pos(2,1);
		 }
		 else
		 {
			jdq_pos(2,0);
		 }
		 if(bg==1)
		 {
			 jdq_pos(1,1);
		 }
		 else
		 {
			jdq_pos(1,0);
		 }
		 if(fs==1)
		 {
			jdq_pos(3,1);
		 }
		 else
		 {
			jdq_pos(3,0);
		 }
 }
}


