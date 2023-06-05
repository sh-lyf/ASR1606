/**
  ******************************************************************************
  * @file    demo_file_system.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of file system operation.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */
 
/* Includes ------------------------------------------------------------------*/
#include "simcom_file.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "simcom_os.h"
#include "simcom_common.h"
#include "simcom_debug.h"

void ui_print(const char *format,...);

 typedef enum{
    SC_FS_DEMO_WRITEFILE = 1,
    SC_FS_DEMO_READFILE,
    SC_FS_DEMO_DELETEFILE,
    SC_FS_DEMO_RENAMEFILE,
    SC_FS_DEMO_GETDISKSIZE,
    SC_FS_DEMO_MAKEDIR,   
    SC_FS_DEMO_REMOVEDIR,
    SC_FS_DEMO_OPENDIR,
    SC_FS_DEMO_MAX
 }SC_FS_DEMO_TYPE;

 typedef enum{
     SC_DATA_READ,
     SC_DATA_WRITE
 }SC_RX_DATA_OP;

#define FALSE 0
#define TRUE 1

#define BUFF_LEN 100
#define MAX_OUTPUT_LEN 512
#define MAX_VALID_USER_NAME_LENGTH  255-7

#define RECV_LEN 50
#define FLASH_PAGE_SIZE (2048) //2k
#define FLASH_BLOCK_SIZE (FLASH_PAGE_SIZE * 64) //128k
#define READ_FILE_PACKET_SIZE FLASH_BLOCK_SIZE //128k
#define SIM_FILE_OUTPUT_SIZE 512


char cur_path_in[MAX_VALID_USER_NAME_LENGTH] = "C:/";  // default path





extern sMsgQRef simcomUI_msgq;
extern void PrintfOptionMenu(char* options_list[], int array_size);
extern void PrintfResp(INT8* format);
extern SIM_MSG_T GetParamFromUart(void);



/**
  * @brief  File system demo operation
  * @param  void
  * @note   This demo will show how to operate file system on ASR1603 based module, such as write\read\seek ,etc
  * @retval void
  */
void FsDemo(void)
{
    SCFILE *file_hdl = NULL;
    SCDIR *dir_hdl = NULL;
    UINT32 ret = 0;
    //char buff[MAX_OUTPUT_LEN] = {0};

    unsigned long buff_data_len = 0;
    UINT32 actul_write_len = 0;
    UINT32 actul_read_len = 0;
    INT64 total_size = 0;
    INT64 free_size = 0;
    INT64 used_size = 0;
    struct dirent_t *info_dir = NULL;
    struct dirent_t info_file = {0};
 
    SIM_MSG_T optionMsg ={0,0,0,NULL};
//    char *dir_name = "D:/";
     char *file_name = "D:/test_file.txt";
    UINT32 opt = 0;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] = {
        "1. Write file",
        "2. Read file",
        "3. Delete file",
        "4. Rename file",
        "5. Get disk size", 
        "6. Make directory",
        "7. Remove directory",
        "8. List file",
        "99. Back",
    };

    while(1)
    {
        ui_print("%s",note);
        PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
        sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
        if(SRV_UART != optionMsg.msg_id)
        {
            sAPI_Debug("%s,msg_id is error!!",__func__);
            break;
        }


        sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
        opt = atoi(optionMsg.arg3);
        sAPI_Debug("opt %d",opt);
        sAPI_Free(optionMsg.arg3);

        switch(opt)
        {
            case SC_FS_DEMO_WRITEFILE:
            {
                char write_str[1000] = {0};
                char fileName[100];
                ui_print("\r\nPlease input fileName\r\n");
                sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                strcpy(fileName,optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);
                ui_print("\r\nPlease input data\r\n");
                sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                strcpy(write_str,optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);
                file_hdl = sAPI_fopen(fileName, "wb+");
                if(file_hdl == NULL)
                {
                    ui_print("open %s fail",fileName);
                    break;
                }
                buff_data_len = strlen(write_str);
                actul_write_len = sAPI_fwrite(write_str,1,buff_data_len ,file_hdl);
                if(actul_write_len != buff_data_len)
                {
                    sAPI_fclose(file_hdl);
                    ui_print("%s write fail [%d]",fileName,actul_write_len);
                    break;
                }
                ret = sAPI_fclose(file_hdl);
                if(ret != 0)
                {
                    ui_print("%s close fail [%d]",fileName,ret);
                    break;
                }
                else
                {
                    file_hdl = NULL;
                }
                break;
            }
            case SC_FS_DEMO_READFILE:
            {
                int position;
                char fileName[100];
                char read_buffer[MAX_OUTPUT_LEN];
                ui_print("\r\nPlease input fileName\r\n");
                sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                strcpy(fileName,optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);
                file_hdl = sAPI_fopen(fileName, "rb");
                if(file_hdl == NULL)
                {
                    ui_print("open %s fail",fileName);
                    break;
                }

                buff_data_len = sAPI_fsize(file_hdl);
                ui_print("sAPI_fsize buff_data_len: %d",  buff_data_len);

                buff_data_len = (buff_data_len <= MAX_OUTPUT_LEN)?buff_data_len:MAX_OUTPUT_LEN;
                memset(read_buffer, 0, MAX_OUTPUT_LEN);
                actul_read_len = sAPI_fread(read_buffer,1, buff_data_len, file_hdl);
                if(actul_read_len <= 0)
                {
                    sAPI_fclose(file_hdl);
                    ui_print("%s read fail [%d]",fileName, actul_read_len);
                    break;
                }
                ui_print("read len: %d,data: %s", actul_read_len,read_buffer);
                sAPI_fseek(file_hdl, 1, 0);
                position = sAPI_ftell(file_hdl);
                ui_print("postion: %d", position);
                memset(read_buffer, 0, MAX_OUTPUT_LEN);
                actul_read_len = sAPI_fread(read_buffer,1, buff_data_len, file_hdl);
                ui_print("read len: %d,data: %s", actul_read_len,read_buffer);
                ret = sAPI_fclose(file_hdl);
                if(ret != 0)
                {
                    ui_print("close %s fail",file_name);
                    break;
                }
                else
                {
                    file_hdl = NULL;
                }
                break;
            }
            case SC_FS_DEMO_DELETEFILE:
            {
                char fileName[100];
                ui_print("\r\nPlease input fileName\r\n");
                sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                strcpy(fileName,optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);
                ret = sAPI_stat(fileName,&info_file);
                if(ret != 0)
                {
                    ui_print("sAPI_stat fail [%d]", ret);
                    break;
                }
                ui_print("[%s]-size[%d]-type[%d]",info_file.name,info_file.size,info_file.type);
                ret = sAPI_remove(fileName);
                if(ret != 0)
                {
                    ui_print("remove %s fail [%d]", fileName,ret);
                    break;
                }
                ui_print("remove %s sucess",fileName);
                ret = sAPI_stat(fileName,&info_file);
                if(ret != 0)
                {
                    ui_print("not find file");
                    break;
                }
                ui_print("[%s]-size[%d]-type[%d]",info_file.name,info_file.size,info_file.type);

                break;
            }
            case SC_FS_DEMO_RENAMEFILE:
            {
                char fileName[100];
                char new_name[100];
                ui_print("\r\nPlease input fileName\r\n");
                sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                strcpy(fileName,optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);
                ui_print("\r\nPlease input new fileName\r\n");
                sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                strcpy(new_name,optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);
                ret = sAPI_stat(fileName,&info_file);
                if(ret != 0)
                {
                    ui_print("sAPI_stat fail [%d]", ret);
                    break;
                }
                ui_print("[%s]-size[%d]-type[%d]",info_file.name,info_file.size,info_file.type);
                ret = sAPI_stat(new_name,&info_file);
                if(ret != 0)
                {
                    ui_print("not find file");
                }
                else
                {
                    ui_print("[%s]-size[%d]-type[%d]",info_file.name,info_file.size,info_file.type);
                    break;
                }
                ret = sAPI_rename(fileName,new_name);
                if(ret != 0)
                {
                    ui_print("rename %s fail [%d]", fileName,ret);
                    break;
                }
                ui_print("rename %s sucess",new_name);
                ret = sAPI_stat(fileName,&info_file);
                if(ret != 0)
                {
                    ui_print("not find file");
                }
                ret = sAPI_stat(new_name,&info_file);
                if(ret != 0)
                {
                    ui_print("sAPI_stat fail [%d]", ret);
                    break;
                }
                ui_print("[%s]-size[%d]-type[%d]",info_file.name,info_file.size,info_file.type);

                break;
            }
            case SC_FS_DEMO_GETDISKSIZE:
            {
                int disk = 0;
                ui_print("\r\nPlease input disk:0 is C:/ and 1 is D:/\r\n");
                sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                disk = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);
                if(disk)
                {
                    total_size = sAPI_GetSize("D:/");
                    free_size = sAPI_GetFreeSize("D:/");
                    used_size = sAPI_GetUsedSize("D:/");
                }
                else
                {
                    total_size = sAPI_GetSize(cur_path_in);
                    free_size = sAPI_GetFreeSize(cur_path_in);
                    used_size = sAPI_GetUsedSize(cur_path_in);
                }
                ui_print("total_size= %lld, free_size= %lld,used_size = %lld", total_size, free_size,used_size);
                
                break;
            }
            case SC_FS_DEMO_MAKEDIR:
            {
                char dir_name[100];
                ui_print("\r\nPlease input dir\r\n");
                sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                strcpy(dir_name,optionMsg.arg3);
                ret = sAPI_mkdir(dir_name,0);
                if(ret != 0)
                {
                    ui_print("creat %s fail [%d]",dir_name,ret);
                    break;
                }
                ui_print("creat %s sucess",dir_name);
                ret = sAPI_stat(dir_name,&info_file);
                if(ret != 0)
                {
                    ui_print("get %s info fail[%d]",dir_name,ret);
                    break;
                }
                ui_print("[%s]-[%ld]-[%d]",info_file.name,info_file.size,info_file.type);
                break;
            }
            case SC_FS_DEMO_REMOVEDIR:
            {
                char dir_name[100];
                ui_print("\r\nPlease input dir\r\n");
                sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                strcpy(dir_name,optionMsg.arg3);
                ret = sAPI_remove(dir_name);
                if(ret != 0)
                {
                    ui_print("delte %s fail [%d]",dir_name,ret);
                    break;
                }
                ui_print("delte %s success",dir_name);
                break;
            }
            case SC_FS_DEMO_OPENDIR:
            {
                char dir_name[100];
                ui_print("\r\nPlease input dir\r\n");
                sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                strcpy(dir_name,optionMsg.arg3);
                dir_hdl = sAPI_opendir(dir_name);
                if(dir_hdl == NULL)
                {
                    ui_print("open %s fail",dir_name);
                    break;
                }
                ui_print("open %s success",dir_name);

                while((info_dir = sAPI_readdir(dir_hdl)) != NULL)
                {
                    ui_print("name[%s]-size[%ld]-type[%d]\r\n",info_dir->name,info_dir->size,info_dir->type);
                }

                ret = sAPI_closedir(dir_hdl);
                if(ret != 0)
                {
                    ui_print("close %s fail[%d]",dir_name,ret);
                    break;
                }
                break;
            }
            case 99:
            {
                return;
            }
            default:
            {
                break;
            }
        }
    }

}

