#include "CLI.h"

#include <Arduino.h>

float CLI_Scan_float_Setting(Stream *Stream, String pInput, String pCMD, float *pValue, float pMin, float pMax,const char* pFormat,String pOutputPrompt)
{
	int ret_val = 0;

	if (pInput.startsWith(pCMD))
	{
		String ResponseAppendString = "";

		int index = pCMD.length();

		bool echo_value = false;

		if (pInput[index] == '?')
		{
			ret_val |= CLI_SCAN_GET_OK;
			echo_value = true;
		}
		else if (pInput[index] == '=')
		{
			int temp_value = strtof(pInput.substring(index + 1).c_str(), 0);        // decimal conversion

			echo_value = true;

			// check the value is within range
			if(temp_value > pMax)
			{
				ret_val |= CLI_SCAN_SET_ERR_MAX;       // out of range!
				ResponseAppendString += ",ERR_MAX";
			}
			else if(temp_value < pMin)
			{
				ret_val |= CLI_SCAN_SET_ERR_MIN;       // out of range!
				ResponseAppendString += ",ERR_MIN";
			}
			else
			{
				if (*pValue != temp_value)
				{
					*pValue = temp_value;
					ret_val |= CLI_SCAN_SET_OK;
					ResponseAppendString += ",OK";
				}
				else
				{
					ResponseAppendString += ",SAME";
				}
			}
		}
		else
		{
			// invalid input - not ? or =
			return ret_val;
		}

		if (echo_value)
		{
			Stream->print(pOutputPrompt);
			Stream->print(pCMD);
			Stream->print("=");
			Stream->printf(pFormat,*pValue);
			Stream->println(ResponseAppendString);
		}
	}

	return ret_val;
}

int32_t CLI_Scan_INT32_Setting(Stream *Stream, String pInput, String pCMD, int32_t *pValue, int32_t pMin, int32_t pMax, uint32_t pBase,String pOutputPrompt)
{	
	int ret_val = 0;
	
	if (pInput.startsWith(pCMD))
	{
		String ResponseAppendString = "";
		
		int index = pCMD.length();
			
		bool echo_value = false;
			
		if (pInput[index] == '?')
		{
			ret_val |= CLI_SCAN_GET_OK;
			echo_value = true;
		}			
		else if (pInput[index] == '=')
		{
			int temp_value = strtol(pInput.substring(index + 1).c_str(), 0, pBase);        // decimal conversion
			
			echo_value = true;
				
			// check the value is within range
			if(temp_value > pMax)
			{
				ret_val |= CLI_SCAN_SET_ERR_MAX;       // out of range!
				ResponseAppendString += ",ERR_MAX";
			}			
			else if(temp_value < pMin)
			{
				ret_val |= CLI_SCAN_SET_ERR_MIN;       // out of range!
				ResponseAppendString += ",ERR_MIN";
			}
			else 
			{
				if (*pValue != temp_value)
				{
					*pValue = temp_value;
					ret_val |= CLI_SCAN_SET_OK;
					ResponseAppendString += ",OK";					
				}
				else
				{
					ResponseAppendString += ",SAME";
				}
			}
		}
		else
		{
			// invalid input - not ? or =
			return ret_val;
		}
		
		if (echo_value)
		{
			Stream->print(pOutputPrompt);
			Stream->print(pCMD);
			Stream->print("=");
			Stream->print(*pValue, pBase);
			Stream->println(ResponseAppendString);
		}		
	}
		
	return ret_val;
}

int16_t CLI_Scan_INT16_Setting(Stream *Stream, String pInput, String pCMD, int16_t *pValue, int16_t pMin, int16_t pMax, uint32_t pBase,String pOutputPrompt)
{
	int ret_val = 0;

	if (pInput.startsWith(pCMD))
	{
		String ResponseAppendString = "";

		int index = pCMD.length();

		bool echo_value = false;

		if (pInput[index] == '?')
		{
			ret_val |= CLI_SCAN_GET_OK;
			echo_value = true;
		}
		else if (pInput[index] == '=')
		{
			int temp_value = strtol(pInput.substring(index + 1).c_str(), 0, pBase);        // decimal conversion

			echo_value = true;

			// check the value is within range
			if(temp_value > pMax)
			{
				ret_val |= CLI_SCAN_SET_ERR_MAX;       // out of range!
				ResponseAppendString += ",ERR_MAX";
			}
			else if(temp_value < pMin)
			{
				ret_val |= CLI_SCAN_SET_ERR_MIN;       // out of range!
				ResponseAppendString += ",ERR_MIN";
			}
			else
			{
				if (*pValue != temp_value)
				{
					*pValue = temp_value;
					ret_val |= CLI_SCAN_SET_OK;
					ResponseAppendString += ",OK";
				}
				else
				{
					ResponseAppendString += ",SAME";
				}
			}
		}
		else
		{
			// invalid input - not ? or =
			return ret_val;
		}

		if (echo_value)
		{
			Stream->print(pOutputPrompt);
			Stream->print(pCMD);
			Stream->print("=");
			Stream->print(*pValue, pBase);
			Stream->println(ResponseAppendString);
		}
	}

	return ret_val;
}

uint32_t CLI_Scan_UINT32_Setting(Stream *Stream, String pInput, String pCMD, uint32_t *pValue, uint32_t pMin, uint32_t pMax, uint32_t pBase, String pOutputPrompt)
{	
	int ret_val = 0;
	
	if (pInput.startsWith(pCMD))
	{
		String ResponseAppendString = "";
		
		int index = pCMD.length();
			
		bool echo_value = false;
			
		if (pInput[index] == '?')
		{
			ret_val |= CLI_SCAN_GET_OK;
			echo_value = true;
		}			
		else if (pInput[index] == '=')
		{
			uint32_t temp_value = strtol(pInput.substring(index + 1).c_str(), 0, pBase);         // decimal conversion
			
			echo_value = true;
				
			// check the value is within range
			if(temp_value > pMax)
			{
				ret_val |= CLI_SCAN_SET_ERR_MAX;        // out of range!
				ResponseAppendString += ",ERR_MAX";
			}			
			else if(temp_value < pMin)
			{
				ret_val |= CLI_SCAN_SET_ERR_MIN;         // out of range!
				ResponseAppendString += ",ERR_MIN";
			}			
			else
			{
				if (*pValue != temp_value)
				{
					*pValue = temp_value;
					ret_val |= CLI_SCAN_SET_OK;
					ResponseAppendString += ",OK";					
				}
				else
				{
					ResponseAppendString += ",SAME";
				}
			}
		}
		else
		{
			// invalid input - not ? or =
			return ret_val;
		}
		
		if (echo_value)
		{
			Stream->print(pOutputPrompt);			
			Stream->print(pCMD);
			Stream->print("=");
			Stream->print(*pValue, pBase);
			Stream->println(ResponseAppendString);			
		}		
	}
		
	return ret_val;
}

uint16_t CLI_Scan_UINT16_Setting(Stream *Stream, String pInput, String pCMD, uint16_t *pValue, uint32_t pMin, uint16_t pMax, uint32_t pBase,String pOutputPrompt)
{	
	int ret_val = 0;
	
	if (pInput.startsWith(pCMD))
	{
		String ResponseAppendString = "";
		
		int index = pCMD.length();
			
		bool echo_value = false;
			
		if (pInput[index] == '?')
		{
			ret_val |= CLI_SCAN_GET_OK;
			echo_value = true;
		}			
		else if (pInput[index] == '=')
		{
			uint32_t temp_value = strtol(pInput.substring(index + 1).c_str(), 0, pBase);          // decimal conversion
			
			echo_value = true;
				
			// check the value is within range
			if(temp_value > pMax)
			{
				ret_val |= CLI_SCAN_SET_ERR_MAX;          // out of range!
				ResponseAppendString += ",ERR_MAX";
			}
			else if(temp_value < pMin)
			{
				ret_val |= CLI_SCAN_SET_ERR_MIN;          // out of range!
				ResponseAppendString += ",ERR_MIN";
			}						
			else
			{
				if (*pValue != temp_value)
				{
					*pValue = temp_value;
					ret_val |= CLI_SCAN_SET_OK;
					ResponseAppendString += ",OK";					
				}
				else
				{
					ResponseAppendString += ",SAME";
				}
			}
		}
		else
		{
			// invalid input - not ? or =
			return ret_val;
		}
		
		if (echo_value)
		{
			Stream->print(pOutputPrompt);			
			Stream->print(pCMD);
			Stream->print("=");
			Stream->print(*pValue, pBase);
			Stream->println(ResponseAppendString);			
		}		
	}
		
	return ret_val;
}

uint8_t CLI_Scan_UINT8_Setting(Stream *Stream, String pInput, String pCMD, uint8_t *pValue, uint32_t pMin, uint8_t pMax, uint32_t pBase, String pOutputPrompt)
{	
	int ret_val = 0;
	
	if (pInput.startsWith(pCMD))
	{
		String ResponseAppendString = "";
		
		int index = pCMD.length();
			
		bool echo_value = false;
			
		if (pInput[index] == '?')
		{
			ret_val |= CLI_SCAN_GET_OK;
			echo_value = true;
		}			
		else if (pInput[index] == '=')
		{
			uint32_t temp_value = strtol(pInput.substring(index + 1).c_str(), 0, pBase);           // decimal conversion
			
			echo_value = true;
				
			// check the value is within range
			if(temp_value > pMax)
			{
				ret_val |= CLI_SCAN_SET_ERR_MAX;          // out of range!
				ResponseAppendString += ",ERR_MAX";
			}
			else if(temp_value < pMin)
			{
				ret_val |= CLI_SCAN_SET_ERR_MIN;           // out of range!
				ResponseAppendString += ",ERR_MIN";
			}			
			else
			{
				if (*pValue != temp_value)
				{
					*pValue = temp_value;
					ret_val |= CLI_SCAN_SET_OK;
					ResponseAppendString += ",OK";					
				}
				else
				{
					ResponseAppendString += ",SAME";
				}
			}
		}
		else
		{
			// invalid input - not ? or =
			return ret_val;
		}
		
		if (echo_value)
		{
			Stream->print(pOutputPrompt);			
			Stream->print(pCMD);
			Stream->print("=");
			Stream->print(*pValue, pBase);
			Stream->println(ResponseAppendString);			
		}		
	}
		
	return ret_val;
}

String Cert_View(const char* pCert)
{
	if(pCert==0)
	{
		return "";
	}

	String temp = pCert;

	int org_len = temp.length();

	temp.replace("\n","\r\n");

	char temp_c[100]={};
	sprintf(temp_c,"\r\n[Cert Size=%d]\r\n",org_len);

	return temp + temp_c;
}

int CLI_Cert_Setting(Stream *pStream, String pInput,String pCMD,char *pValue,int len,String pPrefix)
{
	int index = pCMD.length();
	bool echo_value = false;
	bool processed = false;
	
	if(pInput.startsWith(pCMD))
	{
//		pStream->printf("pPrefix=%s,pInput=%s,pCMD=%s\r\n",pPrefix.c_str(), pInput.c_str(), pCMD.c_str());
//		pStream->printf("index=%d,%c\r\n",index,pInput.charAt(index));	

		if (pInput[index] == '?')
		{
			echo_value=true;
			processed=1;
		}

		if (pInput[index] == '=')
		{
			echo_value=true;
			String temp = pInput.substring(pCMD.length()+1);
			strncpy(pValue,temp.c_str(),len-1);
			processed=1;
		}
	}

	if(echo_value)
	{
		pStream->printf("%s%s=%s\r\n",pPrefix.c_str(), pCMD.c_str(),Cert_View(pValue).c_str());
		pStream->println("OK");
	}

	if(processed)
		return 0;
	else
		return 1;
}


int CLI_String_Setting(Stream *Stream, String pInput,String pCMD,char *pValue,int len,String pPrefix)
{
	int index = pCMD.length();
	bool echo_value = false;
	bool processed = false;
	
	if(pInput.startsWith(pCMD))
	{
//		pSerialPort->printf("pPrefix=%s,pInput=%s,pCmd=%s\r\n",pPrefix.c_str(), pInput.c_str(), pCMD.c_str());
//		pSerialPort->printf("index=%d,%c\r\n",index,pInput.charAt(index));

		if (pInput[index] == '?')
		{
			echo_value=true;
			processed=1;
		}

		if (pInput[index] == '=')
		{
			echo_value=true;
			String temp = pInput.substring(pCMD.length()+1);
			strncpy(pValue,temp.c_str(),len-1);
			processed=1;
		}
	}

	if(echo_value)
	{
		Stream->printf("%s%s:\r\n%s\r\n",pPrefix.c_str(), pCMD.c_str(),pValue);
		Stream->println("OK");
	}

	if(processed)
		return 0;
	else
		return 1;
}
