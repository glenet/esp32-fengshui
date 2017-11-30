#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>
#include "sensors_public.h"

/* functions for DHT22 */
int sensor_dht22_init(uint32_t *, uint32_t);
void sensor_dht22_deinit(void);
int sensor_dht22_read(uint8_t *, uint32_t);
float sensor_dht22_query(SENSOR_QUERY_TYPE eType, uint8_t *);

#endif
