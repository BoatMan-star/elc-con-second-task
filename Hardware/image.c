#include "stm32f10x.h"                  // Device header
#include "Serial.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "OLED.h"

extern int16_t Speed_Target;

extern int16_t Speed1;
extern int16_t Speed2;

void Image_Control(void)
{
	OLED_ShowString(1,1,"Speed_Target:   ");
	OLED_ShowString(2,1,"Speed1:         ");
	OLED_ShowString(3,1,"Speed2:         ");
	OLED_ShowSignedNum(1,14,Speed_Target,2);
	OLED_ShowSignedNum(2,8,Speed1,2);
	OLED_ShowSignedNum(3,8,Speed2,2);
	Serial_Printf("%d,%d,%d\n",Speed_Target,Speed1,Speed2);
}
