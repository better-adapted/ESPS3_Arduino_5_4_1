/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

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

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

void setup()
{
    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();	
}

int loop()
{
    return 0;
}
