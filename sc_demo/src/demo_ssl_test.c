/**
  ******************************************************************************
  * @file    demo_ssl_test.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of ssl feature.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */
#ifdef FEATURE_SIMCOM_MSSL
/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "simcom_ssl.h"
#include "simcom_tcpip.h"
#include "simcom_tcpip_old.h"
#include "simcom_debug.h"
#include "simcom_os.h"
#include "simcom_common.h"

extern void PrintfResp(char* format);

/**
  * @brief  SSL Demo.
  * @param  void
  * @note
  * @retval void
  */
void SslDemoTest(void)
{
    INT32 ret = 0;
    SCSslCtx_t ctx;
    INT32 sockfd = -1;
    SChostent *host_entry = NULL;
    SCsockAddrIn server;
    INT8 recvbuf[1024];
    INT8 sendbuf[] = { /* Packet 8 */
    0x47, 0x45, 0x54, 0x20, 0x2f, 0x20, 0x48, 0x54,
    0x54, 0x50, 0x2f, 0x31, 0x2e, 0x31, 0x0d, 0x0a,
    0x48, 0x6f, 0x73, 0x74, 0x3a, 0x20, 0x77, 0x77,
    0x77, 0x2e, 0x62, 0x61, 0x69, 0x64, 0x75, 0x2e,
    0x63, 0x6f, 0x6d, 0x0d, 0x0a, 0x43, 0x61, 0x63,
    0x68, 0x65, 0x2d, 0x43, 0x6f, 0x6e, 0x74, 0x72,
    0x6f, 0x6c, 0x3a, 0x20, 0x6e, 0x6f, 0x2d, 0x63,
    0x61, 0x63, 0x68, 0x65, 0x0d, 0x0a, 0x43, 0x6f,
    0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, 0x54, 0x79,
    0x70, 0x65, 0x3a, 0x20, 0x74, 0x65, 0x78, 0x74,
    0x2f, 0x70, 0x6c, 0x61, 0x69, 0x6e, 0x0d, 0x0a,
    0x41, 0x63, 0x63, 0x65, 0x70, 0x74, 0x3a, 0x20,
    0x2a, 0x2f, 0x2a, 0x0d, 0x0a, 0x0d, 0x0a, 0x0d,
    0x0a };

    char path[1024 + 256] = {0};
    if (-1 == sAPI_TcpipPdpActive(1,1))
    {
        sAPI_Debug("PDP active err");
        snprintf(path,sizeof(path),"\r\nPDP active err\r\n");
        PrintfResp(path);
    }


    sockfd = sAPI_TcpipSocket(SC_AF_INET, SC_SOCK_STREAM, 0);

    sAPI_Debug("demo_ssl_test sockfd[%d]",sockfd);
    if(sockfd < 0)
    {
        sAPI_Debug("create socket err");
        snprintf(path,sizeof(path),"\r\ncreate socket err\r\n");
        PrintfResp(path);
        if (-1 == sAPI_TcpipPdpDeactive(1,1))
        {
            sAPI_Debug("PDP deactive err");
            snprintf(path,sizeof(path),"\r\nPDP deactive err\r\n");
            PrintfResp(path);
        }
    }

    host_entry = sAPI_TcpipGethostbyname((INT8 *)"www.baidu.com");
    if (host_entry == NULL)
    {
        sAPI_SslClose(0);
        sAPI_TcpipClose(sockfd);
        sAPI_Debug("DNS gethostbyname fail");
        snprintf(path,sizeof(path),"\r\nDNS gethostbyname fail\r\n");
        PrintfResp(path);
        if (-1 == sAPI_TcpipPdpDeactive(1,1))
        {
            sAPI_Debug("PDP deactive err");
            snprintf(path,sizeof(path),"\r\nPDP deactive err\r\n");
            PrintfResp(path);
        }
    }

    server.sin_family = SC_AF_INET;
    server.sin_port = sAPI_TcpipHtons(443);
    server.sin_addr.s_addr= *(UINT32 *)host_entry->h_addr_list[0];

    sAPI_Debug("start connect!!!");
    ret = sAPI_TcpipConnect(sockfd,(SCsockAddr *)&server,sizeof(SCsockAddr));
    if(ret != 0)
    {
        sAPI_SslClose(0);
        sAPI_TcpipClose(sockfd);
        sAPI_Debug("connect server fail");
        snprintf(path,sizeof(path),"\r\nconnect server fail\r\n");
        PrintfResp(path);
        if (-1 == sAPI_TcpipPdpDeactive(1,1))
        {
            sAPI_Debug("PDP deactive err");
            snprintf(path,sizeof(path),"\r\nPDP deactive err\r\n");
            PrintfResp(path);
        }
    }
    memset(&ctx,0,sizeof(ctx));
    ctx.fd = sockfd;
    ctx.ssl_version = SC_SSL_CFG_VERSION_ALL;


    ret = sAPI_SslHandShake(&ctx);
    sAPI_Debug("sAPI_SslHandShake ret[%d]",ret);

    if(ret == 0)
    {
        ret = sAPI_SslSend(0,sendbuf,sizeof(sendbuf));
        sAPI_Debug("ret [%d] sendbuf[%s]",ret,sendbuf);
        memset(recvbuf,0x0,1024);
        ret = sAPI_SslRead(0,recvbuf,1024);
        sAPI_Debug("ret [%d] recvbuf[%s]",ret,recvbuf);
        sAPI_Debug("sApi_SslConnect [%d]",ret);
        sAPI_SslClose(0);
        sAPI_TcpipClose(sockfd);
        snprintf(path,sizeof(path),"\r\nret [%d] recvbuf[%s]\r\n",(int)ret,recvbuf);
        PrintfResp(path);

        if (-1 == sAPI_TcpipPdpDeactive(1,1))
        {
            sAPI_Debug("PDP deactive err");
            snprintf(path,sizeof(path),"\r\nPDP deactive err\r\n");
            PrintfResp(path);
        }

    }
    else
    {
        sAPI_SslClose(0);
        sAPI_TcpipClose(sockfd);
        snprintf(path,sizeof(path),"\r\nhandshake fail,ret:%d\r\n",(int)ret);
        PrintfResp(path);
        if (-1 == sAPI_TcpipPdpDeactive(1,1))
        {
            sAPI_Debug("PDP deactive err");
            snprintf(path,sizeof(path),"\r\nPDP deactive err\r\n");
            PrintfResp(path);
        }
    }
}

/**
  * @brief  SSL Demo initial.
  * @param  void
  * @note
  * @retval void
  */

void SslTestDemoInit(int sslTestTime)
{
    char path[128] = {0};
    int i = 0;
    for(;i < sslTestTime; i++)
    {
        snprintf(path, sizeof(path), "\r\ncurrent time = %d, testTime = %d\r\n",i + 1,sslTestTime);
        PrintfResp(path);
        memset(path,0,sizeof(path));
        SslDemoTest();
    }
}


#endif
