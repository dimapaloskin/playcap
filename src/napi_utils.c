#include <node_api.h>

#include "macros.h"

void try_get_named_property_uint32(napi_env env, napi_value obj, const char* utf8Name, uint32_t* result)
{
    bool hasProp = false;

    NAPI_CALL_NO_RETURN(env,
        napi_has_named_property(env, obj, utf8Name, &hasProp));

    if (!hasProp) {
        return;
    }

    napi_value num;

    NAPI_CALL_NO_RETURN(env,
        napi_get_named_property(env, obj, utf8Name, &num));

    NAPI_CALL_NO_RETURN(env,
        napi_get_value_uint32(env, num, result));
}

