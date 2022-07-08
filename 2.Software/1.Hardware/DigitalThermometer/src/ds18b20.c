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

  /*------------------------------------------------------------------------*/
  /* ���͸�λds18b20оƬ�ź�                                                */
  /*------------------------------------------------------------------------*/

void DS18B20_Reset(void) //
{
    DS18B20_DQ_OUT;
    DS18B20_DQ_LOW;
    delay_1us(600);
    DS18B20_DQ_HIGH;
    delay_1us(20);
}

  /*------------------------------------------------------------------------*/
  /* ����������                                                           */
  /*------------------------------------------------------------------------*/

ErrorStatus DS18B20_Check(void)
{
  uint8_t cnt = 0;
  
  DS18B20_DQ_IN;        /* ����DQ�ܽ�Ϊ����ģʽ */
  /* �ȴ���λ�źŵ͵�ƽ */ 
  while((DS18B20_DQ_STATUS != RESET) && (cnt < RESPONSE_MAX_TIME_1))  
  {
    cnt++; 
    delay_1us(1);
  }
  if(cnt >= RESPONSE_MAX_TIME_1)
    return ERROR;
  else
    cnt = 0;
  
  /* �ٵȴ���λ�ź����ߣ���ʾ��λ���� */
  while((DS18B20_DQ_STATUS == RESET) && (cnt < RESPONSE_MAX_TIME_2))  
  {
    cnt++; 
    delay_1us(1);
  }
  if(cnt >= RESPONSE_MAX_TIME_2)
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
  
  DS18B20_DQ_OUT;
  DS18B20_DQ_LOW;
  delay_1us(2);
  DS18B20_DQ_HIGH;
  
  DS18B20_DQ_IN;
  delay_1us(12);       
  
  if(DS18B20_DQ_STATUS != RESET)   
    data = SET;              
  else 
    data = RESET;	   
  
  delay_1us(50);         
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
  /* ��DS18B20дһ���ֽ�                                                    */
  /*------------------------------------------------------------------------*/

void DS18B20_WriteByte(uint8_t cmd)
{
  uint8_t bit;
  uint8_t i;
  
  DS18B20_DQ_OUT;
  
  for(i = 0; i < 8; i++)
  {
    bit = cmd & 0x01;
    if (bit == 1) 
    {
      DS18B20_DQ_LOW;
      delay_1us(2);
      DS18B20_DQ_HIGH;
      delay_1us(60); 
    }
    else   
    {
       DS18B20_DQ_LOW;
       delay_1us(60);   
       DS18B20_DQ_HIGH;
       delay_1us(2);    
    }  
    cmd = cmd >> 1; /* д��һλ */
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

  /*------------------------------------------------------------------------*/
  /* Pull-Up/Float (Input) or Push-Pull/Open-Drain (Output) modes selection */
  /*------------------------------------------------------------------------*/
/*
s16 DS18B20_GetTemperature(void)  //��ȡ������Ҫ������¶�
{
    unsigned char i;
    unsigned char tl;
    unsigned int  th;
    i=DS18B20_Start();           //��λ
    if(!i)                        //��������û�з���DS18B20�򱨾�
    {
        Alarm_for_No_DS18B20();
        return 0;
    }
    _delay_ms(1);
    DS18B20_SendU8(SKIP_ROM);     //���������кż������
    DS18B20_SendU8(CONVERT_T); //����Ds18b20��ʼת���¶�
    i=0;
    _delay_ms(1);
    while(!R_DS18B20())       //���¶�ת�����ڽ���ʱ,���������߽��յ�0,ת������Ϊ1
    {
        _delay_ms(3);
        if(++i>250) break;              //����ת��ʱ��Ϊ750ms
    }
    DS18B20_Start();                 //��ʼ��
    _delay_ms(1);
    DS18B20_SendU8(SKIP_ROM);    //���������кż������
    DS18B20_SendU8(READ_SCRATCHPAD);  //����ȡ�¶���������
    tl=DS18B20_ReadU8();           //�ȶ���8λ�¶�����
    th=DS18B20_ReadU8()<<8;        //�ٶ���8λ�¶�����    
    DS18B20_PIN_SET_OUT();//��Ϊ�����
    return tl|th;              //�¶ȷŴ���10��,*0.0625=1/16=>>4
}*/
