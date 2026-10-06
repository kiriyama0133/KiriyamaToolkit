#include "platform/platform.h"

#include <sys/eventfd.h>
#include <unistd.h>

#include <poll.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>


struct AsyncPlatform
{
    int wake_fd;
};


AsyncPlatform* async_platform_create(void)
{
    AsyncPlatform* platform =
        calloc(1, sizeof(AsyncPlatform));

    if (!platform)
        return NULL;

    platform->wake_fd =
        eventfd(
            0,
            EFD_CLOEXEC
        );

    if (platform->wake_fd == -1)
    {
        free(platform);
        return NULL;
    }

    return platform;
}


void async_platform_destroy(
    AsyncPlatform* platform
)
{
    if (!platform)
        return;

    if (platform->wake_fd != -1)
    {
        close(platform->wake_fd);
    }

    free(platform);
}


int async_platform_wait(
    AsyncPlatform* platform,
    int timeout_ms
)
{
    if (!platform)
        return -1;

    struct pollfd pfd;

    pfd.fd = platform->wake_fd;
    pfd.events = POLLIN;
    pfd.revents = 0;

    int result;

    do
    {
        result = poll(
            &pfd,
            1,
            timeout_ms
        );
    }
    while (result == -1 && errno == EINTR);

    if (result == -1)
    {
        return -1;
    }

    if (result == 0)
    {
        return 0;
    }

    if (pfd.revents & POLLIN)
    {
        uint64_t value;

        /*
         * 消耗 eventfd 信号。
         */
        if (read(
                platform->wake_fd,
                &value,
                sizeof(value)
            ) == sizeof(value))
        {
            return 1;
        }

        return -1;
    }

    return -1;
}


void async_platform_wakeup(
    AsyncPlatform* platform
)
{
    if (!platform)
        return;

    uint64_t value = 1;

    /*
     * 向 eventfd 写入一个信号，
     * 唤醒正在 poll() 的线程。
     */
    ssize_t result;

    do
    {
        result = write(
            platform->wake_fd,
            &value,
            sizeof(value)
        );
    }
    while (result == -1 && errno == EINTR);
}