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

#define READ_ROM              0x33  //读取序列号
#define SKIP_ROM              0xCC  //跳过ROM
#define MATCH_ROM             0x55  //匹配ROM
#define CONVERT_TEM           0x44  //转换温度
#define READ_RAM              0xBE  //读暂存器
#define WRITE_RAM             0x4E  //写暂存器
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

  /*------------------------------------------------------------------------*/
  /* Pull-Up/Float (Input) or Push-Pull/Open-Drain (Output) modes selection */
  /*------------------------------------------------------------------------*/

void Alarm_for_No_DS18B20(void)
{
    //单总线上没有发现DS18B20则报警,该动作据具体应用具体处理
}

  /*------------------------------------------------------------------------*/
  /* Pull-Up/Float (Input) or Push-Pull/Open-Drain (Output) modes selection */
  /*------------------------------------------------------------------------*/

unsigned char DS18B20_Start(void) //复位ds18b20芯片
{
    unsigned char i,succ=0xff;
    DS18B20_PIN_SET_OUT(); //置为输出口
    //GPIO_ResetBits(GPIOE, GPIO_Pin_7);
    GPIOC->ODR &= (uint8_t)(~0x01);
    _delay_us(600);
    //for(i=0; i<30; i++) _delay_us(20);
    //GPIO_SetBits(GPIOE, GPIO_Pin_7);
    GPIOC->ODR |= 0x01;
    DS18B20_PIN_SET_IN();  //置为输入,主机释放总线,准备接收DS18B20的应答脉冲
    i=0;
    while(R_DS18B20())         //等待DS18B20发出应答脉冲
    {
        _delay_us(5);          //5
        if(++i>12)            //DS18B20检测到总线上升沿后等待15-60us
        {
            succ=0x00;           //如果等待大于约60us，报告复位失败
            break;
        }
    }
    i=0;
    while(!R_DS18B20())       //DS18B20发出存在脉冲，持续60-240us
    {
        _delay_us(5);         //5
        if(++i>48)            //如果等带大于约240us，报告复位失败
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

void DS18B20_SendU8(unsigned char d8)//向DS18B20写一字节函数
{
    unsigned char i;
    DS18B20_PIN_SET_OUT();   //置为输出口
    for(i=0; i<8; i++)
    {
        DS18B20_WR0();       //总线拉低，启动“写时间片”
        _delay_us(2);            //大于1微妙
        if(d8&0x01)DS18B20_WR1();
        _delay_us (32);      //延时至少60微秒，使写入有效
        _delay_us (30);
        DS18B20_WR1();       //总线拉高,释放总线,准备启动下一个“写时间片”
        d8>>=1;
        _delay_us (1);
    }
    DS18B20_PIN_SET_IN();   //主机释放总线
}

  /*------------------------------------------------------------------------*/
  /* Pull-Up/Float (Input) or Push-Pull/Open-Drain (Output) modes selection */
  /*------------------------------------------------------------------------*/

unsigned char DS18B20_ReadU8(void)//从DS18B20读1个字节函数
{
    unsigned char i,d8;
    for(i=0; i<8; i++)
    {
        d8>>=1;
        DS18B20_PIN_SET_OUT();//置为输出口
        DS18B20_WR0();        //总线拉低,启动读“时间片”
        _delay_us(2);         //大于1微妙
        DS18B20_WR1();        //主机释放总线，接下来(2~15)us内读有效
        DS18B20_PIN_SET_IN(); //引脚设定为输入口,准备读取
        _delay_us(2);         //延时2个us后进行读


        if(R_DS18B20())d8|=0x80;//从总线拉低时算起，约15微秒内读取总线数据
        _delay_us(32);       //60us后读完成
        _delay_us(30);
        DS18B20_WR1();       //总线拉高,主机释放总线,准备启动下一个“写时间片”
    }
    DS18B20_PIN_SET_IN();    //主机释放总线
    return(d8);
}

  /*------------------------------------------------------------------------*/
  /* Pull-Up/Float (Input) or Push-Pull/Open-Drain (Output) modes selection */
  /*------------------------------------------------------------------------*/

void Init_DS18B20(void)           //初始化DS18B20
{
    unsigned char i;
    i=DS18B20_Start();             //复位
    if(!i)                          //单总线上没有发现DS18B20则报警
    {
        Alarm_for_No_DS18B20();
        return;
    }
    DS18B20_SendU8 (SKIP_ROM);  //跳过rom匹配
    DS18B20_SendU8 (WRITE_RAM); //设置写模式
    DS18B20_SendU8 (0x64);        //设置温度上限100摄氏度
    DS18B20_SendU8 (0x8a);        //设置温度下线-10摄氏度
    DS18B20_SendU8 (0x7f);        //12bit(默认)
}

  /*------------------------------------------------------------------------*/
  /* Pull-Up/Float (Input) or Push-Pull/Open-Drain (Output) modes selection */
  /*------------------------------------------------------------------------*/

s16 Read_DS18B20(void)  //读取并计算要输出的温度
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
    DS18B20_SendU8(CONVERT_TEM); //命令Ds18b20开始转换温度
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
    DS18B20_SendU8(READ_RAM);  //发读取温度数据命令
    tl=DS18B20_ReadU8();           //先读低8位温度数据
    th=DS18B20_ReadU8()<<8;        //再读高8位温度数据    
    DS18B20_PIN_SET_OUT();//置为输出口
    return tl|th;              //温度放大了10倍,*0.0625=1/16=>>4
}
