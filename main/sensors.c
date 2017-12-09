#include <string.h>
#include "sensors.h"
#include <stdio.h>

static struct Sensor Sensors[] =
{
	{
		SENSOR_DHT22,
		sensor_dht22_init,
		sensor_dht22_deinit,
		sensor_dht22_read,
		5000,
		2,
		{
			SENSOR_DHT22_TEMP,
			SENSOR_DHT22_HUM,
		},
		NULL
	},
	{
		SENSOR_PH_METER,
		sensor_ph_meter_init,
		sensor_ph_meter_deinit,
		sensor_ph_meter_read,
		20,
		1,
		{
			SENSOR_PH,
		},
		NULL
	},
	{
		SENSOR_EC_METER,
		sensor_ec_meter_init,
		sensor_ec_meter_deinit,
		sensor_ec_meter_read,
		20,
		2,
		{
			SENSOR_EC,
			SENSOR_DS18B20_TEMP,
		},
		NULL
	},
};

struct Sensor *getSensor(SENSOR_TYPE eType)
{
	if (eType >= SENSOR_MAX)
		return NULL;

	return &Sensors[eType];
}

void getSensorPackage(uint8_t *pvOutData, uint32_t ui32Type,
                      uint32_t ui32InData)
{
	uint16_t *pvU16OutData;
	uint8_t *pvU8OutData;

	/* Pack start character */
	*pvOutData = '#';
	pvOutData++;
	/* Packet label data */
	pvU16OutData = (uint16_t *)pvOutData;
	*pvU16OutData = (uint16_t)(ui32Type & 0xFFFF);
	pvU16OutData++;
	/* Packet data */
	pvU8OutData = (uint8_t *)pvU16OutData;
	*pvU8OutData = ((ui32InData >> 24) & 0xFF);
	pvU8OutData++;
	*pvU8OutData = ((ui32InData >> 16) & 0xFF);
	pvU8OutData++;
	*pvU8OutData = ((ui32InData >> 8) & 0xFF);
	pvU8OutData++;
	*pvU8OutData = ui32InData& 0xFF;
}
