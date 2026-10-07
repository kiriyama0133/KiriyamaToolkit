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
typedef struct AsyncResult
{
    void* result;
    AsyncOperationState state;
    AsyncError error;
} AsyncResult;
typedef struct AsyncRuntime AsyncRuntime;
typedef struct AsyncOperation AsyncOperation;
typedef void (*AsyncCallback) (AsyncOperation* operation, void* context);
/* Runtime */
AsyncRuntime* async_runtime_create(void);
void async_runtime_destroy(AsyncRuntime* runtime);
extern AsyncRuntime* async_runtime_global;
static void async_runtime_cleanup(void);
// int async_runtime_run(AsyncRuntime* runtime);
void async_runtime_stop(AsyncRuntime* runtime);
// int async_runtime_poll(AsyncRuntime* runtime, int timeout_ms);
void async_runtime_enqueue(AsyncRuntime* runtime, AsyncOperation* operation);
/* Operation */
AsyncOperation* async_operation_create(AsyncCallback callback, void* context);
void async_operation_cancel(AsyncOperation* operation);
int async_operation_submit(AsyncOperation* operation);
AsyncResult async_operation_result(AsyncOperation* operation);
AsyncResult async_operation_await(AsyncOperation* operation);
void async_operation_destroy(AsyncOperation* operation);
AsyncError async_operation_error(AsyncOperation* operation);
AsyncOperationState async_operation_state(AsyncOperation* operation);
// Worker
static void* runtime_worker(void* context);
void limit_async_worker_count(int worker_limit);

// macro
#define ASYNC_WORKER_LIMIT(worker_limit) \
    limit_async_worker_count(worker_limit)

#define ASYNC_CREATE(callback, context) \
    async_operation_create(callback, context)

#define ASYNC_SUBMIT(operation) \
    async_operation_submit(operation)

#define ASYNC_CANCEL(operation) \
    async_operation_cancel(operation)

#define ASYNC_AWAIT(operation) \
    async_operation_await(operation)

#define ASYNC_RESULT(operation) \
    async_operation_result(operation)

#ifdef __cplusplus
}
#endif

