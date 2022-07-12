/**
  ******************************************************************************
  * @file    ds18b20.c
  * @author  netktc
  * @version V1.0.1
  * @date    8-July-2022
  * @brief   DS18B20 program
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ds18b20.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
  
/* Private functions ---------------------------------------------------------*/
  /*------------------------------------------------------------------------*/
  /* ���͸�λds18b20оƬ�ź�                                                */
  /*------------------------------------------------------------------------*/

void DS18B20_Reset(void)
{
    DS18B20_DQ_OUT;
    DS18B20_DQ_LOW;
    delay_1us(RESET_PULSE_TIMESLOT);
    DS18B20_DQ_HIGH;
    delay_1us(20);
}

  /*------------------------------------------------------------------------*/
  /* ����������                                                           */
  /*------------------------------------------------------------------------*/

ErrorStatus DS18B20_Check(void)
{
  uint8_t cnt = 0;
  
  DS18B20_DQ_IN;                        //����DQ�ܽ�Ϊ����ģʽ
  //�ȴ���λ�źŵ͵�ƽ
  while((DS18B20_DQ_STATUS != RESET) && (cnt < WAIT_TIMESLOT))
  {
    cnt++;
    delay_1us(1);
  }
  if(cnt >= WAIT_TIMESLOT)
    return ERROR;
  else
    cnt = 0;
  
  //�ٵȴ���λ�ź����ߣ���ʾ��λ����
  while((DS18B20_DQ_STATUS == RESET) && (cnt < PRESENCE_PULSE_TIMESLOT))
  {
    cnt++;
    delay_1us(1);
  }
  if(cnt >= PRESENCE_PULSE_TIMESLOT)
    return ERROR;
  else
    return SUCCESS;
}

  /*------------------------------------------------------------------------*/
  /* ��DS18B20��һ��λ                                                      */
  /*------------------------------------------------------------------------*/

static BitStatus DS18B20_ReadBit(void)
{
  BitStatus data;
  
  //master device pulling the 1-Wire bus low for a minimum of 1��s
  DS18B20_DQ_OUT;
  DS18B20_DQ_LOW;
  delay_1us(BUS_INIT_TIME);
  //master device releasing the bus
  DS18B20_DQ_IN;
  //After the master initiates the read time slot, the DS18B20 will begin transmitting a 1 or 0 on bus.
  //Output data from the DS18B20 is valid for 15��s after the falling edge that initiated the read time slot.
  delay_1us(RECOVERY_TIME);
  if(DS18B20_DQ_STATUS != RESET)
    data = SET;
  else
    data = RESET;
  delay_1us(WAIT_READ_TIMESLOT);
  return data;
}

  /*------------------------------------------------------------------------*/
  /* ��DS18B20��һ���ֽ�                                                    */
  /*------------------------------------------------------------------------*/

static uint8_t DS18B20_ReadByte(void)
{
  uint8_t t, data;
  uint8_t i;
  
  for(i = 0; i < 8; i++)
  {
    t = DS18B20_ReadBit();
    data = (t << 7) | (data >> 1);
  }
  return data;
}

  /*------------------------------------------------------------------------*/
  /* ��DS18B20дһ��λ                                                      */
  /*------------------------------------------------------------------------*/

void DS18B20_WriteBit(uint8_t data)
{
  //The bus master uses a Write 1 time slot to write a logic 1 to the DS18B20 and a Write 0 time slot to write a logic 0 to the DS18B20.
  //All write time slots must be a minimum of 60��s in duration with a minimum of a 1��s recovery time between individual write slots.
  //Both types of write time slots are initiated by the master pulling the 1-Wire bus low.
  DS18B20_DQ_OUT;
  DS18B20_DQ_LOW;
  delay_1us(BUS_INIT_TIME);
  if (data == 1)
    DS18B20_DQ_IN;
  delay_1us(WAIT_WRITE_TIMESLOT);
  DS18B20_DQ_HIGH;
}

  /*------------------------------------------------------------------------*/
  /* ��DS18B20дһ���ֽ�                                                    */
  /*------------------------------------------------------------------------*/

void DS18B20_WriteByte(uint8_t cmd)
{
  uint8_t bit;
  uint8_t i;
  
  for(i = 0; i < 8; i++)
  {
    bit = cmd & 0x01;
    DS18B20_WriteBit(bit);
    cmd = cmd >> 1;             /* д��һλ */
  }
}

  /*------------------------------------------------------------------------*/
  /* �ɼ��¶�����                                                           */
  /*------------------------------------------------------------------------*/

void DS18B20_GetTemperature(TemperatureTypeDef* Temperature)
{
  uint8_t data_L, data_H;
  uint16_t data;
  
  DS18B20_Reset();
  Temperature->flag = DS18B20_Check();
  if(Temperature->flag == ERROR)
    return;
  DS18B20_WriteByte(SKIP_ROM);
  DS18B20_WriteByte(CONVERT_T);
  
  /* �ȴ��¶Ȳɼ���� */
  //while(DS18B20_ReadByte() != 0xFF);
  
  DS18B20_Reset();
  Temperature->flag = DS18B20_Check();
  if(Temperature->flag == ERROR)
    return;
  DS18B20_WriteByte(SKIP_ROM);
  DS18B20_WriteByte(READ_SCRATCHPAD);
  
  data_L = DS18B20_ReadByte();
  data_H = DS18B20_ReadByte();

#ifdef __UART_H                // ���ڵ��Ե��������
  printf("\n\rTEST DATA: %d %d\n\r", data_L, data_H);
#endif

  data = data_L | (data_H<<8);
  if(data_H > 7)                        //negative number
  {
    Temperature->sign = 1;
    data = (~data) + 1;
  }
  else                                  //positive number
    Temperature->sign = 0;
  
  Temperature->rawT = data;             //unsigned raw data
  Temperature->intT = data >> 4;        //integer
  Temperature->decT = data & 0x0F;      //decimal
}

void DS18B20_GetRoomCode(RomCodeTypeDef* RomCode)
{
  uint8_t i;
  DS18B20_Reset();
  RomCode->flag = DS18B20_Check();
  if(RomCode->flag == ERROR)
    return;
  DS18B20_WriteByte(READ_ROM);
  RomCode->familyId = DS18B20_ReadByte();
  for (i=0; i<6; i++)
  {
    RomCode->sn[i] = DS18B20_ReadByte();
  }
  RomCode->crc = DS18B20_ReadByte();
}
