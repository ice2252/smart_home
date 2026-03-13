#include "stepper_motor.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "misc.h"

// 内部变量
static uint16_t motorSpeed = 1000; // 默认速度 1000us = 1ms
static MotorDirection motorDirection = CLOCKWISE;
static uint8_t currentStep = 0;
static volatile uint8_t isRunning = 0; // 加volatile保证中断访问安全
static uint16_t stepsRemaining = 0;    // 用于步数控制

// 步进序列 (全步进)
static const uint8_t fullStepSequence[4] = {
    0x01,  // IN1 (0001)
    0x02,  // IN2 (0010)
    0x04,  // IN3 (0100)
    0x08   // IN4 (1000)
};

// 初始化GPIO和定时器
void StepperMotor_Init(void) {
	
    GPIO_InitTypeDef GPIO_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    // 1. 初始化GPIO
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStruct.GPIO_Pin = MOTOR_PIN1 | MOTOR_PIN2 ;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_PORT1, &GPIO_InitStruct);
    GPIO_ResetBits(MOTOR_PORT1, MOTOR_PIN1 | MOTOR_PIN2 );
	
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStruct.GPIO_Pin = MOTOR_PIN3 | MOTOR_PIN4 ;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_PORT2, &GPIO_InitStruct);
    GPIO_ResetBits(MOTOR_PORT2, MOTOR_PIN3 | MOTOR_PIN4 );

    // 2. 初始化TIM3定时器
    TIM_TimeBaseInitTypeDef timer;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    
    timer.TIM_Prescaler = 72 - 1;          // 72MHz/72 = 1MHz (1us分辨率)
    timer.TIM_Period = motorSpeed;         // 初始周期
    timer.TIM_CounterMode = TIM_CounterMode_Up;
    timer.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3, &timer);
    
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    
    NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

// TIM3中断服务函数
void TIM3_IRQHandler(void) {
    if(TIM_GetITStatus(TIM3, TIM_IT_Update)) {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        
        if(isRunning) {
            // 执行单步
            if(motorDirection == CLOCKWISE) {
                currentStep = (currentStep + 1) % 4;
            } else {
                currentStep = (currentStep == 0) ? 3 : (currentStep - 1);
            }
            
            // 更新线圈状态
            GPIO_WriteBit(MOTOR_PORT1, MOTOR_PIN1, (fullStepSequence[currentStep] & 0x01) ? Bit_SET : Bit_RESET);
            GPIO_WriteBit(MOTOR_PORT1, MOTOR_PIN2, (fullStepSequence[currentStep] & 0x02) ? Bit_SET : Bit_RESET);
            GPIO_WriteBit(MOTOR_PORT2, MOTOR_PIN3, (fullStepSequence[currentStep] & 0x04) ? Bit_SET : Bit_RESET);
            GPIO_WriteBit(MOTOR_PORT2, MOTOR_PIN4, (fullStepSequence[currentStep] & 0x08) ? Bit_SET : Bit_RESET);
            
            // 步数控制
            if(stepsRemaining > 0) {
                stepsRemaining--;
                if(stepsRemaining == 0) {
                    StepperMotor_Stop();
                }
            }
        }
    }
}

// 设置速度 (单位us)
void StepperMotor_SetSpeed(uint16_t speed_us) {
    if(speed_us < MIN_SPEED) speed_us = MIN_SPEED;
    if(speed_us > MAX_SPEED) speed_us = MAX_SPEED;
    
    motorSpeed = speed_us;
    TIM3->ARR = speed_us;  // 动态更新定时器周期
    
    // 如果定时器已启用，需要手动触发更新
    if(TIM3->CR1 & TIM_CR1_CEN) {
        TIM3->EGR = TIM_PSCReloadMode_Immediate;
    }
}

// 设置方向
void StepperMotor_SetDirection(MotorDirection direction) {
    motorDirection = direction;
}

// 移动指定步数
void StepperMotor_Step(uint16_t steps) {
    stepsRemaining = steps;
    isRunning = 1;
    TIM_Cmd(TIM3, ENABLE);  // 启动定时器
}

// 连续旋转
void StepperMotor_RotateContinuous(void) {
    stepsRemaining = 0;  // 0表示无限旋转
    isRunning = 1;
    TIM_Cmd(TIM3, ENABLE);
}

// 停止电机
void StepperMotor_Stop(void) {
    isRunning = 0;
    TIM_Cmd(TIM3, DISABLE);
    GPIO_ResetBits(MOTOR_PORT1, MOTOR_PIN1 | MOTOR_PIN2 );
		GPIO_ResetBits(MOTOR_PORT2,  MOTOR_PIN3 | MOTOR_PIN4);
}

// 检查是否运行
uint8_t StepperMotor_IsRunning(void) {
    return isRunning;
}
