#include "platform/platform.h"
#include <windows.h>
#include <stdlib.h>
#include <process.h>

struct AsyncPlatform {
    HANDLE wake_event;
    HANDLE* workers;
    int worker_count;
    CRITICAL_SECTION lock;
};

typedef struct {
    void* (*entry)(void*);
    void* context;
} AsyncPlatformWorker;

AsyncPlatform* async_platform_create(void) {
    AsyncPlatform* platform = calloc(1, sizeof(AsyncPlatform));
    if (!platform) return NULL;
    platform->wake_event = CreateEventA(NULL, FALSE, FALSE, NULL); // 
    if (!platform->wake_event) {
        free(platform);
        return NULL;
    }
    InitializeCriticalSection(&platform->lock);
    return platform;
}

void async_platform_destroy(AsyncPlatform* platform) {
    if (!platform) return;
    if (platform->workers) {
        for (int i = 0; i < platform->worker_count; i++) {
            CloseHandle(platform->workers[i]);
            platform->workers[i] = NULL;
        }
        free(platform->workers);
    }
    if (platform->wake_event) {
        CloseHandle(platform->wake_event);
        platform->wake_event = NULL;
    }
    DeleteCriticalSection(&platform->lock);
    free(platform);
}

int async_platform_wait(AsyncPlatform* platform, int timeout_ms) {
    if (!platform) return -1;
    DWORD timeout;
    if (timeout_ms < 0) {
        timeout = INFINITE;
    } else {
        timeout = (DWORD)timeout_ms;
    }
    DWORD result = WaitForSingleObject(platform->wake_event, timeout);
    if (result == WAIT_OBJECT_0) {
        return 1;
    } else if (result == WAIT_TIMEOUT) {
        return 0;
    } else {
        return -1;
    }
}

void async_platform_wakeup(AsyncPlatform* platform) {
    if (!platform) return;
    SetEvent(platform->wake_event);
}

static unsigned __stdcall platform_thread_entry(void* arg) {
    AsyncPlatformWorker* worker = (AsyncPlatformWorker*)arg;
    if (!worker) return 0;
    worker->entry(worker->context);
    free(worker);
    return 0;
}

int async_platform_start_worker(AsyncPlatform* platform, void* (*entry)(void*), void* context) {
    if (!platform || !entry) return -1;
    AsyncPlatformWorker* worker = malloc(sizeof(AsyncPlatformWorker));
    if (!worker) return -1;
    worker->entry = entry;
    worker->context = context;
    uintptr_t handler = _beginthreadex(NULL, 0, platform_thread_entry, worker, 0, NULL);
    if (handler == 0) {
        free(worker);
        return -1;
    }
    HANDLE* workers = realloc(platform->workers, sizeof(HANDLE) * (platform->worker_count + 1));
    if (!workers) {
        CloseHandle((HANDLE)handler);
        free(worker);
        return -1;
    }
    platform->workers = workers;
    platform->workers[platform->worker_count] = (HANDLE)handler;
    platform->worker_count++;

    return 0;
}

void async_platform_join_worker(AsyncPlatform* platform)
{
    if (!platform || !platform->workers)
        return;
    for (int i = 0; i < platform->worker_count; i++)
        WaitForSingleObject(platform->workers[i], INFINITE);
}

void async_platform_lock(AsyncPlatform* platform) {
    if (!platform) return;
    EnterCriticalSection(&platform->lock);
}

void async_platform_unlock(AsyncPlatform* platform) {
    if (!platform) return;
    LeaveCriticalSection(&platform->lock);
}