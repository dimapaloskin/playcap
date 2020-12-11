#ifndef nma_utils_h
#define nma_utils_h

#include <node_api.h>

#include "miniaudio.h"

void create_device_info_object(napi_env env, napi_value obj, ma_device_info* deviceInfo);

#endif
