#include "my_uart.h"

float pos_x=0;
float pos_y=0;
float zangle=0;
float xangle=0;
float yangle=0;
float w_z=0;
char data[] = "ACT0";   //正交编码轮初始化
/**
 * @brief 数据解析函数  
 * @param  rec 串口接收到的字节数据
 */
void Data_Analyse(uint8_t rec)
{
	static uint8_t ch;
	static union
	{
		
		
		uint8_t date[24];
		float ActVal[6];
	}posture;
	static uint8_t count=0;
	static uint8_t i=0;

	ch=rec;
	switch(count)
	{
		case 0:
			if(ch==0x0d)
				count++;
			else
				count=0;
			break;
		case 1:
			if(ch==0x0a)
			{
				i=0;
				count++;
			}
			else if(ch==0x0d);
			else
				count=0;
			break;
		case 2:
			posture.date[i]=ch;
			i++;
			if(i>=24)
			{
				i=0;
				count++;
			}
			break;
		case 3:
			if(ch==0x0a)
				count++;
			else
				count=0;
			break;
		case 4:
			if(ch==0x0d)
			{
				zangle=posture.ActVal[0];
				xangle=posture.ActVal[1];
				yangle=posture.ActVal[2];
				pos_x=posture.ActVal[3];
				pos_y=posture.ActVal[4];
				w_z=posture.ActVal[5];
			}
			count=0;
			break;
		default:
			count=0;
		break;
	}
}

void Data_stop(double*pos_X,double*pos_Y)
{
  *pos_X=0;
	*pos_Y=0;
  HAL_StatusTypeDef status = HAL_UART_Transmit_IT(&huart3, (uint8_t*)data, sizeof(data) - 1);//正交编码器置零
}

//void USART3_IRQHandler(void)                	//串口3中断服务程序
//{
//	u8 Res;
//	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //接收中断
//	{
//		Res =USART_ReceiveData(USART3);//(USART3->DR);	//读取接收到的数据
//		
//		Data_Analyse(Res);
//  }
//} 

