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
  /* 发送复位ds18b20芯片信号                                                */
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
  /* 检测存在脉冲                                                           */
  /*------------------------------------------------------------------------*/

ErrorStatus DS18B20_Check(void)
{
  uint8_t cnt = 0;
  
  DS18B20_DQ_IN;        /* 设置DQ管脚为输入模式 */
  /* 等待复位信号低电平 */ 
  while((DS18B20_DQ_STATUS != RESET) && (cnt < RESPONSE_MAX_TIME_1))  
  {
    cnt++; 
    delay_1us(1);
  }
  if(cnt >= RESPONSE_MAX_TIME_1)
    return ERROR;
  else
    cnt = 0;
  
  /* 再等待复位信号拉高，表示复位结束 */
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
  /* 从DS18B20读一个位                                                      */
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
  /* 从DS18B20读一个字节                                                    */
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
  /* 从DS18B20写一个字节                                                    */
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
    cmd = cmd >> 1; /* 写下一位 */
  }
}

  /*------------------------------------------------------------------------*/
  /* 采集温度数据                                                           */
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

  /* 等待温度采集完成 */
  //while(DS18B20_ReadByte() != 0xFF);  

  DS18B20_Reset();
  Temperature->flag = DS18B20_Check();
  if(Temperature->flag == ERROR)
    return;
  DS18B20_WriteByte(SKIP_ROM);
  DS18B20_WriteByte(READ_SCRATCHPAD);

  data_L = DS18B20_ReadByte();
  data_H = DS18B20_ReadByte();

#ifdef __UART_H                // 用于调试的输出数据
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
s16 DS18B20_GetTemperature(void)  //读取并计算要输出的温度
{
    unsigned char i;
    unsigned char tl;
    unsigned int  th;
    i=DS18B20_Start();           //复位
    if(!i)                        //单总线上没有发现DS18B20则报警
    {
        Alarm_for_No_DS18B20();
        return 0;
    }
    _delay_ms(1);
    DS18B20_SendU8(SKIP_ROM);     //发跳过序列号检测命令
    DS18B20_SendU8(CONVERT_T); //命令Ds18b20开始转换温度
    i=0;
    _delay_ms(1);
    while(!R_DS18B20())       //当温度转换正在进行时,主机读总线将收到0,转换结束为1
    {
        _delay_ms(3);
        if(++i>250) break;              //至多转换时间为750ms
    }
    DS18B20_Start();                 //初始化
    _delay_ms(1);
    DS18B20_SendU8(SKIP_ROM);    //发跳过序列号检测命令
    DS18B20_SendU8(READ_SCRATCHPAD);  //发读取温度数据命令
    tl=DS18B20_ReadU8();           //先读低8位温度数据
    th=DS18B20_ReadU8()<<8;        //再读高8位温度数据    
    DS18B20_PIN_SET_OUT();//置为输出口
    return tl|th;              //温度放大了10倍,*0.0625=1/16=>>4
}*/
