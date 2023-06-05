/*
 * @Author: yonghang.qin
 * @Date: 2021-11-10 13:28:50
 * @LastEditors: yonghang.qin
 * @LastEditTime: 2021-11-10 15:33:21
 */
#include "simcom_api.h"
#include "simcom_os.h"
#include "simcom_ssl.h"
#include "sockets.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#define sleep(x) sAPI_TaskSleep(x * 200)

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

static bool ssl_pdp_active(void)
{
    const int counter = 10;

    for (int i = 0; i < counter; i++)
    {
        if (sAPI_TcpipPdpActive(1, 1) != 0)
        {
            debug("pdp active fail. waiting 3s.\r\n");
            sleep(3);
            continue;
        }

        debug("pdp active success.\r\n");
        return true;
    }

    return false;
}

static int ssl_connect_to_server(const char *domain, const char *port)
{
    if (domain == NULL)
    {
        debug("The paramter is invalid. Please check input parameter.\r\n");
        return -1;
    }

    int fd = -1;
    struct addrinfo hints, *addr_list;

    memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = SC_AF_INET;
    hints.ai_socktype = SC_SOCK_STREAM;
    hints.ai_protocol = SC_IPPROTO_TCP;

    if (getaddrinfo(domain, port, &hints, &addr_list ) != 0 )
    {
        debug("Dns parse domain fail. Please check domain:%s.\r\n", domain);
        return -1;
    }

    bool flag = false;
    for (struct addrinfo *cur = addr_list; cur != NULL; cur = cur->ai_next)
    {
        fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (fd < 0)
        {
            debug("create socket fail.\r\n");
            continue;
        }

        if (connect(fd, cur->ai_addr, cur->ai_addrlen) == 0)
        {
            flag = true;
            debug("connect to server success.\r\n");
            break;
        }

        close(fd);
    }

    freeaddrinfo(addr_list);

    return flag ? fd : -1;
}

static void ssl_verify_server(void *ptr)
{
    const char *domain = "www.baidu.com";
    const char *port = "443";
    const char *request = "GET / HTTP/1.1\r\nConnection: Close\r\n\r\n";
    const char *ca_cert =
        "-----BEGIN CERTIFICATE-----\r\n"
        "MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\r\n"
        "A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\r\n"
        "b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\r\n"
        "MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\r\n"
        "YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\r\n"
        "aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\r\n"
        "jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\r\n"
        "xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\r\n"
        "1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\r\n"
        "snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\r\n"
        "U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\r\n"
        "9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\r\n"
        "BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\r\n"
        "AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\r\n"
        "yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\r\n"
        "38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\r\n"
        "AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\r\n"
        "DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\r\n"
        "HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\r\n"
        "-----END CERTIFICATE-----\r\n";

    debug("Start ssl connect.\r\n");

    if (ssl_pdp_active() != true)
    {
        debug("The network may be unavailable. Please check related equipment.\r\n");
        return;
    }

    int fd = ssl_connect_to_server(domain, port);
    if (fd < 0)
    {
        debug("connect to server:%s fail. error:%d.\r\n", domain, sAPI_TcpipGetErrno());
        return;
    }

    SCSslCtx_t sslCtx;

    memset(&sslCtx, 0, sizeof(SCSslCtx_t));
    sslCtx.ClientId = 0;
    sslCtx.fd = fd;
    sslCtx.ciphersuitesetflg = 0;
    sslCtx.ssl_version = SC_SSL_CFG_VERSION_TLS12;
    sslCtx.enable_SNI = 0;
    sslCtx.auth_mode = SC_SSL_CFG_VERIFY_MODE_REQUIRED;
    sslCtx.ignore_local_time = 1;
    sslCtx.root_ca = (char *)ca_cert;
    sslCtx.root_ca_len = strlen(ca_cert) + 1;

    if (sAPI_SslHandShake(&sslCtx) != 0)
    {
        debug("ssl handshake fail.\r\n");
        return;
    }

    debug("Ssl Handshake success.\r\n");

    int status = 0;
    int send_len = 0;
    int total_len = strlen(request);

    while (send_len < total_len)
    {
        int bytes = sAPI_SslSend(0, (INT8 *)request + send_len, total_len - send_len);
        if (bytes < 0)
        {
            status = 1;
            debug("send ssl data fail.\r\n");
            break;
        }
        else if (bytes == 0)
        {
            status = 2;
            debug("the remote server is closed.\r\n");
            break;
        }

        send_len += bytes;
    }

    if (status != 0)
    {
        goto close_ssl;
    }

    while (true)
    {
        char buffer[512];
        int bytes;

        bytes = sAPI_SslRead(0, buffer, sizeof(buffer) - 1);
        if (bytes < 0)
        {
            debug("read ssl data fail.\r\n");
            break;
        }
        else if (bytes == 0)
        {
            debug("the remote server is closed.\r\n");
            break;
        }

        buffer[bytes] = 0;

        debug("read bytes:%d, read data:%s.\r\n", bytes, buffer);
    }

close_ssl:
    sAPI_SslClose(0);
    close(fd);
    sAPI_TcpipPdpDeactive(1, 1);

    debug("End ssl connect.\r\n");
}

void ssl_verify_server_demo(void)
{
    sTaskRef sslTask;

    if (0 != sAPI_TaskCreate(&sslTask, NULL, 8192, 120, (char *)"ssl verify task", ssl_verify_server, NULL))
    {
        debug("ssl verify task create fail.\r\n");
        return;
    }

    debug("ssl verify task create success.\r\n");
}