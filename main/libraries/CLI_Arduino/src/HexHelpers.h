#include <Arduino.h>

int HH_isdigit(int c);
int HH_gethexval(int c);
int HH_ishexdigit(int c);
int HH_atoi(const char * s);
byte HH_hex_to_byte(const char * s);
int HH_String_Hex_to_Bytes(char *Buffer, uint8_t *Bytes, int pSize);
uint32_t HH_Bytes_to_uint32(const uint8_t * pBytes);

