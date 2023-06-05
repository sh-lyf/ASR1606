/**
  ******************************************************************************
  * @file    demo_htp.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of htp operation, HTP can be used to update time over HTTP.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */
 
/* Includes ------------------------------------------------------------------*/
#ifdef FEATURE_SIMCOM_HTP
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "simcom_os.h"
#include "simcom_htp_client.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "simcom_uart.h"


typedef enum{
    SC_HTP_DEMO_SRVCONFIG         = 1,
    SC_HTP_DEMO_UPDATE        = 2,
    SC_HTP_DEMO_MAX          = 99
}SC_NTP_DEMO_TYPE;


extern sMsgQRef simcomUI_msgq;
extern void PrintfOptionMenu(char* options_list[], int array_size);
extern void PrintfResp(char* format);
sMsgQRef htpUIResp_msgq;

/**
  * @brief  HTP demo
  * @param  void
  * @note   Need to configure HTTP server before start, here with www.baidu.com as example
  * @retval void
  */
void HtpDemo(void)
{
    UINT32 ret = 0;
    char buff[220]={0};
    UINT32 true = 0;

    SIM_MSG_T optionMsg ={0,0,0,NULL};
    UINT32 opt = 0;
    char *resp = NULL;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] = {
        "1. Config server ",
        "2. Update",
        "99. Back",
    };

    while(1)
    {
        SIM_MSG_T htp_result = {SC_SRV_NONE, -1, 0, NULL};
        PrintfResp(note);
        PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
        sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
        if(SRV_UART != optionMsg.msg_id)
        {
            sAPI_Debug("[HTP] %s,msg_id is error!!",__func__);
            break;
        }

        sAPI_Debug("[HTP] arg3 = [%s]",optionMsg.arg3);
        opt = atoi(optionMsg.arg3);
        sAPI_Free(optionMsg.arg3);

        switch(opt)
        {
            case SC_HTP_DEMO_SRVCONFIG:
            {
                sAPI_Debug("[HTP] Htp server config!");

                if(NULL == htpUIResp_msgq)
                {
                    SC_STATUS status;
                    status = sAPI_MsgQCreate(&htpUIResp_msgq, "htpUIResp_msgq", sizeof(SIM_MSG_T), 4, SC_FIFO);
                    if(SC_SUCCESS != status)
                    {
                        sAPI_Debug("[HTP] msgQ create fail");
                        resp = "\r\nHTP Fail!\r\n";
                        sAPI_UartWrite(SC_UART,(UINT8*)resp,strlen(resp));
                        break;
                    }
                }

                ret = sAPI_HtpSrvConfig(SC_HTP_OP_SET, NULL, "ADD", "www.baidu.com", 80, 1, NULL, 0);       //Unavailable addr may cause long time suspend,such as google
                    sAPI_Debug("[HTP]  func[%s] line[%d] ret[%d]", __FUNCTION__,__LINE__,ret);
                    
                ret = sAPI_HtpSrvConfig(SC_HTP_OP_SET, NULL, "ADD", "www.52im.net", 80, 1, NULL, 0);
                    sAPI_Debug("[HTP]  func[%s] line[%d] ret[%d]", __FUNCTION__,__LINE__,ret);
                    
                ret = sAPI_HtpSrvConfig(SC_HTP_OP_GET, buff, NULL, NULL, 0, 0, NULL, 0);
                    sAPI_Debug("[HTP]  func[%s] line[%d] return_string[%s], ret[%d]", __FUNCTION__,__LINE__,buff,ret);

                if(SC_HTP_OK == ret)
                {
                    true = 1;
                    sAPI_Debug("[HTP]  CONFIG SERVER SUCCESSFUL");
                    PrintfResp("\r\nHTP Config Server Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("[HTP]  CONFIG SERVER ERROR,ERROR CODE = [%d]",htp_result.arg2);
                    PrintfResp("\r\nHTP Config Server Fail!\r\n");
                    break;
                }
            }

            case SC_HTP_DEMO_UPDATE:
            {
                if(true) /*Config server successful*/
                {
                    sAPI_Debug("[HTP]  update true = %d!",true);

                    ret = sAPI_HtpUpdate(htpUIResp_msgq);    
                    sAPI_Debug("[HTP]  func[%s] line[%d] ret[%d]", __FUNCTION__,__LINE__,ret);
                    do
                    {
                        sAPI_MsgQRecv(htpUIResp_msgq, &htp_result, SC_SUSPEND); 
                
                        if(SC_SRV_HTP != htp_result.msg_id )                  //wrong msg received 
                        {
                            sAPI_Debug("[HTP] htp_result.msg_id = [%d]",htp_result.msg_id);
                            htp_result.msg_id = SC_SRV_NONE;                   //para reset
                            htp_result.arg1 = -1;                           //the result code
                            htp_result.arg3 = NULL;                         //parameter - arg3 should be NULL for msg recv
                            continue;
                        }
                        if(SC_HTP_OK == htp_result.arg1)                        //it means update succeed
                        {
                            sAPI_Debug("[HTP] successfully update time! ");
                            PrintfResp("\r\nHTP Update Time Successful!\r\n");
                        }
                        else
                        {
                            sAPI_Debug("[HTP] failed to update time! result code = %d",  htp_result.arg1);
                            PrintfResp("\r\nHTP Update Time Failed!\r\n");
                        }
                        break;
                    }while(1);
                }
                else
                {
                    sAPI_Debug("[HTP] failed to update time! result code = %d",  htp_result.arg1);
                    PrintfResp("\r\nHTP Update Time Failed! Please config server first!\r\n");
                }
                break;
            }

            case SC_HTP_DEMO_MAX:
            {
                sAPI_Debug("[HTP] Return to the previous menu!");
                PrintfResp("\r\nReturn to the previous menu!\r\n");
                memset(&htp_result,0,sizeof(htp_result));

                return;
            }
            default :
                break;
        }
    }
}

#endif