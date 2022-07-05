/** 
  ******************************************************************************
  * @file    main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    29-Oct-2012
  * @brief   Main program body.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
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
#include <math.h>

/**
  * @addtogroup LCD_Example1
  * @{
  */
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

//#define ADVANCED_MODE
#define T90 90.0
#define Tambient 25.0

#define ADC_RESOLUTION_12BIT 0.7277 /* (Vref+ / 4096) */

#define FACT_CALIB_ADD  ((u8 *)0x4911)
#define USER_CALIB_ADD1 ((u8 *)0x1000)
#define USER_CALIB_ADD2 ((u8 *)0x1001)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u8 *P_FACT_CALIB_ADD;
u8 *P_USER_CALIB_ADD1;
u8 *P_USER_CALIB_ADD2;

volatile bool wakeup_flag = FALSE;
volatile bool User_Key_Pressed = FALSE;
volatile bool FORCED_CALIB = FALSE;

volatile u16 value[NUM_SAM];
volatile u16 res;
volatile u16 temp_res;
volatile u8 samp_counter;
volatile u8 ADC_data_ready;
volatile u16 Sec2_count = 0;

float temp;
float AVG_SLOPE;

extern u8 t_bar[2];

u16 *P_USER_CALIB_ADD;
u16 VAL_T90;
u16 VAL_Tambient;
u16 temp_T90;
u16 display_array[6];
u8 Nbre1Tmp;
u8 Nbre2Tmp;
u8 state;
s16 temp_int;
s16 temp_max = 0;
s16 temp_min = 900;
s16 new_temp;
s16 prev_temp = 290;//290;
s16 temp_after_5min;
s16 temp_before_5min;
u16 min5_counter;
u16 temp_int_display;

RTC_TimeTypeDef   RTC_TimeStr;
RTC_InitTypeDef   RTC_InitStr;

/* Private function prototypes -----------------------------------------------*/
void GPIO_Initialization(void);
void ADC_Periph_Init(void);
void RTC_Periph_Init(void);
void RTC_restart(void);
void display_temp(void);
void user_calib(void);
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
  GPIO_Init(GPIOC, GPIO_Pin_0, GPIO_Mode_Out_PP_Low_Slow);

  EXTI->CR1 = 0x00; /* PC1 (push-button) ext. interrupt to falling edge low level */
}

void ADC_Periph_Init(void)
{
  u8 idx;
  CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, ENABLE);
  ADC1->CR1 |= 0x01; //ADON
  ADC1->TRIGR[0] = 0x20; //TSON
  for (idx = 0;idx < 50;idx++);
  ADC_Init(ADC1, ADC_ConversionMode_Single,
           ADC_Resolution_12Bit, ADC_Prescaler_1);
  ADC_SamplingTimeConfig(ADC1, ADC_Group_FastChannels, ADC_SamplingTime_192Cycles);
  ADC_ChannelCmd(ADC1, ADC_Channel_TempSensor, ENABLE);
  ADC1->SQR[0] |= 0x80; //DMA disable
}

void RTC_Periph_Init(void)
{
  CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);
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

void RTC_restart(void)
{
  RTC_SetWakeUpCounter(100);
  RTC_WakeUpCmd(ENABLE);
}
void display_temp(void)
{
  if (temp_int < 1000)
  {
    if (temp_int < 0)
    {
      display_array[1] = '-';
      temp_int_display = 0 - temp_int;
    }
    else
    {
      display_array[1] = '+';
      temp_int_display = temp_int;
    }
    Nbre1Tmp = temp_int_display / 100;
    display_array[2] = Nbre1Tmp + 0x30;
    Nbre1Tmp = temp_int_display - (Nbre1Tmp * 100);
    Nbre2Tmp = Nbre1Tmp / 10;
    display_array[3] = Nbre2Tmp + 0x30;

    Nbre1Tmp = Nbre1Tmp - (10 * Nbre2Tmp);
    display_array[4] = Nbre1Tmp + 0x30;
    display_array[5] = 'C';
    display_array[3] |= DOT;
    LCD_GLASS_DisplayStrDeci(display_array);
  }
}

void user_calib(void)
{
  temp_res = (u16) ((0x03 << 8) | *P_FACT_CALIB_ADD);
  temp_T90 = temp_res * ADC_RESOLUTION_12BIT;
  if (temp_T90 >= 580 &&  temp_T90 <= 597)
  {
    VAL_T90 = temp_T90;
    res = 0;
    ADC_TempSensorCmd(ENABLE);
    delay_1us(15); // TS startup time
    for (samp_counter = 0;samp_counter < NUM_SAM;samp_counter++)
    {
      ADC_SoftwareStartConv(ADC1);
      while (!(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)));
      value[samp_counter] = ADC_GetConversionValue(ADC1);
      res += value[samp_counter];
    }
    res = res / NUM_SAM;
    VAL_Tambient = res * ADC_RESOLUTION_12BIT;

    FLASH->DUKR = 0xAE;
    FLASH->DUKR = 0x56;
    while (!(FLASH->IAPSR & 0x08)); //wait for protection ON

    *P_USER_CALIB_ADD2 = (u8)(VAL_Tambient & 0xff); //LSB
    while ((FLASH->IAPSR & 0x04) == 0); //wait for EOP

    *P_USER_CALIB_ADD1 = (u8)((VAL_Tambient >> 8) & 0xff);//MSB
    while ((FLASH->IAPSR & 0x04) == 0); //wait for EOP

    AVG_SLOPE = (float) ((VAL_T90 - VAL_Tambient) / (T90 - Tambient));
    display_array[0] = 'C';
    display_array[1] = 'A';
    display_array[2] = 'L';
    display_array[3] = '-';
    display_array[4] = 'O';
    display_array[5] = 'K';
    LCD_GLASS_DisplayStrDeci(display_array);
    while (User_Key_Pressed != TRUE);
    User_Key_Pressed = FALSE;
    samp_counter = 0;
    res = 0;
  }
  else
  {
    VAL_T90 = 597;
    AVG_SLOPE = 1.62;
    display_array[0] = 'C';
    display_array[1] = 'A';
    display_array[2] = 'L';
    display_array[3] = 'E';
    display_array[4] = 'R';
    display_array[5] = 'R';
    LCD_GLASS_DisplayStrDeci(display_array);
    while (User_Key_Pressed != TRUE);
    User_Key_Pressed = FALSE;
    state = 0;
  }
}

/**
  * @brief main entry point.
  * @par Parameters:
  * None
  * @retval void None
  * @par Required preconditions:
  * None
  */
void main(void)
{
  res = 0;
  samp_counter = 0;
  state = 0;
  ADC_data_ready = FALSE;

  P_FACT_CALIB_ADD  = FACT_CALIB_ADD;
  P_USER_CALIB_ADD1 = USER_CALIB_ADD1;
  P_USER_CALIB_ADD2 = USER_CALIB_ADD2;

  CLK->CKDIVR = 0x00; // Fcpu = 16MHz

  GPIO_Initialization();
  LCD_GLASS_Init();
  ADC_Periph_Init();
  RTC_Periph_Init();
  enableInterrupts();

#ifdef ADVANCED_MODE

  if (*P_FACT_CALIB_ADD != 0xff || *P_FACT_CALIB_ADD != 0x00)
  {
    GPIO_SetBits(LED_BL_PORT, LED_BL_PIN); /* Switch-on blue led */
    *P_USER_CALIB_ADD = (*P_USER_CALIB_ADD1 << 8) & 0xff00;
    *P_USER_CALIB_ADD |= *P_USER_CALIB_ADD2;
    if (*P_USER_CALIB_ADD == 0x0000 || *P_USER_CALIB_ADD == 0xffff)// Calibration not done before
    {
      user_calib();
    }
    else
    {
      VAL_Tambient = (*P_USER_CALIB_ADD1 << 8) & 0xff00;
      VAL_Tambient |= *P_USER_CALIB_ADD2;
      temp_res = (u16) ((0x03 << 8) | *P_FACT_CALIB_ADD);
      temp_T90 = temp_res * ADC_RESOLUTION_12BIT;
      if (temp_T90 >= 580 &&  temp_T90 <= 614)
      {
        VAL_T90 = temp_T90;
        AVG_SLOPE = (float) ((VAL_T90 - VAL_Tambient) / (T90 - Tambient));
      }
      else
      {
        VAL_T90 = 597;
        AVG_SLOPE = 1.62;
      }
    }
  }
  else
  {
    VAL_T90 = 597;
    AVG_SLOPE = 1.62;
  }
#else /* Normal mode */
  /* Switch-off blue led */
  GPIO_ResetBits(LED_BL_PORT, LED_BL_PIN);
  /* V90 and Avg_slope typical value of the datasheet */
  VAL_T90 = 597;
  AVG_SLOPE = 1.62;
#endif

  halt();

  
	
	/* main infinite loop */
  while (1)
  {
#ifdef ADVANCED_MODE

    if (FORCED_CALIB == TRUE)
    {
      FORCED_CALIB = FALSE;
      display_array[0] = 'C';
      display_array[1] = 'A';
      display_array[2] = 'L';
      display_array[3] = '.';
      display_array[4] = '.';
      display_array[5] = '.';
      LCD_GLASS_DisplayStrDeci(display_array);
      while (User_Key_Pressed != TRUE);
      User_Key_Pressed = FALSE;
      if (*P_FACT_CALIB_ADD != 0xff || *P_FACT_CALIB_ADD != 0x00)
      {
        user_calib();
      }
      else
      {
        VAL_T90 = 597;
        AVG_SLOPE = 1.62;
        display_array[0] = 'C';
        display_array[1] = 'A';
        display_array[2] = 'L';
        display_array[3] = 'E';
        display_array[4] = 'R';
        display_array[5] = 'R';
        LCD_GLASS_DisplayStrDeci(display_array);
        while (User_Key_Pressed != TRUE);
        User_Key_Pressed = FALSE;
        state = 0;
      }
    }
#endif

    if (wakeup_flag == TRUE)
    {
      GPIO_ToggleBits(LED_GR_PORT, LED_GR_PIN);
      wakeup_flag = FALSE;
      delay_1us(25); // TS startup time
      ADC_SoftwareStartConv(ADC1);
      while (!(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)));
      value[samp_counter] = ADC_GetConversionValue(ADC1);
      res += value[samp_counter];
      if (++samp_counter >= NUM_SAM)
      {
        samp_counter = 0;
        ADC_data_ready = FALSE;
        res = res / NUM_SAM;
        temp = res * ADC_RESOLUTION_12BIT;
        if (temp > VAL_T90) // temp >90C
        {
          temp = (temp - VAL_T90) / AVG_SLOPE;
          temp = 90 + temp;
        }
        else // temp < 90C
        {
          temp = (VAL_T90 - temp) / AVG_SLOPE;
          temp = 90 - temp;
        }
        res = 0;
        temp_int = temp * 10;

        new_temp = temp_int;
        if (min5_counter++ >= 100) // 5 minutes
        {
          min5_counter = 0;
          temp_after_5min = new_temp;
          if (temp_after_5min > temp_before_5min)
          {
            BAR0_OFF;
            BAR1_OFF;
            BAR2_OFF;
            BAR3_ON;
          }
          else if (temp_after_5min < temp_before_5min)
          {
            BAR0_ON;
            BAR1_OFF;
            BAR2_OFF;
            BAR3_OFF;
          }
          else
          {
            BAR0_OFF;
            BAR1_OFF;
            BAR2_OFF;
            BAR3_OFF;
          }
          temp_before_5min = temp_after_5min;
        }
        if (new_temp > temp_max)
          temp_max = new_temp;
        else if (new_temp < temp_min)
          temp_min = new_temp;

        if (new_temp > prev_temp)
        {
          if (new_temp - prev_temp >= 5 && new_temp - prev_temp < 10)
          {
            temp_int = prev_temp + 1;
          }
          else if (new_temp - prev_temp < 5)
            temp_int = prev_temp;
        }
        else
        {
          if (prev_temp - new_temp >= 5 && prev_temp - new_temp < 10)
          {
            temp_int = prev_temp - 1;
          }
          else if (prev_temp - new_temp < 5)
            temp_int = prev_temp;
        }
        prev_temp = temp_int;
      }
    }

    /* state-machine for push-button management */
    switch (state)
    {
      case 0:
        if (User_Key_Pressed == TRUE)
        {
          state = 1;
          User_Key_Pressed = FALSE;
          temp_int = temp_max;
          display_array[0] = 'M';
        }
        else
          display_array[0] = 'T';
        break;
      case 1:
        if (User_Key_Pressed == TRUE)
        {
          state = 2;
          User_Key_Pressed = FALSE;
          temp_int = temp_min;
          display_array[0] = 'm';
        }
        else
        {
          display_array[0] = 'M';
          temp_int = temp_max;
        }
        break;
      case 2:
        if (User_Key_Pressed == TRUE)
        {
          state = 0;
          User_Key_Pressed = FALSE;
          display_array[0] = 'T';
        }
        else
        {
          display_array[0] = 'm';
          temp_int = temp_min;
        }
        break;
      default:
        break;
    }

    display_temp();
    RTC_restart();
    halt();
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
  {}
}
#endif

/**
  * @}
  */

/******************* (C) COPYRIGHT 1009 STMicroelectronics *****END OF FILE****/
