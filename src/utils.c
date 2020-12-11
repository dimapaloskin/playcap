#include <node_api.h>

#include "miniaudio.h"
#include "macros.h"
#include "napi_utils.h"


void create_device_info_object(napi_env env, napi_value obj, ma_device_info* deviceInfo) {
    napi_value deviceName;
    NAPI_CALL_NO_RETURN(env,
        napi_create_string_utf8(
            env,
            deviceInfo->name,
            NAPI_AUTO_LENGTH,
            &deviceName));

    NAPI_CALL_NO_RETURN(env,
        napi_set_named_property(env, obj, "name", deviceName));
}
