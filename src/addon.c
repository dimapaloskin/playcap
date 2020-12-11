#include <assert.h>
#include <node_api.h>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "macros.h"
#include "context.h"

napi_value Init(napi_env env, napi_value exports)
{
    napi_value Context;

    size_t propsCount = 3;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_METHOD("getDevices", Context_getDevices),
        DECLARE_NAPI_METHOD("refreshDevices", Context_refreshDevices),
        DECLARE_NAPI_METHOD("createDevice", Context_createDevice)
    };

    NAPI_CALL(env,
        napi_define_class(
            env,
            "Context",
            NAPI_AUTO_LENGTH,
            Context_constructor,
            NULL,
            propsCount,
            properties,
            &Context));

    NAPI_CALL(env,
        napi_set_named_property(env, exports, "Context", Context));

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
