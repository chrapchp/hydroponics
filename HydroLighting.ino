/**
*  @file    HydroLighting.ino
*  @author  peter c
*  @date    11/03/2017
*  @version 0.1
*
*
*  @section DESCRIPTION
*  Ligting control - based on Light assembly front left
** @section HISTORY
** 2017Nov2 - created
*/
#include <HardwareSerial.h>
#include <Streaming.h>
#include <DA_DiscreteOutput.h>

#include <DA_NonBlockingDelay.h>


#include "unitModbus.h"
// comment out to  include terminal processing for debugging
 //#define PROCESS_TERMINAL
 //#define TRACE_1WIRE
// #define TRACE_ANALOGS
// #define TRACE_DISCRETES
// #define TRACE_MODBUS
// comment out to disable modbus
#define PROCESS_MODBUS
// refresh intervals
#define POLL_CYCLE_SECONDS 2 // sonar and 1-wire refresh rate

DA_DiscreteOutput B1R1_1A_XY_013 = DA_DiscreteOutput(12, HIGH); // heater

DA_DiscreteOutput B1R1_1A_XY_015 = DA_DiscreteOutput(11, HIGH); // left
DA_DiscreteOutput B1R1_1A_XY_016 = DA_DiscreteOutput(10, HIGH); // right

 
// poll I/O every 2 seconds
DA_NonBlockingDelay pollTimer = DA_NonBlockingDelay( 2000, &doOnPoll);


// HEARTBEAT
unsigned int heartBeat = 0;



#ifdef PROCESS_TERMINAL
HardwareSerial *tracePort = & Serial;
#endif





int polling; // 1=polling analogs, 2=polling digitals, -1 nothing
int currentTogglePin; // -1 none, >0 pin
void setup()
{

#ifdef PROCESS_TERMINAL
  tracePort->begin(9600);
#endif

#ifdef PROCESS_MODBUS
  slave.begin(MB_SPEED);
#endif



  randomSeed(analogRead(3));


}

void loop()
{

#ifdef PROCESS_MODBUS
  refreshModbusRegisters();
  slave.poll(modbusRegisters, MODBUS_REG_COUNT);
  processModbusCommands();
#endif
pollTimer.refresh();

}

// update sonar and 1-wire DHT-22 readings
void doOnPoll()
{
  

  heartBeat++;
}





// 
/*
** Modbus related functions
*/

#ifdef PROCESS_MODBUS
void refreshModbusRegisters()
{

 // modbusRegisters[HR_TEMPERATURE1] =  -127; // B1R1_1A_TT_001.getTempCByIndex(0) * 100;
  modbusRegisters[HR_HEARTBEAT] = heartBeat;

}


bool getModbusCoilValue(unsigned short startAddress, unsigned short bitPos)
{
  // *tracePort << "reading at " << startAddress << " bit offset " << bitPos << "value=" << bitRead(modbusRegisters[startAddress + (int)(bitPos / 16)], bitPos % 16 ) << endl;
  return(bitRead(modbusRegisters[startAddress + (int) (bitPos / 16)], bitPos % 16));
}

void writeModbusCoil(unsigned short startAddress, unsigned short bitPos, bool value)
{
  bitWrite(modbusRegisters[startAddress + (int) (bitPos / 16)], bitPos % 16, value);
}

void checkAndActivateDO(unsigned int bitOffset, DA_DiscreteOutput * aDO)
{
  // look for a change from 0 to 1
  if (getModbusCoilValue(COIL_STATUS_READ_WRITE_OFFSET, bitOffset))
  {
    aDO->activate();

  #ifdef TRACE_MODBUS
    *tracePort << "Activate DO:";
    aDO->serialize(tracePort, true);
    LED.activate();
  #endif

   // writeModbusCoil(COIL_STATUS_READ_WRITE_OFFSET, bitOffset, false); // clear the bit
  }
}

void checkAndResetDO(unsigned int bitOffset, DA_DiscreteOutput * aDO)
{
  // look for a change from 0 to 1
  if (!getModbusCoilValue(COIL_STATUS_READ_WRITE_OFFSET, bitOffset))
  {
    aDO->reset();

  #ifdef TRACE_MODBUS
    *tracePort << "Reset DO:";
    aDO->serialize(tracePort, true);
    LED.reset();
  #endif

   // writeModbusCoil(COIL_STATUS_READ_WRITE_OFFSET, bitOffset, false); // clear the bit
  }
}

void processValveCommands()
{
  checkAndActivateDO(B1R1_1A_XY_013_MB, & B1R1_1A_XY_013);
  checkAndResetDO(B1R1_1A_XY_013_MB, & B1R1_1A_XY_013);

  checkAndActivateDO(B1R1_1A_XY_015_MB, & B1R1_1A_XY_015);
  checkAndResetDO(B1R1_1A_XY_015_MB, & B1R1_1A_XY_015);

    checkAndActivateDO(B1R1_1A_XY_016_MB, & B1R1_1A_XY_016);
    checkAndResetDO(B1R1_1A_XY_016_MB, & B1R1_1A_XY_016);


}

void processModbusCommands()
{
  processValveCommands();
}

#endif
