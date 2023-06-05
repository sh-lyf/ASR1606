/**
  ******************************************************************************
  * @file    demo_poc.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of audio operation,this demo will show how to control the audio component including play\record\set audio configuration\set TX\RX gain ,etc.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */
 
/* Includes ------------------------------------------------------------------*/
#ifdef FEATURE_SIMCOM_POC
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "simcom_os.h"
#include "simcom_file.h"
#include "simcom_common.h"

#ifdef CUS_GWSD
#include "simcom_gwsd_poc.h"
#else
#include "simcom_poc.h"
#endif

extern SIM_MSG_T GetParamFromUart(void);
extern sMsgQRef simcomUI_msgq;
extern void PrintfOptionMenu(char* options_list[], int array_size);
extern void PrintfResp(char* format);

typedef enum{
    SC_POC_DEMO_PLAYPCM           = 1,
    SC_POC_DEMO_STOPPCM           = 2,
    SC_POC_DEMO_RECORDSTART       = 3,
    SC_POC_DEMO_RECORDSTOP        = 4,

    SC_POC_DEMO_MAX               = 99
}SC_AUDIO_DEMO_TYPE;

#define MAX_VALID_USER_NAME_LENGTH  255-7

/**
  * @brief  Create audio demo.
  * @param  void
  * @note   This demo will show how to control audio component with SIMCom OpenSDK APIs.
  * @retval void
  */
void POCDemo(void)
{
    SIM_MSG_T optionMsg ={0,0,0,NULL};
    int opt = 0;
    //static char *buffer = NULL;
    BOOL ret = 0;

    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *optionsList[] = {
        "1. POC Play PCM",
        "2. POC Stop PCM",
        "3. POC Start record",
        "4. POC Stop record",

        "99. back",
    };

    //buffer = sAPI_Malloc(100*1024);

    while(1)
    {
        PrintfResp(note);
        PrintfOptionMenu(optionsList,sizeof(optionsList)/sizeof(optionsList[0]));
        sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
        if(SRV_UART != optionMsg.msg_id)
        {
            printf("%s,msg_id is error!!",__func__);
            break;
        }

        printf("arg3 = [%s]",(char *)optionMsg.arg3);
        opt = atoi(optionMsg.arg3);
        sAPI_Free(optionMsg.arg3);

        switch(opt)
        {
            case SC_POC_DEMO_PLAYPCM:
            {
                SCFILE* file_hdl = {0};
                char file_name[MAX_VALID_USER_NAME_LENGTH] = {0};
                char buff[320];
                int i = 0;
                int actul_read_len = 1;
                int ret = -1;
                memcpy(file_name, "c:/test.wav", strlen("c:/test.wav"));

                file_hdl = sAPI_fopen(file_name, "rb");
                printf("sAPI_FsFileOpen file_hdl = %p",file_hdl->disk);
                actul_read_len = sAPI_fread(buff, 1,44, file_hdl);
                printf("sAPI_FsFileRead actul_read_len = %d",actul_read_len);
                while(actul_read_len > 0)
                {
                    ret = -1;
                    actul_read_len = sAPI_fread(buff, 1,320, file_hdl);
                    if(actul_read_len == 320)
                    {
                        ret = sAPI_PocPlaySound(buff,320);
                        i++;
                    }
                    printf("sAPI_PocPlaySound ret = %d actul_read_len = %d!,i = %d", ret,actul_read_len,i);
                    if(i % 10 == 0)
                        sAPI_TaskSleep(20);
                }
                sAPI_fclose(file_hdl);
                ret = sAPI_PocStopSound();
                printf("sAPI_PocStopSound ret = %d", ret);
                break;
            }
            case SC_POC_DEMO_STOPPCM:
            {
                ret = sAPI_PocStopSound();
                printf("sAPI_PocStopSound: %d !", ret);
                break;
            }
            case SC_POC_DEMO_RECORDSTART:
            {
                int i = 0;
                int ret = sAPI_PocStartRecord(NULL,2);
                printf("sAPI_PocStartRecord ret = %d !", ret);
                while(i<200)
                {
                    char bufferPCm[640];
                    sAPI_TaskSleep(5);
                    ret = sAPI_PocPcmRead(bufferPCm,320);
                    printf("sAPI_PocPcmRead bufferPCm length = %d ret %d!", sizeof(bufferPCm),ret);
                    i++;
                }
                break;
            }
            case SC_POC_DEMO_RECORDSTOP:
            {
                int ret = sAPI_PocStopRecord();
                printf("sAPI_PocStopRecord ret = %d !", ret);
                break;
            }
            case SC_POC_DEMO_MAX:
            {
                return;
            }
        }
    }
}
#endif
