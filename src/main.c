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
    AsyncOperation* operation = ASYNC_CREATE(task, NULL);
    // ASYNC_SUBMIT(operation);
    // AsyncResult result = ASYNC_AWAIT(operation);
    // printf("state = %d\n", result.state);
    // return 0;
    printf("main continues\n");
    Sleep(1000);
    printf("main still running\n");
    Sleep(3000);
    printf("state = %d\n", async_operation_state(operation));
    return 0;
}