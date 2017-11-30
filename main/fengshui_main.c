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

#define TAG "MAIN"

void DHTHandler(void *pvParameter)
{
	struct Sensor *psSensor = (struct Sensor *)pvParameter;
	uint8_t data[5];
	int err = 0;

	if (!psSensor) {
		printf("Failed to find sensor DHT22\n");
	}

	printf("Hander of Sensor #%d is running\n", psSensor->eType);
	while (1) {
		err = psSensor->pfnRead((uint8_t *)&data, sizeof(data));
		if (!err) {
			printf("Hum %1.f, Tmp %1.f\n",
			         psSensor->pfnQuery(SENSOR_DHT22_HUM, (uint8_t *)&data),
					 psSensor->pfnQuery(SENSOR_DHT22_TEMP, (uint8_t *)&data));
		}
		vTaskDelay(3000 / portTICK_RATE_MS);
	}
}

void app_main()
{
	uint32_t ui32DHTGPIO = CONFIG_DHT22_GPIO;
	struct Sensor *psDHT22;

	nvs_flash_init();
	/* Delay 2 secs */
	vTaskDelay(1000 / portTICK_RATE_MS);
	psDHT22 = getSensor(SENSOR_DHT22);
	if (!psDHT22) {
		printf("Failed to find sensor DHT22\n");
	}
	else {
		psDHT22->pfnInit(&ui32DHTGPIO, 1);
		xTaskCreate(&DHTHandler, "DHTHandler", 2048, psDHT22, 5, NULL);
	}
}

