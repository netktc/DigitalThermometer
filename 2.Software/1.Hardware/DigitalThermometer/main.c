/**
  ******************************************************************************
  * @file    main.c
  * @author  netktc
  * @version V1.0.1
  * @date    12-July-2022
  * @brief   Main program body
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm8l15x.h"
#include "main.h"
#include "stm8l_discovery_lcd.h"
#include "ds18b20.h"
#include "delay.h"
#include "stdio.h"
#include "usart.h"

/** @addtogroup STM8L15x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile bool startup_flag = FALSE;

volatile bool User_Key_Pressed = FALSE;
volatile bool FORCED_CALIB = FALSE;
volatile u16 Sec2_count = 0;

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

#define PUTCHAR_PROTOTYPE int putchar (int c)
#define GETCHAR_PROTOTYPE int getchar (void)

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

  EXTI->CR1 = 0x04; /* PC1 (push-button) ext. interrupt to falling edge low level */
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
  
  /*Enable USART1 CLK*/
  CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);
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
  TemperatureTypeDef Temperature = {0, 0, 0, 0, ERROR};
  RomCodeTypeDef RomCode;
  
  GPIO_Initialization();
  LCD_GLASS_Init();
  CLK_Config();
  USART_Config();
  RTC_Periph_Init();
  
  
  enableInterrupts();
  
  DS18B20_Reset();
  flag_init = DS18B20_Check();
  
  if(flag_init == ERROR)
  {
    LCD_GLASS_DisplayString(" ERROR");
    printf("\n\rInit ERROR! Please Reset!\n\r");
    while(1);
  }
  else
  {
    LCD_GLASS_DisplayString(" SUCCESS");
    printf("\n\rInit SUCCESS!\n\r");
  }
  delay_ms(500);
  
  DS18B20_GetRoomCode(&RomCode);
  
  /* Infinite loop */
  while (1)
  {
    if (startup_flag == TRUE)
    {
      DS18B20_GetTemperature(&Temperature);
      display_temp(&Temperature);
    }
    else
      LCD_GLASS_DisplayString(" STOP ");
    
    RTC_restart();
    delay_ms(100);
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