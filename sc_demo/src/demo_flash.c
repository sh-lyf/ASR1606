/**
  ******************************************************************************
  * @file    demo_flash.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of flash operation.
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
#include "sc_flash.h"
#include "simcom_os.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define SECTOR_1 0
#define SECTOR_2 4096
#define ERASE_SIZE 4096

typedef enum
{
    SC_FLASH_ERASE = 1,
    SC_FLASH_WRITE,
    SC_FLASH_READ,
    SC_FLASH_READ_APP,
    SC_FLASH_TYPE,
    SC_FLASH_NVM_ERASE,
    SC_FLASH_NVM_WRITE,
    SC_FLASH_NVM_READ,
    SC_FLAHS_BACK = 99,
}SC_FLASH_OPS;


typedef struct
{
    int a;
    int b;
    int c;
    int d;
    int e;
    int f;
}flash_test;


static flash_test ftest = { 1,2,3,4,5,6};
#if 0

/**
  * @brief  Flash read\write demo
  * @param  void
  * @note   Before write please erase sector
  * @retval void
  */
void sAPP_FlashRWdemo(void)
{
    int ret;
    char tmp[50] = {0};
    ret = sAPI_EraseFlashSector(SECTOR_1,ERASE_SIZE);

    if(ret != 0)
    {
        sprintf(tmp,"\r\nerase error %d\r\n",ret);
        PrintfResp(tmp);
        return;
    }

    ret = sAPI_EraseFlashSector(SECTOR_2,ERASE_SIZE);

    if(ret != 0) return;

    for(int i = 0; i < 10; i++)
    {
        ret = sAPI_WriteFlash(SECTOR_1 + i * sizeof(flash_test), (INT8*)&ftest,sizeof(flash_test));
        if(ret != 0) return;
    }

    flash_test fread_test[10];
    for(int i = 0; i < 10; i++)
    {
        ret = sAPI_ReadFlash(SECTOR_1 + i * sizeof(flash_test), (INT8*)&fread_test[i],sizeof(flash_test));
        if(ret != 0) return;
        int rls = memcmp(&ftest,&fread_test[i],sizeof(flash_test));
        sprintf(tmp,"\r\ncmp %s\r\n",rls==0?"ok":"err");
        PrintfResp(tmp);

    }

// ----------------------------- error condition --------------------------------------------------- 

    //size < 0
    ret = sAPI_WriteFlash(SECTOR_2, (INT8*)&ftest,-5);
    if(ret != 0)
    {
        memset(tmp, 0, 50);
        sprintf(tmp,"\r\n[flash demo]find error ret is %d\r\n",ret);
        PrintfResp(tmp);
    }
    //out of 8K range
    ret = sAPI_WriteFlash(SECTOR_2, (INT8*)&ftest,4097);
    if(ret != 0)
    {
        memset(tmp, 0, 50);
        sprintf(tmp,"\r\n[flash demo]find error ret is %d\r\n",ret);
        PrintfResp(tmp);
    }

    //pointer is NULL
    ret = sAPI_WriteFlash(SECTOR_2, 0, 10);
    if(ret != 0)
    {
        memset(tmp, 0, 50);
        sprintf(tmp,"\r\n[flash demo]find error ret is %d\r\n",ret);
        PrintfResp(tmp);
    }

    //offset is not 4K alligned
    ret = sAPI_EraseFlashSector(2077,ERASE_SIZE);
    if(ret != 0)
    {
        memset(tmp, 0, 50);
        sprintf(tmp,"\r\n[flash demo]find error ret is %d\r\n",ret);
        PrintfResp(tmp);
    }


}
#endif

extern SIM_MSG_T GetParamFromUart(void);
extern sMsgQRef simcomUI_msgq;
extern void PrintfOptionMenu(char * options_list[], int array_size);
extern void PrintfResp(char* format);

/**
  * @brief  Flash read\write\erase operation demo
  * @param  void
  * @note   
  * @retval void
  */
void FlashRWdemo(void)
{
    SIM_MSG_T optionMsg ={0,0,0,NULL};
    UINT32 opt = 0;
    char* note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char* options_list[] = {
        "1. flash erase sector",
        "2. flash write data",
        "3. flash read data",
        "4. flash read app",
        "5. flash read cus_data and customer_app size",
        "6. NVM partition erase data",
        "7. NVM partition write data",
        "8. NVM partition read data",
        "99. back",
    };
    int ret = 0;
    char tmp[50] = {0};
    // char appBuff[5] = {0};
    while(1)
    {
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
            case SC_FLASH_ERASE:
            {
                sAPI_Debug("flash erase sector");
                ret = sAPI_EraseFlashSector(SECTOR_1,ERASE_SIZE);

                if(ret != 0) 
                {
                    sprintf(tmp,"\r\nerase error %d\r\n",ret);
                    PrintfResp(tmp);
                    return;
                }
                break;
            }
            case SC_FLASH_WRITE:
            {
                sAPI_Debug("flash write");

                for(int i = 0; i < 10; i++)
                {
                    ret = sAPI_WriteFlash(SECTOR_1 + i * sizeof(flash_test), (char*)&ftest,sizeof(flash_test));
                    if(ret != 0) return;
                }
                break;
            }
            case SC_FLASH_READ:
            {
                sAPI_Debug("flash read");

                flash_test fread_test[10];

                for(int i = 0; i < 10; i++)
                {
                    ret = sAPI_ReadFlash(SECTOR_1 + i * sizeof(flash_test), (char*)&fread_test[i],sizeof(flash_test));
                    if(ret != 0) return;
                    int rls = memcmp(&ftest,&fread_test[i],sizeof(flash_test));
                    sprintf(tmp,"\r\ncmp %s\r\n",rls==0?"ok":"err");
                    PrintfResp(tmp);
                }
                break;
            }
            case SC_FLASH_READ_APP:
            {
                // unsigned char readBuff[128] = {0};
                // unsigned int binSize = 0;

                // binSize = sAPI_AppPartitionBinSize();
                // ret = sAPI_ReadFlashAppPartition(0,(char *)readBuff,sizeof(readBuff));
                // if(ret)
                // {
                //     PrintfResp("read app flash failed");
                // }
                // PrintfResp("\r\n\r\n");
                // for(int i=0;i<128;i++)
                // {
                //     sprintf(appBuff,"%02x  ",readBuff[i]);
                //     PrintfResp(appBuff);
                //     if((i+1) % 10 == 0)
                //     {
                //         PrintfResp("\r\n");
                //     }
                // }
                break;		
            }
            case SC_FLASH_TYPE:
            {
                // char* readBuffptr = NULL;

                // readBuffptr = sAPI_GetFlashType();
                // sprintf(tmp,"FLASH Type:%s",readBuffptr);
                // PrintfResp(tmp);
                // PrintfResp("\r\n\r\n");
                // free(readBuffptr);

                // unsigned int size1 =0;
                // unsigned int size2 =0;
                // long long size3 = 0;
                // long long totalsize = 0;

                // size1 = sAPI_GetCusdataPartitionSize();
                // sprintf(tmp,"cus_data:%d",size1);
                // PrintfResp(tmp);
                // PrintfResp("\r\n\r\n");
                // totalsize += size1;

                // size2 = sAPI_GetAppPartitionSize();
                // sprintf(tmp,"customer_app:%d",size2);
                // PrintfResp(tmp);
                // PrintfResp("\r\n\r\n");
                // totalsize += size2;

                // size3 = sAPI_GetFsSize("C:/");
                // sprintf(tmp,"FsSize:%d",size3);
                // PrintfResp(tmp);
                // PrintfResp("\r\n\r\n");
                // totalsize += size3;

                // size3 = sAPI_GetFsFreeSize("C:/");
                // sprintf(tmp,"FsFreeSize:%d",size3);
                // PrintfResp(tmp);
                // PrintfResp("\r\n\r\n");

                // size3 = sAPI_GetUsedSize("C:/");
                // sprintf(tmp,"FsUsedSize:%d",size3);
                // PrintfResp(tmp);
                // PrintfResp("\r\n\r\n");

                // sprintf(tmp,"Cusdata+customer_app+FsSize=%d",totalsize);
                // PrintfResp(tmp);
                // PrintfResp("\r\n\r\n");
                break;
            }
            case SC_FLASH_NVM_READ:
            {
                // sAPI_Debug("nvm partition read");

                // flash_test fread_test[10];

                // for(int i = 0; i < 10; i++)
                // {
                //     ret = sAPI_ReadFlashNvmPartition(SECTOR_1 + i * sizeof(flash_test), (INT8*)&fread_test[i],sizeof(flash_test));
                //     if(ret != 0) return;
                //     int rls = memcmp(&ftest,&fread_test[i],sizeof(flash_test));
                //     sprintf(tmp,"\r\ncmp %s\r\n",rls==0?"ok":"err");
                //     PrintfResp(tmp);
                // }
                break;
            }
            case SC_FLASH_NVM_ERASE:
            {
                // sAPI_Debug("NVM partition erase sector");
                // ret = sAPI_EraseFlashNvmPartition(SECTOR_1,0x2F0000);

                // if(ret != 0)
                // {
                //     sprintf(tmp,"\r\nerase error %d\r\n",ret);
                //     PrintfResp(tmp);
                //     return;
                // }
                break;
            }
            case SC_FLASH_NVM_WRITE:
            {
                // sAPI_Debug("NVM partition write");

                // for(int i = 0; i < 10; i++)
                // {
                //     ret = sAPI_WriteFlashNvmPartition(SECTOR_1 + i * sizeof(flash_test), (INT8*)&ftest,sizeof(flash_test));
                //     if(ret != 0) return;
                // }
                break;
            }
            case SC_FLAHS_BACK:
                return;
            }
        }

}


