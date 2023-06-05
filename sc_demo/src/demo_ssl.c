/**
  ******************************************************************************
  * @file    demo_ssl.c
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
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "simcom_ssl.h"
#include "simcom_tcpip.h"
#include "simcom_tcpip_old.h"
#include "simcom_debug.h"
#include "simcom_os.h"
#include "simcom_common.h"

extern sMsgQRef simcomUI_msgq;
extern void PrintfOptionMenu(char* options_list[], int array_size);
extern void PrintfResp(char* format);
extern void SslTestDemoInit(int sslTestTime);

/**
  * @brief  SSL Demo.
  * @param  void
  * @note
  * @retval void
  */
void SslDemo(void)
{
    INT32 ret = 0;
    SCSslCtx_t ctx;
    INT32 sockfd = -1;
    SChostent *host_entry = NULL;
    SCsockAddrIn server;
    UINT32 opt = 0;
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

    SIM_MSG_T optionMsg ={0,0,0,NULL};
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] = {
    "1. SSL CONNECT DEMO",
    "2. TEST FOR SSL",
    "3. ",
    "99. back",
     };
     /*{
    "1. test for ssl",
    "99. back",
     }; */

    while(1)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));

        sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
        if(SRV_UART != optionMsg.msg_id)
        {
            sAPI_Debug("%s,msg_id is error!!",__func__);
            break;
        }

        sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
        opt = atoi(optionMsg.arg3);
        free(optionMsg.arg3);
        switch(opt)
        {
            case SC_SSL_COMMUNICATE:
                {
                    char path[1024 + 256] = {0};
                    if (-1 == sAPI_TcpipPdpActive(1,1))
                    {
                        sAPI_Debug("PDP active err");
                        snprintf(path,sizeof(path),"\r\nPDP active err\r\n");
                        PrintfResp(path);
                        break;
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
                        break;
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
                        break;

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
                        break;
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
                        snprintf(path,sizeof(path),"\r\nret [%d] recvbuf[%s]\r\n",(int)ret,(char *)recvbuf);
                        PrintfResp(path);

                        if (-1 == sAPI_TcpipPdpDeactive(1,1))
                        {
                            sAPI_Debug("PDP deactive err");
                            snprintf(path,sizeof(path),"\r\nPDP deactive err\r\n");
                            PrintfResp(path);
                        }
                        break;

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
                        break;
                    }
                }

            case SC_SSL_TEST:
                {
                    sAPI_Debug("Start SSL demo test thread!");
                    int sslTestTime = 0;
                    PrintfResp("\r\nPlease input test time.\r\n");
                    //optionMsg = GetParamFromUart();
                    SIM_MSG_T option_Msg ={0,0,0,NULL};
                    sAPI_MsgQRecv(simcomUI_msgq,&option_Msg,SC_SUSPEND);
                    sAPI_Debug("arg3 = [%s]",option_Msg.arg3);
                    sslTestTime = atoi(option_Msg.arg3);
                    SslTestDemoInit(sslTestTime);
                    break;
                }

            case SC_SSL_DEMO_MAX:
                {
                    return;
                }

            default :
                 break;
        }

    }
}






















#if 0

#include "simcom_api.h"


#define FALSE 0
#define TRUE 1
sTaskRef sslProcesser;
static UINT32 sslProcesserStack[3072] = {0xA5A5A5A5};

void sTask_SslProcesser(void * arg)
{
#if 0
    sAPI_TaskSleep(200*50);

    int result = 0;
    SCsslContent content = {0};
    result = sAPI_SslSetContextIdMsg("sslversion",0,"1");
    sAPI_Debug("result = [%d]",result);

    result = sAPI_SslSetContextIdMsg("cacert",0,"test.pem");
    sAPI_Debug("result = [%d]",result);

    content = sAPI_SslGetContextIdMsg(0);
    sAPI_Debug("version = [%d]",content.ssl_version);
#else
        sAPI_TaskSleep(200*6);//20s

        sAPI_Debug("ssl Test START");
        demo_ssl_test();
        sAPI_Debug("ssl Test END");

#endif
}



void sAPP_SslTaskDemo()
{

    BOOL is_inited = FALSE;
    SC_STATUS status;
    sAPI_Debug("is_inited = [%d]",is_inited);

    if(TRUE == is_inited)
    return;

    status = sAPI_TaskCreate(&sslProcesser,sslProcesserStack,8000,152,"scssl",sTask_SslProcesser,(void *)0);
    if(SC_SUCCESS != status)
    {
        is_inited = FALSE;
        sAPI_Debug("task create fail");
    }

    if(SC_SUCCESS == status)
    is_inited = TRUE;


}


void demo_ssl_test(void)
{
    INT32 ret = 0;
    SCSslCtx_t ctx;
    INT32 sockfd = -1;
    SChostent *host_entry = NULL;
    SCsockAddrIn sa;
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

    UINT8 pGreg = 0;
    while(1)
    {
        sAPI_NetworkGetCgreg(&pGreg);
        if(1 != pGreg)
        {
            sAPI_Debug("SSL NETWORK STATUS IS [%d]",pGreg);
            sAPI_TaskSleep(10*300);
        }
        else
        {
            sAPI_Debug("SSL NETWORK STATUS IS NORMAL");
            break;
        }
    }

     if (-1 == sAPI_TcpipPdpActive(1,1))
    {
      sAPI_Debug("PDP active err");
      return;
    }


    sockfd = sAPI_TcpipSocket(SC_AF_INET, SC_SOCK_STREAM, 0);

    sAPI_Debug("demo_ssl_test sockfd[%d]",sockfd);
    if(sockfd < 0)
    {
        sAPI_Debug("create socket err");
        return;
    }

    host_entry = sAPI_TcpipGethostbyname("www.baidu.com");
    if (host_entry == NULL)
    {
        sAPI_SslClose(0);
        sAPI_TcpipClose(sockfd);
        sAPI_Debug("DNS gethostbyname fail");
        return;

    }

    server.sin_family = SC_AF_INET;
    server.sin_port = sAPI_TcpipHtons(443);
    server.sin_addr.s_addr= *(UINT32 *)host_entry->h_addr_list[0];

    sAPI_Debug("start connect!!!");
    ret = sAPI_TcpipConnect(sockfd,&server,sizeof(SCsockAddr));
    if(ret != 0)
    {
        sAPI_SslClose(0);
        sAPI_TcpipClose(sockfd);
        sAPI_Debug("connect server fail errno");
        return;
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

    }
    sAPI_Debug("sApi_SslConnect [%d]",ret);

    sAPI_SslClose(0);
    sAPI_TcpipClose(sockfd);

    if (-1 == sAPI_TcpipPdpDeactive(1,1))
    {
         sAPI_Debug("PDP deactive err");
         return;
    }

}


#endif

#endif
