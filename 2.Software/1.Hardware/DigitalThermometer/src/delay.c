/** 
  ******************************************************************************
  * @file    delay.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    29-Oct-2012
  * @brief   delay functions.
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
#include "delay.h"

#define SYS_CLK_HSI_DIV1

/**
  * @brief _delay_us
  * @param nCount
  * @retval None
  */
void _delay_us(uint16_t nCount)
{
    nCount *= 3; 
    while(--nCount);
}

/**
  * @brief _delay_ms
  * @param nCount
  * @retval None
  */
void _delay_ms(uint16_t nCount)
{
    while(nCount--)
    {
        _delay_us(1000);
    }
}

/** @brief  Inserts a delay time.
  * @param  nCount: specifies the delay time length.
  * @retval None
  */
void Delay(__IO uint16_t nCount)
{
  for (; nCount != 0; nCount--);
}

//------------------------------------------------------------------------------
// Function Name : delay_ms
// Description   : delay for some time in ms unit(partial accurate)
// Input         : n_ms is how many ms of time to delay
//------------------------------------------------------------------------------
void delay_ms(u16 n_ms)
{
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);
  // Init TIMER 4 //
  // prescaler: / (2^6) = /64
  TIM4->PSCR = 6;
#ifdef SYS_CLK_HSE_DIV1
  // Auto-Reload value: 16M / 64 = 1/4M, 1/4M / 1k = 250
  TIM4->ARR = 250;
#else
#ifdef SYS_CLK_HSE_DIV2
  // Auto-Reload value: 16M/2 / 64 = 1/8M, 1/8M / 1k = 125
  TIM4->ARR = 125;
#else
#ifdef SYS_CLK_HSI_DIV1
  // Auto-Reload value: 16M / 64 = 1/4M, 1/4M / 1k = 250
  TIM4->ARR = 250;
#else
#ifdef SYS_CLK_HSI_DIV2
  // Auto-Reload value: 16M/2 / 64 = 1/8M, 1/8M / 1k = 125
  TIM4->ARR = 125;
#else
#error "either SYS_CLK_HSE_DIV1/DIV2 or SYS_CLK_HSI_DIV1/DIV2 should be defined"
#endif
#endif
#endif
#endif
  // Counter value: 2, to compensate the initialization of TIMER
  TIM4->CNTR = 2;

  // clear update flag
  TIM4->SR1 &= ~TIM4_SR1_UIF;
  // Enable Counter
  TIM4->CR1 |= TIM4_CR1_CEN;

  while (n_ms--)
  {
    while ((TIM4->SR1 & TIM4_SR1_UIF) == 0) ;
    TIM4->SR1 &= ~TIM4_SR1_UIF;
  }

  // Disable Counter
  TIM4->CR1 &= ~TIM4_CR1_CEN;
}

//------------------------------------------------------------------------------
// Function Name : delay_10us
// Description   : delay for some time in 10us unit(partial accurate)
// Input         : n_10us is how many 10us of time to delay
//------------------------------------------------------------------------------
void delay_1us(u16 n_1us)
{
  //CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);
  // Init TIMER 4 //
  // prescaler: / (2^0) = /1
  TIM4->PSCR = 0;
#ifdef SYS_CLK_HSE_DIV1
  // Auto-Reload value: 16M / 1 = 16M, 16M / 100k = 160
  TIM4->ARR = 160;
#else
#ifdef SYS_CLK_HSE_DIV2
  // Auto-Reload value: 16M/2 / 1 = 8M, 8M / 100k = 80
  TIM4->ARR = 80;
#else
#ifdef SYS_CLK_HSI_DIV1
  // Auto-Reload value: 16M / 1 = 16M, 16M / 100k = 160
  TIM4->ARR = 15;
#else
#ifdef SYS_CLK_HSI_DIV2
  // Auto-Reload value: 16M/2 / 1 = 8M, 8M / 100k = 80
  TIM4->ARR = 80;
#else
#error "either SYS_CLK_HSE_DIV1/DIV2 or SYS_CLK_HSI_DIV1/DIV2 should be defined"
#endif
#endif
#endif
#endif
  // Counter value: 10, to compensate the initialization of TIMER
  TIM4->CNTR = 15;

  // clear update flag
  TIM4->SR1 &= ~TIM4_SR1_UIF;
  // Enable Counter
  TIM4->CR1 |= TIM4_CR1_CEN;

  while (n_1us--)
  {
    while ((TIM4->SR1 & TIM4_SR1_UIF) == 0) ;
    TIM4->SR1 &= ~TIM4_SR1_UIF;
  }

  // Disable Counter
  TIM4->CR1 &= ~TIM4_CR1_CEN;
  //CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, DISABLE);
}

/**
  * @brief  delay for some time in 10us unit(partial accurate)
  * @caller auto_test
  * @param n_10us is how many 10us of time to delay
  * @retval None
  */
void delay_10us(u16 n_10us)
{
/* Init TIMER 4 */
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);

/* prescaler: / (2^0) = /1 */
  TIM4->PSCR = 0;

/* SYS_CLK_HSI_DIV1 Auto-Reload value: 16M / 1 = 16M, 16M / 100k = 160 */
  TIM4->ARR = 160;

/* Counter value: 10, to compensate the initialization of TIMER */
  TIM4->CNTR = 10;

/* clear update flag */
  TIM4->SR1 &= ~TIM4_SR1_UIF;

/* Enable Counter */
  TIM4->CR1 |= TIM4_CR1_CEN;

  while(n_10us--)
  {
    while((TIM4->SR1 & TIM4_SR1_UIF) == 0) ;
    TIM4->SR1 &= ~TIM4_SR1_UIF;
  }

/* Disable Counter */
  TIM4->CR1 &= ~TIM4_CR1_CEN;
 CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, DISABLE);

}
