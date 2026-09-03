#include "CRC.hpp"

#include <stdio.h>

int main(void)
{
	char arr[] = "Hello World!";
	printf("str = %s\n\n", arr);

	{
		CRC64 crc;
		crc.ResetCrcTable(0x42F0E1EBA9EA3693ULL);
		crc.ResetCrcState(0xFFFFFFFFFFFFFFFFULL);
		crc.UpdateCrcState(arr, sizeof(arr) - 1);
		printf("CRC64 = 0x%llX\n", crc.GetCrcState(true, 0xFFFFFFFFFFFFFFFFULL));
	}
	
	{
		CRC32 crc;
		crc.ResetCrcTable(0x04C11DB7U);
		crc.ResetCrcState(0xFFFFFFFFU);
		crc.UpdateCrcState(arr, sizeof(arr) - 1);
		printf("CRC32 = 0x%X\n", crc.GetCrcState(true, 0xFFFFFFFFU));
	}
	
	{
		CRC16 crc;
		crc.ResetCrcTable(0x8005);
		crc.ResetCrcState(0);
		crc.UpdateCrcState(arr, sizeof(arr) - 1);
		printf("CRC16 = 0x%X\n", crc.GetCrcState());

	}

	{
		CRC8 crc;
		crc.ResetCrcTable(0x07);
		crc.ResetCrcState(0);
		crc.UpdateCrcState(arr, sizeof(arr) - 1);
		printf("CRC8 = 0x%X\n", crc.GetCrcState());

	}

	return 0;
}
