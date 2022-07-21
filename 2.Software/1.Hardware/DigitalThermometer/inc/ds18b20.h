#ifndef __DS18B20_H
#define __DS18B20_H

/* Includes ------------------------------------------------------------------*/
#include "stm8l15x.h"
#include "delay.h"

/* Private define ------------------------------------------------------------*/
#define RESET_PULSE_TIMESLOT    600     //480us - 960us
#define WAIT_TIMESLOT           60      //15us - 60us
#define PRESENCE_PULSE_TIMESLOT 240     //60us - 240us
#define BUS_INIT_TIME           1       //minimum of a 1us
#define RECOVERY_TIME           10      //RECOVERY_TIME - 15us
#define WAIT_READ_TIMESLOT      50      //VALID_TIME - 45us
#define WAIT_WRITE_TIMESLOT     60

//Rom Commands
#define SERACH_ROM              0xF0
#define READ_ROM                0x33
#define MATCH_ROM               0x55
#define SKIP_ROM                0xCC
#define ALARM_SEARCH            0xEC
//Function Commands
#define CONVERT_T               0x44    //Initiates a single temperature conversion.
#define WRITE_SCRATCHPAD        0x4E    //Writes data into scratchpad bytes 2, 3, and 4 (TH, TL, and configuration registers).
#define READ_SCRATCHPAD         0xBE    //Reads the entire scratchpad including the CRC byte.
#define COPY_SCRATCHPAD         0x48    //Copies TH, TL, and configuration register data from the scratchpad to EEPROM.
#define RECALL_E2               0xB8    //Recalls TH, TL, and configuration register data from EEPROM to the scratchpad.
#define READ_POWER_SUPPLY       0xB4    //Signals DS18B20 power supply mode to the master.

/* Private macro -------------------------------------------------------------*/
#define DS18B20_DQ_PORT         (GPIOC)
#define DS18B20_DQ_PIN          (GPIO_Pin_0)
#define DS18B20_DQ_OUT          GPIO_Init(DS18B20_DQ_PORT, DS18B20_DQ_PIN, GPIO_Mode_Out_PP_High_Fast)
#define DS18B20_DQ_IN           GPIO_Init(DS18B20_DQ_PORT, DS18B20_DQ_PIN, GPIO_Mode_In_PU_No_IT)
#define DS18B20_DQ_HIGH         GPIO_SetBits(DS18B20_DQ_PORT, DS18B20_DQ_PIN)
#define DS18B20_DQ_LOW          GPIO_ResetBits(DS18B20_DQ_PORT, DS18B20_DQ_PIN)
#define DS18B20_DQ_STATUS       GPIO_ReadInputDataBit(DS18B20_DQ_PORT, DS18B20_DQ_PIN)

/* Structure -----------------------------------------------------------------*/
typedef struct 
{
  uint8_t intT;                 //integer
  uint8_t decT;                 //decimal
  uint8_t sign;                 //sign
  uint16_t rawT;                //unsigned raw
  ErrorStatus flag;             //check flag
}TemperatureTypeDef;

/* Structure -----------------------------------------------------------------*/
typedef struct 
{
  uint8_t familyId;             //integer
  uint8_t sn[6];                //decimal
  uint8_t crc;                  //sign
  ErrorStatus flag;             //check flag
}RomCodeTypeDef;

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
ErrorStatus ow_reset(void);
static BitStatus read_bit(void);
void write_bit(uint8_t bitval);
uint8_t read_byte(void);
void write_byte(uint8_t val);

void FindDevices(void);
uint8_t First(void);
uint8_t Next(void);
uint8_t ow_crc(uint8_t x);

void Read_Temperature(TemperatureTypeDef* Temperature);
void Read_ROMCode(RomCodeTypeDef* RomCode);
uint8_t Send_MatchRom(void);

#endif /* __DS18B20_H */