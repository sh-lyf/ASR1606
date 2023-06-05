/**
  ******************************************************************************
  * @file    demo_ftps_test.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of ftps test.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */
 
/* Includes ------------------------------------------------------------------*/
#ifdef FEATURE_SIMCOM_FTPS

#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "simcom_os.h"
#include "simcom_ftps.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "simcom_uart.h"

extern void PrintfResp(const char *format, ...);
sSemaRef ftpsDemotestSema;
sTaskRef ftpsDemoTestTask;
static UINT8 ftpsDemoTestTaskStack[1024 * 4];
sMsgQRef ftpsDemoTestMsgq;

int ftpsCurrentTime = 1;
int ftpsTestTime = 1;

/**
  * @brief  FTPS test
  * @param  void
  * @note   This test will be a simple procedure showing how to test FTPS
  * @retval void
  */
void FtpsDemoTest(void)
{
    SC_STATUS status;
    SIM_MSG_T ftpsMsg = {0,0,0,NULL};
    enum appChannelType channeltype; 
    SCftpsLoginMsg msg = {0};
    strcpy(msg.host,"000.000.000.000");
    msg.port = 6047;
    strcpy(msg.username,"test");
    strcpy(msg.password,"test");
    msg.serverType = 0;

    SCapiFtpsData* ftpsData;
    char* listDir = "/";
    char* dir = NULL;
    char* fileName = "test.txt";
    int* size = NULL;

/*
    int pGreg = 0;
    while(1)
    {
        sAPI_NetworkGetCgreg(&pGreg);
        if(1 != pGreg)
        {
            sAPI_Debug("FTP NETWORK STATUS IS [%d]",pGreg);
            sAPI_TaskSleep(10*300);
        }
        else
        {
            sAPI_Debug("FTP NETWORK STATUS IS NORMAL");
            break;
        }
    }
*/
    if(NULL == ftpsDemoTestMsgq)
    {
        status = sAPI_MsgQCreate(&ftpsDemoTestMsgq, "ftpsDemoTestMsgq", sizeof(SIM_MSG_T), 8, SC_FIFO);
        if(SC_SUCCESS != status)
        {
            sAPI_Debug("msgQ create fail");
            return;
        }
    }

    sAPI_FtpsInit(SC_FTPS_USB,ftpsDemoTestMsgq);
    
    sAPI_MsgQRecv(ftpsDemoTestMsgq,&ftpsMsg,SC_SUSPEND);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);

    if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
    
        sAPI_Debug("PDP SUCCESS");
        PrintfResp("\r\nFTP Init Successful!\r\n");
    }
    else
    {
        sAPI_Debug("PDP FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
        PrintfResp("\r\nFTP Init Fail!\r\n");
    }

    sAPI_TaskSleep(200);//delay 1s

    memset(&ftpsMsg,0,sizeof(ftpsMsg));
    ftpsMsg.arg3 = NULL;

    sAPI_Debug("FTPS_login");

    sAPI_FtpsLogin(msg);

    sAPI_MsgQRecv(ftpsDemoTestMsgq,&ftpsMsg,SC_SUSPEND);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);

    if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        sAPI_Debug("LOGIN SUCCESSFUL");
        PrintfResp("\r\nFTP Login Successful!\r\n");
    }
    else
    {
        sAPI_Debug("LOGIN ERROR,ERROR CODE = [%d]",ftpsMsg.arg2);
        PrintfResp("\r\nFTP Login Fail!\r\n");
    }
    
    memset(&ftpsMsg,0,sizeof(ftpsMsg));
    ftpsMsg.arg3 = NULL;

    sAPI_Debug("FTPS_list");    
    sAPI_FtpsList(listDir);
    while(1)
    {
        sAPI_Debug("COMING TO THE RECV");
        ftpsMsg.arg3 = NULL;
        status = sAPI_MsgQRecv(ftpsDemoTestMsgq,&ftpsMsg,SC_SUSPEND);
        sAPI_Debug("status = [%d]",status);
        channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
        sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);
        if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
        {
            ftpsData = (SCapiFtpsData*)ftpsMsg.arg3;
            sAPI_Debug("flag = [%d]",ftpsData->flag);
            if(SC_DATA_COMPLETE == ftpsData->flag)
            {
                sAPI_Debug("get data complete!");
                sAPI_Debug("flag = [%d],len = [%d]",ftpsData->flag,ftpsData->len);
                free(ftpsData);
                break;
            }
            else if(SC_DATA_RESUME == ftpsData->flag)
            {
                sAPI_Debug("get data successful!");
                sAPI_Debug("flag = [%d],len = [%d]",ftpsData->flag,ftpsData->len);

                //just for waiting uart ok now
                sAPI_Debug("data = [%s]",(UINT8*)ftpsData->data);
                sAPI_UartWrite(SC_UART,(UINT8*)ftpsData->data,ftpsData->len);

                free(ftpsData->data);
                free(ftpsData);
            }
            else
            {
                sAPI_Debug("ERROR HAPPEN!");
                PrintfResp("\r\nFtp list fail!\r\n");
                break;
            }
        }
        else
        {
            sAPI_Debug("error code = [%d]",ftpsMsg.arg2);
            PrintfResp("\r\nFtp list fail!\r\n");
            break;
        }
    
    }

    memset(&ftpsMsg,0,sizeof(ftpsMsg));
    ftpsMsg.arg3 = NULL;
    
    sAPI_Debug("FTPS_getCurrentDirectory");
    sAPI_FtpsGetCurrentDirectory();
    sAPI_MsgQRecv(ftpsDemoTestMsgq,&ftpsMsg,SC_SUSPEND);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);
    
    if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        char path[256] = {0};
        dir = (char *)ftpsMsg.arg3;
        sAPI_Debug("GET CURRENT DIRECTORY SUCCESS = [%s]",dir);
        snprintf(path,sizeof(path),"\r\ncurrent path = %s\r\n",dir);
        PrintfResp(path);
        free(dir);
    }
    else
    {
        sAPI_Debug("GET CURRENT DIRECTORY FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
        PrintfResp("\r\nGet Directory Fail\r\n"); 
    }
    
    memset(&ftpsMsg,0,sizeof(ftpsMsg));
    ftpsMsg.arg3 = NULL;
    
    sAPI_Debug("FTPS_downloadFile");
    
    sAPI_FtpsDownloadFile(fileName,SC_FTPS_FILE_FLASH);
    sAPI_MsgQRecv(ftpsDemoTestMsgq,&ftpsMsg,SC_SUSPEND);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);
    
    if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        sAPI_Debug("DOWNLOAD FILE SUCCESSFUL");
        PrintfResp("\r\nDownload File Successful\r\n");
    }
    else
    {
        sAPI_Debug("DOWNLOAD FILE FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
        PrintfResp("\r\nDownload File Fail\r\n");
    }

    memset(&ftpsMsg,0,sizeof(ftpsMsg));
    ftpsMsg.arg3 = NULL;    
    sAPI_Debug("FTPS_downloadFileToBuffer");

    sAPI_FtpsDownloadFileToBuffer(fileName,0);
    while(1)
    {
        sAPI_Debug("COMING TO THE RECV");
        ftpsMsg.arg3 = NULL;
        status = sAPI_MsgQRecv(ftpsDemoTestMsgq,&ftpsMsg,SC_SUSPEND);
        sAPI_Debug("status = [%d]",status);
        channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
        sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);
        if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
        {
            ftpsData = (SCapiFtpsData*)ftpsMsg.arg3;
            sAPI_Debug("flag = [%d]",ftpsData->flag);
            if(SC_DATA_COMPLETE == ftpsData->flag)
            {
                sAPI_Debug("get data complete!");
                sAPI_Debug("flag = [%d],len = [%d]",ftpsData->flag,ftpsData->len);
                free(ftpsData);
                break;
            }
            else if(SC_DATA_RESUME == ftpsData->flag)
            {
                sAPI_Debug("get data successful!");
                sAPI_Debug("flag = [%d],len = [%d]",ftpsData->flag,ftpsData->len);

                //just for waiting uart ok now
                sAPI_Debug("data = [%s]",(UINT8*)ftpsData->data);
                sAPI_UartWrite(SC_UART,(UINT8*)ftpsData->data,ftpsData->len);

                free(ftpsData->data);
                free(ftpsData);
            }
            else
            {
                sAPI_Debug("ERROR HAPPEN!");
                PrintfResp("FTP DOWNLOAD FILE TO BUFFER FAIL!");
                break;
            }
        }
        else
        {
            sAPI_Debug("error code = [%d]",ftpsMsg.arg2);
            PrintfResp("FTP DOWNLOAD FILE TO BUFFER FAIL!");
            break;
        }
    
    }
    
    memset(&ftpsMsg,0,sizeof(ftpsMsg));
    ftpsMsg.arg3 = NULL;    
    sAPI_Debug("FTPS_uploadFile");
                       
    sAPI_FtpsUploadFile(fileName,1,0);
    sAPI_MsgQRecv(ftpsDemoTestMsgq,&ftpsMsg,SC_SUSPEND);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);

    if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        sAPI_Debug("uploadFile SUCCESSFUL");
        PrintfResp("\r\nUpload File Successful\r\n");
    }
    else
    {
        sAPI_Debug("UOLOAD FILE FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
        PrintfResp("\r\nUpload File Fail\r\n"); 
    }

    memset(&ftpsMsg,0,sizeof(ftpsMsg));
    ftpsMsg.arg3 = NULL;
    sAPI_Debug("FTPS_getFileSize");
                       
    sAPI_FtpsGetFileSize(fileName);
    sAPI_MsgQRecv(ftpsDemoTestMsgq,&ftpsMsg,SC_SUSPEND);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);
    
    if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        char path[256] = {0};
        size = (int *)ftpsMsg.arg3;
        sAPI_Debug("GET SIZE SUCESS,SIZE = [%d]",*size);
        snprintf(path,sizeof(path),"\r\nfile size = %d\r\n",*size);
        PrintfResp(path);
        free(size);
    }
    else
    {
        sAPI_Debug("GET SIZE FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
    }

    memset(&ftpsMsg,0,sizeof(ftpsMsg));
    ftpsMsg.arg3 = NULL;
    
    sAPI_Debug("FTPS_logout");  
    sAPI_FtpsLogout();
    sAPI_MsgQRecv(ftpsDemoTestMsgq,&ftpsMsg,SC_SUSPEND);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);
    
    if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        sAPI_Debug("LOGOUT SUCCESSFUL");
        PrintfResp("\r\nFTP Logout Successful!\r\n");
    }
    else
    {
        sAPI_Debug("LOGOUT FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
        PrintfResp("\r\nFTP Logout Fail!\r\n");
    }

    sAPI_TaskSleep(200);//delay 1s

    memset(&ftpsMsg,0,sizeof(ftpsMsg));
    ftpsMsg.arg3 = NULL;
    sAPI_FtpsDeInit(SC_FTPS_USB);

    sAPI_MsgQRecv(ftpsDemoTestMsgq,&ftpsMsg,SC_SUSPEND);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);

    if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
    
        sAPI_Debug("DEINIT SUCCESS");
        PrintfResp("\r\nFTP DeInit Successful!\r\n");
    }
    else
    {
        sAPI_Debug("DEINIT FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
        PrintfResp("\r\nFTP DeInit Fail!\r\n");
    }

    sAPI_Debug("DEMO SUCCESSFUL!");

}

/**
  * @brief  FTPS test task processor
  * @param  arg
  * @note   
  * @retval void
  */
void sTask_ftpsDemoTestProcesser(void * arg)
{
    SC_STATUS status = SC_SUCCESS;
    char buff[30] = {0};
    
    sAPI_Debug("sTask_ftpsDemoTestProcesser enter");
    while(1)
    {
        status = sAPI_SemaphoreAcquire(ftpsDemotestSema, SC_SUSPEND);
        if(SC_SUCCESS != status)
        {
            sAPI_Debug("status = [%d], accquire seamphore error!");
            continue;
        }

        memset(buff, 0, sizeof(buff));
        snprintf(buff, sizeof(buff), "\r\ncurrent time = %d\r\n",ftpsCurrentTime);
        PrintfResp(buff);
        
        FtpsDemoTest();

        ftpsTestTime--;
        ftpsCurrentTime++;
        
        if(0 == ftpsTestTime)
        {
            sAPI_Debug("test complete!");
            continue;
        }
        else if(ftpsTestTime > 0)
        {
            sAPI_SemaphoreRelease(ftpsDemotestSema);
        }
    }
}

/**
  * @brief  FTPS test task initial
  * @param  void
  * @note   
  * @retval void
  */
void FtpsTestDemoInit(void)
{
    static UINT16 isCreated = 0;
    SC_STATUS status = SC_SUCCESS;
    sAPI_Debug("FtpsTestDemoInit enter");

    if(1 == isCreated)
    {
        sAPI_Debug("init successful!");
        if(ftpsDemotestSema)
            sAPI_SemaphoreRelease(ftpsDemotestSema);
        return;
    }

    if(!ftpsDemoTestTask)
    {
        status = sAPI_TaskCreate(&ftpsDemoTestTask,ftpsDemoTestTaskStack,1024 * 5,100,
                "ftpsDemoTestProcesser",sTask_ftpsDemoTestProcesser,(void *)0);
        if(SC_SUCCESS != status)
        {
            sAPI_Debug("task create fail");
        }
    }

    if(!ftpsDemotestSema)
    {
        status = sAPI_SemaphoreCreate(&ftpsDemotestSema, 1, SC_FIFO);
        if(SC_SUCCESS != status)
        {
            sAPI_Debug("flag create fail");
        }
    }

    isCreated = 1;

    sAPI_Debug("FtpsTestDemoInit end");

}

#endif
