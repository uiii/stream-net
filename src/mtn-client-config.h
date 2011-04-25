#ifndef MTN_CLIENT_CONFIG_H
#define MTN_CLIENT_CONFIG_H

#include <stdbool.h>

#include "mtn-config.h"

void init_client_config();
void load_client_config(const char* config_file);

//void set_property(const char* property, const char* value);

#endif
