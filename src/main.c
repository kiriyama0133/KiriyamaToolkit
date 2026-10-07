#include <stdio.h>
#include <windows.h>
#include "runtime.h"

static void task1(AsyncOperation* operation, void* context)
{
    (void)operation;
    (void)context;
    printf("task start\n");
    Sleep(3000);
    printf("task done\n");
}
static void task2(AsyncOperation* operation, void* context)
{
    (void)operation;
    (void)context;
    printf("task start\n");
    Sleep(3000);
    printf("task done\n");
}

int main(void)
{
    AsyncOperation* operation1 = ASYNC_CREATE(task1, NULL);
    AsyncOperation* operation2 = ASYNC_CREATE(task2, NULL);
    // ASYNC_SUBMIT(operation);
    // AsyncResult result = ASYNC_AWAIT(operation);
    // printf("state = %d\n", result.state);
    // return 0;
    printf("main continues\n");
    Sleep(4000);
    printf("state = %d\n", async_operation_state(operation1));
    printf("state = %d\n", async_operation_state(operation2));
    return 0;
}