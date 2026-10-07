#include "platform/platform.h"

struct AsyncRuntime
{
    AsyncPlatform* platform;
    AsyncOperation* pending_head;
    AsyncOperation* pending_tail;
    AsyncOperation* operations;
    int running;
    int stopped;
    int dispatching;
    int worker_count;
    int worker_limit;
};
struct AsyncOperation
{
    AsyncRuntime* runtime;
    AsyncOperationState state;
    AsyncCallback callback;
    void* context;
    void* result;
    AsyncError error;
    AsyncOperation* next;          // pending queue
    AsyncOperation* registry_next;  // runtime operation registry
    int queued;
};