/* Copyright (C) Michael Yang
 * Written by Michael Yang <czhe.yang@gmail.com>, December 2017
 */

#ifndef SENSORS_PUBLIC_H
#define SENSORS_PUBLIC_H

#define SENSOR_PACKAGE_SIZE           (8)
#define SENSOR_PACKAGE_SRTART_CHT     (1)
#define SENSOR_PACKAGE_ID_SIZE        (2)
#define SENSOR_PACKAGE_DATA_SIZE      (4)
#define SENSOR_PACKAGE_RESERVERD      (1)

typedef enum {
	SENSOR_DHT22,
	SENSOR_PH_METER,
	SENSOR_EC_METER,
	SENSOR_TEMT6000,
	SENSOR_MAX
} SENSOR_TYPE;

typedef enum {
	SENSOR_PH,
	SENSOR_EC,
	SENSOR_DS18B20_TEMP,
	SENSOR_TEMT6000_LIGHT,
	SENSOR_DHT22_TEMP = 10,
	SENSOR_DHT22_HUM,
} SENSOR_QUERY_TYPE;

struct Sensor {
	SENSOR_TYPE eType;
	int (*pfnInit)(uint32_t *, uint32_t);
	void (*pfnDeInit)(void);
	int (*pfnRead)(uint32_t *paData);
	uint32_t ui32SamplingInterval;
	uint32_t ui32NumType;
	SENSOR_QUERY_TYPE eQueryTypes[3];
	void *pvPriv;
};

struct Sensor *getSensor(SENSOR_TYPE);
void getSensorPackage(uint8_t *, uint32_t, uint32_t);

#endif
