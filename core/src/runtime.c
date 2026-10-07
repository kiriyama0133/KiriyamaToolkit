#include "runtime.h"
#include "platform/platform.h"
#include <stdlib.h>
#include "runtime_internal.h"

AsyncRuntime* async_runtime_global = NULL;
static int async_worker_limit = 1;

void limit_async_worker_count(int worker_limit)
{
    async_worker_limit = worker_limit;
}

AsyncRuntime* async_runtime_create()
{
    AsyncRuntime* runtime = calloc(1, sizeof(AsyncRuntime));

    if (!runtime)
        return NULL;

    runtime->platform = async_platform_create();

    if (!runtime->platform)
    {
        free(runtime);
        return NULL;
    }

    runtime->running = 0;
    runtime->stopped = 0;
    runtime->dispatching = 0;
    runtime->worker_count = 0;
    runtime->worker_limit = async_worker_limit;

    return runtime;
}

// B-mode: poll
// int async_runtime_poll(AsyncRuntime* runtime, int timeout_ms) {
//     if (!runtime) return -1;
//     if (runtime->stopped) return 0;
//     // wait for platform events
//     runtime->dispatching = 1;
//     int result = async_platform_wait(runtime->platform, timeout_ms);
//     if (result < 0) return -1;
//     // execute pending operations
//     AsyncOperation* operation = runtime->pending_head;
//     if (!operation) return 0;
//     runtime->pending_head = operation->next;
//     if (!runtime->pending_head) {
//         runtime->pending_tail = NULL;
//     }
//     operation->next = NULL;
//     // running
//     operation->queued = 0;
//     if (operation->state == ASYNC_OPERATION_CANCELLED) return 1;
//     operation->state = ASYNC_OPERATION_RUNNING;
//     // execute callback
//     if (operation->callback) {
//         operation->callback(operation, operation->context);
//     }
//     runtime->dispatching = 0;
//     // completed
//     if (operation->state == ASYNC_OPERATION_RUNNING) {
//         operation->state = ASYNC_OPERATION_COMPLETED;
//         operation->error = ASYNC_ERROR_NONE;
//     }
//     return 1;
// }

// A-mode: run
// int async_runtime_run(AsyncRuntime* runtime) {
//     if (!runtime) return -1;
//     if (runtime->running) return 0;
//     runtime->running = 1;
//     runtime->stopped = 0;
//     while (!runtime->stopped)
//     {
//         /* code */
//         async_runtime_poll(runtime, -1);
//     }
//     runtime->running = 0;
//     return 0;
// }

void async_runtime_stop(AsyncRuntime* runtime) {
    if (!runtime) return;
    async_platform_lock(runtime->platform);
    runtime->stopped = 1;
    // if run() is blocking and waiting for platform events, wakeup it
    async_platform_unlock(runtime->platform);
    async_platform_wakeup(runtime->platform);
}

static void async_runtime_enqueue(AsyncRuntime* runtime, AsyncOperation* operation) {
    operation->next = NULL;
    if (!runtime->pending_head) {
        runtime->pending_head = operation;
    } else {
        runtime->pending_tail->next = operation;
    }
    runtime->pending_tail = operation;
}

static void async_runtime_cleanup(void)
{
    if (async_runtime_global)
    {
        async_runtime_destroy(async_runtime_global);
        async_runtime_global = NULL;
    }
}

int async_operation_submit(AsyncOperation* operation) {
    if (!operation) return -1;
    AsyncRuntime* runtime = operation->runtime;
    if (!runtime) return -1;
    if (operation->state != ASYNC_OPERATION_PENDING) return -1;
    if (operation->queued) return -1;
    async_platform_lock(runtime->platform);
    operation->queued = 1;
    async_runtime_enqueue(runtime, operation);   
    // join in pending queue
    async_platform_unlock(runtime->platform);
    if (runtime->worker_count < runtime->worker_limit)
    {
        if (async_platform_start_worker(
            runtime->platform,
            runtime_worker,
            runtime) == 0)
        {
            runtime->worker_count++;
        }
    }
    async_platform_wakeup(runtime->platform);
    return 0;
}
AsyncOperation* async_operation_create(AsyncCallback callback, void* context) {
    AsyncOperation* operation = calloc(1, sizeof(AsyncOperation));
    if (!operation) return NULL;
    if (!async_runtime_global) {
        async_runtime_global = async_runtime_create();
        atexit(async_runtime_cleanup);
    }
    operation->runtime = async_runtime_global;
    operation->state = ASYNC_OPERATION_PENDING;
    operation->callback = callback;
    operation->context = context;
    operation->result = NULL;
    operation->error = ASYNC_ERROR_NONE;

    // part in runtime operation registry
    operation->registry_next = async_runtime_global->operations;
    async_runtime_global->operations = operation;
    async_operation_submit(operation);
    return operation;
}

void async_runtime_destroy(AsyncRuntime* runtime)
{
    if (!runtime) return;
    if (runtime->running || runtime->dispatching) return;
    async_runtime_stop(runtime);
    async_platform_join_worker(runtime->platform);
    AsyncOperation* operation = runtime->operations;
    while (operation)
    {
        AsyncOperation* next = operation->registry_next;
        free(operation);
        operation = next;
    }
    runtime->operations = NULL;
    async_platform_destroy(runtime->platform);
    free(runtime);
}

void async_operation_destroy(AsyncOperation* operation)
{
    if (!operation)
        return;
    AsyncRuntime* runtime = operation->runtime;
    if (!runtime)
        return;
    /*
     * 还在 pending queue 中，不能释放。
     */
    if (operation->queued) return;
    AsyncOperation** current = &runtime->operations;
    while (*current)
    {
        if (*current == operation)
        {
            *current = operation->registry_next;
            free(operation);
            return;
        }
        current = &(*current)->registry_next;
    }
}
int async_operation_wait(AsyncOperation* operation)
{
    if (!operation) return -1;
    while (operation->state == ASYNC_OPERATION_PENDING ||
           operation->state == ASYNC_OPERATION_RUNNING)
    {
        async_platform_wait(operation->runtime->platform, 1);
    }
    return 0;
}

void async_operation_cancel(AsyncOperation* operation) {
    if (!operation) return;
    if (operation->state != ASYNC_OPERATION_PENDING) return;
    operation->state = ASYNC_OPERATION_CANCELLED;
    operation->error = ASYNC_ERROR_CANCELLED;
}

AsyncOperationState async_operation_state(AsyncOperation* operation) {
    if (!operation) return ASYNC_OPERATION_FAILED;
    return operation->state;
}

AsyncResult async_operation_result(AsyncOperation* operation) {
    if (!operation) return (AsyncResult){NULL, ASYNC_OPERATION_FAILED, ASYNC_ERROR_UNKNOWN};
    if (operation->state != ASYNC_OPERATION_COMPLETED) return (AsyncResult){NULL, ASYNC_OPERATION_FAILED, ASYNC_ERROR_UNKNOWN};
    return (AsyncResult){operation->result, operation->state, operation->error};
}

AsyncError async_operation_error(AsyncOperation* operation) {
    if (!operation) return ASYNC_ERROR_UNKNOWN;
    return operation->error;
}

static void* runtime_worker(void* context)
{
    AsyncRuntime* runtime = (AsyncRuntime*)context;

    for (;;)
    {
        async_platform_lock(runtime->platform);
        if (runtime->stopped)
        {
            async_platform_unlock(runtime->platform);
            break;
        }

        AsyncOperation* operation = runtime->pending_head;
        if (operation)
        {
            runtime->pending_head = operation->next;
            if (!runtime->pending_head)
                runtime->pending_tail = NULL;
            operation->next = NULL;
            operation->queued = 0;
        }

        async_platform_unlock(runtime->platform);
        if (!operation)
        {
            async_platform_wait(runtime->platform, -1);
            continue;
        }

        if (operation->state == ASYNC_OPERATION_CANCELLED)
            continue;

        operation->state = ASYNC_OPERATION_RUNNING;
        if (operation->callback)
            operation->callback(operation, operation->context);
        if (operation->state == ASYNC_OPERATION_RUNNING)
        {
            operation->state = ASYNC_OPERATION_COMPLETED;
            operation->error = ASYNC_ERROR_NONE;
        }
    }
    return NULL;
}

AsyncResult async_operation_await(AsyncOperation* operation)
{
    async_operation_wait(operation);
    AsyncResult result;
    result.result = operation->result;
    result.state = operation->state;
    result.error = operation->error;
    return result;
}
