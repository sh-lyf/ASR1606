/**
  ******************************************************************************
  * @file    demo_wifi.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of wifi scan funtion,it is not full function wifi, just wifi RX for nearby hotpot scaning, mostly for wifi location.
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
#include "simcom_wifi.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "simcom_uart.h"

enum
{
    SC_WIFI_DEMO_START_SCANNING           = 1,
    SC_WIFI_DEMO_STOP_SCANNING            = 2,
    SC_WIFI_DEMO_MAX                      = 99
};

extern sMsgQRef simcomUI_msgq;
extern void PrintfOptionMenu(char* options_list[], int array_size);
extern void PrintfResp(char* format);

/**
  * @brief  Event handler for wifi scan.
  * @param  param
  * @note   Will output scaned MAC\channel number\rssi.
  * @retval void
  */
static void wifi_handle_event(const SC_WIFI_EVENT_T *param)
{
    char rspBuf[128];

    switch (param->type)
    {
        case SC_WIFI_EVENT_SCAN_INFO:
            snprintf(rspBuf, sizeof(rspBuf),
                "\r\nscan result: mac address(%02x:%02x:%02x:%02x:%02x:%02x), channel(%d), rssi(%d).\r\n",
                param->info.mac_addr[5], param->info.mac_addr[4], param->info.mac_addr[3],
                param->info.mac_addr[2], param->info.mac_addr[1], param->info.mac_addr[0],
                param->info.channel_number, param->info.rssi);
            break;
        case SC_WIFI_EVENT_SCAN_STOP:
            snprintf(rspBuf, sizeof(rspBuf), "\r\nscan stop.\r\n");
            break;
        default:
            snprintf(rspBuf, sizeof(rspBuf), "\r\ninvalid wifi event.\r\n");
    }

    PrintfResp((char *)rspBuf);
}

/**
  * @brief  UI demo for WIFI scan.
  * @param  void
  * @note   
  * @retval void
  */
void WIFIDemo(void)
{
    char flag = 1;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] = {
       "1. Start scanning",
       "2. Stop scanning",
       "99. back",
    };

    while (flag)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list, sizeof(options_list) / sizeof(options_list[0]));

        SIM_MSG_T optionMsg;
        sAPI_MsgQRecv(simcomUI_msgq, &optionMsg, SC_SUSPEND);
        if (SRV_UART != optionMsg.msg_id)
        {
            sAPI_Debug("%s,msg_id is error!!", __func__);
            break;
        }

        unsigned char opt = atoi(optionMsg.arg3);
        sAPI_Free(optionMsg.arg3);

        switch (opt)
        {
            case SC_WIFI_DEMO_START_SCANNING:
            {
                sAPI_WifiSetHandler(wifi_handle_event);
                sAPI_WifiScanStart();
                PrintfResp("\r\nPlease wait a moment, scanning...\r\n");
                break;
            }

            case SC_WIFI_DEMO_STOP_SCANNING:
            {
                sAPI_WifiScanStop();
                PrintfResp("\r\nPlease wait a moment, stopping...\r\n");
                break;
            }

            case SC_WIFI_DEMO_MAX:
            {
                flag = 0;
                PrintfResp("\r\nReturn to the previous menu!\r\n");
                break;
            }

            default:
                PrintfResp("\r\nPlease select again:\r\n");
        }
    }
}
