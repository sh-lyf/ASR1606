/**
  ******************************************************************************
  * @file    demo_ntp.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of ntp demo operation.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#ifdef FEATURE_SIMCOM_NTP
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "simcom_os.h"
#include "simcom_ntp_client.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "simcom_uart.h"


typedef enum{
    SC_NTP_DEMO_UPDATE         = 1,
    SC_NTP_DEMO_MAX          = 99
}SC_NTP_DEMO_TYPE;

extern sMsgQRef simcomUI_msgq;
extern void PrintfOptionMenu(char* options_list[], int array_size);
extern void PrintfResp(char* format);
sMsgQRef ntpUIResp_msgq;

/**
  * @brief  NTP Demo operation
  * @param  void
  * @note   Please set NTP server accordingly.
  * @retval void
  */
void NtpDemo(void)
{
    UINT32 ret = 0;
    SCsysTime_t currUtcTime;
    char buff[220]={0};

    SIM_MSG_T optionMsg ={0,0,0,NULL};
    UINT32 opt = 0;
    char *resp = NULL;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] = {
        "1. Update",
        "99. Back",
    };

    while(1)
    {
        SIM_MSG_T ntp_result = {SC_SRV_NONE, -1, 0, NULL};
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
        sAPI_Free(optionMsg.arg3);

        switch(opt)
        {
            case SC_NTP_DEMO_UPDATE:
            {
                if(NULL == ntpUIResp_msgq)
                {
                    SC_STATUS status;
                    status = sAPI_MsgQCreate(&ntpUIResp_msgq, "htpUIResp_msgq", sizeof(SIM_MSG_T), 4, SC_FIFO);
                    if(SC_SUCCESS != status)
                    {
                        sAPI_Debug("[CNTP]msgQ create fail");
                        resp = "\r\nNTP Update Fail!\r\n";
                        sAPI_UartWrite(SC_UART,(UINT8*)resp,strlen(resp));
                        break;
                    }
                }

                memset(&currUtcTime,0,sizeof(currUtcTime));

                sAPI_GetSysLocalTime(&currUtcTime);
                sAPI_Debug("[CNTP] sAPI_GetSysLocalTime %d - %d - %d - %d : %d : %d   %d",currUtcTime.tm_year,currUtcTime.tm_mon,currUtcTime.tm_mday,
                  currUtcTime.tm_hour,currUtcTime.tm_min,currUtcTime.tm_sec,currUtcTime.tm_wday);
                ret = sAPI_NtpUpdate(SC_NTP_OP_SET, "ntp3.aliyun.com", 32, NULL);                        //Unavailable addr may cause long time suspend
                sAPI_Debug("[CNTP] func[%s] line[%d] ret[%d]", __FUNCTION__,__LINE__,ret);

                ret = sAPI_NtpUpdate(SC_NTP_OP_GET, buff, 0, NULL);
                sAPI_Debug("[CNTP] func[%s] line[%d] ret[%d] buff[%s]", __FUNCTION__,__LINE__,ret, buff);
            
                ret = sAPI_NtpUpdate(SC_NTP_OP_EXC, NULL, 0, ntpUIResp_msgq);
                sAPI_Debug("[CNTP] func[%s] line[%d] ret[%d] ", __FUNCTION__,__LINE__,ret );
                do
                {
                    sAPI_MsgQRecv(ntpUIResp_msgq, &ntp_result, SC_SUSPEND);

                    if(SC_SRV_NTP != ntp_result.msg_id )                  //wrong msg received 
                    {
                        sAPI_Debug("[CNTP] ntp_result.msg_id =[%d], ntp_result.msg_id ");
                        ntp_result.msg_id = SC_SRV_NONE;                   //para reset
                        ntp_result.arg1 = -1;
                        ntp_result.arg3 = NULL;
                        continue;
                    }
                    if(SC_NTP_OK == ntp_result.arg1)                        //it means update succeed
                    {
                        sAPI_Debug("[CNTP] successfully update time! ");
                        PrintfResp("\r\nNTP Update Time Successful!\r\n");
                        break;
                    }
                    else
                    {
                        sAPI_Debug("[CNTP] failed to update time! result code: %d", ntp_result.arg1);
                        PrintfResp("\r\nNTP Update Time Failed!\r\n");
                        break;
                    }
                }while(1);

                memset(&currUtcTime,0,sizeof(currUtcTime));
                sAPI_GetSysLocalTime(&currUtcTime);
                sAPI_Debug("[CNTP] sAPI_GetSysLocalTime %d - %d - %d - %d : %d : %d   %d",currUtcTime.tm_year,currUtcTime.tm_mon,currUtcTime.tm_mday,
                    currUtcTime.tm_hour,currUtcTime.tm_min,currUtcTime.tm_sec,currUtcTime.tm_wday);

                break;
            }

            case SC_NTP_DEMO_MAX:
            {
                sAPI_Debug("Return to the previous menu!");
                PrintfResp("\r\nReturn to the previous menu!\r\n");
                memset(&ntp_result,0,sizeof(ntp_result));

                return;
            }

            default :
                break;
        }
    }
}
#endif

