// core/include/platform.h
#pragma once
typedef struct AsyncPlatform AsyncPlatform;
AsyncPlatform* async_platform_create(void);
void async_platform_destroy(AsyncPlatform* platform);
int async_platform_wait(AsyncPlatform* platform, int timeout_ms);
void async_platform_wakeup(AsyncPlatform* platform);
int async_platform_start_worker(AsyncPlatform* platform, void* (*entry)(void*), void* context);
void async_platform_join_worker(AsyncPlatform* platform);
void async_platform_lock(AsyncPlatform* platform);
void async_platform_unlock(AsyncPlatform* platform);

