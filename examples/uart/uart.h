/*
 * @Author: yonghang.qin
 * @Date: 2021-11-11 10:26:21
 * @LastEditors: yonghang.qin
 * @LastEditTime: 2021-11-15 17:34:04
 */
#ifndef __UART_H__
#define __UART_H__

//"/dev/uart1"  => SC_UART
//"/dev/uart2"  => SC_UART2
//"/dev/uart3"  => SC_UART3
int uart_open(const char *filename, int oflag);
int uart_close(int fd);
int uart_read(int fd, void *buffer, int size);
int uart_write(int fd, const void *buffer, int size);

#endif
