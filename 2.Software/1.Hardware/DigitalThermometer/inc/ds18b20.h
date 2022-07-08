/* Includes ------------------------------------------------------------------*/
#include "stm8l15x.h"
#include "delay.h"

/* Private define ------------------------------------------------------------*/
#define RESPONSE_MAX_TIME_1     60
#define RESPONSE_MAX_TIME_2     240
//Rom Commands
#define SERACH_ROM              0xF0
#define READ_ROM                0x33
#define MATCH_ROM               0x55
#define SKIP_ROM                0xCC
#define ALARM_SEARCH            0xEC
//Function Commands
#define CONVERT_T               0x44    //Initiates a single temperature conversion
#define WRITE_SCRATCHPAD        0x4E
#define READ_SCRATCHPAD         0xBE
#define COPY_SCRATCHPAD         0x48
#define RECALL_E2               0xB8    //Recalls the alarm trigger values and configuration
#define READ_POWER_SUPPLY       0xB4    //Determine parasite power

/* Private macro -------------------------------------------------------------*/
#define DS18B20_DQ_PORT         (GPIOC)
#define DS18B20_DQ_PIN          (GPIO_Pin_0)
#define DS18B20_DQ_OUT          GPIO_Init(DS18B20_DQ_PORT, DS18B20_DQ_PIN, GPIO_Mode_Out_PP_High_Fast)
#define DS18B20_DQ_IN           GPIO_Init(DS18B20_DQ_PORT, DS18B20_DQ_PIN, GPIO_Mode_In_PU_No_IT)
#define DS18B20_DQ_HIGH         GPIO_SetBits(DS18B20_DQ_PORT, DS18B20_DQ_PIN)
#define DS18B20_DQ_LOW          GPIO_ResetBits(DS18B20_DQ_PORT, DS18B20_DQ_PIN)
#define DS18B20_DQ_STATUS       GPIO_ReadInputDataBit(DS18B20_DQ_PORT, DS18B20_DQ_PIN)

/***********Structure***********/
typedef struct 
{
  uint8_t intT;                 //integer
  uint8_t decT;                 //decimal
  uint8_t sign;                 //sign
  uint16_t rawT;                //unsigned raw
  ErrorStatus flag;             //check flag
}TemperatureTypeDef;

void DS18B20_Reset(void);                                       //Master issues reset pulse
ErrorStatus DS18B20_Check(void);                                //Check DS18B20 respond with presence pulse
static BitStatus DS18B20_ReadBit(void);
static uint8_t DS18B20_ReadByte(void);
void DS18B20_WriteByte(uint8_t cmd);
void DS18B20_GetTemperature(TemperatureTypeDef* Temperature);