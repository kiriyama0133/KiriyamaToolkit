#include <stdio.h>
#include <windows.h>

#include "runtime.h"

static void task(AsyncOperation* operation, void* context)
{
    (void)operation;
    (void)context;

    printf("task start\n");
    Sleep(3000);
    printf("task done\n");
}

int main(void)
{
    AsyncRuntime* runtime = async_runtime_create();
    AsyncOperation* operation = async_operation_create(runtime, task, NULL);
    async_operation_submit(operation);
    printf("main continues\n");
    Sleep(1000);
    printf("state = %d\n", async_operation_state(operation));
    async_runtime_destroy(runtime);
    return 0;
}