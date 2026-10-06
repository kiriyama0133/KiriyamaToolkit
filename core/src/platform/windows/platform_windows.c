#include "platform/platform.h"
#include <windows.h>

struct AsyncPlatform {
    HANDLE wake_event;
};

AsyncPlatform* async_platform_create(void) {
    AsyncPlatform* platform = calloc(1, sizeof(AsyncPlatform));
    if (!platform) return NULL;
    platform->wake_event = CreateEventA(NULL, FALSE, FALSE, NULL); // 
    if (!platform->wake_event) {
        free(platform);
        return NULL;
    }
    return platform;
}

void async_platform_destroy(AsyncPlatform* platform) {
    if (!platform) return;
    if (platform->wake_event) {
        CloseHandle(platform->wake_event);
        platform->wake_event = NULL;
    }
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
