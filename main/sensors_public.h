#ifndef SENSORS_PUBLIC_H
#define SENSORS_PUBLIC_H

typedef enum {
	SENSOR_DHT22,
	SENSOR_MAX
} SENSOR_TYPE;

typedef enum {
	SENSOR_DHT22_TEMP,
	SENSOR_DHT22_HUM,
} SENSOR_QUERY_TYPE;

struct Sensor {
	SENSOR_TYPE eType;
	int (*pfnInit)(uint32_t *, uint32_t);
	void (*pfnDeInit)(void);
	int (*pfnRead)(uint8_t *paData, uint32_t ui32Size);
	float (*pfnQuery)(SENSOR_QUERY_TYPE eType, uint8_t *pvData);
};

struct Sensor *getSensor(SENSOR_TYPE);

#endif
