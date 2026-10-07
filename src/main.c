#include <stdio.h>
#include <windows.h>
#include "runtime.h"

static void task1(AsyncOperation* operation, void* context)
{
    (void)operation; (void)context;
    printf("task1 start\n");
    Sleep(3000);
    printf("task1 done\n");
}

static void task2(AsyncOperation* operation, void* context)
{
    (void)operation; (void)context;
    printf("task2 start\n");
    Sleep(3000);
    printf("task2 done\n");
}

int main(void)
{
    AsyncOperation* operation1 = ASYNC_CREATE(task1, NULL);
    ASYNC_SUBMIT(operation1);
    AsyncOperation* operation2 = async_operation_create_child(operation1, task2, NULL);
    printf("main continues\n");

    ASYNC_AWAIT(operation1);
    ASYNC_AWAIT(operation2);

    printf("op1 state = %d\n", async_operation_state(operation1));
    printf("op2 state = %d\n", async_operation_state(operation2));
    return 0;
}