/**
  ******************************************************************************
  * @file    demo_ftps.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of ftps function.
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

typedef enum{
    SC_FTPS_DEMO_INIT                  = 1,
    SC_FTPS_DEMO_LOGIN                 = 2,
    SC_FTPS_DEMO_LOGOUT                = 3,
    SC_FTPS_DEMO_GETDIR                = 4,
    SC_FTPS_DEMO_LIST                  = 5,
    SC_FTPS_DEMO_CHANGEDIR             = 6,
    SC_FTPS_DEMO_CREATDIR              = 7,
    SC_FTPS_DEMO_DELETEFILE            = 8,
    SC_FTPS_DEMO_DELETEDIR             = 9, 
    SC_FTPS_DEMO_SIZE                  = 10,
    SC_FTPS_DEMO_DOWNLOADFILE          = 11,
    SC_FTPS_DEMO_DOWNLOADFILETOBUFFER  = 12,
    SC_FTPS_DEMO_UPLOADFILE            = 13,
    SC_FTPS_DEMO_DEINIT                = 14,
    SC_FTPS_DEMO_TEST                  = 15,

    SC_FTPS_DEMO_MAX                   = 99
}SC_FTPS_DEMO_TYPE;


extern sMsgQRef simcomUI_msgq;
extern int ftpsTestTime;
extern int ftpsCurrentTime;
sMsgQRef ftpsUIResp_msgq;

extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(const char *format, ...);
extern SIM_MSG_T GetParamFromUart(void);
extern void FtpsTestDemoInit(void);


/**
  * @brief  FTPS demo operation
  * @param  void
  * @note   
  * @retval void
  */
void FtpsDemo(void)
{
    enum appChannelType channeltype; 
    SCapiFtpsData* ftpsData;
    char* dir = NULL;
    char* file = NULL;
    int* size = NULL;
    SC_STATUS status;
    SC_FTPS_RETURNCODE ret = SC_FTPS_RESULT_OK;

    SIM_MSG_T optionMsg ={0,0,0,NULL};
    UINT32 opt = 0;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] = {
        "1. Init",
        "2. Login",
        "3. Logout",
        "4. Print work directory",
        "5. List",
        "6. Change work directory",
        "7. Mkdir",
        "8. Delete file",
        "9. Delete folder",
        "10. Get file size",
        "11. Download file",
        "12. Download file to buffer",
        "13. Upload file",
        "14. DeInit",
        "15. Test",
        "99. back",
    }; 

    while(1)
    {
        SIM_MSG_T ftpsMsg = {0,0,0,NULL};
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
            case SC_FTPS_DEMO_INIT:
            {
                if(NULL == ftpsUIResp_msgq)
                {
                    SC_STATUS status;
                    status = sAPI_MsgQCreate(&ftpsUIResp_msgq, "ftpsUIResp_msgq", sizeof(SIM_MSG_T), 8, SC_FIFO);
                    if(SC_SUCCESS != status)
                    {
                        sAPI_Debug("msgQ create fail");
                        PrintfResp("\r\nFTP Init Fail!\r\n");
                        break;
                    }
                }
                sAPI_FtpsInit(SC_FTPS_USB,ftpsUIResp_msgq);

                sAPI_MsgQRecv(ftpsUIResp_msgq,&ftpsMsg,SC_SUSPEND);
                channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
                sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);

                if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
                {
                    sAPI_Debug("Init SUCCESS");
                    PrintfResp("\r\nFTP Init Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("Init FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
                    PrintfResp("\r\nFTP Init Fail!\r\n");
                    break;
                }
            }

            case SC_FTPS_DEMO_LOGIN:
            {
            
                sAPI_Debug("FTPS_login");
                SCftpsLoginMsg msg = {0};
                msg.serverType = 0;
                
                PrintfResp("\r\nPlease input Ip.\r\n");                    
                optionMsg = GetParamFromUart();
                strcpy(msg.host,optionMsg.arg3);
                free(optionMsg.arg3);

                PrintfResp("\r\nPlease input Port.\r\n");                    
                optionMsg = GetParamFromUart();
                msg.port = atoi(optionMsg.arg3);
                free(optionMsg.arg3);


                PrintfResp("\r\nPlease input User.\r\n");                    
                optionMsg = GetParamFromUart();
                strcpy(msg.username,optionMsg.arg3);
                free(optionMsg.arg3);

                PrintfResp("\r\nPlease input Pass.\r\n");                    
                optionMsg = GetParamFromUart();
                strcpy(msg.password,optionMsg.arg3);
                free(optionMsg.arg3);

                sAPI_Debug("host = [%s],user = [%s],pass = [%s]",msg.host,msg.username,msg.password);

                ret = sAPI_FtpsLogin(msg);
                sAPI_MsgQRecv(ftpsUIResp_msgq,&ftpsMsg,SC_SUSPEND);
                channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
                sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",
                    channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);
                if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
                {
                    sAPI_Debug("LOGIN SUCCESSFUL");
                    PrintfResp("\r\nFTP Login Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("LOGIN ERROR,ERROR CODE = [%d]",ftpsMsg.arg2);
                    PrintfResp("\r\nFTP Login Fail!\r\n");
                    break;
                }
            }

            case SC_FTPS_DEMO_LOGOUT:
            {
                sAPI_Debug("FTPS_logout");
                ret = sAPI_FtpsLogout();
                sAPI_MsgQRecv(ftpsUIResp_msgq,&ftpsMsg,SC_SUSPEND);
                channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
                sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);

                if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
                {
                    sAPI_Debug("LOGOUT SUCCESSFUL");
                    PrintfResp("\r\nFTP Logout Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("LOGOUT FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
                    PrintfResp("\r\nFTP Logout Fail!\r\n");
                    break;
                }
            }

            case SC_FTPS_DEMO_GETDIR:
            {
                char path[256] = {0};
                sAPI_Debug("FTPS_getCurrentDirectory");
                ret = sAPI_FtpsGetCurrentDirectory();
                sAPI_MsgQRecv(ftpsUIResp_msgq,&ftpsMsg,SC_SUSPEND);
                channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
                sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);
                
                if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
                {
                    dir = (char *)ftpsMsg.arg3;
                    sAPI_Debug("GET CURRENT DIRECTORY SUCCESS = [%s]",dir);
                    snprintf(path,sizeof(path),"\r\ncurrent path = %s\r\n",dir);
                    PrintfResp(path);
                    break;
                }
                else
                {
                    sAPI_Debug("GET CURRENT DIRECTORY FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
                    PrintfResp("\r\nGet Directory Fail\r\n");           
                    break;     
                }

            }

            case SC_FTPS_DEMO_LIST:
            {

                sAPI_Debug("FTPS_list");
                PrintfResp("\r\nPlease input directory.\r\n");
                optionMsg = GetParamFromUart();
                dir = (char *)optionMsg.arg3;
                
                ret = sAPI_FtpsList(dir);
                if(ret != SC_FTPS_RESULT_OK)
                {
                    sAPI_Debug("LIST FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
                    PrintfResp("\r\nFTP LIST Fail!\r\n");
                    break;
                }
                while(1)
                {
                    sAPI_Debug("COMING TO THE RECV");
                    ftpsMsg.arg3 = NULL;
                    status = sAPI_MsgQRecv(ftpsUIResp_msgq,&ftpsMsg,SC_SUSPEND);
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
                            sAPI_UartWrite(SC_UART,(UINT8*)ftpsData->data,ftpsData->len);

                            free(ftpsData->data);
                            free(ftpsData);
                        }
                        else
                        {
                            sAPI_Debug("ERROR HAPPEN!");
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
                free(dir);
                break;

            }

            case SC_FTPS_DEMO_CHANGEDIR:
            {

                PrintfResp("\r\nPlease input directory.\r\n");
                sAPI_Debug("sAPI_FtpsChangeDirectory");
                optionMsg = GetParamFromUart();
                sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                dir = (char *)optionMsg.arg3;
                
                ret = sAPI_FtpsChangeDirectory(dir);
                sAPI_MsgQRecv(ftpsUIResp_msgq,&ftpsMsg,SC_SUSPEND);
                channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
                sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);
                
                if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
                {
                    sAPI_Debug("Change Directory Successful");
                    PrintfResp("\r\nChange Directory Successful!\r\n");
                }
                else
                {
                    sAPI_Debug("Change Directory FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
                    PrintfResp("\r\nChange Directory Fail!\r\n");
                }
                
                free(dir);
                break;

            }
            
            case SC_FTPS_DEMO_CREATDIR:
            {

                PrintfResp("\r\nPlease input directory.\r\n");
                sAPI_Debug("sAPI_FtpsCreateDirectory");
                optionMsg = GetParamFromUart();
                sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                dir = (char *)optionMsg.arg3;

                ret = sAPI_FtpsCreateDirectory(dir);
                sAPI_MsgQRecv(ftpsUIResp_msgq,&ftpsMsg,SC_SUSPEND);
                channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
                sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);
                
                if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
                {
                    sAPI_Debug("Create Directory Successful");
                    PrintfResp("\r\nCreate Directory Successful!\r\n");
                }
                else
                {
                    sAPI_Debug("Create Directory FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
                    PrintfResp("\r\nCraete Directory Fail!\r\n");
                }
                
                free(dir);
                break;

            }

            case SC_FTPS_DEMO_DELETEFILE:
            {

                PrintfResp("\r\nPlease input file.\r\n");
                sAPI_Debug("sAPI_FtpsDeleteFile");
                optionMsg = GetParamFromUart();
                sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                file = (char *)optionMsg.arg3;

                ret = sAPI_FtpsDeleteFile(file);
                sAPI_MsgQRecv(ftpsUIResp_msgq,&ftpsMsg,SC_SUSPEND);
                channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
                sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);
                
                if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
                {
                    sAPI_Debug("Delete File Successful");
                    PrintfResp("\r\nDelete File Successful!\r\n");
                }
                else
                {
                    sAPI_Debug("Delete File Fail");
                    PrintfResp("\r\nDelete File Fail!\r\n");
                }
                
                free(file);
                break;

            }   

            case SC_FTPS_DEMO_DELETEDIR:
            {

                PrintfResp("\r\nPlease input directory.\r\n");                   
                sAPI_Debug("sAPI_FtpsDeleteDirectory");
                optionMsg = GetParamFromUart();
                sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                dir = (char *)optionMsg.arg3;

                ret = sAPI_FtpsDeleteDirectory(dir);
                sAPI_MsgQRecv(ftpsUIResp_msgq,&ftpsMsg,SC_SUSPEND);
                channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
                sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);
                
                if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
                {
                    sAPI_Debug("Delete Directory Successful");
                    PrintfResp("\r\nDelete Directory Successful!\r\n");
                }
                else
                {
                    sAPI_Debug("Delete Directory Fail");
                    PrintfResp("\r\nDelete Directory Fail!\r\n"); 
                }
                
                free(dir);
                break;

            }

            case SC_FTPS_DEMO_SIZE:
            {
                
                char path[256] = {0};
                PrintfResp("\r\nPlease input directory.\r\n");
                sAPI_Debug("sAPI_FtpsGetFileSize");
                optionMsg = GetParamFromUart();
                sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                file = (char *)optionMsg.arg3;

                ret = sAPI_FtpsGetFileSize(file);
                sAPI_MsgQRecv(ftpsUIResp_msgq,&ftpsMsg,SC_SUSPEND);
                channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
                sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);
                
                if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
                {
                    size = (int *)ftpsMsg.arg3;
                    sAPI_Debug("GET SIZE SUCESS,SIZE = [%d]",*size);
                    snprintf(path,sizeof(path),"\r\nfile size = %d\r\n",*size);
                    PrintfResp(path); 
                    free(size);
                }
                else
                {
                    sAPI_Debug("GET SIZE FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
                    PrintfResp("\r\nGet File Size Fail!\r\n"); 
                }
                
                free(file);
                break;

            }

            case SC_FTPS_DEMO_DOWNLOADFILE:
            {
                
                PrintfResp("\r\nPlease input directory.\r\n");
                sAPI_Debug("sAPI_FtpsDownloadFile");
                optionMsg = GetParamFromUart();
                sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                file = (char *)optionMsg.arg3;

                ret = sAPI_FtpsDownloadFile(file,SC_FTPS_FILE_FLASH);
                sAPI_MsgQRecv(ftpsUIResp_msgq,&ftpsMsg,SC_SUSPEND);
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
                
                free(file);
                break;

            }

            case SC_FTPS_DEMO_DOWNLOADFILETOBUFFER:
            {
                
                PrintfResp("\r\nPlease input File name.\r\n");
                sAPI_Debug("sAPI_FtpsDownloadFileToBuffer");
                optionMsg = GetParamFromUart();
                sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                file = (char *)optionMsg.arg3; 

                ret = sAPI_FtpsDownloadFileToBuffer(file,0);
                if(ret != SC_FTPS_RESULT_OK)
                {
                    sAPI_Debug("DownloadFileToBuffer FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
                    PrintfResp("\r\nFTP DownloadFileToBuffer Fail!\r\n");
                    break;
                }

                while(1)
                {
                    sAPI_Debug("COMING TO THE RECV");
                    ftpsMsg.arg3 = NULL;
                    status = sAPI_MsgQRecv(ftpsUIResp_msgq,&ftpsMsg,SC_SUSPEND);
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
                            sAPI_UartWrite(SC_UART,(UINT8*)ftpsData->data,ftpsData->len);

                            free(ftpsData->data);
                            free(ftpsData);
                        }
                        else
                        {
                            sAPI_Debug("ERROR HAPPEN!");
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

                free(file);
                break;
                
            }

            case SC_FTPS_DEMO_UPLOADFILE:
            {
                
                PrintfResp("\r\nPlease input directory.\r\n");
                sAPI_Debug("sAPI_FtpsUploadFile");
                optionMsg = GetParamFromUart();
                sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                file = (char *)optionMsg.arg3;

                ret = sAPI_FtpsUploadFile(file,SC_FTPS_FILE_FLASH,0);
                sAPI_MsgQRecv(ftpsUIResp_msgq,&ftpsMsg,SC_SUSPEND);
                channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
                sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);
                
                if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
                {
                    sAPI_Debug("UPLOAD FILE SUCCESSFUL");
                    PrintfResp("\r\nUpload File Successful\r\n"); 
                }
                else
                {
                    sAPI_Debug("UPLOAD FILE FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
                    PrintfResp("\r\nUpload File Fail\r\n"); 
                }

                free(file);
                break;
            }

            case SC_FTPS_DEMO_DEINIT:
            {

                sAPI_Debug("FTPS_DeInit");
                sAPI_FtpsDeInit(SC_FTPS_USB);
                sAPI_MsgQRecv(ftpsUIResp_msgq,&ftpsMsg,SC_SUSPEND);
                channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
                sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",
                    channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);

                if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
                {
                    sAPI_Debug("DEINIT SUCCESS");
                    PrintfResp("\r\nFTP DeInit Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("DEINIT FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
                    PrintfResp("\r\nFTP DeInit Fail!\r\n");
                    break; 
                }
            }

            case SC_FTPS_DEMO_TEST:
            {
                
                sAPI_Debug("Start ftps demo test thread!");
                PrintfResp("\r\nPlease input test time.\r\n");                    
                optionMsg = GetParamFromUart();
                sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                ftpsTestTime = atoi(optionMsg.arg3);
                ftpsCurrentTime = 1;
                free(optionMsg.arg3);
                
                FtpsTestDemoInit();
                break;
            }
            
            case SC_FTPS_DEMO_MAX:
            {
                sAPI_Debug("Return to the previous menu!");
                PrintfResp("\r\nReturn to the previous menu!\r\n");

                /*Logout first*/
                sAPI_Debug("FTPS_logout");
                sAPI_FtpsLogout();
                sAPI_MsgQRecv(ftpsUIResp_msgq,&ftpsMsg,SC_SUSPEND);
                channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
                sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);

                if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
                {
                    sAPI_Debug("LOGOUT SUCCESSFUL");
                }
                else
                {
                    sAPI_Debug("LOGOUT FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
                }

                memset(&ftpsMsg,0,sizeof(ftpsMsg));

                /*Then DeInit*/
                sAPI_Debug("FTPS_DeInit");
                sAPI_FtpsDeInit(SC_FTPS_USB);
                sAPI_MsgQRecv(ftpsUIResp_msgq,&ftpsMsg,SC_SUSPEND);
                channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
                sAPI_Debug("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);

                if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
                {
                    sAPI_Debug("DEINIT SUCCESS");
                }
                else
                {
                    sAPI_Debug("DEINIT FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
                }

                return;
            }

            default :
                break;

        }
    }
}

#endif