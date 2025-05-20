#include "HexHelpers.h"

int HH_isdigit(int c)
{
	return (c <= (int)'9' && c >= (int)'0');
}

int HH_gethexval(int c)
{
	if(c <= (int)'9' && c >= (int)'0')
	{
		return c-(int)'0';
	}
	
	if(c <= (int)'f' && c >= (int)'a')
	{
		return c-(int)'a' + 10;
	}

	if(c <= (int)'F' && c >= (int)'A')
	{
		return c-(int)'A' + 10;
	}

	return -1;
}

int HH_ishexdigit(int c)
{
	if(HH_gethexval(c)>=0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int HH_atoi(const char * s) {
	int a;
	unsigned char sign;
	int digits=0;

	while (*s == ' ' || *s == '\t')
	s++;
	a = 0;
	sign = 0;
	if (*s == '-') {
		sign++;
		s++;
	} else if (*s == '+')
	s++;
	while( (HH_isdigit(*s)) && (digits<4) )
	{
		a = a * 10 + (*s -'0');
		s++;
		digits++;
	}
	if (sign)
	return -a;
	return a;
}

byte HH_hex_to_byte(const char *s)
{
	int digits=0;
	byte a;
	int Max=2;
	a = 0;

	while( (HH_ishexdigit(*s))&& (digits<Max) )
	{
		a = (a << 4) + HH_gethexval(*s);
		s++;
		digits++;
	}
	return a;
}

int HH_String_Hex_to_Bytes(char *Buffer, uint8_t *Bytes,int pSize)
{	
	for(int x = 0;x<pSize;x++)
	{
		Bytes[x] = HH_hex_to_byte((char *)Buffer[x*2]);
	}
}

uint32_t HH_Bytes_to_uint32(const uint8_t * pBytes)
{
	uint32_t TempU32 = 0;
	TempU32 |= pBytes[0];
	TempU32 <<= 8;
	TempU32 |= pBytes[1];
	TempU32 <<= 8;
	TempU32 |= pBytes[2];
	TempU32 <<= 8;
	TempU32 |= pBytes[3];
	
	return TempU32;	
}