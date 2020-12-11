#ifndef nma_napi_utils_h
#define nma_napi_utils_h

#include <node_api.h>

void try_get_named_property_uint32(napi_env env, napi_value obj, const char* utf8Name, uint32_t* result);

#endif
