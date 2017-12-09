#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>
#include "sensors_public.h"

/* functions for DHT22 */
int sensor_dht22_init(uint32_t *, uint32_t);
void sensor_dht22_deinit(void);
int sensor_dht22_read(uint32_t *);

/* functions for PH meter */
int sensor_ph_meter_init(uint32_t *, uint32_t);
void sensor_ph_meter_deinit(void);
int sensor_ph_meter_read(uint32_t *);

/* functions for EC meter */
int sensor_ec_meter_init(uint32_t *, uint32_t);
void sensor_ec_meter_deinit(void);
int sensor_ec_meter_read(uint32_t *);

/* functions for tmet6000  */
int sensor_tmet6000_init(uint32_t *, uint32_t);
void sensor_tmet6000_deinit(void);
int sensor_tmet6000_read(uint32_t *);

#endif
