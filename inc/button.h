#ifndef __BUTTON_H
#define __BUTTON_H

#include "stm32f10x.h"

// 按钮引脚定义
#define BUTTON1_PIN          GPIO_Pin_7
#define BUTTON1_GPIO_PORT    GPIOA
#define BUTTON2_PIN          GPIO_Pin_6
#define BUTTON2_GPIO_PORT    GPIOA
#define BUTTON3_PIN          GPIO_Pin_1
#define BUTTON3_GPIO_PORT    GPIOA

// 按钮状态
typedef enum {
    BUTTON_STATE_RELEASED,  // 按钮释放
    BUTTON_STATE_PRESSED,   // 按钮按下
    BUTTON_STATE_LONG_PRESS // 按钮长按
} ButtonState;

// 按钮事件
typedef enum {
    BUTTON_EVENT_NONE,      // 无事件
    BUTTON_EVENT_CLICK,     // 点按事件
    BUTTON_EVENT_LONG_PRESS // 长按事件
} ButtonEvent;

// 按钮结构体
typedef struct {
    GPIO_TypeDef *GPIOx;    // GPIO端口
    uint16_t GPIO_Pin;      // GPIO引脚
    ButtonState state;      // 按钮状态
    ButtonEvent event;      // 按钮事件
    uint32_t pressTime;     // 按下时间（毫秒）
} Button;

// 函数声明
void Button_Init(void);
void Button_Update(void);
ButtonEvent Button_GetEvent(Button *button);

// 外部变量声明
extern Button button1;
extern Button button2;
extern Button button3;

#endif
