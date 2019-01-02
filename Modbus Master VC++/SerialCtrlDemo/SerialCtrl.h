#ifndef _SERIAL_CTRL_H
#define _SERIAL_CTRL_H

#pragma once
#include "StdAfx.h"
// #include "ModbusMaster.h"

UINT16 crc_modbus(unsigned char *bufData, int num_bytes);

class SerialCtrl
{
private:
	BOOL m_portStatus;                    
	HANDLE m_portHandle;                  
	DCB m_portConfig;
public:
	SerialCtrl(void);
	~SerialCtrl(void);             
	void SetPortStatus(BOOL bOnOff);
	BOOL GetPortStatus(void);
	HANDLE GetPortHandle(void);
	
	BOOL Read(char * inputData, const unsigned int & sizeBuffer, unsigned long & length);
	BOOL Read(unsigned char * inputData, const unsigned int & sizeBuffer, unsigned long & length);	
	BOOL ClosePort(void);
	void msDelay(int milliseconds);	
	int  WriteSerialPort(BYTE *outputData, UINT16 outLength, unsigned long &length);
	int  ReadSerialPort(BYTE *ptrPacket);	
	BOOL OpenPort(const char * portName, UINT16 baudrate);
};

typedef union
{
	BYTE b[2];
	UINT16 integer;
} MConvertType;

#endif