#include "stm32f10x.h"                  // Device header
#include "Encoder.h"
#include "PWM.h"
#include "Moter.h"
#include "Key.h"
#include "Serial.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "Timer.h"
#include "OLED.h"

uint8_t Key_Num;   //小车启动按键

uint8_t Route_Flag;  //小车路线判断

//电机旋转速度的实际值
int16_t Speed1;
int16_t Speed2;

//PID控制启动标志变量
uint8_t PIDControl;


//电机目标速度
int16_t Speed_Target;


//提取接收到的数据包中的数字（速度）
int16_t ExtractSpeed(void);

int main(void)
{
	
	Moter_Init();
	Key_Init();
	Time_Init();
  Encoder_Init();
	Serial_Init();
	OLED_Init();
	
	while (1)
	{
	}
		
}

int16_t ExtractSpeed(void)
{
    static int16_t last = 0;
    if (Serial_RxFlag == 1)
    {
        Serial_RxFlag = 0;
        
      
        
        // 直接解析符号和数字
        int8_t sign = 1;
        uint8_t start_index = 0;
        
        if (Serial_RxPacket[7] == '+')
        {
            sign = 1;
            start_index = 1;
           
        }
        else if (Serial_RxPacket[7] == '-')
        {
            sign = -1;
            start_index = 1;
          
        }
        
        // 解析数字部分
        uint16_t speed = 0;
        for (uint8_t i = start_index; Serial_RxPacket[i] != '\0'; i++)
        {
            if (Serial_RxPacket[i] >= '0' && Serial_RxPacket[i] <= '9')
            {
                speed = speed * 10 + (Serial_RxPacket[i] - '0');
            }
        }
        
        last = sign * speed;
      
    }
    return last;
}

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		static int16_t cnt;
		cnt++;
		
		if (cnt>=10)
		{
		cnt=0;
			
		PIDControl++;
		
		//每10ms读取一次电机旋转速度实际值
		Speed1=Encoder_GetSpeed1();
		Speed2=Encoder_GetSpeed2();
		
		Key_Num=Key_GetNum();   //监视按键状态
	
		}
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}