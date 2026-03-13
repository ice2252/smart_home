#include "sys.h"


void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_X;  // NVIC初始化结构体
  
  /* 配置中断优先级分组：组2（共4个抢占优先级，4个响应优先级）
     抢占优先级：数值越小，优先级越高，高优先级可打断低优先级中断
     响应优先级：数值越小，优先级越高，仅当抢占优先级相同时生效 */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  NVIC_X.NVIC_IRQChannel = USART1_IRQn;  // 配置串口1中断通道
  NVIC_X.NVIC_IRQChannelPreemptionPriority = 3;  // 抢占优先级为3（较低）
  NVIC_X.NVIC_IRQChannelSubPriority = 0;  // 响应优先级为0（较高）
  NVIC_X.NVIC_IRQChannelCmd = ENABLE;  // 使能串口1中断
  NVIC_Init(&NVIC_X);  // 初始化NVIC配置
}
