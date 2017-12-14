/* Copyright (C) Michael Yang
 * Written by Michael Yang <czhe.yang@gmail.com>, December 2017
 */

#ifndef CONNECTORS_H
#define CONNECTORS_H

#include <stdint.h>
#include "connectors_public.h"

int connector_wifi_init(void);
void connector_wifi_deinit(void);
int connector_wifi_connect(void);
int connector_wifi_disconnect(void);
int connector_wifi_send(uint8_t *, uint32_t);

#endif
