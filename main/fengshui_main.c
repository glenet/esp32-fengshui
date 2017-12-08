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
#define PH_METER_GPIO 34
#endif

void SensorHandler(void *pvParameter)
{
	struct Sensor *psSensor = (struct Sensor *)pvParameter;
	uint8_t data[psSensor->ui32DataSize];
	uint8_t package[SENSOR_PACKAGE_SIZE];
	struct Connector *psConnector;
	int err = 0;

	if (!psSensor) {
		ESP_LOGE(TAG, "Failed to find sensor DHT22\n");
	}

	psConnector = (struct Connector *)psSensor->pvPriv;
	ESP_LOGI(TAG, "Hander of Sensor #%d is running\n", psSensor->eType);

	while (1) {
		err = psSensor->pfnRead((uint8_t *)&data);
		if (err == ESP_OK) {
			if (psConnector) {
				getSensorPackage((uint8_t *)&package, (uint8_t *)&data,
				                 psSensor->ui32DataSize, psSensor->eType);
				/* Sending package */
				err = psConnector->pfnSend((uint8_t *)&package,
				                           SENSOR_PACKAGE_SIZE);
				if (err) {
					ESP_LOGE(TAG, "Failed to send ID via connector\n");
				}
			}
		}
		vTaskDelay(psSensor->ui32SamplingInterval / portTICK_RATE_MS);
	}
}

void app_main()
{
	uint32_t ui32DHTGPIO = DHT22_GPIO, ui32PHGPIO = PH_METER_GPIO;
	struct Connector *psWifiConnector;
	struct Sensor *psDHT22, *psPH;

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

	psDHT22 = getSensor(SENSOR_DHT22);
	if (!psDHT22) {
		ESP_LOGE(TAG, "Failed to find sensor DHT22\n");
		return;
	}
	psDHT22->pfnInit(&ui32DHTGPIO, 1);
	psDHT22->pvPriv = (void *)psWifiConnector;
	xTaskCreate(&SensorHandler, "DHTHandler", 2048, psDHT22, 5, NULL);

	psPH = getSensor(SENSOR_PH_METER);
	if (!psPH) {
		ESP_LOGE(TAG, "Failed to find sensor PH meter\n");
		return;
	}
	psPH->pfnInit(&ui32PHGPIO, 1);
	psPH->pvPriv = (void *)psWifiConnector;
	xTaskCreate(&SensorHandler, "PHHandler", 2048, psPH, 5, NULL);
}

