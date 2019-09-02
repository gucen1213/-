#ifndef _KEY_H_
#define _KEY_H_

#include "stm32f10x.h"
#include "system.h"
#include <stdbool.h>


#define IS_KEY1_DOWN()      GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)
           
	   
				
#define IS_KEY1_UP()		GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)                           
				
#define IS_KEY2_UP()		GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)

				
#define IS_KEY2_DOWN()      GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)

				
extern void Key_gpio_init(void);

#endif

