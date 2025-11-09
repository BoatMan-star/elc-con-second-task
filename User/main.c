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

//当Key_Num=1时，考核任务是读取电机的旋转速度并且可以实现pid闭环控制
//当Key_Num=0时，考核任务是电机传动

uint8_t Key_Num;   //切换考核任务

//电机旋转速度的实际值
int16_t Speed1;
int16_t Speed2;

//PID控制启动标志变量
uint8_t PIDControl;

//pid控制定义参数变量kp，ki，kd（增量式pid控制速度）
float kp=0.3,ki=0.6,kd=0.02;
//定义误差变量
float error0=0,error1=0,error2=0,errorInt=0;
//Out=（kp+ki+kd）的积分
float Out1;

//pid控制定义参数变量kp，ki，kd（增量式pid控制位置）
float kp_loc=3,ki_loc=0.4,kd_loc=0.01;
//定义误差变量
float error0_loc=0,error1_loc=0,error2_loc=0,errorInt_loc=0;
//Out=（kp+ki+kd）的积分
float Out2;

//电机目标速度
int16_t Speed_Target;

//电机的目标位置（电机2的位置），电机的实际位置（电机1的位置）
int16_t Location2_Target=0,Location1_Actually=0;

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
		if (Key_Num==1)
	{
		OLED_ShowString(1,1,"Speed_Control   ");
		OLED_ShowString(2,1,"Tar:            ");
		OLED_ShowString(3,1,"Act:            ");
		OLED_ShowString(4,1,"Out:            ");
		OLED_ShowSignedNum(2,5,Speed_Target,4);
		OLED_ShowSignedNum(3,5,Speed1,4);
		OLED_ShowSignedNum(4,5,Out1,4);
			
	}
	else if (Key_Num==0)
	{
		OLED_ShowString(1,1,"Moter_Location  ");
		OLED_ShowString(2,1,"Tar:            ");
		OLED_ShowString(3,1,"Act:            ");
		OLED_ShowString(4,1,"Out:            ");
		OLED_ShowSignedNum(2,5,Location2_Target,4);
		OLED_ShowSignedNum(3,5,Location1_Actually,4);
		OLED_ShowSignedNum(4,5,Out2,4);
	}
		
		//考核任务1（KeyNum=1）
		if (Key_Num==1)
		{
			
			Serial_Printf("%d,%d,%d\n",Speed_Target,Speed1,Out1);	
			
			Speed_Target=ExtractSpeed();
		  
			//PID控制(电机1)
			if (PIDControl>=1)
			{
				PIDControl=0;
				
				
        error2 = error1;
        error1 = error0;
        error0 = Speed_Target-Speed1;
            
        // 增量式PID
        Out1+=kp*(error0 - error1)+ki*error0+ kd*(error0-2*error1+error2);
            
        // 输出限幅
        if (Out1 > 100) Out1 = 100;
        if (Out1 < -100) Out1 = -100;
				
				// 执行控制
        Moter_SetPower1(Out1);
        }
			
			
		}
		//考核任务2
		else if (Key_Num==0)
		{
			
			Serial_Printf("%d,%d,%d\n",Location2_Target,Location1_Actually,Out2);
			
			
			//获取主电机2的位置
			Location2_Target+=Speed2;
			Location2_Target%=9999;
			
			//获取电机1的位置
			Location1_Actually+=Speed1;
			Location1_Actually%=9999;
			
			//利用PID实现电机1的跟随转动
			if (PIDControl>=1)
			{
				PIDControl=0;
				
				error2_loc= error1_loc;
        error1_loc= error0_loc;
        error0_loc= Location2_Target-Location1_Actually;
            
        // 增量式PID
        Out2+=kp_loc*(error0_loc- error1_loc)+ki_loc*error0_loc+kd_loc*(error0_loc-2*error1_loc+error2_loc);
            
        // 输出限幅
        if (Out2>100) Out2=100;
        if (Out2<-100) Out2=-100;
            
        // 执行控制
        Moter_SetPower1(Out2);
			}
			
		}
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