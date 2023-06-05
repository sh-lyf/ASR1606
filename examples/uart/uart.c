/*
 * @Author: yonghang.qin
 * @Date: 2021-11-11 10:26:34
 * @LastEditors: yonghang.qin
 * @LastEditTime: 2021-11-15 17:35:19
 */
#include "simcom_api.h"
#include "simcom_uart.h"
#include "simcom_os.h"
#include <string.h>
#include <stdbool.h>

#define sleep(x) sAPI_TaskSleep(x * 200)
#define malloc(size) sAPI_Malloc(size)
#define free(ptr) sAPI_Free(ptr)
#define printf(...)

#define UART_DEFAULT_READ_BUFFER_SIZE (512)

#define Min(x, y) (((x) < (y)) ? (x) : (y))

struct uart_buffer_t
{
    int size;
    char *buffer;
    int writed_bytes;
    char *read;
    char *write;
};

struct uart_t
{
    int number;
    sFlagRef read_flag;
    struct uart_buffer_t read_buffer;
};

static struct uart_t *__uart_fd_array[SC_UART_MAX];

static int uart_get_unused_fd(void)
{
    for (int i = 0; i < SC_UART_MAX; i++)
    {
        if (__uart_fd_array[i] == NULL)
        {
            return i;
        }
    }

    return -1;
}

static struct uart_t *uart_get_struct_by_number(int number)
{
    int i;

    for (i = 0; i < sizeof(__uart_fd_array) / sizeof(__uart_fd_array[0]); i++)
    {
        if (__uart_fd_array[i] && __uart_fd_array[i]->number == number)
        {
            return __uart_fd_array[i];
        }
    }

    return NULL;
}

static void uart_free_struct_by_fd(int fd)
{
    if (__uart_fd_array[fd] != NULL)
    {
        free(__uart_fd_array[fd]);
        __uart_fd_array[fd] = NULL;
    }
}

static int uart_get_number_by_filename(const char *filename)
{
    if (strcmp(filename, "/dev/uart1") == 0)
    {
        return SC_UART;
    }
    else if (strcmp(filename, "/dev/uart2") == 0)
    {
        return SC_UART2;
    }
    else if (strcmp(filename, "/dev/uart3") == 0)
    {
        return SC_UART3;
    }

    return -1;
}

static void uart_callback(SC_Uart_Port_Number number, int read_size, void *param)
{
    struct uart_t *uart = uart_get_struct_by_number(number);
    if (uart == NULL)
    {
        printf("not know error.\r\n");
        return;
    }

    if (uart->read_buffer.writed_bytes + read_size > uart->read_buffer.size)
    {
        printf("data overflow.\r\n");
        return;
    }

    bool flag = (uart->read_buffer.writed_bytes == 0);

    if (uart->read_buffer.write + read_size > uart->read_buffer.buffer + uart->read_buffer.size)
    {
        int size = uart->read_buffer.buffer + uart->read_buffer.size - uart->read_buffer.write;
        sAPI_UartRead(uart->number, (unsigned char *)uart->read_buffer.write, size);
        sAPI_UartRead(uart->number, (unsigned char *)uart->read_buffer.buffer, read_size - size);
    }
    else
    {
        sAPI_UartRead(uart->number, (unsigned char *)uart->read_buffer.write, read_size);
    }

    uart->read_buffer.write += read_size;
    uart->read_buffer.write = (uart->read_buffer.write > uart->read_buffer.buffer + uart->read_buffer.size)
        ? (uart->read_buffer.write - uart->read_buffer.size) : uart->read_buffer.write;
    uart->read_buffer.writed_bytes += read_size;

    if (flag)
    {
        sAPI_FlagSet(uart->read_flag, 1, SC_FLAG_OR);
    }
}

int uart_open(const char *filename, int oflag)
{
    int fd = uart_get_unused_fd();
    if (fd < 0)
    {
        printf("get a unused fd fail.\r\n");
        return -1;
    }

    int number = uart_get_number_by_filename(filename);
    if (number < 0)
    {
        printf("not know device.\r\n");
        return -1;
    }

    struct uart_t *uart = (struct uart_t *)malloc(sizeof(struct uart_t));
    if (uart == NULL)
    {
        printf("no memory to create uart.\r\n");
        return -1;
    }

    char *buffer = (char *)malloc(UART_DEFAULT_READ_BUFFER_SIZE);
    if (buffer == NULL)
    {
        printf("no memory to create buffer.\r\n");
        goto error;
    }

    sFlagRef read_flag;
    int ret = sAPI_FlagCreate(&read_flag);
    if (ret != 0)
    {
        printf("create read flag for uart fail.\r\n");
        goto error;
    }

    sAPI_UartRegisterCallbackEX(number, uart_callback, NULL);

    uart->number = number;
    uart->read_flag = read_flag;
    uart->read_buffer.size = UART_DEFAULT_READ_BUFFER_SIZE;
    uart->read_buffer.buffer = buffer;
    uart->read_buffer.writed_bytes = 0;
    uart->read_buffer.read = buffer;
    uart->read_buffer.write = buffer;

    __uart_fd_array[fd] = uart;

    return fd;

error:
    free(buffer);
    free(uart);
    return -1;
}

int uart_close(int fd)
{
    struct uart_t *uart = (fd < 0 || fd >= SC_UART_MAX) ? NULL : __uart_fd_array[fd];
    if (uart == NULL)
    {
        printf("invalid fd.\r\n");
        return -1;
    }

    sAPI_UartRegisterCallbackEX(uart->number, NULL, NULL);

    uart_free_struct_by_fd(fd);

    return 0;
}

int uart_read(int fd, void *buffer, int size)
{
    if (buffer == NULL || size <= 0)
    {
        printf("invalid paramter.\r\n");
        return -1;
    }

    struct uart_t *uart = (fd < 0 || fd >= SC_UART_MAX) ? NULL : __uart_fd_array[fd];
    if (uart == NULL)
    {
        printf("invalid fd.\r\n");
        return -1;
    }

    if (uart->read_buffer.writed_bytes <= 0)
    {
        UINT32 flags;
        sAPI_FlagWait(uart->read_flag, 1, SC_FLAG_OR_CLEAR, &flags, SC_SUSPEND);
    }

    size = Min(size, uart->read_buffer.writed_bytes);

    if (uart->read_buffer.read + size > uart->read_buffer.buffer + uart->read_buffer.size)
    {
        int first_read = uart->read_buffer.buffer + uart->read_buffer.size - uart->read_buffer.read;
        memcpy(buffer, uart->read_buffer.read, first_read);
        memcpy(buffer + first_read, uart->read_buffer.buffer, size - first_read);
    }
    else
    {
        memcpy(buffer, uart->read_buffer.read, size);
    }

    uart->read_buffer.read += size;
    uart->read_buffer.read = (uart->read_buffer.read > uart->read_buffer.buffer + uart->read_buffer.size)
        ? (uart->read_buffer.read - uart->read_buffer.size) : uart->read_buffer.read;
    uart->read_buffer.writed_bytes -= size;

    return size;
}

int uart_write(int fd, const void *buffer, int size)
{
    struct uart_t *uart = (fd < 0 || fd >= SC_UART_MAX) ? NULL : __uart_fd_array[fd];
    if (uart == NULL)
    {
        printf("invalid fd.\r\n");
        return -1;
    }

    sAPI_UartWrite(uart->number, (UINT8 *)buffer, size);

    return size;
}
