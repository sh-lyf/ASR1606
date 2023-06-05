/**
  ******************************************************************************
  * @file    demo_loc_test.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of LBS location test.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "simcom_os.h"
#include "simcom_loc.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "simcom_uart.h"
#include "simcom_network.h"

sSemaRef lbsDemotestSema;
sTaskRef lbsDemoTestTask;
static unsigned char lbsDemoTestTaskStack[1024 * 4];
sMsgQRef lbsDemoTestMsgq;
extern int lbsCurrentTime;
extern int lbsTestTime;
sMsgQRef SC_LbstestResp_msgq;
extern void PrintfResp(char* format);

/**
  * @brief  LBS test demno
  * @param  void
  * @note  
  * @retval void
  */
void LbsDemoTest(void)
{
    INT16 channel = 0;
    INT16 type;
    int pGreg = 0;
    SC_LBS_RETURNCODE ret;
    
    while(1)
    {
        sAPI_NetworkGetCgreg(&pGreg);
        if(1 != pGreg)
        {
            sAPI_Debug("LBS NETWORK STATUS IS [%d]",pGreg);
            sAPI_TaskSleep(10*300);
        }
        else
        {
            sAPI_Debug("LBS NETWORK STATUS IS NORMAL");
            break;
        }
    }

    if(SC_LbstestResp_msgq == NULL)
    {
        SC_STATUS status = sAPI_MsgQCreate(&SC_LbstestResp_msgq, "SC_LbsResp_msgq", (sizeof(SIM_MSG_T)), 4, SC_FIFO);
        if(SC_SUCCESS != status)
        {
            sAPI_Debug("osaStatus = [%d],create msg error!",status);
        }
    }
    else
    {
        sAPI_Debug("msg has been created!"); 
    }

    type = 1;
    char path[300] = {0};
    ret = sAPI_LocGet(channel, SC_LbstestResp_msgq, type);
    if(ret != SC_LBS_SUCCESS)
    {
        sAPI_Debug("get location fail!\r\n");
        PrintfResp("\r\nLBS get longitude and latitude Fail!\r\n");
    }
    else
    {
        sAPI_Debug("get location success!\r\n");
        SIM_MSG_T msgQ_data_recv = {0, 0, 0, NULL};                 
        SC_lbs_info_t *sub_data = NULL;
        SC_STATUS osaStatus = sAPI_MsgQRecv(SC_LbstestResp_msgq, &msgQ_data_recv, SC_SUSPEND); 
        if(SC_SUCCESS != osaStatus)
        {
            sAPI_Debug("osaStatus = [%d],recv msg error!",osaStatus);
            PrintfResp("\r\nLBS get longitude and latitude Fail!\r\n");
        }
        else
        {
            if(msgQ_data_recv.msg_id == SC_SRV_LBS)
            {
                sub_data = (SC_lbs_info_t *)msgQ_data_recv.arg3;
                if(sub_data->u8ErrorCode != 0)
                {
                    snprintf(path,sizeof(path),"\r\nLBS get longitude and latitude Fail!Recv LBS info:errocode:%d\r\n",sub_data->u8ErrorCode);
                    PrintfResp(path); 
                    sAPI_Free(sub_data);
                }
                else
                {   
                    snprintf(path,sizeof(path),"\r\nRecv LBS info:errocode:%d,Lng:%ld.%ld,Lat:%ld.%ld,Acc:%d\r\n",sub_data->u8ErrorCode,(sub_data->u32Lng)/1000000,(sub_data->u32Lng)%1000000,(sub_data->u32Lat)/1000000,(sub_data->u32Lat)%1000000,sub_data->u16Acc);
                    sAPI_Debug("%s",path);
                    sAPI_Free(sub_data);
                    PrintfResp(path);
                }
            }
        }
    }

    type = 2;
    memset(path,0,300);
    ret = sAPI_LocGet(channel, SC_LbstestResp_msgq, type);
    if(ret != SC_LBS_SUCCESS)
    {
        sAPI_Debug("get location fail!\r\n");
        PrintfResp("\r\nLBS get detail address Fail!\r\n");
    }
    else
    {
        sAPI_Debug("get location success!\r\n");
        SIM_MSG_T msgQ_data_recv = {0, 0, 0, NULL};                 
        SC_lbs_info_t *sub_data = NULL;

        
        SC_STATUS osaStatus = sAPI_MsgQRecv(SC_LbstestResp_msgq, &msgQ_data_recv, SC_SUSPEND); 
        if(SC_SUCCESS != osaStatus)
        {
            sAPI_Debug("osaStatus = [%d],recv msg error!",osaStatus);
            PrintfResp("\r\nLBS get detail address Fail!\r\n");
        }
        else
        {
            if(msgQ_data_recv.msg_id == SC_SRV_LBS)
            {
                sub_data = (SC_lbs_info_t *)msgQ_data_recv.arg3;
                if(sub_data->u8ErrorCode != 0)
                {
                    snprintf(path,sizeof(path),"\r\nLBS get detail address Fail!Recv LBS info:errocode:%d\r\n",sub_data->u8ErrorCode);
                    sAPI_Free(sub_data);

                    PrintfResp(path); 
                }
                else
                {
                    char tempBuf[151];
                    memset(tempBuf,0,151);
                    char *p_tmp=tempBuf;
                    UINT16 i;	
                    for(i=0; i<sub_data->u32AddrLen; i++)
                    {
                        p_tmp+= sprintf(p_tmp,"%02x", sub_data->u8LocAddress[i]); 
                        sAPI_Debug("%02x",sub_data->u8LocAddress[i]);
                    }
                    snprintf(path,sizeof(path),"\r\nRecv LBS info:errocode:%d,detail_addr:%s\r\n",sub_data->u8ErrorCode,tempBuf);
                    sAPI_Free(sub_data);
                    PrintfResp(path);
                }
            }
        }
    }

    type = 3;
    //char path[300] = {0};
    memset(path,0,300);
    ret = sAPI_LocGet(channel, SC_LbstestResp_msgq, type);
    if(ret != SC_LBS_SUCCESS)
    {
        sAPI_Debug("get location fail!\r\n");
        PrintfResp("\r\nLBS get error number Fail!\r\n");
    }
    else
    {
        sAPI_Debug("get location success!\r\n");
        SIM_MSG_T msgQ_data_recv = {0, 0, 0, NULL};                 
        SC_lbs_info_t *sub_data = NULL;
        SC_STATUS osaStatus = sAPI_MsgQRecv(SC_LbstestResp_msgq, &msgQ_data_recv, SC_SUSPEND); 
        if(SC_SUCCESS != osaStatus)
        {
            sAPI_Debug("osaStatus = [%d],recv msg error!",osaStatus);
            PrintfResp("\r\nLBS get error number Fail!\r\n");
        }
        else
        {
            if(msgQ_data_recv.msg_id == SC_SRV_LBS)
            {
                sub_data = (SC_lbs_info_t *)msgQ_data_recv.arg3;
                if(sub_data->u8ErrorCode != 0)
                {
                    snprintf(path,sizeof(path),"\r\nLBS get error number Fail!Recv LBS info:errocode:%d\r\n",sub_data->u8ErrorCode);
                    sAPI_Free(sub_data);
                    PrintfResp(path); 
                }
                else
                {
                    snprintf(path,sizeof(path),"\r\nRecv LBS info:errocode:%d\r\n",sub_data->u8ErrorCode);
                    sAPI_Free(sub_data);

                    PrintfResp(path);
                    
                }
            }
        }
    }

    type = 4;
    //char path[300] = {0};
    memset(path,0,300);
    ret = sAPI_LocGet(channel, SC_LbstestResp_msgq, type);
    if(ret != SC_LBS_SUCCESS)
    {
        sAPI_Debug("get location fail!\r\n");
        PrintfResp("\r\nLBS get longitude latitude and date time Fail!\r\n");
    }
    else
    {
        sAPI_Debug("get location success!\r\n");
        SIM_MSG_T msgQ_data_recv = {0, 0, 0, NULL};                 
        SC_lbs_info_t *sub_data = NULL;
        SC_STATUS osaStatus = sAPI_MsgQRecv(SC_LbstestResp_msgq, &msgQ_data_recv, SC_SUSPEND); 
        if(SC_SUCCESS != osaStatus)
        {
            sAPI_Debug("osaStatus = [%d],recv msg error!",osaStatus);
            PrintfResp("\r\nLBS get longitude latitude and date time Fail!\r\n");
        }
        else
        {
            if(msgQ_data_recv.msg_id == SC_SRV_LBS)
            {
                sub_data = (SC_lbs_info_t *)msgQ_data_recv.arg3;
                if(sub_data->u8ErrorCode != 0)
                {
                    snprintf(path,sizeof(path),"\r\nLBS get longitude latitude and date time Fail!Recv LBS info:errocode:%d\r\n",sub_data->u8ErrorCode);
                    sAPI_Debug("%s",path);
                    sAPI_Free(sub_data);
                    PrintfResp(path); 
                }
                else
                {   
                    snprintf(path,sizeof(path),"\r\nRecv LBS info:errocode %d,Lng:%ld.%ld,Lat:%ld.%ld,Acc:%d,DateAndTime:%s\r\n",sub_data->u8ErrorCode,(sub_data->u32Lng)/1000000,(sub_data->u32Lng)%1000000,(sub_data->u32Lat)/1000000,(sub_data->u32Lat)%1000000, sub_data->u16Acc,sub_data->u8DateAndTime);
                    sAPI_Free(sub_data);
                    PrintfResp(path);
                }
         
            }
        }
    }

    
}

/**
  * @brief  LBS test processor
  * @param  void
  * @note  
  * @retval void
  */
void sTask_lbsDemoTestProcesser(void * arg)
{
    SC_STATUS status = SC_SUCCESS;
    char path[256] = {0};
    while(1)
    {
        status = sAPI_SemaphoreAcquire(lbsDemotestSema, SC_SUSPEND);
        if(SC_SUCCESS != status)
        {
            sAPI_Debug("status = [%d], accquire seamphore error!");
            continue;
        }

        memset(path, 0, sizeof(path));
        snprintf(path, sizeof(path), "\r\ncurrent time = %d, testTime = %d\r\n",lbsCurrentTime,lbsTestTime);
        PrintfResp(path);
        
        LbsDemoTest();

        lbsTestTime = lbsTestTime-1;
        lbsCurrentTime++;
        
        if(0 == lbsTestTime)
        {
            sAPI_Debug("test complete!");
            continue;
        }
        else if(lbsTestTime > 0)
        {
            sAPI_SemaphoreRelease(lbsDemotestSema);
        }
         
    }

}

/**
  * @brief  LBS test demno task initial
  * @param  void
  * @note  
  * @retval void
  */
void LbsTestDemoInit(void)
{
    static UINT16 isCreated = 0;
    SC_STATUS status = SC_SUCCESS;

    if(1 == isCreated)
    {
        sAPI_Debug("init successful!");
        sAPI_SemaphoreRelease(lbsDemotestSema);
        return;
    }
    status = sAPI_TaskCreate(&lbsDemoTestTask,lbsDemoTestTaskStack,1024 * 8,100,"lbsDemoTestProcesser",sTask_lbsDemoTestProcesser,(void *)0);
    if(SC_SUCCESS != status)
    {
        sAPI_Debug("task create fail");
    }

    status = sAPI_SemaphoreCreate(&lbsDemotestSema, 1, SC_FIFO);
    if(SC_SUCCESS != status)
    {
        sAPI_Debug("flag create fail");
    }

    isCreated = 1;

}


