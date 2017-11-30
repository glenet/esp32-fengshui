#include <string.h>
#include "sensors.h"

static struct Sensor Sensors[3] =
{
	{
		SENSOR_DHT22,
		sensor_dht22_init,
		sensor_dht22_deinit,
		sensor_dht22_read,
		sensor_dht22_query,
	},
};

struct Sensor *getSensor(SENSOR_TYPE eType)
{
	if (eType >= SENSOR_MAX)
		return NULL;

	return &Sensors[eType];
}
