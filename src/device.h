#ifndef nma_device_h
#define nma_device_h

#include <node_api.h>
#include <uv.h>

#include "miniaudio.h"
#include "context.h"

typedef struct {
    ma_device* maDevice;
    napi_threadsafe_function threadsafeCallback;
    napi_ref contextRef;
    uv_mutex_t mutex;
    uv_cond_t cond;
} Device;

typedef struct {
    Device* device;
    float* input;
    float* output;
    ma_uint32 frameCount;
} ThreadItem;

void refresh_devices(napi_env env, Context* context);
napi_value create_Device(napi_env env, ma_device_config* config, Context* context, napi_value jsContext, napi_value jsCallback);
napi_value start_Device(napi_env env, napi_callback_info info);

#endif
