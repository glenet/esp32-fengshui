#include <errno.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_system.h"
#include "esp_adc_cal.h"
#include "esp_system.h"
#include "esp_log.h"
#include "sensors.h"

#define PH_METER_OFFSET (-1.66)

#define TAG "PH"
#define PH_MAX_SAM_SIZE  200 /* 200 * 20 (ms) */

static uint32_t sGPIO;
static bool bIsInit = false;
static uint32_t sNumSamplingSize = 0;
static int aSamples[PH_MAX_SAM_SIZE];

int sensor_ph_meter_init(uint32_t *aGPIOs, uint32_t ui32NumGPIOs)
{
	if (!aGPIOs)
		return -EINVAL;

	if (!ui32NumGPIOs)
		return -EINVAL;

	if (ui32NumGPIOs > 1)
		ESP_LOGW(TAG, "More than one GPIO is set for PH\n");

	sGPIO = *aGPIOs;

	ESP_LOGI(TAG, "GPIO %u for PH\n", sGPIO);

	bIsInit = true;

	return 0;
}

void sensor_ph_meter_deinit(void)
{
}

static float getAverge(int *paData, uint32_t ui32Size)
{
	float average = 0;

	for (int i = 0; i < ui32Size; i++)
		average += paData[i];

	return (average / ui32Size);
}

int sensor_ph_meter_read(uint32_t *paData)
{
	float fPHValue, fVoltage;
	uint32_t ui32Channel;
	int err = -EINVAL;

	if (!bIsInit)
		goto err_out;

	if (!paData) {
		err = -EINVAL;
		goto err_out;
	}

	switch (sGPIO) {
		case ADC1_CHANNEL_0_GPIO_NUM:
			ui32Channel = ADC1_CHANNEL_0;
			break;
		case ADC1_CHANNEL_1_GPIO_NUM:
			ui32Channel = ADC1_CHANNEL_1;
			break;
		case ADC1_CHANNEL_2_GPIO_NUM:
			ui32Channel = ADC1_CHANNEL_2;
			break;
		case ADC1_CHANNEL_3_GPIO_NUM:
			ui32Channel = ADC1_CHANNEL_3;
			break;
		case ADC1_CHANNEL_4_GPIO_NUM:
			ui32Channel = ADC1_CHANNEL_4;
			break;
		case ADC1_CHANNEL_5_GPIO_NUM:
			ui32Channel = ADC1_CHANNEL_5;
			break;
		case ADC1_CHANNEL_6_GPIO_NUM:
			ui32Channel = ADC1_CHANNEL_6;
			break;
		case ADC1_CHANNEL_7_GPIO_NUM:
			ui32Channel = ADC1_CHANNEL_7;
			break;
		default:
			ui32Channel = ADC1_CHANNEL_0;
	}
	adc1_config_width(ADC_WIDTH_BIT_10);
	adc1_config_channel_atten(ui32Channel, ADC_ATTEN_DB_11);
	aSamples[sNumSamplingSize++] = adc1_get_voltage(ui32Channel);
	/* If we're sampling for 4 ms, going to send the data out */
	if (sNumSamplingSize == PH_MAX_SAM_SIZE)
	{
		fVoltage = getAverge(aSamples, sNumSamplingSize) * 5.0 / 1024;
		fPHValue = 3.5 * fVoltage + PH_METER_OFFSET;
		ESP_LOGI(TAG, "PH %f\n", fPHValue);
		sNumSamplingSize = 0;
		/* packed data and normalize */
		paData[0] = (uint32_t)(fPHValue * 100);
		return 0;
	}

	err = -EAGAIN;

err_out:
	return err;
}
