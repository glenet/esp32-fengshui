#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>
#include "sensors_public.h"

/* functions for DHT22 */
int sensor_dht22_init(uint32_t *, uint32_t);
void sensor_dht22_deinit(void);
int sensor_dht22_read(uint8_t *);
float sensor_dht22_query(SENSOR_QUERY_TYPE eType, uint8_t *);

/* functions for PH meter */
int sensor_ph_meter_init(uint32_t *, uint32_t);
void sensor_ph_meter_deinit(void);
int sensor_ph_meter_read(uint8_t *);
float sensor_ph_meter_query(SENSOR_QUERY_TYPE eType, uint8_t *);

#endif
