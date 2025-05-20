#include "app_main_defines.h"
// #include <Arduino.h>
#include <inttypes.h>
#include "esp_heap_caps.h"
#include "esp_chip_info.h"
#include "esp_system.h"
#include "esp_log.h"
#include "soc/rtc.h"	//real time clock
#include "esp_pm.h"		//power management
#include "esp_partition.h"
#include "esp_system.h"
#include "spi_flash_mmap.h"
#include "soc/clk_tree_defs.h"

void esp_print_info(void);
void print_chip_MACS();
