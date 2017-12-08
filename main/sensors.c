#include <string.h>
#include "sensors.h"

static struct Sensor Sensors[] =
{
	{
		SENSOR_DHT22,
		sensor_dht22_init,
		sensor_dht22_deinit,
		sensor_dht22_read,
		sensor_dht22_query,
		5,
		5000,
		NULL
	},
	{
		SENSOR_PH_METER,
		sensor_ph_meter_init,
		sensor_ph_meter_deinit,
		sensor_ph_meter_read,
		sensor_ph_meter_query,
		2,
		20,
		NULL
	},
};

struct Sensor *getSensor(SENSOR_TYPE eType)
{
	if (eType >= SENSOR_MAX)
		return NULL;

	return &Sensors[eType];
}

void getSensorPackage(uint8_t *pvOutData, uint8_t *pvInData,
                      uint32_t ui32InSize, uint32_t ui32Type)
{
	uint16_t *pvU16OutData = (uint16_t *)pvOutData;
	uint8_t *pvOut;

	/* pvOutData[0] and pvOutData[1] will be presented ID */
	*pvU16OutData = (uint16_t)(ui32Type & 0xFFFF);
	pvU16OutData++;
	pvOut = (uint8_t *)pvU16OutData;

	/* pvOutData[2] will be presented size of real data */
	*pvOut = (uint8_t)(ui32InSize & 0x00FF);
	pvOut++;

	/* Copy the read data to the package */
	for (int i = 0; i < ui32InSize; i++)
		pvOut[i] = pvInData[i];
}
