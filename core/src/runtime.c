#include "runtime.h"
#include "platform/platform.h"
#include <stdlib.h>
#include "runtime_internal.h"

AsyncRuntime* async_runtime_create(void) {
    AsyncRuntime* runtime = calloc(1, sizeof(AsyncRuntime));
    if (!runtime) return NULL;
    runtime->platform=async_platform_create();
    if(!runtime->platform) {
        free(runtime);
        return NULL;
    }
    runtime->running = 0;
    runtime->stopped = 0;
    return runtime;
}

// B-mode: poll
int async_runtime_poll(AsyncRuntime* runtime, int timeout_ms) {
    if (!runtime) return -1;
    if (runtime->stopped) return 0;
    // wait for platform events
    runtime->dispatching = 1;
    int result = async_platform_wait(runtime->platform, timeout_ms);
    if (result < 0) return -1;
    // execute pending operations
    AsyncOperation* operation = runtime->pending_head;
    if (!operation) return 0;
    runtime->pending_head = operation->next;
    if (!runtime->pending_head) {
        runtime->pending_tail = NULL;
    }
    operation->next = NULL;
    // running
    operation->queued = 0;
    if (operation->state == ASYNC_OPERATION_CANCELLED) return 1;
    operation->state = ASYNC_OPERATION_RUNNING;
    // execute callback
    if (operation->callback) {
        operation->callback(operation, operation->context);
    }
    runtime->dispatching = 0;
    // completed
    if (operation->state == ASYNC_OPERATION_RUNNING) {
        operation->state = ASYNC_OPERATION_COMPLETED;
        operation->error = ASYNC_ERROR_NONE;
    }
    return 1;
}

// A-mode: run
int async_runtime_run(AsyncRuntime* runtime) {
    if (!runtime) return -1;
    if (runtime->running) return 0;
    runtime->running = 1;
    runtime->stopped = 0;
    while (!runtime->stopped)
    {
        /* code */
        async_runtime_poll(runtime, -1);
    }
    runtime->running = 0;
    return 0;
}

void async_runtime_stop(AsyncRuntime* runtime) {
    if (!runtime) return;
    runtime->stopped = 1;
    // if run() is blocking and waiting for platform events, wakeup it
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

int async_operation_submit(AsyncOperation* operation) {
    if (!operation) return -1;
    AsyncRuntime* runtime = operation->runtime;
    if (!runtime) return -1;
    if (operation->state != ASYNC_OPERATION_PENDING) return -1;
    if (operation->queued) return -1;
    operation->queued = 1;
    async_runtime_enqueue(runtime, operation);
    async_platform_wakeup(runtime->platform);
    return 0;
}
AsyncOperation* async_operation_create(AsyncRuntime* runtime, AsyncCallback callback, void* context) {
    if (!runtime) return NULL;
    AsyncOperation* operation = calloc(1, sizeof(AsyncOperation));
    if (!operation) return NULL;
    operation->runtime = runtime;
    operation->state = ASYNC_OPERATION_PENDING;
    operation->callback = callback;
    operation->context = context;
    operation->result = NULL;
    operation->error = ASYNC_ERROR_NONE;

    // part in runtime operation registry
    operation->registry_next = runtime->operations;
    runtime->operations = operation;
    return operation;
}

void async_runtime_destroy(AsyncRuntime* runtime)
{
    if (!runtime) return;
    if (runtime->running || runtime->dispatching) return;
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
    AsyncRuntime* runtime =
        operation->runtime;
    if (!runtime)
        return;
    /*
     * 还在 pending queue 中，不能释放。
     */
    if (operation->queued)
        return;
    AsyncOperation** current =
        &runtime->operations;
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

void* async_operation_result(AsyncOperation* operation) {
    if (!operation) return NULL;
    if (operation->state != ASYNC_OPERATION_COMPLETED) return NULL;
    return operation->result;
}

AsyncError async_operation_error(AsyncOperation* operation) {
    if (!operation) return ASYNC_ERROR_UNKNOWN;
    return operation->error;
}