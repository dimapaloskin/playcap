#include <stdlib.h>
#include <node_api.h>
#include <uv.h>

#include "miniaudio.h"

#include "macros.h"
#include "device.h"
#include "context.h"

void data_callback(ma_device* maDevice,
    void* output,
    const void* input,
    ma_uint32 frameCount)
{
    Device* device = (Device*)maDevice->pUserData;

    ThreadItem* threadItem = (ThreadItem*)malloc(sizeof(ThreadItem));
    threadItem->device = device;
    threadItem->input = (float*)input;
    threadItem->output = (float*)output;
    threadItem->frameCount = frameCount;

    napi_threadsafe_function cb = (napi_threadsafe_function)device->threadsafeCallback;

    napi_call_threadsafe_function(cb, threadItem, napi_tsfn_blocking);

    uv_mutex_lock(&device->mutex);
    uv_cond_wait(&device->cond, &device->mutex);
    uv_mutex_unlock(&device->mutex);
}


void jsDataCallbackCaller(napi_env env, napi_value js_callback, void* context, void* data)
{
    ThreadItem* threadItem = (ThreadItem*)data;
    Device* device = threadItem->device;

    float* outputData[device->maDevice->playback.channels];
    float* inputData[device->maDevice->capture.channels];

    napi_value outputs;
    NAPI_CALL_NO_RETURN(env,
        napi_create_array(env, &outputs));

    for (ma_uint32 chNum = 0; chNum < device->maDevice->playback.channels; chNum++) {
        napi_value ab;
        napi_value ta;

        NAPI_CALL_NO_RETURN(env,
            napi_create_arraybuffer(env, (size_t)(sizeof(float) * threadItem->frameCount), (void**)&outputData[chNum], &ab));

        NAPI_CALL_NO_RETURN(env,
            napi_create_typedarray(env, napi_float32_array, threadItem->frameCount, ab, 0, &ta));

        napi_set_element(env, outputs, chNum, ta);
    }

    napi_value inputs;
    NAPI_CALL_NO_RETURN(env,
        napi_create_array(env, &inputs));

    for (ma_uint32 chNum = 0; chNum < device->maDevice->capture.channels; chNum++) {
        napi_value ab;
        napi_value ta;

        NAPI_CALL_NO_RETURN(env,
            napi_create_arraybuffer(env, (size_t)(sizeof(float) * threadItem->frameCount), (void**)&inputData[chNum], &ab));

        NAPI_CALL_NO_RETURN(env,
            napi_create_typedarray(env, napi_float32_array, threadItem->frameCount, ab, 0, &ta));

        napi_set_element(env, inputs, chNum, ta);
    }

    if (device->maDevice->type == ma_device_type_capture || device->maDevice->type == ma_device_type_duplex) {
        for (ma_uint64 iFrame = 0; iFrame < threadItem->frameCount; iFrame += 1) {
            for (ma_uint32 chNum = 0; chNum < device->maDevice->capture.channels; chNum += 1) {
                inputData[chNum][iFrame] = threadItem->input[iFrame * device->maDevice->capture.channels + chNum];
            }
        }
    }

    size_t argc = 2;
    napi_value argv[argc];
    argv[0] = inputs;
    argv[1] = outputs;

    napi_value global;
    NAPI_CALL_NO_RETURN(env,
        napi_get_global(env, &global));

    NAPI_CALL_NO_RETURN(env,
        napi_call_function(env, global, js_callback, argc, argv, NULL));

    if (device->maDevice->type == ma_device_type_duplex || device->maDevice->type == ma_device_type_playback) {
        for (ma_uint64 iFrame = 0; iFrame < threadItem->frameCount; iFrame += 1) {
            for (ma_uint32 chNum = 0; chNum < device->maDevice->playback.channels; chNum += 1) {
                threadItem->output[iFrame * device->maDevice->playback.channels + chNum] = outputData[chNum][iFrame];
            }
        }
    }

    uv_mutex_lock(&device->mutex);
    uv_cond_signal(&device->cond);
    uv_mutex_unlock(&device->mutex);

    free(threadItem);
}

void finalize_Device(napi_env env, void* data, void* hint)
{
    UNUSED(env);
    UNUSED(data);
    UNUSED(hint);
    // TODO: need destroy somehow?
}

napi_value destroy_Device(napi_env env, napi_callback_info info)
{
    Device* device;
    NAPI_CALL(env,
        napi_get_cb_info(env, info, NULL, NULL, NULL, (void**)&device));

    uv_mutex_destroy(&device->mutex);
    uv_cond_destroy(&device->cond);

    ma_device_stop(device->maDevice);
    ma_device_uninit(device->maDevice);

    NAPI_CALL(env,
        napi_delete_reference(env, device->contextRef));

    NAPI_CALL(env,
        napi_unref_threadsafe_function(env, device->threadsafeCallback));

    napi_unref_threadsafe_function(env, device->threadsafeCallback);

    free(device);

    return NULL;
}

napi_value start_Device(napi_env env, napi_callback_info info)
{
    Device* device;
    NAPI_CALL(env,
        napi_get_cb_info(env, info, NULL, NULL, NULL, (void**)&device));

    MA_CALL(env, "Unable to start device",
        ma_device_start(device->maDevice));

    return NULL;
}

napi_value create_Device(napi_env env, ma_device_config* config, Context* context, napi_value jsContext, napi_value jsCallback)
{
    config->dataCallback = data_callback;

    ma_device* maDevice = (ma_device*)malloc(sizeof(ma_device));
    Device* device = (Device*)malloc(sizeof(Device));
    device->maDevice = maDevice;
    uv_mutex_init(&device->mutex);
    uv_cond_init(&device->cond);

    napi_value tsfnName;
    napi_create_string_utf8(env, "node-playcap", NAPI_AUTO_LENGTH, &tsfnName);

    NAPI_CALL(env,
        napi_create_threadsafe_function(
            env,
            jsCallback,
            NULL,
            tsfnName,
            0,
            1,
            NULL,
            NULL,
            NULL,
            jsDataCallbackCaller,
            &device->threadsafeCallback
        ));

    config->pUserData = device;

    // prevent context destroying
    NAPI_CALL(env,
        napi_create_reference(env, jsContext, 1, &device->contextRef));

    MA_CALL(env, "Unable to init device",
        ma_device_init(&context->maContext, config, device->maDevice));

    napi_value jsObj;
    napi_value jsStart;
    napi_value jsDestroy;

    NAPI_CALL(env,
        napi_create_object(env, &jsObj));

    NAPI_CALL(env,
        napi_create_function(env, NULL, 0, start_Device, device, &jsStart));

    NAPI_CALL(env,
        napi_set_named_property(env, jsObj, "start", jsStart));

    NAPI_CALL(env,
        napi_create_function(env, NULL, 0, destroy_Device, device, &jsDestroy));

    NAPI_CALL(env,
        napi_set_named_property(env, jsObj, "destroy", jsDestroy));

    NAPI_CALL(env,
        napi_wrap(env, jsObj, device, &finalize_Device, NULL, NULL));

    return jsObj;
}

void refresh_devices(napi_env env, Context* context)
{
    MA_CALL(env, "Unable to retrieve device list",
        ma_context_get_devices(
            &context->maContext,
            &context->playbackDevicesInfos,
            &context->playbackCount,
            &context->captureDevicesInfos,
            &context->captureCount
        ));

    for (ma_uint32 i = 0; i < context->playbackCount; i++) {
        if (context->playbackDevicesInfos[i].isDefault) {
            context->defaultPlaybackDeviceIndex = i;
            break;
        }
    }

    for (ma_uint32 i = 0; i < context->captureCount; i++) {
        if (context->captureDevicesInfos[i].isDefault) {
            context->defaultCaptureDeviceIndex = i;
            break;
        }
    }
}
