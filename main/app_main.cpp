/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "app_main.h"

#ifdef APP_SUPPORT_OTA_PULL
#define OTA_PULL_JSON_URL   "http://www.sperrinlogic.com/firmware/update.json" // this is where you'll post your JSON filter file
#include "ESP32OTAPull.h"
ESP32OTAPull ota;
static int ota_ret;
#endif

#ifdef  APP_SUPPORT_OTA_BASIC
#include <HTTPUpdate.h>
#endif

#ifdef APP_SUPPORT_OTA_COMMON
static uint32_t ota_check_last_ms=0;
static uint32_t ota_check_flags=0;
static uint32_t ota_check_request=0;
static uint32_t ota_check_interval_mins = 60; // mins
#else
static uint32_t ota_check_last_ms=1;
#endif

const char *hardware_version = { "SLL_PULSER" };
const char *firmware_version = { "0.0.0.0" };

#ifdef APP_SUPPORT_CLI
#include <CLI.h>
void CLI_Service(Stream *pStream);
static uint32_t ESC_pressed=0;
#endif

Stream *Stream_UART0 = nullptr;
#ifdef APP_SUPPORT_UART2
Stream *Stream_UART2 = nullptr;
#endif

void WDT_Feed(int pTrace,bool pPoll);
static bool WDT_feed_disable;

static App_debug_flags_t DebugFlags;

String CLI_Prefix()
{
	static String temp;
	char temp_str[100]={};

	sprintf(temp_str,"@%010lu",millis());

	temp=temp_str;
	return temp;
}

#define HASH_LEN 32
static void print_sha256(Stream *pStream,const uint8_t *image_hash, const char *label)
{
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    pStream->printf("%s %s\r\n", label, hash_print);
}

static void get_sha256_of_partitions(Stream *pStream)
{
    uint8_t sha_256[HASH_LEN] = { 0 };
    esp_partition_t partition;

    // get sha256 digest for bootloader
    partition.address   = ESP_BOOTLOADER_OFFSET;
    partition.size      = ESP_PARTITION_TABLE_OFFSET;
    partition.type      = ESP_PARTITION_TYPE_APP;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(pStream,sha_256, "SHA-256 for bootloader: ");

    // get sha256 digest for running partition
    esp_partition_get_sha256(esp_ota_get_running_partition(), sha_256);
    print_sha256(pStream,sha_256, "SHA-256 for current firmware: ");
}

String Get_Version_Info_String()
{
	char temp[512]={};
	sprintf(temp,"HW=%s,FW=%s,DT=%s,TM=%s,MAC:%s", hardware_version,firmware_version, __DATE__, __TIME__,WiFi.macAddress().c_str());
	return temp;
}

void Version_Info(Stream *pStream)
{
	pStream->println(Get_Version_Info_String().c_str());
	get_sha256_of_partitions(pStream);
}

bool BootSwitchGet(void)
{
	if(digitalRead(_APP_BOOT_SWITCH)==0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

#ifdef APP_SUPPORT_PCNT_COUNT_UP
void pcnt_init_and_start()
{
    /* Prepare configuration for the PCNT unit */
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = PCNT_INPUT_SIG_IO,
        .ctrl_gpio_num = -1,
        .lctrl_mode = PCNT_MODE_KEEP, // Reverse counting direction if low
        .hctrl_mode = PCNT_MODE_KEEP,    // Keep the primary counter mode if high
        .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge
        .neg_mode = PCNT_COUNT_DIS,   // Keep the counter value on the negative edge
        .counter_h_lim = PCNT_H_LIM_VAL,
        .counter_l_lim = PCNT_L_LIM_VAL,
        .unit = PCNT_TEST_UNIT,
        .channel = PCNT_CHANNEL_0,
        // What to do on the positive / negative edge of pulse input?
        // What to do when control input is low or high?
        // Set the maximum and minimum limit values to watch
    };
    /* Initialize PCNT unit */
    pcnt_unit_config(&pcnt_config);

    /* Configure and enable the input filter */
    pcnt_set_filter_value(PCNT_TEST_UNIT, 100);
    pcnt_filter_enable(PCNT_TEST_UNIT);

    /* Initialize PCNT's counter */
    pcnt_counter_pause(PCNT_TEST_UNIT);
    pcnt_counter_clear(PCNT_TEST_UNIT);
    pcnt_counter_resume(PCNT_TEST_UNIT);
}

int16_t pcnt_get()
{
	int16_t PCNT_temp;
	pcnt_get_counter_value(PCNT_TEST_UNIT,&PCNT_temp);
	return PCNT_temp;
}

void pcnt_clear()
{
    pcnt_counter_clear(PCNT_TEST_UNIT);
}

#endif


void Setup_IO()
{
	pinMode(_APP_PULSE_OP0,INPUT);
	digitalWrite(_APP_PULSE_OP0,HIGH);
	pinMode(_APP_PULSE_OP0,OUTPUT);
	
	pinMode(_APP_PULSE_OP1,INPUT);
	digitalWrite(_APP_PULSE_OP1,HIGH);
	pinMode(_APP_PULSE_OP1,OUTPUT);
}

void setup()
{
	Setup_IO();

	// BASIC VERSION INFO
	Serial.begin(115200);
	Stream_UART0 = &Serial;
	Stream_UART0->println();
	Version_Info(Stream_UART0);

#ifdef APP_SUPPORT_UART2
	Serial2.begin(115200, SERIAL_8N1, 16, 17);
	Stream_UART2 = &Serial2;
	Stream_UART2->println();
	Version_Info(Stream_UART2);
#endif


	Stream_UART0->println("setup() started");
	Stream_UART2->println("setup() started");

    esp_restart();	
}

void mem_check()
{
  static uint32_t last_mem_check;
  if((millis()- last_mem_check)>10000)
  {
    last_mem_check=millis();

    static const char *TAG = "Main File";
    ESP_LOGI(TAG, "[APP] Free memory: %lu bytes", (unsigned long)esp_get_free_heap_size());
    ESP_LOGI(TAG, "This Task watermark: %lu bytes",(unsigned long) uxTaskGetStackHighWaterMark(NULL));
  }
}

void CLI_ESC_Req()
{
	ESC_pressed=millis();
}

void CLI_ESC_Cancel()
{
	ESC_pressed=0;
}

void ESC_service()
{
	if((millis() - ESC_pressed) > 900000)
	{
		ESC_pressed=0;
	}	
}

bool ESC_Active()
{
	if(ESC_pressed)
	{
		return true;
	}
	else
	{
		return false;		
	}
}

uint32_t ESC_ActiveTimestamp()
{
	return ESC_pressed;
}

String Wifi_Status_As_String(int pMode)
{
	String Temp="";
	bool Known=false;
	
	if(pMode==WL_NO_SHIELD)
	{
		Known=true;
		Temp="WL_NO_SHIELD";
	}
	
	if(pMode==WL_STOPPED)
	{
		Known=true;
		Temp="WL_STOPPED";
	}
	
	if(pMode==WL_IDLE_STATUS)
	{
		Known=true;
		Temp="WL_IDLE_STATUS";
	}
	
	if(pMode==WL_NO_SSID_AVAIL)
	{
		Known=true;
		Temp="WL_NO_SSID_AVAIL";
	}
	
	if(pMode==WL_SCAN_COMPLETED)
	{
		Known=true;
		Temp="WL_SCAN_COMPLETED";
	}
	
	if(pMode==WL_CONNECTED)
	{
		Known=true;
		Temp="WL_CONNECTED";
	}
	
	if(pMode==WL_CONNECT_FAILED)
	{
		Known=true;
		Temp="WL_CONNECT_FAILED";
	}
	
	if(pMode==WL_CONNECTION_LOST)
	{
		Known=true;
		Temp="WL_CONNECTION_LOST";
	}
	
	if(pMode==WL_DISCONNECTED)
	{
		Known=true;
		Temp="WL_CONNECTION_LOST";
	}

	if(!Known)
	{
		Temp = "UNKOWN:" + pMode;	
	}	
	
	return Temp;
}

int CLI_Process_String(Stream *pOpStream,String pInput)
{
	String Cmd;
	int processed = 0;
	String Prompt = ">";	

	if (pInput.startsWith("RESET"))
	{
		processed = 1;
		pOpStream->println("OK");
		ESP.restart();
	}

	if (pInput.startsWith("ESC?"))
	{
		processed = 1;
		pOpStream->printf("ESC_pressed=%ld\r\n",ESC_ActiveTimestamp());
		pOpStream->println("OK");
	}

	if (pInput.startsWith("ESC=1"))
	{
		processed = 1;
		CLI_ESC_Req();
		pOpStream->println("OK");
	}

	if (pInput.startsWith("ESC=0"))
	{
		processed = 1;
		CLI_ESC_Cancel();
		pOpStream->println("OK");
	}
	
	if (pInput.startsWith("MACS?"))
	{
		processed = 1;
		print_chip_MACS();
		pOpStream->println("OK");
	}
	
	if (pInput.startsWith("I2C_SCAN"))
	{
		processed = 1;
		//Scan_i2c(pOpStream);
		pOpStream->println("OK");
	}
	
	// WIFI_CREDS=ESP32_Test,mezzanine
	Cmd="WIFI_CREDS=";
	if (pInput.startsWith(Cmd))
	{
		int comma_index = pInput.indexOf(",");
		if(comma_index>0)
		{
			processed = 1;
			String temp_ssid = pInput.substring(Cmd.length(),comma_index);
			String temp_password = pInput.substring(comma_index+1);
			pOpStream->printf("SSID=%s,PASS=%s\r\n",temp_ssid.c_str(),temp_password.c_str());
			pOpStream->println("OK");

			WiFi.disconnect();
			WiFi.begin(temp_ssid,temp_password);
		}
	}

	if (pInput.startsWith("WDT_TEST"))
	{
		processed = 1;
		WDT_Feed(4,false); // one last feed before a disable!
		WDT_feed_disable=true;
		pOpStream->println("OK");
	}
	
	if (pInput.startsWith("VERSION"))
	{
		processed = 1;
		Version_Info(pOpStream);
		pOpStream->println("OK");
	}

	if (pInput.startsWith("MAC?"))
	{
		processed = 1;
		char temp[200]={};
		String mac_str = WiFi.macAddress().c_str();
		mac_str.replace(":","");
		sprintf(temp,"MAC:%s", mac_str.c_str());
		pOpStream->println(temp);
		pOpStream->println("OK");
	}
	
	if(CLI_Scan_UINT32_Setting(pOpStream, pInput, "DBG",&DebugFlags.all, 0, 0xFFFFFFFF, 16,Prompt))
	{
		processed = 1;
		pOpStream->println("OK");
		return 0;
	}	

#ifdef APP_SUPPORT_OTA_COMMON
	if (pInput.startsWith("OTA_CHECK"))
	{
		ota_check_request=1;
		pOpStream->println("OK");
	}
#endif		

	if (pInput.startsWith("EPOCH"))
	{
		processed = 1;
		char temp[200];
		sprintf(temp, "EPOCH=%lu", TimeEpochGet(pOpStream));
		pOpStream->println(temp);
		pOpStream->println("OK");
	}

#ifdef APP_SUPPORT_FILESYSTEM
	if (pInput.startsWith("FS_FORMAT"))
	{
		processed = 1;
		FileSystem.format(pOpStream);
	}

	if (pInput.startsWith("FS_INFO"))
	{
		processed = 1;
		FileSystem.Info(pOpStream);
	}

	if (pInput.startsWith("FILE_"))
	{
		processed = FileCommmand(pOpStream,&pInput);
	}
#endif

	if ((processed == 0) && (pInput.length() > 0))
	{
		pOpStream->printf(">%s ?\r\n",pInput.c_str());
	}
	
	return processed;
}


void CLI_Service(Stream *pInStream,Stream *pOutStream)
{
	static bool stringComplete = false;
	static String inputString;
	
	while (pInStream->available())
	{
		// get the new byte:
		char inChar = (char) pInStream->read();

		if(inChar == 27) 
		{
			if(millis()<90000)
			{
				CLI_ESC_Req();
			}
		}

		// if the incoming character is a newline, set a flag
		// so the main loop can do something about it:
		if (inChar == '\r' || inChar == 0x04)
		{
			stringComplete = true;
			pOutStream->println(inputString);
		}
		else
		{
			// add it to the inputString:
			inputString += inChar;
		}
	}

	if (stringComplete)
	{
		CLI_Process_String(pOutStream,inputString);
		
		stringComplete = false;
		inputString = "";
	}
}

void CLI_loop()
{
	CLI_Service(Stream_UART0,Stream_UART0);
	CLI_Service(Stream_UART2,Stream_UART2);
}

#ifdef APP_HTTP_POST
void http_post()
{
  WiFiClient client;
  HTTPClient http;

  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  
  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Data to send with HTTP POST
  char post_temp[1024]={};
  snprintf(post_temp,sizeof(post_temp),"HW=%s&FW=%s&DT=%s&TM=%s&MAC:%s&EP=%lu", hardware_version,firmware_version, __DATE__, __TIME__,WiFi.macAddress().c_str(),TimeEpochGet());           

  // Send HTTP POST request
  int httpResponseCode = http.POST(post_temp);
       
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
    
  // Free resources
  http.end();
}
#endif

#ifdef APP_SUPPORT_OTA_PULL
void ota_callback(int offset, int totallength)
{
	Stream_UART0->printf("OTA,Updating %d of %d (%02d%%)...\r\n", offset, totallength, 100 * offset / totallength);
	WDT_Feed(2,true);
}

const char *ota_errtext(int code)
{
	switch(code)
	{
		case ESP32OTAPull::UPDATE_AVAILABLE_FORCE_REQ:
			return "An update is available but wasn't installed - FORCE requested";		
		case ESP32OTAPull::UPDATE_AVAILABLE:
			return "An update is available but wasn't installed";
		case ESP32OTAPull::NO_UPDATE_PROFILE_FOUND:
			return "No profile matches";
		case ESP32OTAPull::NO_UPDATE_AVAILABLE:
			return "Profile matched, but update not applicable";
		case ESP32OTAPull::UPDATE_OK:
			return "An update was done, but no reboot";
		case ESP32OTAPull::HTTP_FAILED:
			return "HTTP GET failure";
		case ESP32OTAPull::WRITE_ERROR:
			return "Write error";
		case ESP32OTAPull::JSON_PROBLEM:
			return "Invalid JSON";
		case ESP32OTAPull::OTA_UPDATE_FAIL:
			return "Update fail (no OTA partition?)";
		default:
			if (code > 0)
				return "Unexpected HTTP response code";
			break;
	}
	return "Unknown error";
}
#endif

#ifdef APP_SUPPORT_OTA_COMMON
void ota_check(ESP32OTAPull::ActionType pAction)
{
    if (WiFi.status() != WL_CONNECTED)
    {
		static uint32_t ota_wifi_offline_msg;
		
		if((millis()-ota_wifi_offline_msg)>5000)
		{
			ota_wifi_offline_msg=millis();
			Stream_UART0->printf("%s,OTA,pAction=%d,No Wifi-returning\r\n",CLI_Prefix().c_str(),(int)pAction);
		}
		return;
	}
	
	static bool done_once;
	
	ota_check_flags=0;
	
	if(!done_once)
	{
		ota_check_flags=1;
		done_once=true;
	}
				
	if((millis()-ota_check_last_ms) > (ota_check_interval_mins*60*1000))
	{
		ota_check_flags=2;
	}

	if(ota_check_request)
	{
		ota_check_flags=3;
		ota_check_request=0;
	}

	if(ota_check_flags)
	{
		ota_check_last_ms=millis();
		String mac_str = WiFi.macAddress().c_str();
		mac_str.replace(":","");
				
		Stream_UART0->println();
    	Stream_UART0->printf("%s,OTA,Free memory: %lu bytes", CLI_Prefix().c_str(),esp_get_free_heap_size());		
		Stream_UART0->println();		
		Stream_UART0->printf("%s,OTA,check_flags=%ld,pAction=%d,FW:%s,HW:='%s',MAC:='%s'.\r\n",CLI_Prefix().c_str(),ota_check_flags,(int)pAction, firmware_version, hardware_version, mac_str.c_str());
		
#ifdef APP_SUPPORT_OTA_PULL
		ota.SetCallback(ota_callback);
		ota.OverrideBoard(hardware_version);
		ota.OverrideDevice(mac_str.c_str());
		ota.SetConfig("16MB");
		ota.AllowDowngrades(true);
		
		Stream_UART0->printf("%s,OTA,Checking %s\r\n",CLI_Prefix().c_str(), OTA_PULL_JSON_URL);
		ota_ret = ota.CheckForOTAUpdate(OTA_PULL_JSON_URL, firmware_version,pAction);
		Stream_UART0->printf("%s,OTA,CheckForOTAUpdate()=%d (%s)\r\n",CLI_Prefix().c_str(), ota_ret, ota_errtext(ota_ret));		
		Stream_UART0->println();
		Stream_UART0->println();

		if(ota_ret == ESP32OTAPull::UPDATE_AVAILABLE_FORCE_REQ)
		{
	      	Serial.printf("%s,UPDATE_AVAILABLE_FORCE_REQ\r\n",CLI_Prefix().c_str());
      		Serial.printf("%s,RESETING\r\n",CLI_Prefix().c_str());
      		delay(5000);
      		ESP.restart();
	    }		
#endif

#ifdef APP_SUPPORT_OTA_BASIC
		NetworkClient client;
		String binfile_url = "http://multisense.ddns.net/firmware/MultiSenseApp.bin";
		Stream_UART0->printf("%s,Free memory: %lu bytes", CLI_Prefix().c_str(),esp_get_free_heap_size());
		Stream_UART0->printf("%s,OTA,Checking %s\r\n",CLI_Prefix().c_str(), binfile_url.c_str());
	    t_httpUpdate_return ret = httpUpdate.update(client, binfile_url.c_str());
	
	    switch (ret)
	    {
	      case HTTP_UPDATE_FAILED: 
	      	Serial.printf("%s,HTTP_UPDATE_FAILED Error (%d): %s\r\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
	      	break;
	
	      case HTTP_UPDATE_NO_UPDATES:
	      	Serial.printf("%s,HTTP_UPDATE_NO_UPDATES\r\n",CLI_Prefix().c_str());
	      	break;
	
	      case HTTP_UPDATE_OK: 
	      	Serial.printf("%s,HTTP_UPDATE_OK\r\n",CLI_Prefix().c_str()); 
	      	break;
	
	      default: 
	      	Serial.printf("%s,HTTP_UPDATE_UNKOWN=%d\r\n",CLI_Prefix().c_str(),(int)ret); 
	      	break;
	    }
#endif
	}
}
#endif

void wifi_check()
{
  if(!DebugFlags.bits.ShowWiFiStatus)
  {
	  return;
  }
  	
  static int WiFi_status_now = WL_IDLE_STATUS;
  static int WiFi_status_prev = WL_IDLE_STATUS;

  WiFi_status_now = WiFi.status();

  static uint32_t last_wifi_check;

  char temp[300]={};
  sprintf(temp,"%s,",CLI_Prefix().c_str());

  if ( (WiFi_status_prev!=WiFi_status_now) || ((millis()- last_wifi_check)>=10000) )
  {
    last_wifi_check=millis();
    WiFi_status_prev = WiFi_status_now;
    
    strcat(temp,Wifi_Status_As_String(WiFi_status_now).c_str());

    Stream_UART0->println(temp);
    Stream_UART2->println(temp);
  }
}

void WDT_Feed(int pTrace,bool pPoll)
{
	if(DebugFlags.bits.ShowWDTfeed)
	{
		Stream_UART0->printf("%s,WDT_Feed(%d,%d)\r\n",CLI_Prefix().c_str(),pTrace,pPoll);
	}
}

void loop()
{
	static bool loop_started_msg;
	if(!loop_started_msg)
	{
		Stream_UART0->println("loop() started");
		Stream_UART2->println("loop() started");
		loop_started_msg=true;		
	}
	
#ifdef APP_SUPPORT_PCNT_COUNT_UP
	 app_pcnt_poll();
#endif

	CLI_loop();	
	ESC_service();
	//Service_ConfigSwitch();
	//Service_BootSwitch();	

	if(!ESC_Active())
	{

		wifi_check();

		mem_check();
	}
	
	if(!WDT_feed_disable)
	{
		// WDT_service
	}
	
	delay(10);
}

