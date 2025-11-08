#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>

char Serial_RxPacket[100];
uint8_t Serial_RxFlag;

void Serial_Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate=9600;
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode=USART_Mode_Tx|USART_Mode_Rx;
	USART_InitStructure.USART_Parity=USART_Parity_No;
	USART_InitStructure.USART_StopBits=USART_StopBits_1;
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;
	USART_Init(USART1,&USART_InitStructure);
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);			
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			
	
	NVIC_InitTypeDef NVIC_InitStructure;					
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;       
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;        
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;		
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;      
	NVIC_Init(&NVIC_InitStructure);                        
	
	USART_Cmd(USART1,ENABLE);
	
}


//串口单字节发送函数
void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART1,Byte);
	while (USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
}


//串口数组发送函数，逐个发送数组内数据
void Serial_SendArray(uint8_t *Array,uint16_t Length)
{
	uint16_t i;
	for (i=0;i<Length;i++)
	{
		Serial_SendByte(Array[i]);
	}
}


//串口字符串发送函数，通过遍历字符串发送
void Serial_SendString(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)
	{
		Serial_SendByte(String[i]);
	}
}


//将输入的字符转给此函数并通过串口打印
int fputc(int ch, FILE *f)
{
	Serial_SendByte(ch);
	return ch;
}


//自定义可变参数串口打印函数
void Serial_Printf(char *format, ...) //可输出格式化字符串以及一系列不确定数据
{
	char String[100];
	va_list arg;   //定义一个可变参数指针
	va_start(arg, format);   //初始化arg，使它指向第一个可变参数
	vsprintf(String, format, arg);  //将可变参数写入string
	va_end(arg);   //结束访问
	Serial_SendString(String);  //串口发送
}


void USART1_IRQHandler(void)
{
    static uint8_t RxState = 0;
    static uint8_t pRxPacket = 0;
    
    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {
        uint8_t RxData = USART_ReceiveData(USART1);
        
        if (RxData == '\r' || RxData == '\n')  // 收到回车或换行符结束
        {
            Serial_RxPacket[pRxPacket] = '\0';  // 添加字符串结束符
            Serial_RxFlag = 1;
            pRxPacket = 0;
        }
        else if (pRxPacket < 99)  // 防止数组越界
        {
            Serial_RxPacket[pRxPacket] = RxData;
            pRxPacket++;
        }
        else
        {
            pRxPacket = 0;  // 数据包过长，重置
        }
        
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}
                                                                                       