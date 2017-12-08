#ifndef SENSORS_PUBLIC_H
#define SENSORS_PUBLIC_H

/* Package format
 * ====|====|==================|
 *  ID |Size|       data       |
 * ====|====|==================|
 * 0  2     3                  9
 */
#define SENSOR_PACKAGE_SIZE           (10)
#define SENSOR_PACKAGE_ID_SIZE        (2)
#define SENSOR_PACKAGE_DATA_SIZE      (1)
#define SENSOR_PACKAGE_MAX_DATA_SIZE  (SENSOR_PACKAGE_SIZE - \
		                               SENSOR_PACKAGE_ID_SIZE - \
                                       SENSOR_PACKAGE_DATA_SIZE)

typedef enum {
	SENSOR_DHT22,
	SENSOR_PH_METER,
	SENSOR_MAX
} SENSOR_TYPE;

typedef enum {
	SENSOR_DHT22_TEMP,
	SENSOR_DHT22_HUM,
	SENSOR_PH,
} SENSOR_QUERY_TYPE;

struct Sensor {
	SENSOR_TYPE eType;
	int (*pfnInit)(uint32_t *, uint32_t);
	void (*pfnDeInit)(void);
	int (*pfnRead)(uint8_t *paData);
	float (*pfnQuery)(SENSOR_QUERY_TYPE eType, uint8_t *pvData);
	uint32_t ui32DataSize;
	uint32_t ui32SamplingInterval;
	void *pvPriv;
};

struct Sensor *getSensor(SENSOR_TYPE);
void getSensorPackage(uint8_t *, uint8_t *, uint32_t, uint32_t);

#endif
