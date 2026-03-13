#include "delay.h"
#include "sys.h"
#include "oled.h"
#include "bmp.h"
#include "LED.h"         // LED驱动头文件（自定义）
#include "IIC.h"         // IIC通信驱动头文件（自定义，用于GY-39模块通信）
#include "stdio.h"
#include "stm32f10x_it.h"
#include <string.h>
#include "usart.h"
#include "button.h"
#include "stepper_motor.h"

extern uint32_t hc05_rec;
extern uint16_t ICID;

// 定义结构体存储GY-39模块的环境数据
typedef struct
{
    uint32_t P;    // 气压（原始数据，需转换）
    uint16_t Temp; // 温度（原始数据，需转换）
    uint16_t Hum;  // 湿度（原始数据，需转换）
    uint16_t Alt;  // 海拔（原始数据，需转换）
} bme;

bme Bme = {0,0,0,0};  // 声明结构体变量并初始化为0

 int main(void)
  {	
		u8 lux_flag=0;//0 窗帘关闭 1 窗帘开启
		u8 tem_flag=0;//0 风扇关闭 1 风扇开启
		u8 t;
		char temp_str[30];     // 存储温度字符串的缓冲区
		char Hum_str[30];     // 存储湿度字符串的缓冲区
		char Alt_str[30];     // 存储海拔字符串的缓冲区
		char Lux_str[30];     // 存储光强字符串的缓冲区
		char p_str[30];     // 存储气压字符串的缓冲区
		u8  raw_data[13] = {0};  // 用于存储IIC读取的原始数据（13字节缓冲区）
		uint16_t data_16[2] = {0};  // 用于临时存储16位数据
		uint32_t Lux;  // 光照强度（原始数据，需转换）
		
		ButtonEvent event1;
		ButtonEvent event2;
		ButtonEvent event3;
		delay_init();	    	 //延时函数初始化	  
		Button_Init(); // 初始化按钮
		StepperMotor_Init();  //步进电机初始化
		delay_init();	    	 //延时函数初始化	  
		uart2_init(9600);
		uart_init3(9600);
		NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级 	LED_Init();			     //LED端口初始化
	//		delay_ms(8000);
		OLED_Init();			//初始化OLED  
		OLED_Clear()  	; 
		


		
		LED_Int(GPIOB, GPIO_Pin_9, RCC_APB2Periph_GPIOB);  // 初始化LED（引脚GPIOB.9，使能GPIOB时钟）
		//NVIC_Configuration();  // 配置NVIC中断优先级
		//Usart_Int(9600);  // 初始化串口（此处波特率为9600，与注释的115200可能存在冲突，需注意）
		I2C_GPIO_Config();  // 初始化IIC通信的GPIO引脚（PB6-SCL，PB7-SDA）
		delay_ms(100);  // 延时100ms，等待GY-39模块完成初始化
		t=' ';
//		OLED_ShowCHinese(0,0,0);//中
//				OLED_ShowCHinese(18,0,1);//景
//		OLED_ShowCHinese(36,0,2);//园
//		OLED_ShowCHinese(54,0,3);//电
//		OLED_ShowCHinese(72,0,4);//子
//		OLED_ShowCHinese(90,0,5);//科
//		OLED_ShowCHinese(108,0,6);//技
		
	
	while(1) 
	{	

		Button_Update();  // 更新按钮状态
		
		 event1 = Button_GetEvent(&button1);
		 event2 = Button_GetEvent(&button2);
		 event3 = Button_GetEvent(&button3);
		
		if (hc05_rec==1) {    //设置确认按键
				                 
			  StepperMotor_SetDirection(CLOCKWISE);
        StepperMotor_SetSpeed(3000);
        StepperMotor_RotateContinuous();	
		}
		
		if (hc05_rec==3) {  
				
				StepperMotor_SetDirection(COUNTER_CLOCKWISE);
        StepperMotor_SetSpeed(3000);
        StepperMotor_RotateContinuous();			
		}	
		if (hc05_rec==2) { 
				
				StepperMotor_Stop();  // 优雅停止
				
		}
		// 从GY-39模块（IIC地址0xb6）的0x04寄存器读取10字节数据（温度、气压、湿度、海拔）
    if(Single_ReadI2C(0xb6, 0x04, raw_data, 10))
    {
      // 解析温度：raw_data[0]（高8位）+ raw_data[1]（低8位）→ 16位原始值
      Bme.Temp = (raw_data[0] << 8) | raw_data[1];
      // 解析气压（32位）：先将raw_data[2-3]和raw_data[4-5]转为两个16位值
      data_16[0] = (((uint16_t)raw_data[2]) << 8) | raw_data[3];
      data_16[1] = (((uint16_t)raw_data[4]) << 8) | raw_data[5];
      // 合并为32位气压原始值
      Bme.P = (((uint32_t)data_16[0]) << 16) | data_16[1];
      // 解析湿度：raw_data[6]（高8位）+ raw_data[7]（低8位）→ 16位原始值
      Bme.Hum = (raw_data[6] << 8) | raw_data[7];
      // 解析海拔：raw_data[8]（高8位）+ raw_data[9]（低8位）→ 16位原始值
      Bme.Alt = (raw_data[8] << 8) | raw_data[9];
    }
    
    // 从GY-39模块的0x00寄存器读取4字节数据（光照强度）
    if(Single_ReadI2C(0xb6, 0x00, raw_data, 4))
      // 解析光照（32位）：raw_data[0-1]（高16位）+ raw_data[2-3]（低16位）
      data_16[0] = (((uint16_t)raw_data[0]) << 8) | raw_data[1];
    data_16[1] = (((uint16_t)raw_data[2]) << 8) | raw_data[3];  
    Lux = (((uint32_t)data_16[0]) << 16) | data_16[1];  // 合并为32位光照原始值
    
    // 串口打印数据（原始值除以100得到实际物理量）
    //printf("Temp: %.2f  DegC  ", (float)Bme.Temp / 100);  // 温度（单位：℃）
    //printf("  P: %.2f  Pa ", (float)Bme.P / 100);  // 气压（单位：Pa）
    //printf("  Hum: %.2f   ", (float)Bme.Hum / 100);  // 湿度（单位：%）
    //printf("  Alt: %.2f  m\r\n ", (float)Bme.Alt);  // 海拔（单位：m）
    //printf("\r\n Lux: %.2f  lux\r\n ", (float)Lux / 100);  // 光照（单位：lux）
    
    delay_ms(200);  // 延时200ms，降低数据打印频率
		OLED_Clear();
		OLED_ShowCHinese(0,0,0);//温
		OLED_ShowCHinese(18,0,1);//度
		memset(temp_str, 0, sizeof(temp_str));
		// 将温度转换为“保留2位小数”的字符串
    snprintf(temp_str, sizeof(temp_str), "%.2f`C", (float)Bme.Temp / 100);
		OLED_ShowString(36,0,(u8*)temp_str,16);
		
		OLED_ShowCHinese(0,3,2);//湿
		OLED_ShowCHinese(18,3,3);//度
		// 将湿度转换为“保留2位小数”的字符串
    snprintf(Hum_str, sizeof(Hum_str), "%.2f%%", (float)Bme.Hum / 100);
		OLED_ShowString(36,3,(u8*)Hum_str,16);
		//OLED_ShowCHinese(63,0,2);//湿
		//OLED_ShowCHinese(81,0,3);//度
		//OLED_ShowString(99,0,"10",16);
		OLED_ShowCHinese(0,6,4);//海
		OLED_ShowCHinese(18,6,5);//拔
		// 将海拔转换为“保留2位小数”的字符串
    snprintf(Alt_str, sizeof(Alt_str), "%.2fm",(float)Bme.Alt);
		OLED_ShowString(36,6,(u8*)Alt_str,16);
		delay_ms(8000);
		OLED_Clear();
		
		
		
		OLED_ShowCHinese(0,0,6);//光
		OLED_ShowCHinese(18,0,7);//强
		// 将光强转换为“保留2位小数”的字符串
    snprintf(Lux_str, sizeof(Lux_str), "%.2flux",(float)Lux / 100);
		OLED_ShowString(36,0,(u8*)Lux_str,16);
		
		OLED_ShowCHinese(0,3,8);//气
		OLED_ShowCHinese(18,3,9);//压
		// 将气压转换为“保留2位小数”的字符串
    snprintf(p_str, sizeof(p_str), "%.2fpa",(float)Bme.P / 100);
		OLED_ShowString(36,3,(u8*)p_str,16);
		
		//OLED_ShowString(36,6,"30",16);
		//OLED_ShowString(6,3,"0.96' OLED TEST",16);
		//OLED_ShowString(8,2,"ZHONGJINGYUAN");  
	 //	OLED_ShowString(20,4,"2014/05/01");  
		//OLED_ShowString(0,6,"ASCII:",16);  
		//OLED_ShowString(63,6,"CODE:",16);  
		//OLED_ShowChar(48,6,t,16);//显示ASCII字符	   
		//t++;
		//if(t>'~')t=' ';
		//OLED_ShowNum(103,6,t,3,16);//显示ASCII字符的码值 	
			//delay_ms(8000);
		//delay_ms(8000);

					//delay_ms(8000);
		//delay_ms(8000);
		//delay_ms(8000);
		//OLED_DrawBMP(0,0,128,8,BMP1);  //图片显示(图片显示慎用，生成的字表较大，会占用较多空间，FLASH空间8K以下慎用)
		//delay_ms(8000);
					//delay_ms(8000);
		//delay_ms(8000);
		//delay_ms(8000);
		//OLED_DrawBMP(0,0,128,8,BMP1);
		//delay_ms(8000);
					//delay_ms(8000);
		//delay_ms(8000);
		
		if((int)(Lux/100)>200)
		{
			lux_flag = 2;
		}
		else if((Lux/100)<100)
		{
			lux_flag = 1;
		}
		else 
		{
			lux_flag = 0;
		}
		if(lux_flag == 2)
		{
			StepperMotor_SetDirection(CLOCKWISE);
			StepperMotor_Step(600);
      StepperMotor_SetSpeed(3000);
		}
		if(lux_flag == 1)
		{
			StepperMotor_SetDirection(COUNTER_CLOCKWISE);
			StepperMotor_Step(600);
      StepperMotor_SetSpeed(3000);
		}
		
		if(((float)Bme.Temp/100)>28)
		{	
			tem_flag = 1;
		}
		if(((float)Bme.Temp/100)<28)
		{
			tem_flag = 0;
		}
		if(tem_flag)
		{
			StepperMotor_SetDirection(CLOCKWISE);
      StepperMotor_SetSpeed(3000);
      StepperMotor_RotateContinuous();
			
		}
		else if(tem_flag == 0 && lux_flag ==0)
		{
		StepperMotor_Stop();
		}
			if(ICID || hc05_rec ==4)
		{
			StepperMotor_SetDirection(CLOCKWISE);
			StepperMotor_Step(200);
      StepperMotor_SetSpeed(3000);
			ICID =0;
			hc05_rec =0;
		}
		
		delay_ms(8000);
	}	  
	
}
	