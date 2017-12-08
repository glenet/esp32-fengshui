#include <errno.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_system.h"
#include "esp_adc_cal.h"
#include "esp_system.h"
#include "esp_log.h"
#include "sensors.h"

#ifdef CONFIG_PH_METER_OFFSET
#define PH_METER_OFFSET CONFIG_PH_METER_OFFSET
#else
#define PH_METER_OFFSET (0)
#endif

#define TAG "PH"
#define PH_MAX_SAM_SIZE  200 /* 200 * 20 (ms) */
#define V_REF   1100

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

	ESP_LOGD(TAG, "GPIO %u for PH\n", sGPIO);

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

int sensor_ph_meter_read(uint8_t *paData)
{
    esp_adc_cal_characteristics_t characteristics;
	float fPHValue, fVoltage;
	uint32_t ui32Channel;
	int err = -EINVAL;
	uint16_t u16Out;
	uint8_t *pvData;

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
	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_config_channel_atten(ui32Channel, ADC_ATTEN_DB_0);
	esp_adc_cal_get_characteristics(V_REF, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_12,
									&characteristics);
	aSamples[sNumSamplingSize++] = adc1_to_voltage(ui32Channel, &characteristics);
	/* If we're sampling for 4 ms, going to send the data out */
	if (sNumSamplingSize == PH_MAX_SAM_SIZE)
	{
		fVoltage = getAverge(aSamples, sNumSamplingSize) * 5.0 / 1024;
		fPHValue = 3.5 * fVoltage + PH_METER_OFFSET;
		ESP_LOGE(TAG, "PH %f\n", fPHValue);
		/* Normalize the data */
		u16Out = (uint16_t)(fPHValue * 100);
		pvData = (uint8_t *)&u16Out;
		paData[0] = *(pvData)++;
		paData[1] = *pvData;
		sNumSamplingSize = 0;
		return 0;
	}

	err = -EAGAIN;

err_out:
	return err;
}

float sensor_ph_meter_query(SENSOR_QUERY_TYPE eType, uint8_t *pvData)
{
	float fPHValue = 0, fVoltage;

	if (eType == SENSOR_PH)
	{
		fVoltage = getAverge(aSamples, sNumSamplingSize) * 5.0 / 1024;
		fPHValue = 3.5 * fVoltage + PH_METER_OFFSET;
	}

	return fPHValue;
}
