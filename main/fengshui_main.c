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

void DHTHandler(void *pvParameter)
{
	struct Sensor *psSensor = (struct Sensor *)pvParameter;
	struct Connector *psConnector;
	uint8_t data[5];
	int err = 0;

	if (!psSensor) {
		ESP_LOGE(TAG, "Failed to find sensor DHT22\n");
	}

	psConnector = (struct Connector *)psSensor->pvPriv;
	ESP_LOGI(TAG, "Hander of Sensor #%d is running\n", psSensor->eType);

	while (1) {
		err = psSensor->pfnRead((uint8_t *)&data, sizeof(data));
		if (err == ESP_OK) {
			printf("Hum %1.f, Tmp %1.f\n",
			         psSensor->pfnQuery(SENSOR_DHT22_HUM, (uint8_t *)&data),
					 psSensor->pfnQuery(SENSOR_DHT22_TEMP, (uint8_t *)&data));
			if (psConnector) {
				/* Sending ID */
				err = psConnector->pfnSend(&psSensor->eType, sizeof(SENSOR_TYPE));
				if (err) {
					ESP_LOGE(TAG, "Failed to send ID via connector\n");
				}
				/* Sending data */
				err = psConnector->pfnSend(data, sizeof(data));
				if (err) {
					ESP_LOGE(TAG, "Failed to send data via connector\n");
				}
			}
		}
		vTaskDelay(5000 / portTICK_RATE_MS);
	}
}

void app_main()
{
	uint32_t ui32DHTGPIO = DHT22_GPIO;
	struct Connector *psWifiConnector;
	struct Sensor *psDHT22;

	nvs_flash_init();
	/* Delay 2 secs */
	vTaskDelay(1000 / portTICK_RATE_MS);
	psDHT22 = getSensor(SENSOR_DHT22);
	if (!psDHT22) {
		ESP_LOGE(TAG, "Failed to find sensor DHT22\n");
		return;
	}

	psDHT22->pfnInit(&ui32DHTGPIO, 1);

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
	psDHT22->pvPriv = (void *)psWifiConnector;
	xTaskCreate(&DHTHandler, "DHTHandler", 2048, psDHT22, 5, NULL);
}

