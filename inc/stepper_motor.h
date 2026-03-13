#ifndef __STEPPER_MOTOR_H
#define __STEPPER_MOTOR_H

#include "stm32f10x.h"

// 引脚定义
#define MOTOR_PORT1 GPIOB
#define MOTOR_PORT2 GPIOC
#define MOTOR_PIN1 GPIO_Pin_4
#define MOTOR_PIN2 GPIO_Pin_5
#define MOTOR_PIN3 GPIO_Pin_14
#define MOTOR_PIN4 GPIO_Pin_15

// 旋转方向
typedef enum {
    CLOCKWISE = 0,
    COUNTER_CLOCKWISE = 1
} MotorDirection;

// 速度范围 (us)
#define MIN_SPEED 500   // 500us = 0.5ms
#define MAX_SPEED 5000  // 5000us = 5ms

// 函数声明
void StepperMotor_Init(void);
void StepperMotor_SetSpeed(uint16_t speed_us); // 参数改为微秒
void StepperMotor_SetDirection(MotorDirection direction);
void StepperMotor_Step(uint16_t steps);
void StepperMotor_RotateContinuous(void);
void StepperMotor_Stop(void);
uint8_t StepperMotor_IsRunning(void);

#endif /* __STEPPER_MOTOR_H */
