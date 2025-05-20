#include "App_Common.h"
#include "HardwareSerial.h"
#include "esp32-hal.h"

unsigned long TimeEpochGet(Stream* pStream)
{
	time_t now;
	struct tm timeinfo;
	uint32_t start_us =  micros();
	if (!getLocalTime(&timeinfo))
	{
		return (0);
	}
	uint32_t stop_us =  micros();
	time(&now);
	uint32_t diff_us = stop_us - start_us;
	
	if(pStream)
		pStream->printf("TimeEpochGet(),diff_us=%ld\r\n",diff_us);
	return now;
}

String WiFi_macAddress_as_String()
{
	uint8_t mac_temp[6];
			
	uint64_t temp = ESP.getEfuseMac();

	mac_temp[0] = (temp >> 0);
	mac_temp[1] = (temp >> 8);
	mac_temp[2] = (temp >> 16);
	mac_temp[3] = (temp >> 24);
	mac_temp[4] = (temp >> 32);
	mac_temp[5] = (temp >> 40);
		
	static String MacTemp = "";
	char temp_str[20];
		
	for (int x = 0; x < sizeof(mac_temp); x++)
	{
		sprintf(temp_str, "%02X", mac_temp[x]);
		MacTemp += temp_str;
	}		
		
	return MacTemp;
}

uint64_t MacBytesToU64(uint8_t * pMacBytes)
{
	// lsb byte first
	uint64_t Mac = 0;
	
	uint64_t Byte0 =  uint64_t(pMacBytes[0]);
	uint64_t Byte1 =  uint64_t(pMacBytes[1]);
	uint64_t Byte2 =  uint64_t(pMacBytes[2]);
	uint64_t Byte3 =  uint64_t(pMacBytes[3]);
	uint64_t Byte4 =  uint64_t(pMacBytes[4]);
	uint64_t Byte5 =  uint64_t(pMacBytes[5]);
	
	Byte0 <<= 0;
	Byte1 <<= 8;
	Byte2 <<= 16;
	Byte3 <<= 24;
	Byte4 <<= 32;
	Byte5 <<= 40;
	
	Mac = Byte5 | Byte4 | Byte3 | Byte2 | Byte1 | Byte0;
	
	return Mac;
}

void MacU64toString(uint64_t pMac, char *MacStringTemp)
{
	// lsb byte first
	String MacString = "";
	
	unsigned int Byte0 =  (pMac >> 0) & 0xFF;
	unsigned int Byte1 =  (pMac >> 8) & 0xFF;
	unsigned int Byte2 =  (pMac >> 16) & 0xFF;
	unsigned int Byte3 =  (pMac >> 24) & 0xFF;
	unsigned int Byte4 =  (pMac >> 32) & 0xFF;
	unsigned int Byte5 =  (pMac >> 40) & 0xFF;

	sprintf(MacStringTemp, "%02X%02X%02X%02X%02X%02X", Byte5, Byte4, Byte3, Byte2, Byte1, Byte0);
}

void Hex_bytes_to_Ascii(const uint8_t *data,int pLength,char *pDest)
{
	int i = 0;

	char temp[10];

	for (int x = 0; x < pLength; x++)
	{
		sprintf(temp, "%02X", data[x]);
		pDest[i] = temp[0];
		pDest[i+1] = temp[1];
		pDest[i+2] = 0;
		i += 2;
	}
}

int Tiny_gethexval(int c)
{
	if (c <= (int)'9' && c >= (int)'0')
	{
		return c - (int)'0';
	}

	if (c <= (int)'f' && c >= (int)'a')
	{
		return c - (int)'a' + 10;
	}

	if (c <= (int)'F' && c >= (int)'A')
	{
		return c - (int)'A' + 10;
	}

	return -1;
}

uint8_t ConvertAsciiToHex(uint8_t char1, uint8_t char2)
{
	uint8_t ByteLow = 0;
	uint8_t ByteHigh = 0;

	ByteLow = (uint8_t)Tiny_gethexval(char2);
	ByteHigh = (uint8_t)Tiny_gethexval(char1);
	ByteHigh <<= 4;

	return (ByteHigh | ByteLow);
}


String file_md5 (File & f)
{
    if (!f) {
        return String();
    }

    if (f.seek(0, SeekSet)) {

        MD5Builder md5;
        md5.begin();
        md5.addStream(f, f.size());
        md5.calculate();
        return md5.toString();
    }

    return String();
}


