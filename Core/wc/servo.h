#ifndef __SERVO_H__
#define __SERVO_H__

#include <stdint.h>

#define GET_LOW_BYTE(A) ((uint8_t)(A))
#define GET_HIGH_BYTE(A) ((uint8_t)((A) >> 8))
#define FRAME_HEADER 0x55
#define CMD_SERVO_MOVE 0x03
#define CMD_ACTION_GROUP_RUN 0x06
#define CMD_ACTION_GROUP_STOP 0x07
#define CMD_ACTION_GROUP_SPEED 0x0B
#define CMD_GET_BATTERY_VOLTAGE 0x0F

typedef struct {
	uint8_t ID;
	uint16_t Position;
} ServoStatus;

void moveServo(uint8_t servoID, uint16_t Position, uint16_t Time);
void moveServosByArray(ServoStatus servos[], uint8_t Num, uint16_t Time);
void runActionGroup(uint8_t numOfAction, uint16_t Times);
void stopActionGroup(void);
#endif


