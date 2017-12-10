#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "rom/ets_sys.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "esp_log.h"
#include "sensors_public.h"
#include "connectors_public.h"

#define TAG "MAIN"
#ifdef CONFIG_DHT22_GPIO
#define DHT22_GPIO CONFIG_DHT22_GPIO
#else
#define DHT22_GPIO 4
#endif
#ifdef CONFIG_PH_METER_GPIO
#define PH_METER_GPIO CONFIG_PH_METER_GPIO
#else
#define PH_METER_GPIO 32
#endif
#ifdef CONFIG_EC_METER_GPIO
#define EC_METER_GPIO CONFIG_EC_METER_GPIO
#else
#define EC_METER_GPIO 33
#endif
#ifdef CONFIG_TMET6000_GPIO
#define TMET6000_GPIO CONFIG_TMET6000_GPIO
#else
#define TMET6000_GPIO 34
#endif
#ifdef CONFIG_DS18B20_GPIO
#define DS18B20_GPIO CONFIG_DS18B20_GPIO
#else
#define DS18B20_GPIO 2
#endif
#ifdef CONFIG_DS18B20_GPIO
#define DS18B20_GPIO CONFIG_DS18B20_GPIO
#else
#define DS18B20_GPIO 2
#endif

void SensorHandler(void *pvParameter)
{
	struct Sensor *psSensor = (struct Sensor *)pvParameter;
	uint32_t data[psSensor->ui32NumType];
	uint8_t package[SENSOR_PACKAGE_SIZE];
	struct Connector *psConnector;
	int err = 0;

	if (!psSensor) {
		ESP_LOGE(TAG, "Failed to find sensor %d\n", psSensor->eType);
	}

	psConnector = (struct Connector *)psSensor->pvPriv;
	ESP_LOGI(TAG, "Hander of Sensor #%d is running\n", psSensor->eType);

	while (1) {
		err = psSensor->pfnRead((uint32_t *)&data);
		if (err == ESP_OK) {
			for (int j = 0; j < SENSOR_PACKAGE_SIZE; j++)
					package[j] = 0;
			if (psConnector) {
				for (int i = 0; i < psSensor->ui32NumType; i++) {
					getSensorPackage((uint8_t *)&package,
					             psSensor->eQueryTypes[i], data[i]);
					/* Sending package */
					for (int j = 0; j < SENSOR_PACKAGE_SIZE; j++)
						ESP_LOGE(TAG, "package[j]=%x\n", package[j]);
					err = psConnector->pfnSend((uint8_t *)&package,
					                           SENSOR_PACKAGE_SIZE);
					if (err) {
						ESP_LOGE(TAG, "Failed to send ID via connector\n");
					}
					vTaskDelay(2000 / portTICK_RATE_MS);
				}
			}
		}
		vTaskDelay(psSensor->ui32SamplingInterval / portTICK_RATE_MS);
	}
}

void app_main()
{
	uint32_t ui32ECGPIOs[] = { EC_METER_GPIO, DS18B20_GPIO };
	uint32_t ui32GPIOs[] = { DHT22_GPIO, PH_METER_GPIO };
	uint32_t ui32TMET6000 = TMET6000_GPIO;
	struct Connector *psWifiConnector;
	struct Sensor *psSensor;

	nvs_flash_init();
	/* Delay 2 secs */
	vTaskDelay(2000 / portTICK_RATE_MS);

	psWifiConnector = getConnector(CONNECTOR_WIFI);
	if (!psWifiConnector) {
		ESP_LOGE(TAG, "Failed to get wifi connector\n");
		return;
	}
	if (psWifiConnector->pfnInit() != ESP_OK) {
		ESP_LOGE(TAG, "Failed to init wifi connector\n");
		return;
	}
	if (psWifiConnector->pfnConnect() != ESP_OK) {
		ESP_LOGE(TAG, "Failed to connect wifi connector\n");
		return;
	}

	for (int i = 0; i < SENSOR_EC_METER; i++) {
		psSensor = getSensor(i);
		if (!psSensor) {
			ESP_LOGE(TAG, "Failed to find sensor %d\n", psSensor->eType);
			return;
		}
		/* Make sure the GPIO sequences are mapping to the sensors */
		psSensor->pfnInit(&ui32GPIOs[i], 1);
		psSensor->pvPriv = (void *)psWifiConnector;
		xTaskCreate(&SensorHandler, "SensorHandler", 2048, psSensor, 5, NULL);
	}
	psSensor = getSensor(SENSOR_EC_METER);
	if (!psSensor) {
		ESP_LOGE(TAG, "Failed to find sensor %d\n", psSensor->eType);
		return;
	}
	psSensor->pfnInit((uint32_t *)&ui32ECGPIOs, 2);
	psSensor->pvPriv = (void *)psWifiConnector;
	xTaskCreate(&SensorHandler, "SensorHandler", 2048, psSensor, 5, NULL);
	psSensor = getSensor(SENSOR_TEMT6000);
	if (!psSensor) {
		ESP_LOGE(TAG, "Failed to find sensor %d\n", psSensor->eType);
		return;
	}
	psSensor->pfnInit(&ui32TMET6000, 1);
	psSensor->pvPriv = (void *)psWifiConnector;
	xTaskCreate(&SensorHandler, "SensorHandler", 2048, psSensor, 5, NULL);
}

