#include "stm8l15x.h"

/* Private function prototypes -----------------------------------------------*/
#define PUTCHAR_PROTOTYPE int putchar (int c)
#define GETCHAR_PROTOTYPE int getchar (void)

void USART_Config(void)
{
  //USART1_TX mapped on PA2 and USART1_RX mapped on PA3
  SYSCFG->RMPCR1 |= 0X10;
  
  USART_DeInit(USART1);
  USART_Init(USART1, (uint32_t)115200, USART_WordLength_8b, USART_StopBits_1,
                   USART_Parity_No, (USART_Mode_TypeDef)(USART_Mode_Tx | USART_Mode_Rx));
  USART_Cmd(USART1,ENABLE);
  GPIO_ExternalPullUpConfig(GPIOA, GPIO_Pin_2|GPIO_Pin_3, ENABLE);
}

/**
  * @brief Retargets the C library printf function to the USART.
  * @param[in] c Character to send
  * @retval char Character sent
  * @par Required preconditions:
  * - None
  */
PUTCHAR_PROTOTYPE
{
  /* Write a character to the USART */
  USART_SendData8(USART1, c);
  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

  return (c);
}
/**
  * @brief Retargets the C library scanf function to the USART.
  * @param[in] None
  * @retval char Character to Read
  * @par Required preconditions:
  * - None
  */
GETCHAR_PROTOTYPE
{
  int c = 0;
  /* Loop until the Read data register flag is SET */
  while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
    c = USART_ReceiveData8(USART1);
    return (c);
}