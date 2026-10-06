#include <stdio.h>
#include "runtime.h"

static void callback_a(AsyncOperation* operation, void* context)
{
    const char* name = (const char*)context;
    printf("callback: %s\n", name);
    (void)operation;
}

static void callback_b(AsyncOperation* operation, void* context)
{
    const char* name = (const char*)context;
    printf("callback: %s\n", name);
    (void)operation;
}

int main(void)
{
    printf("=== create runtime ===\n");

    AsyncRuntime* runtime = async_runtime_create();
    if (!runtime) {
        printf("failed to create runtime\n");
        return 1;
    }

    AsyncOperation* op_a = async_operation_create(runtime, callback_a, "operation A");
    AsyncOperation* op_b = async_operation_create(runtime, callback_b, "operation B");

    if (!op_a || !op_b) {
        printf("failed to create operation\n");
        async_runtime_destroy(runtime);
        return 1;
    }

    printf("op_a state = %d\n", async_operation_state(op_a));
    printf("op_b state = %d\n", async_operation_state(op_b));

    printf("\n=== submit operations ===\n");

    async_operation_submit(op_a);
    async_operation_submit(op_b);

    printf("\n=== poll #1 ===\n");

    int result = async_runtime_poll(runtime, 0);
    printf("poll result = %d\n", result);
    printf("op_a state = %d\n", async_operation_state(op_a));
    printf("op_b state = %d\n", async_operation_state(op_b));

    printf("\n=== poll #2 ===\n");

    result = async_runtime_poll(runtime, 0);
    printf("poll result = %d\n", result);
    printf("op_a state = %d\n", async_operation_state(op_a));
    printf("op_b state = %d\n", async_operation_state(op_b));

    printf("\n=== cancel test ===\n");

    AsyncOperation* op_c = async_operation_create(runtime, callback_a, "operation C");
    if (!op_c) {
        printf("failed to create op_c\n");
        async_runtime_destroy(runtime);
        return 1;
    }

    async_operation_submit(op_c);

    printf("before cancel: state = %d\n", async_operation_state(op_c));

    async_operation_cancel(op_c);

    printf("after cancel: state = %d\n", async_operation_state(op_c));

    printf("\n=== poll cancelled operation ===\n");

    result = async_runtime_poll(runtime, 0);
    printf("poll result = %d\n", result);
    printf("op_c state = %d\n", async_operation_state(op_c));

    printf("\n=== destroy runtime ===\n");

    async_runtime_destroy(runtime);

    printf("runtime destroyed\n");

    return 0;
}