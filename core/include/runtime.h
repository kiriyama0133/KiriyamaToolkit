// core/include/runtime.h
#pragma once
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum AsyncOperationState {
    ASYNC_OPERATION_PENDING,
    ASYNC_OPERATION_RUNNING,
    ASYNC_OPERATION_COMPLETED,
    ASYNC_OPERATION_CANCELLED,
    ASYNC_OPERATION_FAILED
} AsyncOperationState;
typedef enum AsyncError {
    ASYNC_ERROR_NONE = 0,
    ASYNC_ERROR_UNKNOWN,
    ASYNC_ERROR_CANCELLED,
    ASYNC_ERROR_TIMEOUT,
    ASYNC_ERROR_IO,
    ASYNC_ERROR_OUT_OF_MEMORY
} AsyncError;
typedef struct AsyncRuntime AsyncRuntime;
typedef struct AsyncOperation AsyncOperation;
typedef void (*AsyncCallback) (AsyncOperation* operation, void* context);
/* Runtime */
AsyncRuntime* async_runtime_create(void);
void async_runtime_destroy(AsyncRuntime* runtime);
// int async_runtime_run(AsyncRuntime* runtime);
void async_runtime_stop(AsyncRuntime* runtime);
// int async_runtime_poll(AsyncRuntime* runtime, int timeout_ms);
void async_runtime_enqueue(AsyncRuntime* runtime, AsyncOperation* operation);
/* Operation */
AsyncOperation* async_operation_create(AsyncRuntime* runtime, AsyncCallback callback, void* context);
void async_operation_cancel(AsyncOperation* operation);
int async_operation_submit(AsyncOperation* operation);
void* async_operation_result(AsyncOperation* operation);
void async_operation_destroy(AsyncOperation* operation);
AsyncError async_operation_error(AsyncOperation* operation);
AsyncOperationState async_operation_state(AsyncOperation* operation);
// Worker
static void* runtime_worker(void* context);
#ifdef __cplusplus
}
#endif
