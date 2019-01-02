/* RoscidModbus.cpp
 *
 *
 *
 */

#pragma once
#include "stdafx.h"
#include "ModbusMaster.h"

ModbusMaster::ModbusMaster()
{
	;
}

ModbusMaster::~ModbusMaster()
{
	;
}

// Format for request packet sent by Modbus master to slave:
// 0,        1,             2-3,                4-5,                   6-7,                 8-9,                    10,               11...
// Slave ID, Function Code, Read start address, Num Registers to Read, Write start address, Num Registers to write, Write byte count, Write Data
#define READ_WRITE_FUNC_CODE 23
int ModbusMaster::ReadWriteModbusDevice(BYTE slaveID, UINT16 StartWriteAddress, UINT16 NumberIntegersToWrite, UINT16 *WriteData, UINT16 StartReadAddress, UINT16 NumberOfIntegersToRead, UINT16 *ReadData, CString *strErrors)
{
	BYTE OutPacket[MAX_MODBUS_PACKET_LENGTH], InPacket[MAX_MODBUS_PACKET_LENGTH];
	ConvertType convert;
	UINT16 i, TxIndex, RxIndex, OutPacketLength, InPacketLength, NumberOfIncomingIntegers, ComErrors;

	if (NumberIntegersToWrite > MAX_MODBUS_DATA_LENGTH) return SYSTEM_ERROR;
	if (NumberOfIntegersToRead > MAX_MODBUS_DATA_LENGTH) return SYSTEM_ERROR;
		
	// Construct outgoing Modbus packet. Bytes #0 and #1 are always the slave ID and function code:
	OutPacket[0] = slaveID;
	OutPacket[1] = READ_WRITE_FUNC_CODE;

	if (WriteData != NULL && ReadData != NULL)
	{
		// Define start address and number of registers to read:
		convert.integer = StartReadAddress;
		OutPacket[2] = convert.b[1];
		OutPacket[3] = convert.b[0];
		convert.integer = NumberOfIntegersToRead;
		OutPacket[4] = convert.b[1];
		OutPacket[5] = convert.b[0];

		// Next define start address and number of registers to write:
		convert.integer = StartWriteAddress;
		OutPacket[6] = convert.b[1];
		OutPacket[7] = convert.b[0];
		convert.integer = NumberIntegersToWrite;
		OutPacket[8] = convert.b[1];
		OutPacket[9] = convert.b[0];

		// This byte is the nuumber of bytes to write - not sure why this is necesssary:
		OutPacket[10] = (BYTE)(NumberIntegersToWrite * 2);

		// Add data to write beginning at packet byte #11:
		TxIndex = 11;
		for (i = 0; i < NumberIntegersToWrite; i++)
		{
			convert.integer = WriteData[i];
			OutPacket[TxIndex++] = convert.b[1];
			OutPacket[TxIndex++] = convert.b[0];
		}
		OutPacketLength = TxIndex;

		ComErrors = SendReceiveModbus(OutPacket, OutPacketLength, InPacket, &InPacketLength, strErrors);
		if (ComErrors == NO_ERRORS)
		{
			// Make sure number of integers matches number expected:
			NumberOfIncomingIntegers = (InPacketLength - 3) / 2;
			if (NumberOfIncomingIntegers != NumberOfIntegersToRead) return SYSTEM_ERROR;

			NumberOfIncomingIntegers = InPacket[2] / 2;
			if (NumberOfIncomingIntegers != NumberOfIntegersToRead) return SYSTEM_ERROR;

			RxIndex = 3; // Incoming data begins with third byte
			// Get data bytes read and convert to two byte integers:
			for (i = 0; i < NumberOfIncomingIntegers; i++)
			{
				convert.b[1] = InPacket[RxIndex++];
				convert.b[0] = InPacket[RxIndex++];
				ReadData[i] = convert.integer;
			}			
		}
		else return SYSTEM_ERROR;
		return PASS;
	}
	else return SYSTEM_ERROR;
}

int ModbusMaster::SendReceiveModbus(BYTE *OutModbusPacket, UINT16 OutLength, BYTE *InModbusPacket, UINT16 *InLength, CString *strErrors)
{
	int errors = 0;
	unsigned long bytesWritten;

	MConvertType convert;

	convert.integer = crc_modbus(OutModbusPacket, OutLength);
	OutModbusPacket[OutLength++] = convert.b[0];
	OutModbusPacket[OutLength++] = convert.b[1];

	// TODO: Add your control notification handler code here
	if (GetPortStatus())
	{
		errors = WriteSerialPort(OutModbusPacket, OutLength, bytesWritten);
		if (errors)
		{
			*strErrors = "Serial WRITE error";
			return errors;
		}
		else if (bytesWritten == 0)
		{
			*strErrors = "Serial WRITE error - no bytes written";
			return 3;
		}

		*InLength = ReadSerialPort(InModbusPacket);
		convert.b[0] = InModbusPacket[*InLength - 2];
		convert.b[1] = InModbusPacket[*InLength - 1];

		int intCRC = convert.integer;

		if (*InLength == 0)
		{
			*strErrors = "No bytes received.";
			return 4;
		}

		*InLength = *InLength - 2; // Now that we read the two CRC bytes at the end of the packet, we ignore them.

		if (crc_modbus(InModbusPacket, *InLength) != intCRC)
		{
			*strErrors = "CRC error";
			return 5;
		}
		else (*strErrors).Format("RX: %02X, %02X, %02X, %02X, %02X, %02X", InModbusPacket[0], InModbusPacket[1], InModbusPacket[2], InModbusPacket[3], InModbusPacket[4], InModbusPacket[5]);
	}
	else
	{
		(*strErrors).Format("Port not open");
		return 6;
	}

	return 0;
}
