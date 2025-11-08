#include "stm32f10x.h"                  // Device header
#include "Delay.h"

uint8_t KeyNum=1; //用于切换考核题目

void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure_A;
	GPIO_InitStructure_A.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStructure_A.GPIO_Pin=GPIO_Pin_0;
	GPIO_InitStructure_A.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure_A);

}

uint8_t Key_GetNum(void)
{
	static uint8_t KeyNum = 1;
	static uint8_t Time_Key;
	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0)
	{
		Time_Key++;
		if (Time_Key>=2)
		{
		Time_Key=0;
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0);
		KeyNum = !KeyNum;
		}
	}
	
	return KeyNum;
}

