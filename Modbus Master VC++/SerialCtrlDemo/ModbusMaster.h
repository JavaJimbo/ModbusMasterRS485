/* ModbusMaster.h
 *
 *
 *
 */

#pragma once
#include "stdafx.h"
#include "SerialCtrl.h"

class ModbusMaster : public SerialCtrl
{
public:	
	ModbusMaster();
	virtual ~ModbusMaster();
	
	// void SetPointerToSerialControl(SerialCtrl *pSCTL) { m_pSCTL = pSCTL; };	
	int ReadWriteModbusDevice(BYTE slaveID, UINT16 StartWriteAddress, UINT16 NumberIntegersToWrite, UINT16 *WriteData, UINT16 StartReadAddress, UINT16 NumberOfIntegersToRead, UINT16 *ReadData, CString *strErrors);
	int SendReceiveModbus(BYTE *OutModbusPacket, UINT16 OutLength, BYTE *InModbusPacket, UINT16 *InLength, CString *strErrors);

private:
	// SerialCtrl *m_pSCTL;
};

#define MAX_MODBUS_LENGTH 256
UINT16 crc_modbus(unsigned char *bufData, int num_bytes);


typedef union 
{
	BYTE b[2];
	UINT16 integer;
} ConvertType;

enum STATUS {
	HALTED = 0,
	IN_PROGRESS,
	PASS,
	FAIL,
	SYSTEM_ERROR
};

#define NO_ERRORS 0
#define MAX_MODBUS_PACKET_LENGTH 256
#define MAX_MODBUS_DATA_LENGTH 125







