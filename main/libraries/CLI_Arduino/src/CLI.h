#pragma once

#include <Arduino.h>

//TODO: add your function declarations here
#define CLI_SCAN_GET_OK						0x0002
#define CLI_SCAN_SET_OK						0x0001
#define CLI_SCAN_SET_ERR_MIN				0x8000
#define CLI_SCAN_SET_ERR_MAX				0x4000
	
float CLI_Scan_float_Setting(Stream *Stream, String pInput, String pCMD, float *pValue, float pMin, float pMax,const char* pFormat,String pOutputPrompt);
int32_t CLI_Scan_INT32_Setting(Stream *Stream, String pInput, String pCMD, int32_t *pValue, int32_t pMin, int32_t pMax, uint32_t pBase, String pOutputPrompt);
int16_t CLI_Scan_INT16_Setting(Stream *Stream, String pInput, String pCMD, int16_t *pValue, int16_t pMin, int16_t pMax, uint32_t pBase,String pOutputPrompt);
uint32_t CLI_Scan_UINT32_Setting(Stream *Stream, String pInput, String pCMD, uint32_t *pValue, uint32_t pMin, uint32_t pMax, uint32_t pBase, String pOutputPrompt);
uint16_t CLI_Scan_UINT16_Setting(Stream *Stream, String pInput, String pCMD, uint16_t *pValue, uint32_t pMin, uint16_t pMax, uint32_t pBase, String pOutputPrompt);
uint8_t CLI_Scan_UINT8_Setting(Stream *Stream, String pInput, String pCMD, uint8_t *pValue, uint32_t pMin, uint8_t pMax, uint32_t pBase, String pOutputPrompt);
String Cert_View(const char* pCert);
int CLI_Cert_Setting(Stream *Stream, String pInput,String pCMD,char *pValue,int len,String pPrefix);
int CLI_String_Setting(Stream *Stream, String pInput,String pCMD,char *pValue,int len,String pPrefix);
