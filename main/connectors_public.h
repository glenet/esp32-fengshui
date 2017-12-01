#ifndef CONNECTORS_PUBLIC_H
#define CONNECTORS_PUBLIC_H

#include <stdint.h>

typedef enum {
	CONNECTOR_WIFI,
	CONNECTOR_MAX
} CONNECTOR_TYPE;

struct Connector {
	CONNECTOR_TYPE eType;
	int (*pfnInit)(void);
	void (*pfnDeInit)(void);
	int (*pfnConnect)(void);
	int (*pfnDisconnect)(void);
	int (*pfnSend)(uint8_t *, uint32_t);
};

struct Connector *getConnector(CONNECTOR_TYPE);

#endif
