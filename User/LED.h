#ifndef __LED_H_
#define __LED_H_

#include "stm32f10x.h"

void led_gpio_init(void);

#define  LED1_ON()    GPIO_SetBits(GPIOB,GPIO_Pin_0)
#define  LED1_OFF()   GPIO_ResetBits(GPIOB,GPIO_Pin_0)

#define  LED2_ON()	  GPIO_ResetBits(GPIOF,GPIO_Pin_7)
#define  LED2_OFF()   GPIO_SetBits(GPIOF,GPIO_Pin_7)

#endif
