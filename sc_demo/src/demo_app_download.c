/**
  ******************************************************************************
  * @file    demo_app_download.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of app download demo,this demo will show how to download and update app for OpenSDK solution with HTTP\HTTPS\FTP\FTPS server remotely.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sc_app_download.h"
#include "simcom_os.h"
#include "simcom_common.h"
#include "sc_app_update.h"
#include "sc_power.h"
#include "simcom_uart.h"

extern SIM_MSG_T GetParamFromUart(void);
extern sMsgQRef simcomUI_msgq;
extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(char *format);

#define APP_OTA_URL "http://47.108.134.22/lugc_fota/customer_app.bin" //support http/https, the url is filled in by the customer
//#define APP_OTA_URL "ftp://guocheng.lu:123456@183.230.174.137:6081/fota_pkg/customer_app.bin" //support http/https, the url is filled in by the customer

//"ftp://username:password@ip_address:port/customer_app.bin

/**
  * @brief  Demo for app download with HTTP\FTP, need to upload new customer_app.bin file to server in advance.
  * @param  void
  * @note   Need to select FTP or HTTP by inputing in serial terminal.
  * @retval void
  */
void AppDownloadDemo(void)
{
    SIM_MSG_T optionMsg = {0, 0, 0, NULL};
    UINT32 opt = 0;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] = {
        "1. HTTP/HTTPS APP DOWNLOAD",
        "2. FTP/FTPS APP DOWNLOAD",
        "99. back",
    };


    while (1) {
        PrintfResp(note);
        PrintfOptionMenu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        sAPI_MsgQRecv(simcomUI_msgq, &optionMsg, SC_SUSPEND);
        if (SRV_UART != optionMsg.msg_id) {
            printf("%s,msg_id is error!!", __func__);
            break;
        }

        printf("arg3 = [%s]", (char *)optionMsg.arg3);
        opt = atoi(optionMsg.arg3);
        free(optionMsg.arg3);

        switch (opt) {
        case 1:
        case 2: {
            SCAppDownloadPram pram;
            SCAppDwonLoadReturnCode ret = SC_APP_DOWNLOAD_SUCESSED;
            SCAppPackageInfo gAppUpdateInfo = {0};

            pram.url = APP_OTA_URL;
            if (strncmp(pram.url, "https://", strlen("https://")) == 0 || strncmp(pram.url, "http://", strlen("http://")) == 0) {
                pram.mod = SC_APP_DOWNLOAD_HTTP_MOD;
            }
            if (strncmp(pram.url, "ftp://", strlen("ftp://")) == 0 || strncmp(pram.url, "ftps://", strlen("ftps://")) == 0) {
                pram.mod = SC_APP_DOWNLOAD_FTP_MOD;
            }
            pram.recvtimeout = 20000;

            ret = sAPI_AppDownload(&pram);
            if ( ret == SC_APP_DOWNLOAD_SUCESSED )
                PrintfResp("app download success.\r\n");
            else
                PrintfResp("app download fail.\r\n");
            printf("sAPI_AppDownload download customer_app.bin ret[%d] ... ", ret);

            ret = sAPI_AppPackageCrc(&gAppUpdateInfo);
            if (SC_APP_DOWNLOAD_SUCESSED == ret) {
                printf("app crc success --> reset!");
                PrintfResp("app crc success.\r\n");
                sAPI_TaskSleep(20);
                sAPI_SysReset();
            } else {
                printf("app package crc fail, errcode = [%d]", ret);
                PrintfResp("app crc fail.\r\n");

            }
            break;
        }

        case 88:
            PrintfResp("app download test!\r\n");
            break;

        case 99:

            return;

        }

    }
}




#if 0
#include "simcom_api.h"

#define APP_OTA_TASK_SIZE (1024 * 10)
#define APP_OTA_URL "https://test.com/customer_app.bin" //support http/https, the url is filled in by the customer

static sTaskRef app_download_task_ref;

static void sAPP_AppDownloadProcesser(void *arg)
{
    SCAppDownloadPram pram;
    SCAppDwonLoadReturnCode ret;
    SCAppPackageInfo gAppUpdateInfo = {0};
    printf("sAPP_FotaProcesser start");
    int pGreg = 0;

    while (1) {
        sAPI_NetworkGetCgreg(&pGreg);
        if (1 != pGreg) {
            printf("fota NETWORK STATUS IS [%d]", pGreg);
            sAPI_TaskSleep(5 * 200);
        } else {
            printf("fota NETWORK STATUS IS NORMAL");
            break;
        }
    }
    pram.url = APP_OTA_URL;
    if (strncmp(pram.url, "https://", strlen("https://")) == 0 || strncmp(pram.url, "http://", strlen("http://")) == 0) {
        pram.mod = SC_APP_DOWNLOAD_HTTP_MOD;
    }
    if (strncmp(pram.url, "ftp://", strlen("ftp://")) == 0 || strncmp(pram.url, "ftps://", strlen("ftps://")) == 0) {
        pram.mod = SC_APP_DOWNLOAD_FTP_MOD;
    }
    pram.recvtimeout = 5000;

    ret = sAPI_AppDownload(&pram);

    printf("sc_app_download_test download 2.bin ret[%d] ... ", ret);


    ret = sAPI_AppPackageCrc(&gAppUpdateInfo);
    if (SC_APP_DOWNLOAD_SUCESSED == ret) {
        printf("the next is reset");
        sAPI_SysReset();
    } else
        printf("app package crc fail, errcode = [%d]", ret);

}

void sAPP_AppDownloadTaskDemo(void)
{

    SC_STATUS status;

    status = sAPI_TaskCreate(&app_download_task_ref, NULL, APP_OTA_TASK_SIZE, 70, (char *)"fota_demo_thread", sAPP_AppDownloadProcesser, (void *)0);
    if (status != SC_SUCCESS)
        printf("create app_download task fail status[%d]", status);
}
#endif