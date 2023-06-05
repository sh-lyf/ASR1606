/**
  ******************************************************************************
  * @file    cus_urc.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of urc(Unsolicited Result Codes)task,it will process the receiving event when there is following ones:
			 1.Phone Book
			 2.SMS
			 3.SIM1\SIM2
			 4.Network state notification
			 5.AT cmd response and URC
			 6.WIFI scan response from core
			 7.GNSS NMEA data from core
			 8.Voice call
			 9.Audio
			 10.TTS
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "simcom_os.h"
#include "simcom_common.h"
#include "stdlib.h"
#include "string.h"
#include "simcom_debug.h"
#include "simcom_uart.h"


#define URC_PROCESS_TASK_STACK_SIZE (1024 * 3)
#define URC_PROCESS_TASK_PRIORITY (150)

static sMsgQRef gUrcMsgQueue = NULL;
static sTaskRef gUrcProcessTask = NULL;
static UINT8 gUrcProcessTaskStack[URC_PROCESS_TASK_STACK_SIZE] = {0xA5};
#if (defined SIMCOM_A7680C_V5_01) || (defined SIMCOM_A7670C_V7_01)
#define SC_UART    SC_UART4
#else
#define SC_UART    SC_UART
#endif
//extern sMsgQRef g_sms_demo_urc_process_msgQ;

/**
  * @brief  URC task processing
  * @param  pointer *ptr
  * @note   There will be additional urc events when there is new request from market.
  * @retval void
  */
void sTask_UrcProcesser(void *ptr)
{
    INT32 ret = 0;
    SC_STATUS osStatus = SC_FAIL;
    SIM_MSG_T msg = {0};
    //char tempStr[10];
    INT8 *Pbbuf = NULL;
    INT8 *Netbuf = NULL;
    //INT8 *SMSbuf = NULL;
#if defined(FEATURE_SIMCOM_GPS) || defined(JACANA_GPS_SUPPORT)
    INT8 *GPSbuf = NULL;
    INT8 *GNSSbuf = NULL;
    INT8 *NMEADATAbuf = NULL;
#endif
    UINT8 *AUDIObuf = NULL;
    osStatus = sAPI_MsgQCreate(&gUrcMsgQueue, "URC QUEUE", sizeof(SIM_MSG_T), 20, SC_FIFO);
    if (SC_SUCCESS != osStatus)
    {
        return;
    }

    ret = sAPI_UrcRefRegister(gUrcMsgQueue, SC_MODULE_ALL);
    if (0 != ret)
    {
        sAPI_MsgQDelete(gUrcMsgQueue);
        return;
    }

    while(1)
    {
        if (sAPI_MsgQRecv(gUrcMsgQueue, &msg, SC_SUSPEND) != SC_SUCCESS)
        {
            continue;
        }

        if (SRV_URC != msg.msg_id)
        {
            free(msg.arg3);
            continue;
        }

        sAPI_Debug("module:%d, urc:%d", msg.arg1, msg.arg2);
        switch(msg.arg1)
        {
            case SC_URC_PB_MASK:
                sAPI_Debug("URC_PB_MASK process!!");
                if(msg.arg2 == SC_URC_PBDOWN)
                {
                    Pbbuf = (INT8 *)msg.arg3;
                    sAPI_Debug("URC_PB_MASK:%s",Pbbuf);
                }
            break;

            case SC_URC_SMS_MASK:
            {
            #if 0
                sAPI_Debug("URC_SMS_MASK process! sim_urc_code=[%d]", msg.arg2);
                sAPI_Debug("msg.arg3[%p]",msg.arg3);
                if(msg.arg2 == SC_URC_FLASH_MSG)
                {
                    /*flash msg recved, +CMT*/
                    INT8 *p_msgpload = (INT8*)msg.arg3;
                    SIM_MSG_T SmsUrcProcessReq = {0};
                    if(p_msgpload != NULL)
                    {
                        sAPI_Debug("SC_URC_FLASH_MSG, new msg[%s]",p_msgpload);
                    }

                    /*send msg to process task and read this msg*/
                    SmsUrcProcessReq.msg_id = SC_URC_FLASH_MSG; //msgid
                    SmsUrcProcessReq.arg3 = malloc(strlen(msg.arg3)+1);//msg.arg3 will free after this process
                    memset(SmsUrcProcessReq.arg3, 0, (strlen(msg.arg3)+1));
                    memcpy(SmsUrcProcessReq.arg3, (INT8*)msg.arg3,strlen(msg.arg3)); //msg string
                    /*Trigger urc process thread*/
                    if(NULL != g_sms_demo_urc_process_msgQ){
                        osStatus = sAPI_MsgQSend(g_sms_demo_urc_process_msgQ, &SmsUrcProcessReq);
                    }
                    if(osStatus != SC_SUCCESS)
                    {
                        sAPI_Debug("ERROR: %s, %d, send sms msg rsp failed, osaStatus=%d", __func__, __LINE__, osStatus);
                    }
                }
                else if(msg.arg2 == SC_URC_NEW_MSG_IND)
                {
                    /*new msg recved*/
                    INT8 *p_smsindex = (INT8*)msg.arg3;
                    SIM_MSG_T SmsUrcProcessReq = {0};
                    if(p_smsindex != NULL)
                    {
                        sAPI_Debug("SC_URC_NEW_MSG_IND, new sms string[%s]",p_smsindex);
                        memset(tempStr, 0, 10);
                        memcpy(tempStr, p_smsindex+12,2);//remove +CMTI: "SM", get index string
                        sAPI_Debug("SC_URC_NEW_MSG_IND, new sms string[%d]",atoi(tempStr));

                        /*send msg to process task and read this msg*/
                        SmsUrcProcessReq.msg_id = SC_URC_NEW_MSG_IND; //msgid
                        SmsUrcProcessReq.arg1 = atoi(tempStr); //msg index
                        sAPI_Debug("SC_URC_NEW_MSG_IND, index[%d],msg_id[%d]",SmsUrcProcessReq.arg1, SmsUrcProcessReq.msg_id);
                        /*Trigger urc process thread*/
                        if(NULL != g_sms_demo_urc_process_msgQ){
                            osStatus = sAPI_MsgQSend(g_sms_demo_urc_process_msgQ, &SmsUrcProcessReq);
                        }
                        if(osStatus != SC_SUCCESS)
                        {
                            sAPI_Debug("ERROR: %s, %d, send sms msg rsp failed, osaStatus=%d", __func__, __LINE__, osStatus);
                        }

                    }
                }
                else if(msg.arg2 == SC_URC_STATUS_REPORT)
                {
                    /*sms send status, +CDS*/
                    INT8 *p_smsStatus = (INT8*)msg.arg3;
                    if(p_smsStatus != NULL)
                    {
                        sAPI_Debug("SC_URC_STATUS_REPORT, p_smsStatus[%s]",p_smsStatus);
                    }
                }
                else if(msg.arg2 == SC_URC_SMSDWON)
                {
                    /*SMS DONE?*/
                    SMSbuf = (INT8*)msg.arg3;
                    sAPI_Debug("SC_URC_SMSDWON, msgbuf[%s]",SMSbuf);
                }
                else if(msg.arg2 == SC_URC_SMSFULL)
                {
                    /*SMS FULL*/
                    INT8 *p_msgpload = (INT8*)msg.arg3;
                    if(p_msgpload != NULL)
                    {
                        sAPI_Debug("SC_URC_SMSFULL,p_msgpload[%s]",p_msgpload);
                    }
                }
                #endif
                break;
            }

            case SC_URC_SIM_MASK:
            {
                sAPI_Debug("SC_URC_SIM_MASK process!! pin status:%d",msg.arg2);
                switch(msg.arg2)
                {
                    case SC_URC_CPIN_READY:
                    /*add usercode here*/
                        break;

                    case SC_URC_CPIN_REMOVED:
                    /*add usercode here*/
                        break;

                    default:
                        break;
                }
                break;
            }

            /*for SIM2*/
            case SC_URC_SIM2_MASK:
            {
                sAPI_Debug("SC_URC_SIM2_MASK process!! pin status:%d",msg.arg2);
                switch(msg.arg2)
                {
                    case SC_URC_CPIN_READY:
                    /*add usercode here*/
                        break;

                    case SC_URC_CPIN_REMOVED:
                    /*add usercode here*/
                        break;

                    default:
                        break;
                }
                break;
            }


            case SC_URC_NETSTATUE_MASK:
            {
                sAPI_Debug("URC_NETSTATUE_MASK process!!");
                switch(msg.arg2)
                {
                    case SC_URC_NETACTED:
                    {
                        Netbuf = (INT8 *)msg.arg3;
                        sAPI_Debug("URC_NET_MASK:%s",Netbuf);
                        /*add usercode here*/
                        break;
                    }
                    case SC_URC_NETDIS:
                    {
                        Netbuf = (INT8 *)msg.arg3;
                        sAPI_Debug("URC_NET_MASK:%s",Netbuf);
                        /*add usercode here*/
                        break;
                    }
                    case SC_URC_PDP_ACTIVE:
                    {
                        Netbuf = (INT8 *)msg.arg3;
                        sAPI_Debug("URC_NET_MASK:%s",Netbuf);
                        /*add usercode here*/
                        break;
                    }
                    case SC_URC_PDP_DEACT:
                    {
                        UINT8 *p_cid = (UINT8*)(msg.arg3);
                        sAPI_Debug("SC_URC_PDP_DEACT cid[%d]",*p_cid);
                        break;
                    }

                    default:
                        break;
                }
                break;

            }
            case SC_URC_INTERNAL_AT_RESP_MASK:
            {
                INT8 * respstr = (INT8 *)msg.arg3;

                if(respstr != NULL)
                    sAPI_Debug("SC_URC_INTERNAL_AT_RESP_MASK rsplen[%d] respstr[%s]",msg.arg2,respstr);

                break;
            }
#if defined(FEATURE_SIMCOM_GPS) || defined(JACANA_GPS_SUPPORT)
            case SC_URC_GNSS_MASK:
            {
                 switch(msg.arg2)
                {
                    case SC_URC_GPS_INFO:
                    {
                        GPSbuf = (INT8 *)msg.arg3;
                        sAPI_Debug("GPS INFO:%s",GPSbuf);

                        /*add usercode here*/
                        sAPI_UartWrite(SC_UART,(UINT8*)GPSbuf,strlen((char *)GPSbuf)); //for GPSINFO UIDemo

                        break;
                    }
                    case SC_URC_GNSS_INFO:
                    {
                        GNSSbuf = (INT8 *)msg.arg3;
                        sAPI_Debug("GNSS INFO:%s",GNSSbuf);

                        /*add usercode here*/
                        sAPI_UartWrite(SC_UART,(UINT8*)GNSSbuf,strlen((char *)GNSSbuf)); //for GNSSINFO UIDemo
                        break;
                    }
                    case SC_URC_NMEA_DATA:  //get the orignal NMEA data
                    {
                        NMEADATAbuf = (INT8 *)msg.arg3;
                        sAPI_Debug("NMEA data:%s",NMEADATAbuf);

                        /*add usercode here*/
                        sAPI_UartWrite(SC_UART,(UINT8*)NMEADATAbuf,strlen((char *)NMEADATAbuf)); //for NMEA data UIDemo
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
#endif
            case SC_URC_CALL_MASK:
            {
                sAPI_Debug("SC_URC_CALL_MASK [%d]!!",msg.arg2);
                switch(msg.arg2)
                {
                    case SC_URC_RING_IND:
                    {
                      INT8 *pCallBuf = (INT8*)msg.arg3;
                      if(pCallBuf != NULL)
                      {
                          sAPI_Debug("SC_URC_RING_IND[%s]",pCallBuf);
                      }
                      break;
                    }
                    case SC_URC_DTMF_IND:
                    {
                      INT8 *pDtmfBuf = (INT8*)msg.arg3;
                      if(pDtmfBuf != NULL)
                      {
                        sAPI_Debug("SC_URC_DTMF_IND[%s]",pDtmfBuf);
                      }
                      break;
                    }

                }
                break;
            }
            case SC_URC_AUDIO_MASK:
            {
                sAPI_Debug("SC_URC_AUDIO_MASK process!!");
                switch(msg.arg2)
                {
                    case SC_URC_AUDIO_START:
                    {
                        AUDIObuf = (UINT8 *)msg.arg3;
                        sAPI_Debug("SC_URC_AUDIO_MASK:%s", AUDIObuf);
                        
                        sAPI_UartWrite(SC_UART,AUDIObuf, strlen((char *)AUDIObuf));
                        break;
                    }
                    case SC_URC_AUDIO_STOP:
                    {
                        AUDIObuf = (UINT8 *)msg.arg3;
                        sAPI_Debug("SC_URC_AUDIO_MASK:%s", AUDIObuf);

                        sAPI_UartWrite(SC_UART,AUDIObuf, strlen((char *)AUDIObuf));
                        break;
                    }
                    case SC_URC_CREC_START:
                    {
                        AUDIObuf = (UINT8 *)msg.arg3;
                        sAPI_Debug("SC_URC_AUDIO_MASK:%s", AUDIObuf);

                        sAPI_UartWrite(SC_UART,AUDIObuf, strlen((char *)AUDIObuf));
                        break;
                    }
                    case SC_URC_CREC_STOP:
                    {
                        AUDIObuf = (UINT8 *)msg.arg3;
                        sAPI_Debug("SC_URC_AUDIO_MASK:%s", AUDIObuf);

                        sAPI_UartWrite(SC_UART,AUDIObuf, strlen((char *)AUDIObuf));
                        break;
                    }
                    case SC_URC_CREC_FIFULL:
                    {
                        AUDIObuf = (UINT8 *)msg.arg3;
                        sAPI_Debug("SC_URC_AUDIO_MASK:%s", AUDIObuf);

                        sAPI_UartWrite(SC_UART,AUDIObuf, strlen((char *)AUDIObuf));
                        break;
                    }
                    default:
                        break;
                }

                break;
            }
            case SC_URC_TTS_MASK:
            {
                sAPI_Debug("SC_URC_TTS_IND process!!");
                if(msg.arg2 == SC_URC_TTS_IND)
                {
                    AUDIObuf = (UINT8 *)msg.arg3;
                    sAPI_Debug("AUDIObuf INFO:%s",AUDIObuf);

                    sAPI_UartWrite(SC_UART,AUDIObuf,strlen((char*)AUDIObuf)); //for AUDIObuf UIDemo
                }
                break;
            }

            default:
                sAPI_Debug("default process!!");
                break;
        }

        free(msg.arg3);
        msg.arg3 = NULL;
    }

}

/**
  * @brief  Create URC task 
  * @param  void
  * @note   There will be additional urc events when there is new request from market.
  * @retval void
  */
void sAPP_UrcTask(void)
{
    if (NULL != gUrcProcessTask)
    {
        return;
    }

    if (sAPI_TaskCreate(&gUrcProcessTask,
        gUrcProcessTaskStack,
        URC_PROCESS_TASK_STACK_SIZE,
        URC_PROCESS_TASK_PRIORITY,
        (char*)"urc processer" ,
        sTask_UrcProcesser,
        (void *)0) != SC_SUCCESS)
    {
        gUrcProcessTask = NULL;
        sAPI_Debug("sAPI_TaskCreate error!\n");
    }
}



