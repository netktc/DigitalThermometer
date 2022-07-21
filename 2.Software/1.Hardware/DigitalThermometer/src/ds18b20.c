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
#include "delay.h"
#include "stdio.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t ROM[8]; // ROM Bit
uint8_t lastDiscrep = 0;
uint8_t doneFlag = 0; // Done flag
uint8_t FoundROM[6][8]; // table of found ROM codes
uint8_t numROMs;
uint8_t dowcrc;
uint8_t dscrc_table[] = {
  0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65, 
  157,195, 33,127,252,162, 64, 30, 95, 1,227,189, 62, 96,130,220, 
  35,125,159,193, 66, 28,254,160,225,191, 93, 3,128,222, 60, 98, 
  190,224, 2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255, 
  70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89, 7, 
  219,133,103, 57,186,228, 6, 88, 25, 71,165,251,120, 38,196,154, 
  101, 59,217,135, 4, 90,184,230,167,249, 27, 69,198,152,122, 36, 
  248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91, 5,231,185, 
  140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205, 
  17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80, 
  175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238, 
  50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115, 
  202,148,118, 40,171,245, 23, 73, 8, 86,180,234,105, 55,213,139, 
  87, 9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22, 
  233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168, 
  116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};

/* Private functions ---------------------------------------------------------*/

//////////////////////////////////////////////////////////////////////////////
// OW_RESET - performs a reset on the one-wire bus and
// returns the presence detect. Reset is 480us, so delay
// value is (480-24)/16 = 28.5 - we use 29. Presence checked
// another 70us later, so delay is (70-24)/16 = 2.875 - we use 3.
//
ErrorStatus ow_reset(void)
{
  uint8_t cnt = 0;
  DS18B20_DQ_OUT;
  DS18B20_DQ_LOW;                       //pull DQ line low
  delay_1us(RESET_PULSE_TIMESLOT);      // leave it low for 480us
  DS18B20_DQ_HIGH;                      // allow line to return high
  delay_1us(20);                        // wait for presence
  DS18B20_DQ_IN;
  
  //等待复位信号低电平
  while((DS18B20_DQ_STATUS != RESET) && (cnt < WAIT_TIMESLOT))
  {
    cnt++;
    delay_1us(1);
  }
  if(cnt >= WAIT_TIMESLOT)
    return ERROR;
  else
    cnt = 0;
  
  //再等待复位信号拉高，表示复位结束
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

//////////////////////////////////////////////////////////////////////////////
// READ_BIT - reads a bit from the one-wire bus. The delay
// required for a read is 15us, so the DELAY routine won't work.
// We put our own delay function in this routine in the form of a
// for() loop.
//
static BitStatus read_bit(void)
{
  BitStatus data;
  DS18B20_DQ_OUT;
  DS18B20_DQ_LOW;                       // pull DQ low to start timeslot
  delay_1us(BUS_INIT_TIME);
  //master device releasing the bus
  DS18B20_DQ_IN;                        // then return high
  //After the master initiates the read time slot, the DS18B20 will begin transmitting a 1 or 0 on bus.
  //Output data from the DS18B20 is valid for 15μs after the falling edge that initiated the read time slot.
  delay_1us(RECOVERY_TIME);
  if (DS18B20_DQ_STATUS != RESET)
    data = SET;
  else
    data = RESET;
  delay_1us(WAIT_READ_TIMESLOT);
  return data;
}

//////////////////////////////////////////////////////////////////////////////
// WRITE_BIT - writes a bit to the one-wire bus, passed in bitval.
//
void write_bit(uint8_t bitval)
{
  DS18B20_DQ_OUT;
  DS18B20_DQ_LOW;                       // pull DQ low to start timeslot
  delay_1us(BUS_INIT_TIME);             // hold value for remainder of timeslot
  if(bitval==1) DS18B20_DQ_IN;          // return DQ high if write 1
  delay_1us(WAIT_WRITE_TIMESLOT);       // hold value for remainder of timeslot
  DS18B20_DQ_HIGH;
}// Delay provides 16us per loop, plus 24us. Therefore delay(5) = 104us

//////////////////////////////////////////////////////////////////////////////
// READ_BYTE - reads a byte from the one-wire bus.
//
uint8_t read_byte(void)
{
  uint8_t i;
  uint8_t value = 0;
  for (i = 0; i < 8; i++)
  {
    if (read_bit()) value |= 0x01 << i;  // reads byte in, one byte at a time and then
    // shifts it left
  }
  return(value);
}

//////////////////////////////////////////////////////////////////////////////
// WRITE_BYTE - writes a byte to the one-wire bus.
//
void write_byte(uint8_t val)
{
  uint8_t i;
  uint8_t temp;
  for (i=0; i<8; i++) // writes byte, one bit at a time
  {
    temp = val>>i; // shifts val right 'i' spaces
    temp &= 0x01; // copy that bit to temp
    write_bit(temp); // write bit in temp into
  }
  //delay(5);
}

// FIND DEVICES
//
//
//
void FindDevices(void)
{
  uint8_t m, i;
  if(ow_reset()) //Begins when a presence is detected
  {
    if(First()) //Begins when at least one part is found
    {
      numROMs = 0;
      do
      {
        for(m = 0; m < 8; m++)
        {
          FoundROM[numROMs][m]=ROM[m]; //Identifies ROM
          //number on found device
        }
        printf("\nROM CODE = ");
        for(i = 0; i < m; i++)
        {
          printf("%02X",FoundROM[numROMs][i]);
          if (i < m - 1)
            printf(":");
        }
        printf("\n");
        numROMs++;
      }
      while (Next()&&(numROMs<10)); //Continues until no additional devices are found
    }
  }
}

// FIRST
// The First function resets the current state of a ROM search and calls
// Next to find the first device on the 1-Wire bus.
//
uint8_t First(void)
{
  lastDiscrep = 0; // reset the rom search last discrepancy global
  doneFlag = FALSE;
  return Next(); // call Next and return its return value
}

// NEXT
// The Next function searches for the next device on the 1-Wire bus. If
// there are no more devices on the 1-Wire then false is returned.
//
uint8_t Next(void)
{
  uint8_t m = 1;                // ROM Bit index
  uint8_t n = 0;                // ROM Byte index
  uint8_t k = 1;                // bit mask
  uint8_t x = 0;
  uint8_t discrepMarker = 0;    // discrepancy marker
  uint8_t g;                    // Output bit
  uint8_t nxt;                  // return value
  int flag;
  nxt = FALSE;                  // set the next flag to false
  dowcrc = 0;                   // reset the dowcrc
  
  flag = !ow_reset();           // reset the 1-Wire
  
  if (flag || doneFlag)         // no parts -> return false
  {
    lastDiscrep = 0;            // reset the search
    return FALSE;
  }
  
  write_byte(SERACH_ROM);       // send SearchROM command
  
  do
  // for all eight bytes
  {
    x = 0;
    if (read_bit() == 1) x = 2;
    
    delay_1us(10);
    
    if (read_bit() == 1) x |= 1;         // and its complement
    
    if (x == 3)                         // there are no devices on the 1-Wire
      break;
    else
    {
      if (x > 0)                        // all devices coupled have 0 or 1
        g = x >> 1;                     // bit write value for search
      else
      {
        // if this discrepancy is before the last discrepancy on a previous Next then pick the same as last time
        if (m < lastDiscrep)
          g = ((ROM[n] & k) > 0);
        else                            // if equal to last pick 1
          g = (m == lastDiscrep);       // if not then pick 0
        // if 0 was picked then record position with mask k
        if (g == 0) discrepMarker = m;
      }
      if (g == 1)                       // isolate bit in ROM[n] with mask k
        ROM[n] |= k;
      else
        ROM[n] &= ~k;
      
      write_bit(g);                     // ROM search write
      
      m++;                              // increment bit counter m and shift the bit mask k
      k = k << 1;
      if(k == 0)                        // if the mask is 0 then go to new ROM
      {
        // byte n and reset mask
        ow_crc(ROM[n]);                 // accumulate the CRC
        n++; k++;
      }
    }
  }while(n < 8);                        //loop until through all ROM bytes 0-7
  if (m < 65 || dowcrc)                 // if search was unsuccessful then reset the last discrepancy to 0
    lastDiscrep = 0;
  else
  {
    // search was successful, so set lastDiscrep, lastOne, nxt
    lastDiscrep = discrepMarker;
    doneFlag = (lastDiscrep == 0);
    nxt = TRUE;                         // indicates search is not complete yet, more parts remain
  }
  return nxt;
}

//////////////////////////////////////////////////////////////////////////////
// ONE WIRE CRC
//
uint8_t ow_crc(uint8_t x)
{
  dowcrc = dscrc_table[dowcrc^x];
  return dowcrc;
}

  /*------------------------------------------------------------------------*/
  /* 采集温度数据                                                           */
  /*------------------------------------------------------------------------*/

void Read_Temperature(TemperatureTypeDef* Temperature)
{
  uint8_t data_L, data_H;
  uint16_t data;
  
  ow_reset();
  Temperature->flag = ow_reset();
  if(Temperature->flag == ERROR)
    return;
  write_byte(SKIP_ROM);
  write_byte(CONVERT_T);
  
  delay_1us(10);
  
  ow_reset();
  Temperature->flag = ow_reset();
  if(Temperature->flag == ERROR)
    return;
  write_byte(SKIP_ROM);
  write_byte(READ_SCRATCHPAD);
  
  data_L = read_byte();
  data_H = read_byte();
  
  printf("\n\rTEST DATA: %d %d\n\r", data_L, data_H);

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
  /* 采集温度数据                                                           */
  /*------------------------------------------------------------------------*/

void Read_ROMCode(RomCodeTypeDef* RomCode)
{
  uint8_t i;
  
  printf("\nROM CODE = ");
  RomCode->flag = ow_reset();
  if(RomCode->flag == ERROR)
    return;
  write_byte(READ_ROM);
  RomCode->familyId = read_byte();
  printf("%02X:",RomCode->familyId);
  for (i = 0; i < 6; i++)
  {
    RomCode->sn[i] = read_byte();
    printf("%02X:",RomCode->sn[i]);
  }
  RomCode->crc = read_byte();
  printf("%02X\n",RomCode->crc);
}
