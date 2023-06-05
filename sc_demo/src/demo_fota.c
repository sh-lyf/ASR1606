/**
  ******************************************************************************
  * @file    demo_fota.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of fota function,  this is used to update core FW.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "simcom_fota_download.h"
#include "simcom_uart.h"
#include "simcom_common.h"
#include "simcom_file.h"

#define SYS_PATCH_BIN1   "sys_patch.bin_1"
#define SYS_PATCH_BIN2   "sys_patch.bin_2"

#define LOG_ERR()         do{printf("%s error, line: %d\r\n", __func__, __LINE__);}while (0)


extern sMsgQRef simcomUI_msgq;
extern SIM_MSG_T GetParamFromUart(void);
extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(char *format);


enum SC_FOTA_SWITCH {
    SC_FTP_FOTA = 1,
    //SC_FTP_FOTA_2,
    SC_HTTP_FOTA,
    //SC_HTTP_FOTA_2,
    SC_LOCAL_FOTA_1,
    SC_LOCAL_FOTA_2,
    SC_FOTA_BACK = 99
};

/**
  * @brief  Fota operation callback to indicated the downloading finishing percentage.
  * @param  void
  * @note   isok will be used to indicate the download result.
  * @retval void
  */
int fotacb(int isok)
{
    SC_MiniSysStatus MiniSysStatus ={0,0};
    sAPI_GetMiniSysStatus(&MiniSysStatus);
    if (isok == 100)
    {
         if(MiniSysStatus.stage == 1)//first stage
         {
             PrintfResp("fist download is successful.\r\n");
         }
         else if(MiniSysStatus.stage == 2)//second stage
         {
             PrintfResp("second download is successful.\r\n");
         }
         else
         {
             PrintfResp("pre download is successful.\r\n");
         }
     }
     else
     {
         if(MiniSysStatus.stage == 1)//first stage
         {
             PrintfResp("fist download is fail.\r\n");
         }
         else if(MiniSysStatus.stage == 2)//second stage
         {
             PrintfResp("second download is fail.\r\n");
         }
         else
         {
             PrintfResp("pre download is fail.\r\n");
         }
     }
     return 0;
}

/**
  * @brief  Fota demo
  * @param  void
  * @note   suppor HTTP and FTP\local update. For local update need sAPI_FsSwitchDir to change director
  * @retval void
  */
void FotaDemo(void)
{
    SIM_MSG_T optionMsg = {0, 0, 0, NULL};
    UINT32 opt = 0;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] = {
        "1. FTP fota",
        //"2. FTP fota SEC",
        "2. HTTP fota",
        //"4. HTTP fota SEC",
        "5. LOCAL fota FIR",
        "6. LOCAL fota SEC",
        //"7. Enter mini sys",
        "8. Check Current mini sys Status",
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
        sAPI_Free(optionMsg.arg3);

        switch (opt) {

        case SC_FTP_FOTA:
        {
            struct SC_FotaApiParam param = {0};
            strcpy(param.host, "183.230.174.137:6121/zyk_fota/system_patch.bin");
            strcpy(param.username, "yangkui.zhang");
            strcpy(param.password, "simcom");
            param.mode = 0;//ftp
            param.sc_fota_cb = fotacb;

            sAPI_PreMiniCheck((void *)&param);
            break;
        }

        case SC_HTTP_FOTA:
        {
            struct SC_FotaApiParam param = {0};
            strcpy(param.host, "47.108.134.22/zyk_fota/system_patch.bin");
            strcpy(param.username, "root");
            strcpy(param.password, "Simcom1234!");
            param.mode = 1;//http
            param.sc_fota_cb = fotacb;

            sAPI_PreMiniCheck((void *)&param);
            break;
        }

        case SC_LOCAL_FOTA_1: {
            int ret = -1;
            FILE *fp = NULL;
            size_t read_len = 0;
            int filesize = 0;

            void *read_data = (void *)malloc(1024);
            if (!read_data) {
                LOG_ERR();
                goto error;
            }

            ret = sAPI_FsSwitchDir("sys_patch.bin_1", DIR_C_TO_SIMDIR);
            if (ret) {
                LOG_ERR();
                goto error;
            }

            fp = fopen("c:/sys_patch.bin_1", "rb");
            if (fp == NULL) {
                LOG_ERR();
                goto error;
            }
            filesize = sAPI_fsize((SCFILE *)fp);
            printf("file size : [%d]", filesize);

            while (1) {
                memset(read_data, 0, 1024);
                read_len = fread(read_data, 1024, 1, fp);
                if (read_len == 0)
                    break;
                if (sAPI_FotaImageWrite(read_data, read_len, filesize) != 0) {
                    LOG_ERR();
                    goto error;
                }
            }

            if (sAPI_FotaImageVerify(filesize) != 0) {
                LOG_ERR();
                goto error;
            }
            printf("first fota image write suc, verify suc");
error:
            if (fp)
                fclose(fp);
            if (read_data)
                free(read_data);

            break;
        }

        case SC_LOCAL_FOTA_2: {
            int ret = -1;
            FILE *fp = NULL;
            size_t read_len = 0;
            int filesize = 0;

            void *read_data = (void *)malloc(1024);
            if (!read_data) {
                LOG_ERR();
                goto sec_error;
            }

            ret = sAPI_FsSwitchDir("sys_patch.bin_2", DIR_C_TO_SIMDIR);
            if (ret) {
                LOG_ERR();
                goto sec_error;
            }

            fp = fopen("c:/sys_patch.bin_2", "rb");
            if (fp == NULL) {
                LOG_ERR();
                goto sec_error;
            }
            filesize = sAPI_fsize((SCFILE *)fp);
            printf("file size : [%d]", filesize);

            while (1) {
                memset(read_data, 0, 1024);
                read_len = fread(read_data, 1024, 1, fp);
                if (read_len == 0)
                    break;
                if (sAPI_FotaImageWrite(read_data, read_len, filesize) != 0) {
                    LOG_ERR();
                    goto sec_error;
                }
            }

            if (sAPI_FotaImageVerify(filesize) != 0) {
                LOG_ERR();
                goto sec_error;
            }
            printf("second fota image write suc, verify suc --> reset update!");
sec_error:
            if (fp)
                fclose(fp);
            if (read_data)
                free(read_data);

            break;
        }


        case 8:
        {
            char tmp[60];
            SC_MiniSysStatus MiniSysStatus ={0,0};
            sAPI_GetMiniSysStatus(&MiniSysStatus);
            sprintf(tmp, "\r\nCurrent minisys enable.enable %d stage %d\r\n", MiniSysStatus.enable, MiniSysStatus.stage);

            PrintfResp(tmp);
            break;
        }

        case SC_FOTA_BACK:
            return;
        }
    }
}
