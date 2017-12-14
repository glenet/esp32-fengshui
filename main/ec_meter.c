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
#include "utils.h"
#include "sensors.h"
#include "ds18b20.h"

#define TAG "EC"
#define EC_MAX_SAM_SIZE  200 /* 200 * 20 (ms) */

static uint32_t sGPIOEC, sGPIODS18B20;
static bool bIsInit = false;
static uint32_t sNumSamplingSize = 0;
static int aSamples[EC_MAX_SAM_SIZE];

int sensor_ec_meter_init(uint32_t *aGPIOs, uint32_t ui32NumGPIOs)
{
	int err = -EINVAL;

	if (!aGPIOs)
		goto err_out;

	if (!ui32NumGPIOs)
		goto err_out;

	/* We should get two gpios, one for EC meter and the onther one for DS18B20 */
	if (ui32NumGPIOs != 2) {
		ESP_LOGE(TAG, "Two GPIOs, EC meter and DS18B20 but %u\n", ui32NumGPIOs);
		goto err_out;
	}

	/* We assume that aGPIO[0] for EC, aGPIO[1] for DS18B20 */
	sGPIOEC = aGPIOs[0];
	sGPIODS18B20 = aGPIOs[1];

	/* Initialize dsb18b20 library */
	ds18b20_init(sGPIODS18B20);

	ESP_LOGI(TAG, "GPIO %u for EC %u for DS18B20\n", sGPIOEC, sGPIODS18B20);

	bIsInit = true;
	err = 0;

err_out:
	return err;
}

void sensor_ec_meter_deinit(void)
{
	bIsInit = false;
}

static float getAverge(int *paData, uint32_t ui32Size)
{
	float average = 0;

	for (int i = 0; i < ui32Size; i++)
		average += paData[i];

	return (average / ui32Size);
}

int sensor_ec_meter_read(uint32_t *paData)
{
	float fVoltage, fTemperature, fTempCoefficient;
	float fCoefficientVolatge, fECcurrent;
	uint32_t ui32Channel;
	int err = -EINVAL;

	if (!bIsInit)
		goto err_out;

	if (!paData) {
		err = -EINVAL;
		goto err_out;
	}

	ui32Channel = getADCChannel(sGPIOEC);
	adc1_config_width(ADC_WIDTH_BIT_10);
	adc1_config_channel_atten(ui32Channel, ADC_ATTEN_DB_11);
	aSamples[sNumSamplingSize++] = adc1_get_raw(ui32Channel);
	/* If we're sampling for 4 ms, going to send the data out */
	if (sNumSamplingSize == EC_MAX_SAM_SIZE)
	{
		fTemperature = ds18b20_get_temp();
		fVoltage = getAverge(aSamples, sNumSamplingSize) * 5000 / 1024;
		fTempCoefficient = 1.0 + 0.01413 * (fTemperature - 25.0);
		fCoefficientVolatge = fVoltage / fTempCoefficient;
		ESP_LOGI(TAG, "analog %f, temp %f \n", getAverge(aSamples, sNumSamplingSize) , fTemperature);
		if ((fCoefficientVolatge < 150) || (fCoefficientVolatge > 3300))
		{
			/* Invalid data for EC value 
			 * Sending fake data to service.
			 */
			ESP_LOGE(TAG, "EC value out of range\n");
			goto err_out;
		}
		if (fCoefficientVolatge <= 448)
			fECcurrent = 6.84 * fCoefficientVolatge - 64.32;
		else if (fCoefficientVolatge <= 1457)
			fECcurrent= 6.98 * fCoefficientVolatge - 127;
		else
			fECcurrent= 5.3 * fCoefficientVolatge + 2278;
		/* Initialize the conter */
		sNumSamplingSize = 0;

		ESP_LOGI(TAG, "EC %f, temp %f \n", fECcurrent, fTemperature);
		paData[0] = (uint32_t)(fECcurrent);
		paData[1] = (uint32_t)(fTemperature * 100);
		return 0;
	}

	err = -EAGAIN;

err_out:
	return err;
}
