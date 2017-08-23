#ifndef __LED_H
#define	__LED_H

#define ON  0
#define OFF 1

#define LED1(a)	if (a)	\
					GPIO_SetBits(GPIOG,GPIO_Pin_6);\
					else		\
					GPIO_ResetBits(GPIOG,GPIO_Pin_6)

#define LED2(a)	if (a)	\
					GPIO_SetBits(GPIOB,GPIO_Pin_1);\
					else		\
					GPIO_ResetBits(GPIOB,GPIO_Pin_1)


void Led_GPIO_Config(void);
#endif


