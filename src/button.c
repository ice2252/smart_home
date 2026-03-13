#include "button.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "Delay.h"  // 包含你的延时函数


// 按钮对象
Button button1 = {BUTTON1_GPIO_PORT, BUTTON1_PIN, BUTTON_STATE_RELEASED, BUTTON_EVENT_NONE, 0};
Button button2 = {BUTTON2_GPIO_PORT, BUTTON2_PIN, BUTTON_STATE_RELEASED, BUTTON_EVENT_NONE, 0};
Button button3 = {BUTTON3_GPIO_PORT, BUTTON3_PIN, BUTTON_STATE_RELEASED, BUTTON_EVENT_NONE, 0};

// 按钮初始化
void Button_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能 GPIO 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // 配置按钮引脚为输入模式
    GPIO_InitStructure.GPIO_Pin = BUTTON1_PIN | BUTTON2_PIN | BUTTON3_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

// 按钮状态更新
void Button_Update(void) {
    static uint32_t lastTime = 0;  // 上一次更新时间
    uint32_t currentTime = 0;

    // 获取当前时间（毫秒）
    currentTime = lastTime + 10;  // 假设每 10ms 调用一次 Button_Update
    lastTime = currentTime;

    // 检查按钮1
    if (GPIO_ReadInputDataBit(button1.GPIOx, button1.GPIO_Pin) == Bit_RESET) {  // 按钮按下
        if (button1.state == BUTTON_STATE_RELEASED) {
            button1.state = BUTTON_STATE_PRESSED;
            button1.pressTime = currentTime;  // 记录按下时间
        } else if (button1.state == BUTTON_STATE_PRESSED) {
            if (currentTime - button1.pressTime > 1000) {  // 长按时间阈值（1秒）
                button1.state = BUTTON_STATE_LONG_PRESS;
                button1.event = BUTTON_EVENT_LONG_PRESS;
            }
        }
    } else {  // 按钮释放
        if (button1.state == BUTTON_STATE_PRESSED) {
            button1.event = BUTTON_EVENT_CLICK;
        } else if (button1.state == BUTTON_STATE_LONG_PRESS) {
            button1.event = BUTTON_EVENT_NONE;
        }
        button1.state = BUTTON_STATE_RELEASED;
    }

    // 检查按钮2（与按钮1逻辑相同）
    if (GPIO_ReadInputDataBit(button2.GPIOx, button2.GPIO_Pin) == Bit_RESET) {
        if (button2.state == BUTTON_STATE_RELEASED) {
            button2.state = BUTTON_STATE_PRESSED;
            button2.pressTime = currentTime;
        } else if (button2.state == BUTTON_STATE_PRESSED) {
            if (currentTime - button2.pressTime > 1000) {
                button2.state = BUTTON_STATE_LONG_PRESS;
                button2.event = BUTTON_EVENT_LONG_PRESS;
            }
        }
    } else {
        if (button2.state == BUTTON_STATE_PRESSED) {
            button2.event = BUTTON_EVENT_CLICK;
        } else if (button2.state == BUTTON_STATE_LONG_PRESS) {
            button2.event = BUTTON_EVENT_NONE;
        }
        button2.state = BUTTON_STATE_RELEASED;
    }

    // 检查按钮3（仅支持点按）
    if (GPIO_ReadInputDataBit(button3.GPIOx, button3.GPIO_Pin) == Bit_RESET) {
        if (button3.state == BUTTON_STATE_RELEASED) {
            button3.state = BUTTON_STATE_PRESSED;
            button3.event = BUTTON_EVENT_CLICK;
        }
    } else {
        button3.state = BUTTON_STATE_RELEASED;
        button3.event = BUTTON_EVENT_NONE;
    }
}

// 获取按钮事件
ButtonEvent Button_GetEvent(Button *button) {
    ButtonEvent event = button->event;
    button->event = BUTTON_EVENT_NONE;  // 清除事件
    return event;
}
