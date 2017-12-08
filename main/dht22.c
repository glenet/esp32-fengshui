#include <errno.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "sensors.h"

#define TAG "DHT22"
#define MAX_DHT_DATA (5) // data for 5 bytes

static uint32_t sGPIO;
static bool bIsInit = false;

static int getSignalLevel(uint32_t ui32Timeout, bool bState)
{
	uint32_t ui32Sec = 0;

	while (gpio_get_level(sGPIO) == bState) {
		if (ui32Sec > ui32Timeout)
			return -1;
		ui32Sec++;
		ets_delay_us(1);
	}

	return ui32Sec;
}

int sensor_dht22_init(uint32_t *aGPIOs, uint32_t ui32NumGPIOs)
{
	if (!aGPIOs)
		return -EINVAL;

	if (!ui32NumGPIOs)
		return -EINVAL;

	if (ui32NumGPIOs > 1)
		ESP_LOGW(TAG, "More than one GPIO is set for DHT22\n");

	sGPIO = *aGPIOs;
	
	ESP_LOGD(TAG, "GPIO %u for DHT22\n", sGPIO);

	bIsInit = true;

	return 0;
}

void sensor_dht22_deinit(void)
{
	
}

int sensor_dht22_read(uint8_t *paData)
{
	uint8_t byteInx = 0;
	uint8_t bitInx = 7;
	int err = -EAGAIN;

	if (!bIsInit)
		goto err_out;

	if (!paData) {
		err = -EINVAL;
		goto err_out;
	}

	/* Signaling DHT22 */
	gpio_set_direction(sGPIO, GPIO_MODE_OUTPUT);
	/* pull down for 3 ms for a smooth and nice wake up */
	gpio_set_level(sGPIO, 0);
	ets_delay_us(3000);
	/* pull up for 25 us for a gentile asking for data */
	gpio_set_level(sGPIO, 1 );
	ets_delay_us(25);
	/* switch to input mode for starting receiving data */
	gpio_set_direction(sGPIO, GPIO_MODE_INPUT);
	/* DHT will keep the line low for 80 us and then high for 80us */
	err = getSignalLevel(85, 0);
	if (err < 0) {
		ESP_LOGV(TAG, "Timeout for waiting on low level over 80 us\n");
		goto err_out;
	}
	err = getSignalLevel(85, 1);
	if (err < 0) {
		ESP_LOGE(TAG, "Timeout for waiting on high level over 80 us\n");
		goto err_out;
	}

	/* Starting new 40 bytes data transmission */
	for (int i = 0; i < 40; i++) {
		/* starts new data transmission with > 50us low signal */
		err = getSignalLevel(56, 0);
		if (err < 0) {
			ESP_LOGV(TAG, "Timeout for waiting on low level over 56 us\n");
			goto err_out;
		}
		/* check if after >70us rx data is a 0 or a 1 */
		err = getSignalLevel(75, 1);
		if (err < 0) {
			ESP_LOGV(TAG, "Timeout for waiting on high level over 75 us\n");
			goto err_out;
		}
		/* add the current read to the output data
		 * since all dhtData array where set to 0 at the start,
		 * only look for "1" (>28us us)
		 */
	
		if (err > 40)
			paData[byteInx] |= (1 << bitInx);
		/* index to next byte */
		if (bitInx == 0) {
			bitInx = 7;
			++byteInx;
		}
		else
			bitInx--;
	}

	/* Checksum is the sum of Data 8 bits masked out 0xFF */
	if (paData[4] != ((paData[0] + paData[1] + paData[2] +
	                     paData[3]) & 0xFF)) {
		err = -ENOMEM;
		goto err_out;
	}

	ESP_LOGI(TAG, "Hum %1.f, Tmp %1.f\n",
	               sensor_dht22_query(SENSOR_DHT22_TEMP, paData),
	               sensor_dht22_query(SENSOR_DHT22_HUM, paData));
	err = 0;

err_out:
	return err;
}

float sensor_dht22_query(SENSOR_QUERY_TYPE eType, uint8_t *pvData)
{
	float value = 0.0;

	switch (eType) {
		case SENSOR_DHT22_TEMP:
		{
			value = pvData[2] & 0x7F;
			value *= 0x100;
			value += pvData[3];
			value /= 10;
			if (pvData[2] & 0x80 )
				value *= -1;
			break;
		}
		case SENSOR_DHT22_HUM:
		{
			value  = pvData[0];
			value  *= 0x100;
			value += pvData[1];
			value  /= 10;
			break;
		}
		default:
			break;
	}
	return value;
}
