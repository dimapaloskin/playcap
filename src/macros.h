#ifndef nma_macros_h
#define nma_macros_h

#define DECLARE_NAPI_METHOD(name, func)                          \
  { name, 0, func, 0, 0, 0, napi_default, 0 }


// From https://nodejs.org/api/n-api.html#n_api_n_api_version_matrix
#define NAPI_CALL(env, call)                                      \
  do {                                                            \
    napi_status status = (call);                                  \
    if (status != napi_ok) {                                      \
      const napi_extended_error_info* error_info = NULL;          \
      napi_get_last_error_info((env), &error_info);               \
      bool is_pending;                                            \
      napi_is_exception_pending((env), &is_pending);              \
      if (!is_pending) {                                          \
        const char* message = (error_info->error_message == NULL) \
            ? "empty error message"                               \
            : error_info->error_message;                          \
        napi_throw_error((env), NULL, message);                   \
        return NULL;                                              \
      }                                                           \
    }                                                             \
  } while(0)

#define NAPI_CALL_NO_RETURN(env, call)                                      \
  do {                                                            \
    napi_status status = (call);                                  \
    if (status != napi_ok) {                                      \
      const napi_extended_error_info* error_info = NULL;          \
      napi_get_last_error_info((env), &error_info);               \
      bool is_pending;                                            \
      napi_is_exception_pending((env), &is_pending);              \
      if (!is_pending) {                                          \
        const char* message = (error_info->error_message == NULL) \
            ? "empty error message"                               \
            : error_info->error_message;                          \
        napi_throw_error((env), NULL, message);                   \
      }                                                           \
    }                                                             \
  } while(0)

#define MA_CALL(env, message, call)                              \
    do {                                                        \
      ma_result miniaudio_result = (call);                      \
      if (miniaudio_result != MA_SUCCESS) {                     \
          napi_throw_error((env), NULL, (message));             \
      }                                                         \
    } while (0)

#define UNUSED(x) (void)(x)

#endif
