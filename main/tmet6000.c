/* Copyright (C) Michael Yang
 * Written by Michael Yang <czhe.yang@gmail.com>, December 2017
 */

#include <errno.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_system.h"
#include "esp_adc_cal.h"
#include "esp_system.h"
#include "esp_log.h"
#include "sensors.h"
#include "utils.h"

#define TAG "TMET6000"

static uint32_t sGPIO;
static bool bIsInit = false;

int sensor_tmet6000_init(uint32_t *aGPIOs, uint32_t ui32NumGPIOs)
{
	if (!aGPIOs)
		return -EINVAL;

	if (!ui32NumGPIOs)
		return -EINVAL;

	if (ui32NumGPIOs > 1)
		ESP_LOGW(TAG, "More than one GPIO is set for TMET6000\n");

	sGPIO = *aGPIOs;

	ESP_LOGI(TAG, "GPIO %u for TMET6000\n", sGPIO);

	bIsInit = true;

	return 0;
}

void sensor_tmet6000_deinit(void)
{
	bIsInit = false;
}

int sensor_tmet6000_read(uint32_t *paData)
{
	adc1_config_width(ADC_WIDTH_BIT_10);
	adc1_config_channel_atten(getADCChannel(sGPIO), ADC_ATTEN_DB_11);
	paData[0] = adc1_get_raw(getADCChannel(sGPIO));

	return 0;
}
