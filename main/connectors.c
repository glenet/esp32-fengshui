#include <string.h>
#include "connectors.h"

static struct Connector Connectors[] =
{
	{
		CONNECTOR_WIFI,
		connector_wifi_init,
		connector_wifi_deinit,
		connector_wifi_connect,
		connector_wifi_disconnect,
		connector_wifi_send,
	},
};

struct Connector *getConnector(CONNECTOR_TYPE eType)
{
	if (eType >= CONNECTOR_MAX)
		return NULL;

	return &Connectors[eType];
}
