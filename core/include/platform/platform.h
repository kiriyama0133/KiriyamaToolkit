// core/include/platform.h
#pragma once
typedef struct AsyncPlatform AsyncPlatform;
AsyncPlatform* async_platform_create(void);
void async_platform_destroy(AsyncPlatform* platform);
int async_platform_wait(AsyncPlatform* platform, int timeout_ms);
void async_platform_wakeup(AsyncPlatform* platform);
#pragma once

typedef struct AsyncPlatform AsyncPlatform;

/*
 * 创建平台运行时对象
 */
AsyncPlatform* async_platform_create(void);

/*
 * 销毁平台运行时对象
 */
void async_platform_destroy(
    AsyncPlatform* platform
);

/*
 * 等待平台事件

 * timeout_ms:
 *   < 0  : 无限等待
 *   = 0  : 不等待，立即返回
 *   > 0  : 最多等待 timeout_ms 毫秒

 * 返回值:
 *   1 : 被事件/唤醒
 *   0 : 超时
 *  -1 : 错误
 */
int async_platform_wait(
    AsyncPlatform* platform,
    int timeout_ms
);

/*
 * 唤醒正在 wait 的线程
 */
void async_platform_wakeup(
    AsyncPlatform* platform
);