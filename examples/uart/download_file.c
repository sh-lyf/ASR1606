/*
 * @Author: yonghang.qin
 * @Date: 2021-11-10 17:19:22
 * @LastEditors: yonghang.qin
 * @LastEditTime: 2021-11-15 17:22:54
 */
#include "simcom_api.h"
#include "simcom_os.h"
#include "simcom_file_system.h"
#include "uart.h"
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#define sleep(x) sAPI_TaskSleep((x) * 200)
#define malloc(size) sAPI_Malloc(size)
#define free(ptr) sAPI_Free(ptr)
#define printf debug

#define FILE    SCFILE
#define fopen   sAPI_fopen
#define fclose  sAPI_fclose
#define fwrite  sAPI_fwrite
#define fread   sAPI_fread

static int debug(const char *format, ...)
{
    char buffer[128];
    int size;
    va_list va;

    va_start(va, format);
    size = vsnprintf(buffer, sizeof(buffer), format, va);
    va_end(va);

    sAPI_UartWrite(SC_UART, (UINT8 *)buffer, size);

    return size;
}

static inline int sAPI_feof(FILE *fp)
{
    return sAPI_fsize(fp) == sAPI_ftell(fp);
}

static void download_file_from_uart(void *param)
{
    const char *filename = "C:/baidu.pem";
    int filesize = 1282;
    char buffer[64];

    int fd = uart_open("/dev/uart1", 0);
    if (fd < 0)
    {
        printf("open uart fail.\r\n");
        return;
    }

    FILE *fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        uart_close(fd);
        printf("open file fail.\r\n");
        return;
    }

    printf("Please start enter data: \r\n");

    int total_read_bytes = 0;
    while (total_read_bytes < filesize)
    {
        int read_bytes = uart_read(fd, buffer, sizeof(buffer));
        if (read_bytes < 0)
        {
            printf("read data from uart fail.\r\n");
            goto error;
        }

        int write_bytes = 0;
        while (write_bytes < read_bytes)
        {
            int bytes = fwrite(buffer + write_bytes, 1, read_bytes - write_bytes, fp);
            if (bytes < 0)
            {
                printf("write data to file fail.\r\n");
                goto error;
            }

            write_bytes += bytes;
        }

        total_read_bytes += write_bytes;
    }

    printf("write file success.\r\n");

error:
    fclose(fp);
    uart_close(fd);
}

static void read_file_for_test(void *param)
{
    const char *filename = "C:/baidu.pem";
    char buffer[64];

    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        printf("open file fail.\r\n");
        return;
    }

    printf("Start output file: \r\n");

    while (!sAPI_feof(fp))
    {
        int bytes = sAPI_fread(buffer, 1, sizeof(buffer) - 1, fp);
        buffer[bytes] = 0;

        printf("%s", buffer);
    }

    printf("\r\nEnd output file.\r\n");

    fclose(fp);
}

void download_file_from_uart_demo(void)
{
    sTaskRef uartTask;

    if (0 != sAPI_TaskCreate(&uartTask, NULL, 4096, 120, (char *)"uart task", download_file_from_uart, NULL))
    {
        debug("uart task create fail.\r\n");
        return;
    }

    debug("uart task create success.\r\n");

    sleep(30);

    if (0 != sAPI_TaskCreate(&uartTask, NULL, 4096, 120, (char *)"test task", read_file_for_test, NULL))
    {
        debug("test task create fail.\r\n");
        return;
    }

    debug("test task create success.\r\n");
}