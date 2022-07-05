/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm8l15x.h"

/* Private define ------------------------------------------------------------*/
#define BUTTON_GPIO_PORT              GPIOC
#define FUNCTION_GPIO_PIN             GPIO_Pin_1

#define LED_GR_PORT              GPIOE
#define LED_GR_PIN               GPIO_Pin_7

#define LED_BL_PORT              GPIOC
#define LED_BL_PIN               GPIO_Pin_7

/* Private macro -------------------------------------------------------------*/
#define GPIO_HIGH(a,b) a->ODR!=b
#define GPIO_LOW(a,b) a->ODR&=~b
#define GPIO_TOGGLE(a,b) a->ODR^=b

#endif /* __MAIN_H*/