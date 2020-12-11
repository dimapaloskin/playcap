#ifndef nma_context_h
#define nma_context_h

#include <node_api.h>
#include <miniaudio.h>

typedef struct {
    ma_context maContext;
    ma_device_info* playbackDevicesInfos;
    ma_uint32 playbackCount;
    ma_device_info* captureDevicesInfos;
    ma_uint32 captureCount;
    ma_uint32 defaultPlaybackDeviceIndex;
    ma_uint32 defaultCaptureDeviceIndex;
} Context;

napi_value Context_constructor(napi_env env, napi_callback_info info);
napi_value Context_getDevices(napi_env env, napi_callback_info info);
napi_value Context_refreshDevices(napi_env env, napi_callback_info info);
napi_value Context_createDevice(napi_env env, napi_callback_info info);

#endif
