#include "app_main_defines.h"

// Arduino EPS32 core version
//Commit: 8d5a026c986aa6edc09a003267ee65946a7f6046 [8d5a026]
//Parents: 6f92b604f6
//Author: Me No Dev <me-no-dev@users.noreply.github.com>
//Date: 15 May 2025 10:56:48
//Committer: GitHub
//IDF release/v5.4 (#11357)
//* IDF release/v5.4 8ad0d3d8
//* IDF release/v5.4 fd37cd46
//* IDF release/v5.4 fe753553

// ESP-IDF version used!
//Commit: fe75355314cf7beb5a7bfa40a31b0b653cca7fe7 [fe75355]
//Parents: fd37cd46b2, cccccd6da8
//Author: Marius Vikhammer <marius.vikhammer@espressif.com>
//Date: 12 May 2025 11:50:39
//Committer: Marius Vikhammer
//Merge branch 'bugfix/heap_in_flash_v5.4' into 'release/v5.4'
//fix(heap): fixed CONFIG_HEAP_PLACE_FUNCTION_INTO_FLASH not working together with CONFIG_HEAP_TLSF_USE_ROM_IMPL (v5.4)
//See merge request espressif/esp-idf!38121


#include <Arduino.h>

#include "App_Common.h"

#include <WiFi.h>

#include "esp_info.h"
#include <stdio.h>
#include <inttypes.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include <esp_task_wdt.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include <esp_wifi.h>

#include "esp_ota_ops.h"

void rgb_led_service();
void Service_BootSwitch();
void Service_ConfigSwitch();
void rgb_led_set_state(uint32_t pState,uint32_t pMilli,int pTrace);

typedef struct
{
	union
	{
		uint32_t all;
		struct
		{
			bool ShowMemUsage:1;
			bool PingDisable:1;			
			bool ShowWiFiStatus:1;
			bool ShowPCNTpoll:1;
			bool ShowLEDstatus:1;
			bool ShowLED_IO_writes:1;
			bool ShowWDTfeed:1;			
		}bits;
	};
}App_debug_flags_t;