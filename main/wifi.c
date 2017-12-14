/* Copyright (C) Michael Yang
 * Written by Michael Yang <czhe.yang@gmail.com>, December 2017
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "connectors.h"

#define TAG "WIFI"

#ifdef CONFIG_WIFI_SSID
#define SSID CONFIG_WIFI_SSID
#else
#define SSID "12345678"
#endif
#ifdef CONFIG_WIFI_PASSWORD
#define PASSWORD CONFIG_WIFI_PASSWORD
#else
#define PASSWORD "12345678"
#endif
#ifdef CONFFIG_SOCKET_PORT
#define PORT CONFFIG_SOCKET_PORT
#else
#define PORT (3010)
#endif
#ifdef CONFIG_SOCKET_TCP_IP
#define SOCKET_TCP_IP CONFIG_SOCKET_TCP_IP
#else
#define SOCKET_TCP_IP "127.0.0.1"
#endif
#define WIFI_CONNECTED_BIT BIT0

static EventGroupHandle_t sEventHandle;
static QueueHandle_t sQueue;
static int si32SocketFd;
static bool bIsConnect;

struct AMessage
{
	uint8_t  *pvData;
	uint32_t ui32Size;
} sMessage;

static void wifi_connect()
{
	wifi_config_t cfg = {
		.sta = {
			.ssid = SSID,
			.password = PASSWORD,
		},
	};

	/* TODO: what can we do if failing to connect esp wifi */
	if (esp_wifi_disconnect() != ESP_OK)
		ESP_LOGE(TAG, "Failed to disconnect esp wifi\n");
	if (esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg) != ESP_OK)
		ESP_LOGE(TAG, "Failed to set config for esp wifi\n");
    if (esp_wifi_connect() != ESP_OK)
		ESP_LOGE(TAG, "Failed to connect esp wifi\n");
}

static esp_err_t event_handler(void *pvCtx, system_event_t *event)
{
	switch(event->event_id) {
		case SYSTEM_EVENT_STA_START:
			wifi_connect();
			break;
		case SYSTEM_EVENT_STA_GOT_IP:
			xEventGroupSetBits(sEventHandle, WIFI_CONNECTED_BIT);
			break;
		case SYSTEM_EVENT_STA_DISCONNECTED:
			xEventGroupClearBits(sEventHandle, WIFI_CONNECTED_BIT);
			break;
		default:
			break;
    }
    return ESP_OK;
}

void WIFIHandler(void *pvParam)
{
    struct sockaddr_in tcpServerAddr;
	struct AMessage *psMsg;
	int i32SocketFd;

    tcpServerAddr.sin_addr.s_addr = inet_addr(SOCKET_TCP_IP);
    tcpServerAddr.sin_family = AF_INET;
    tcpServerAddr.sin_port = htons(PORT);

	ESP_LOGI(TAG,"WIFIHandler is running \n");

	xEventGroupWaitBits(sEventHandle, WIFI_CONNECTED_BIT,
	                    false, true, portMAX_DELAY);
	i32SocketFd = socket(AF_INET, SOCK_STREAM, 0);
	while (i32SocketFd < 0) {
		ESP_LOGE(TAG, "Failed to allocate socket, will re-try\n");
		vTaskDelay(4000 / portTICK_PERIOD_MS);
		i32SocketFd = socket(AF_INET, SOCK_STREAM, 0);
	}
	while (connect(i32SocketFd, (struct sockaddr *)&tcpServerAddr,
		sizeof(tcpServerAddr)) != 0) {
		ESP_LOGE(TAG, "Failed to connect errno=%d \n", errno);
		close(i32SocketFd);
		vTaskDelay(4000 / portTICK_PERIOD_MS);
		i32SocketFd = socket(AF_INET, SOCK_STREAM, 0);
	}
	si32SocketFd = i32SocketFd;
	ESP_LOGI(TAG, "Socket is connecting \n");

	bIsConnect = true;
    while (bIsConnect) {
		if (xQueueReceive(sQueue, &psMsg, (TickType_t)10)) {
			if (write(i32SocketFd , psMsg->pvData, psMsg->ui32Size) < 0) {
				ESP_LOGE(TAG, "Failed to send data\n");
				vTaskDelay(4000 / portTICK_PERIOD_MS);
				continue;
			}
		}
		vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

int connector_wifi_init(void)
{
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	int err = ESP_OK;

    sEventHandle = xEventGroupCreate();
	sQueue = xQueueCreate(10, sizeof(struct AMessage *));
	if (!sQueue) {
		ESP_LOGE(TAG, "Failed to allocate queue\n");
		goto err_out;
	}

    err = esp_event_loop_init(event_handler, NULL);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Failed to init event loop\n");
		goto err_out;
	}
    tcpip_adapter_init();

	err = esp_wifi_init(&cfg);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Failed to init wifi\n");
		goto err_out;
	}
	err = esp_wifi_set_mode(WIFI_MODE_STA);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Failed to set sta mode\n");
		goto err_out;
	}
	err = esp_wifi_start();
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Failed to start wifi\n");
		goto err_out;
	}

 err_out:
	return err;
}

void connector_wifi_deinit(void)
{
}

int connector_wifi_connect(void)
{
	xTaskCreate(&WIFIHandler,"wifi handler", 4048, NULL, 5, NULL);

	return ESP_OK;
}

int connector_wifi_disconnect(void)
{
	bIsConnect = false;
	close(si32SocketFd);
	return ESP_OK;
}

int connector_wifi_send(uint8_t *pvData, uint32_t ui32Size)
{
	struct AMessage *msg = &sMessage;

	if (bIsConnect) {
		msg->pvData = pvData;
		msg->ui32Size = ui32Size;

		xQueueSend(sQueue, (void *)&msg, (TickType_t)0);
	}
	return ESP_OK;
}
