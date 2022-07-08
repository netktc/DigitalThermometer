/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.6.1
  * @date    30-September-2014
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm8l15x.h"
#include "main.h"
#include "stm8l_discovery_lcd.h"
#include "ds18b20.h"
#include "delay.h"

/** @addtogroup STM8L15x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile bool wakeup_flag = FALSE;

u16 display_array[6];
u16 Nbre1Tmp;
u16 Nbre2Tmp;
u32 temp_int;
u16 temp_int_display;

RTC_TimeTypeDef   RTC_TimeStr;
RTC_InitTypeDef   RTC_InitStr;
/* Private function prototypes -----------------------------------------------*/
void GPIO_Initialization(void);
void RTC_Periph_Init(void);
void RTC_restart(void);
void display_temp(TemperatureTypeDef* Temperature);
void delay_1us(u16 n_1us);

/* Private functions ---------------------------------------------------------*/
void GPIO_Initialization(void)
{
  /* Push-button initialization */
  GPIO_Init(BUTTON_GPIO_PORT, FUNCTION_GPIO_PIN, GPIO_Mode_In_FL_IT);
  /* Led ports initialization */
  GPIO_Init(LED_GR_PORT, LED_GR_PIN, GPIO_Mode_Out_PP_High_Fast);
  GPIO_Init(LED_BL_PORT, LED_BL_PIN, GPIO_Mode_Out_PP_Low_Fast);
  /* PC0 in output push-pull low because never used by the application */
  //GPIO_Init(GPIOC, GPIO_Pin_0, GPIO_Mode_Out_PP_Low_Slow);

  EXTI->CR1 = 0x00; /* PC1 (push-button) ext. interrupt to falling edge low level */
}

void RTC_Periph_Init(void)
{
#ifdef USE_LSE
  CLK_RTCClockConfig(CLK_RTCCLKSource_LSE, CLK_RTCCLKDiv_1);
#else
  CLK_RTCClockConfig(CLK_RTCCLKSource_LSI, CLK_RTCCLKDiv_1);
#endif

  /* Configures the RTC wakeup timer_step = RTCCLK/16 = LSE/16 = 488.28125 us */
  RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);
  /* Enable wake up unit Interrupt */
  RTC_ITConfig(RTC_IT_WUT, ENABLE);
  RTC_SetWakeUpCounter(100);
  RTC_WakeUpCmd(ENABLE);
  RTC_InitStr.RTC_AsynchPrediv = 0x7f;
  RTC_InitStr.RTC_SynchPrediv = 0x00ff;
  RTC_Init(&RTC_InitStr);
}

static void CLK_Config(void)
{
  /*High speed internal clock prescaler:1*/
  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
  
  /*Enable RTC CLK*/
  CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);
  
  /*Enable TIM4 CLK*/
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM4,ENABLE);
}

void RTC_restart(void)
{
  RTC_SetWakeUpCounter(100);
  RTC_WakeUpCmd(ENABLE);
}

void display_temp(TemperatureTypeDef* Temperature)
{
  uint8_t dummy = 0x01;
  
  if (Temperature->sign == 1)
    display_array[0] = '-';
  else if (Temperature->rawT == 0)
    display_array[0] = dummy;
  else
    display_array[0] = '+';
  
  temp_int = Temperature->rawT;
  temp_int_display = temp_int * 100 >> 4;
  Nbre1Tmp = temp_int_display / 10000;
  if (Nbre1Tmp == 0)
    display_array[1] = dummy;
  else
    display_array[1] = Nbre1Tmp + 0x30;               //Hundreds
  Nbre1Tmp = temp_int_display - (Nbre1Tmp * 10000);
  Nbre2Tmp = Nbre1Tmp / 1000;
  if (Nbre2Tmp == 0)
    display_array[2] = dummy;
  else
    display_array[2] = Nbre2Tmp + 0x30;               //Tens
  Nbre1Tmp = Nbre1Tmp - (Nbre2Tmp * 1000);
  Nbre2Tmp = Nbre1Tmp / 100;
  display_array[3] = Nbre2Tmp + 0x30;                 //Ones
  display_array[3] |= DOT;
  Nbre1Tmp = Nbre1Tmp - (Nbre2Tmp * 100);
  Nbre2Tmp = Nbre1Tmp / 10;
  display_array[4] = Nbre2Tmp + 0x30;                 //One decimal places
  Nbre1Tmp = Nbre1Tmp - (Nbre2Tmp * 10);
  display_array[5] = Nbre1Tmp + 0x30;                 //Two decimal places
  LCD_GLASS_DisplayStrDeci(display_array);
}

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
void main(void)
{
  ErrorStatus flag_init;
  TemperatureTypeDef Temperature = {0, 0, 1, ERROR};
  
  GPIO_Initialization();
  LCD_GLASS_Init();
  CLK_Config();
  RTC_Periph_Init();
  enableInterrupts();
  
  DS18B20_Reset();
  flag_init = DS18B20_Check();
  
  if(flag_init == ERROR)
  {
    //printf("\n\rInit ERROR! Please Reset!\n\r");
    while(1);
  }
  else 
    //printf("\n\rInit SUCCESS!\n\r");
  
  /* Infinite loop */
  while (1)
  {
    DS18B20_GetTemperature(&Temperature);
    display_temp(&Temperature);
    RTC_restart();
    halt();
    //delay_ms(10);
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/