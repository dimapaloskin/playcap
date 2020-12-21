#include <stdlib.h>
#include <node_api.h>

#include "miniaudio.h"

#include "macros.h"
#include "napi_utils.h"
#include "context.h"
#include "device.h"
#include "utils.h"


Context* create_Context()
{
    Context* context  = (Context*)malloc(sizeof(Context));
    context->defaultPlaybackDeviceIndex = 0;
    context->defaultCaptureDeviceIndex = 0;

    return context;
}

void finalize_Context(napi_env env, void* data, void* hint)
{
    UNUSED(hint);
    Context* context = (Context*)data;
    MA_CALL(env, "Unable to unint ma_context",
        ma_context_uninit(&context->maContext));

    free(context);
}

/**
 * TODOS:
 *  - setup backend
 */
napi_value Context_constructor(napi_env env, napi_callback_info info)
{
    napi_value jsthis;

    size_t argc = 1;
    napi_value argv[1];

    NAPI_CALL(env,
        napi_get_cb_info(env, info, &argc, argv, &jsthis, NULL));

    Context* context  = create_Context();

    if (argc == 0) {
        MA_CALL(env, "Unable to initialize context",
            ma_context_init(NULL, 0, NULL, &context->maContext));
    } else {
        ma_backend backends[1] = {};
        NAPI_CALL(env,
            napi_get_value_uint32(env, argv[0], (uint32_t)&backends[0]));

        MA_CALL(env, "Unable to initialize context",
            ma_context_init(&backends[0], 1, NULL, &context->maContext));
    }

    refresh_devices(env, context);

    napi_wrap(env, jsthis, context, &finalize_Context, NULL, NULL);
    return jsthis;
}

napi_value Context_getDevices(napi_env env, napi_callback_info info)
{
    napi_value jsthis;

    NAPI_CALL(env,
        napi_get_cb_info(env, info, NULL, NULL, &jsthis, NULL));

    Context* context;
    NAPI_CALL(env,
        napi_unwrap(env, jsthis, (void **) &context));

    napi_value playbackDevices;
    napi_value captureDevices;

    NAPI_CALL(env,
        napi_create_array(env, &playbackDevices));

    NAPI_CALL(env,
        napi_create_array(env, &captureDevices));

    for (ma_uint32 i = 0; i < context->playbackCount; i++) {
        napi_handle_scope scope;
        NAPI_CALL(env,
            napi_open_handle_scope(env, &scope));

        napi_value deviceObj;
        NAPI_CALL(env,
            napi_create_object(env, &deviceObj));

        create_device_info_object(env, deviceObj, &(context->playbackDevicesInfos[i]));

        NAPI_CALL(env,
            napi_set_element(env, playbackDevices, i, deviceObj));

        NAPI_CALL(env,
            napi_close_handle_scope(env, scope));
    }

    for (ma_uint32 i = 0; i < context->captureCount; i++) {
        napi_handle_scope scope;
        NAPI_CALL(env,
            napi_open_handle_scope(env, &scope));

        napi_value deviceObj;
        NAPI_CALL(env,
            napi_create_object(env, &deviceObj));

        create_device_info_object(env, deviceObj, &(context->captureDevicesInfos[i]));

        NAPI_CALL(env,
            napi_set_element(env, captureDevices, i, deviceObj));

        NAPI_CALL(env,
            napi_close_handle_scope(env, scope));
    }

    napi_value result;
    NAPI_CALL(env,
        napi_create_object(env, &result));

    NAPI_CALL(env,
        napi_set_named_property(env, result, "playback", playbackDevices));
    NAPI_CALL(env,
        napi_set_named_property(env, result, "capture", captureDevices));

    return result;
}

napi_value Context_refreshDevices(napi_env env, napi_callback_info info)
{
    napi_value jsthis;

    NAPI_CALL(env,
        napi_get_cb_info(env, info, NULL, NULL, &jsthis, NULL));

    Context* context;
    NAPI_CALL(env,
        napi_unwrap(env, jsthis, (void **) &context));

    refresh_devices(env, context);

    return NULL;
}

napi_value Context_createDevice(napi_env env, napi_callback_info info)
{
    napi_value jsthis;

    size_t argc = 2;
    napi_value argv[2];

    NAPI_CALL(env,
        napi_get_cb_info(env, info, &argc, argv, &jsthis, NULL));

    if (argc < 2) {
        napi_throw_error(env, NULL, "Context.createDevice: Not enough arguments");
    }

    Context* context;
    NAPI_CALL(env,
        napi_unwrap(env, jsthis, (void **) &context));

    napi_value options = argv[0];
    napi_value jsDataCallback = argv[1];

    // 1 - playback
    // 2 - caputre
    // 3 - duplex
    // 4 - loopback
    ma_device_type deviceType = ma_device_type_playback;
    try_get_named_property_uint32(env, options, "deviceType", (uint32_t*)&deviceType);

    ma_device_config config = ma_device_config_init(deviceType);
    config.playback.format = ma_format_f32;
    config.capture.format = ma_format_f32;

    uint32_t playbackDeviceIndex = context->defaultPlaybackDeviceIndex;
    uint32_t captureDeviceIndex = context->defaultCaptureDeviceIndex;

    try_get_named_property_uint32(env, options, "playbackDeviceIndex", &playbackDeviceIndex);
    try_get_named_property_uint32(env, options, "captureDeviceIndex", &captureDeviceIndex);
    try_get_named_property_uint32(env, options, "playbackChannels", &config.playback.channels);
    try_get_named_property_uint32(env, options, "captureChannels", &config.capture.channels);
    try_get_named_property_uint32(env, options, "sampleRate", &config.sampleRate);

    config.playback.pDeviceID = &context->playbackDevicesInfos[playbackDeviceIndex].id;
    config.capture.pDeviceID = &context->captureDevicesInfos[captureDeviceIndex].id;

    return create_Device(env, &config, context, jsthis, jsDataCallback);
}
