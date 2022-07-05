/**
  ******************************************************************************
  * @file    ds18b20.c
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
#include "stm8l_discovery_lcd.h"
//#include "delay.h"

/** @addtogroup STM8L15x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DS18B20_GPIO_PORT  (GPIOC)
#define DS18B20_GPIO_PIN   (GPIO_Pin_0)
#define DS18B20_PIN_SET_OUT()   GPIO_Init(DS18B20_GPIO_PORT, (GPIO_Pin_TypeDef)DS18B20_GPIO_PIN, GPIO_Mode_Out_PP_High_Fast)
#define DS18B20_PIN_SET_IN()    GPIO_Init(DS18B20_GPIO_PORT, (GPIO_Pin_TypeDef)DS18B20_GPIO_PIN, GPIO_Mode_In_PU_No_IT)
#define DS18B20_WR1()           GPIO_SetBits(DS18B20_GPIO_PORT, (GPIO_Pin_TypeDef)DS18B20_GPIO_PIN)
#define DS18B20_WR0()           GPIO_ResetBits(DS18B20_GPIO_PORT, (GPIO_Pin_TypeDef)DS18B20_GPIO_PIN)
#define R_DS18B20()             GPIO_ReadInputDataBit(DS18B20_GPIO_PORT, (GPIO_Pin_TypeDef)DS18B20_GPIO_PIN)

#define READ_ROM              0x33  //��ȡ���к�
#define SKIP_ROM              0xCC  //����ROM
#define MATCH_ROM             0x55  //ƥ��ROM
#define CONVERT_TEM           0x44  //ת���¶�
#define READ_RAM              0xBE  //���ݴ���
#define WRITE_RAM             0x4E  //д�ݴ���
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

  /*------------------------------------------------------------------------*/
  /* Pull-Up/Float (Input) or Push-Pull/Open-Drain (Output) modes selection */
  /*------------------------------------------------------------------------*/

void Alarm_for_No_DS18B20(void)
{
    //��������û�з���DS18B20�򱨾�,�ö����ݾ���Ӧ�þ��崦��
}

  /*------------------------------------------------------------------------*/
  /* Pull-Up/Float (Input) or Push-Pull/Open-Drain (Output) modes selection */
  /*------------------------------------------------------------------------*/

unsigned char DS18B20_Start(void) //��λds18b20оƬ
{
    unsigned char i,succ=0xff;
    DS18B20_PIN_SET_OUT(); //��Ϊ�����
    //GPIO_ResetBits(GPIOE, GPIO_Pin_7);
    GPIOC->ODR &= (uint8_t)(~0x01);
    _delay_us(600);
    //for(i=0; i<30; i++) _delay_us(20);
    //GPIO_SetBits(GPIOE, GPIO_Pin_7);
    GPIOC->ODR |= 0x01;
    DS18B20_PIN_SET_IN();  //��Ϊ����,�����ͷ�����,׼������DS18B20��Ӧ������
    i=0;
    while(R_DS18B20())         //�ȴ�DS18B20����Ӧ������
    {
        _delay_us(5);          //5
        if(++i>12)            //DS18B20��⵽���������غ�ȴ�15-60us
        {
            succ=0x00;           //����ȴ�����Լ60us�����渴λʧ��
            break;
        }
    }
    i=0;
    while(!R_DS18B20())       //DS18B20�����������壬����60-240us
    {
        _delay_us(5);         //5
        if(++i>48)            //����ȴ�����Լ240us�����渴λʧ��
        {
            succ=0x00;
            break;
        }
    }
    _delay_us(20);
    return succ;
}

  /*------------------------------------------------------------------------*/
  /* Pull-Up/Float (Input) or Push-Pull/Open-Drain (Output) modes selection */
  /*------------------------------------------------------------------------*/

void DS18B20_SendU8(unsigned char d8)//��DS18B20дһ�ֽں���
{
    unsigned char i;
    DS18B20_PIN_SET_OUT();   //��Ϊ�����
    for(i=0; i<8; i++)
    {
        DS18B20_WR0();       //�������ͣ�������дʱ��Ƭ��
        _delay_us(2);            //����1΢��
        if(d8&0x01)DS18B20_WR1();
        _delay_us (32);      //��ʱ����60΢�룬ʹд����Ч
        _delay_us (30);
        DS18B20_WR1();       //��������,�ͷ�����,׼��������һ����дʱ��Ƭ��
        d8>>=1;
        _delay_us (1);
    }
    DS18B20_PIN_SET_IN();   //�����ͷ�����
}

  /*------------------------------------------------------------------------*/
  /* Pull-Up/Float (Input) or Push-Pull/Open-Drain (Output) modes selection */
  /*------------------------------------------------------------------------*/

unsigned char DS18B20_ReadU8(void)//��DS18B20��1���ֽں���
{
    unsigned char i,d8;
    for(i=0; i<8; i++)
    {
        d8>>=1;
        DS18B20_PIN_SET_OUT();//��Ϊ�����
        DS18B20_WR0();        //��������,��������ʱ��Ƭ��
        _delay_us(2);         //����1΢��
        DS18B20_WR1();        //�����ͷ����ߣ�������(2~15)us�ڶ���Ч
        DS18B20_PIN_SET_IN(); //�����趨Ϊ�����,׼����ȡ
        _delay_us(2);         //��ʱ2��us����ж�


        if(R_DS18B20())d8|=0x80;//����������ʱ����Լ15΢���ڶ�ȡ��������
        _delay_us(32);       //60us������
        _delay_us(30);
        DS18B20_WR1();       //��������,�����ͷ�����,׼��������һ����дʱ��Ƭ��
    }
    DS18B20_PIN_SET_IN();    //�����ͷ�����
    return(d8);
}

  /*------------------------------------------------------------------------*/
  /* Pull-Up/Float (Input) or Push-Pull/Open-Drain (Output) modes selection */
  /*------------------------------------------------------------------------*/

void Init_DS18B20(void)           //��ʼ��DS18B20
{
    unsigned char i;
    i=DS18B20_Start();             //��λ
    if(!i)                          //��������û�з���DS18B20�򱨾�
    {
        Alarm_for_No_DS18B20();
        return;
    }
    DS18B20_SendU8 (SKIP_ROM);  //����romƥ��
    DS18B20_SendU8 (WRITE_RAM); //����дģʽ
    DS18B20_SendU8 (0x64);        //�����¶�����100���϶�
    DS18B20_SendU8 (0x8a);        //�����¶�����-10���϶�
    DS18B20_SendU8 (0x7f);        //12bit(Ĭ��)
}

  /*------------------------------------------------------------------------*/
  /* Pull-Up/Float (Input) or Push-Pull/Open-Drain (Output) modes selection */
  /*------------------------------------------------------------------------*/

s16 Read_DS18B20(void)  //��ȡ������Ҫ������¶�
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
    DS18B20_SendU8(CONVERT_TEM); //����Ds18b20��ʼת���¶�
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
    DS18B20_SendU8(READ_RAM);  //����ȡ�¶���������
    tl=DS18B20_ReadU8();           //�ȶ���8λ�¶�����
    th=DS18B20_ReadU8()<<8;        //�ٶ���8λ�¶�����    
    DS18B20_PIN_SET_OUT();//��Ϊ�����
    return tl|th;              //�¶ȷŴ���10��,*0.0625=1/16=>>4
}
