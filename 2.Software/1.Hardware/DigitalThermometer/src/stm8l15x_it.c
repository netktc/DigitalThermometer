/** 
  ******************************************************************************
  * @file    stm8l15x_it.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    29-Oct-2012
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all peripherals interrupt service routine.
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
#include "stm8l15x_it.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern volatile bool startup_flag;
//extern volatile u16 value[NUM_SAM];
//extern volatile u16 res;
//extern volatile u8 samp_counter;
//extern volatile u8 ADC_data_ready;
extern volatile bool User_Key_Pressed;
extern volatile bool FORCED_CALIB;
extern volatile u16 Sec2_count;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

/** @addtogroup IT_Functions
  * @{
  */
INTERRUPT_HANDLER(NonHandledInterrupt, 0)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}
/**
  * @brief TRAP interrupt routine
  * @par Parameters:
  * None
  * @retval
  * None
*/
INTERRUPT_HANDLER_TRAP(TRAP_IRQHandler)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}
/**
  * @brief FLASH Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(FLASH_IRQHandler, 1)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}
/**
  * @brief DMA1 channel0 and channel1 Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(DMA1_CHANNEL0_1_IRQHandler, 2)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}
/**
  * @brief DMA1 channel2 and channel3 Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(DMA1_CHANNEL2_3_IRQHandler, 3)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}
/**
  * @brief RTC Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */

INTERRUPT_HANDLER(RTC_IRQHandler, 4)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
//  while (1);

  if (((RTC->ISR2) & 0x4) == 0x4)
  {
    RTC->ISR1 = 0x0;
    RTC->ISR2 = 0x0;
    //ADC_TempSensorCmd(ENABLE);
    //wakeup_flag = TRUE;
  }

  return;
}
/**
  * @brief External IT PORTE/F and PVD Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(EXTIE_F_PVD_IRQHandler, 5)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}

/**
  * @brief External IT PORTB Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(EXTIB_IRQHandler, 6)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}

/**
  * @brief External IT PORTD Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(EXTID_IRQHandler, 7)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}

/**
  * @brief External IT PIN0 Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */

INTERRUPT_HANDLER(EXTI0_IRQHandler, 8)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}
/**
  * @brief External IT PIN1 Interrupt routine.
  *   On User button pressed:
  *   Check if button presse a long time (4-5 sec.) if yes --> Set Autotest
  *   Else update status_machine to pass to next measuremetn.
  *   Update the LCD bar graph
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(EXTI1_IRQHandler, 9)
{
  disableInterrupts();
  
  if (User_Key_Pressed == TRUE)
  {/*
    if (Sec2_count++ >= 50000) //~2 sec
    {
      FORCED_CALIB = TRUE;
      User_Key_Pressed = FALSE;
      Sec2_count = 0;
    }*/
    startup_flag = FALSE;
    User_Key_Pressed = FALSE;
  }
  else
  {
    startup_flag = TRUE;
    Sec2_count = 0;
    User_Key_Pressed = TRUE;
  }

  EXTI_ClearITPendingBit(EXTI_IT_Pin1);
  enableInterrupts();
  //return;
}

/**
  * @brief External IT PIN2 Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */

INTERRUPT_HANDLER(EXTI2_IRQHandler, 10)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}

/**
  * @brief External IT PIN3 Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(EXTI3_IRQHandler, 11)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}

/**
  * @brief External IT PIN4 Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(EXTI4_IRQHandler, 12)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}

/**
  * @brief External IT PIN5 Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(EXTI5_IRQHandler, 13)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}

/**
  * @brief External IT PIN6 Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(EXTI6_IRQHandler, 14)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  //while (1);
  return;
}

/**
  * @brief External IT PIN7 Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(EXTI7_IRQHandler, 15)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */

  while (1);

}
/**
  * @brief LCD start of new frame Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(LCD_IRQHandler, 16)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}
/**
  * @brief CLK switch/CSS/TIM1 break Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(SWITCH_CSS_BREAK_DAC_IRQHandler, 17)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}

/**
  * @brief ADC1/Comparator Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(ADC1_COMP_IRQHandler, 18)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  //while (1);
  /*value[samp_counter] = ADC_GetConversionValue(ADC1);
  res += value[samp_counter];
  if (++samp_counter >= NUM_SAM)
  {
    samp_counter = 0;
    ADC_data_ready = TRUE;
  }
  return;*/
}

/**
  * @brief TIM2 Update/Overflow/Trigger/Break Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(TIM2_UPD_OVF_TRG_BRK_IRQHandler, 19)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}

/**
  * @brief Timer2 Capture/Compare Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(TIM2_CAP_IRQHandler, 20)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}


/**
  * @brief Timer3 Update/Overflow/Trigger/Break Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(TIM3_UPD_OVF_TRG_BRK_IRQHandler, 21)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}
/**
  * @brief Timer3 Capture/Compare Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(TIM3_CAP_IRQHandler, 22)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}
/**
  * @brief TIM1 Update/Overflow/Trigger/Commutation Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(TIM1_UPD_OVF_TRG_COM_IRQHandler, 23)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}
/**
  * @brief TIM1 Capture/Compare Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(TIM1_CAP_IRQHandler, 24)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}

/**
  * @brief TIM4 Update/Overflow/Trigger Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(TIM4_UPD_OVF_TRG_IRQHandler, 25)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}
/**
  * @brief SPI1 Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(SPI1_IRQHandler, 26)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}

/**
  * @brief USART1 TX Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(USART1_TX_IRQHandler, 27)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}

/**
  * @brief USART1 RX Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(USART1_RX_IRQHandler, 28)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);

}

/**
  * @brief I2C1 Interrupt routine.
  * @par Parameters:
  * None
  * @retval
  * None
  */
INTERRUPT_HANDLER(I2C1_IRQHandler, 29)
{
  /* In order to detect unexpected events during development,
     it is recommended to set a breakpoint on the following instruction.
  */
  while (1);
}

/**
  * @}
  */
/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/

