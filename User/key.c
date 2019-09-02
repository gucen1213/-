#include "key.h"

void Key_gpio_init(void)
{
		/*key1 PA0*/
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
		/*key2 PC13*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}





