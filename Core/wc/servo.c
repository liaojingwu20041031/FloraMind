#include "servo.h"
#include "main.h"
#include "usart.h"
uint8_t send_buf[128];
/*********************************************************************************
 * Function:  moveServo
 * Description： 控制单个舵机转动
 * Parameters:   sevoID:舵机ID，Position:目标位置,Time:转动时间
                    舵机ID取值:0<=舵机ID<=31,Time取值: Time > 0
 * Return:       无返回
 * Others:
 **********************************************************************************/
void moveServo(uint8_t servoID, uint16_t Position, uint16_t Time)
{
	if (servoID > 31 || !(Time > 0)) {
		return;
	}
	send_buf[0] = send_buf[1] = FRAME_HEADER;
	send_buf[2] = 8;
	send_buf[3] = CMD_SERVO_MOVE;          
	send_buf[4] = 1;                       
	send_buf[5] = GET_LOW_BYTE(Time);      
	send_buf[6] = GET_HIGH_BYTE(Time);     
	send_buf[7] = servoID;                 
	send_buf[8] = GET_LOW_BYTE(Position);  
	send_buf[9] = GET_HIGH_BYTE(Position); 
	HAL_UART_Transmit_IT(&huart4, send_buf, 10);
}
/*********************************************************************************
 * Function:  moveServosByArray
 * Description： 控制多个舵机转动
 * Parameters:   servos[]:舵机结体数组，Num:舵机个数,Time:转动时间
                    0 < Num <= 32,Time > 0
 * Return:       无返回
 * Others:
 **********************************************************************************/
void moveServosByArray(ServoStatus servos[], uint8_t Num, uint16_t Time)
{
	uint8_t index = 7;
	if (Num < 1 || Num > 32 || !(Time > 0)) {
		return;
	}
	send_buf[0] = send_buf[1] = FRAME_HEADER;
	send_buf[2] = Num * 3 + 5;
	send_buf[3] = CMD_SERVO_MOVE;
	send_buf[4] = Num;
	send_buf[5] = GET_LOW_BYTE(Time);
	send_buf[6] = GET_HIGH_BYTE(Time);
	for (uint8_t i = 0; i < Num; i++) {                  
		send_buf[index++] = servos[i].ID;      
		send_buf[index++] = GET_LOW_BYTE(servos[i].Position);
		send_buf[index++] = GET_HIGH_BYTE(servos[i].Position);
	}
	HAL_UART_Transmit_IT(&huart4, send_buf, send_buf[2]+2);
}
/*********************************************************************************
 * Function:  runActionGroup
 * Description： 运行指定动作组
 * Parameters:   NumOfAction:动作组序号, Times:执行次数
 * Return:       无返回
 * Others:       Times = 0 时无限循环
 **********************************************************************************/
void runActionGroup(uint8_t numOfAction, uint16_t Times)
{
	send_buf[0] = send_buf[1] = FRAME_HEADER;
	send_buf[2] = 5;
	send_buf[3] = CMD_ACTION_GROUP_RUN;
	send_buf[4] = numOfAction;
	send_buf[5] = GET_LOW_BYTE(Times);
	send_buf[6] = GET_HIGH_BYTE(Times);

	HAL_UART_Transmit_IT(&huart4, send_buf, 7);
}
/*********************************************************************************
 * Function:  stopActiongGroup
 * Description： 停止动作组运行
 * Parameters:   Speed: 目标速度
 * Return:       无返回
 * Others:
 **********************************************************************************/
void stopActionGroup(void)
{
	send_buf[0] = FRAME_HEADER;
	send_buf[1] = FRAME_HEADER;
	send_buf[2] = 2;
	send_buf[3] = CMD_ACTION_GROUP_STOP;

	HAL_UART_Transmit_IT(&huart4, send_buf, 4);
}

