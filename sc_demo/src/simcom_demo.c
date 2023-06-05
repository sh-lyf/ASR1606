/**
  ******************************************************************************
  * @file    simcom_demo.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source code for all OpenSDK demo task management with UI.
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
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "simcom_uart.h"
#include "simcom_debug.h"
#include "simcom_common.h"

#ifdef FEATURE_SIMCOM_MQTT
sMsgQRef urc_mqtt_msgq_1;
#endif

typedef enum
{
    SC_DEMO_FOR_NETWORK             =  1,  //API test for network
    SC_DEMO_FOR_SIMCARD             =  2,  //API test for SIM Card
    SC_DEMO_FOR_SMS                 =  3,  //API test for SMS
    SC_DEMO_FOR_UART                =  4,  //API test for UART
    SC_DEMO_FOR_USB                 =  5,  //API test for USB
    SC_DEMO_FOR_GPIO                =  6,  //API test for GPIO
    SC_DEMO_FOR_PMU                 =  7,  //API test for PMU
    SC_DEMO_FOR_I2C                 =  8,  //API test for I2C
#ifdef FEATURE_SIMCOM_AUDIO
    SC_DEMO_FOR_AUDIO               =  9,  //API test for audio
#endif
    SC_DEMO_FOR_FILE_SYSTEM         =  10,  //API test for File System
    SC_DEMO_FOR_TCP_IP              =  11,  //API test for TCP/IP
    SC_DEMO_FOR_HTTP_HTTPS          =  12,  //API test for HTTP(s)
#ifdef FEATURE_SIMCOM_FTPS
    SC_DEMO_FOR_FTP_FTPS            =  13,  //API test for FTP(s)
#endif
#ifdef FEATURE_SIMCOM_MQTT
    SC_DEMO_FOR_MQTT_MQTTS          =  14,  //API test for MQTT(s)
#endif
#ifdef FEATURE_SIMCOM_MSSL
    SC_DEMO_FOR_SSL                 =  15,  //API test for SSL
#endif
    SC_DEMO_FOR_OTA                 =  16,  //API test for OTA
    SC_DEMO_FOR_LBS                 =  17,  //API test for LBS
#ifdef FEATURE_SIMCOM_NTP
    SC_DEMO_FOR_NTP                 =  18,  //API test for NTP
#endif
#ifdef FEATURE_SIMCOM_HTP
    SC_DEMO_FOR_HTP                 =  19,  //API test for HTP
#endif
    SC_DEMO_FOR_INTERNET_SERVICE    =  20,  //API test for Internet service
#ifdef FEATURE_SIMCOM_TTS
    SC_DEMO_FOR_TTS                 =  21,  //API test for TTS
#endif
    SC_DEMO_FOR_CALL                =  22,  //API test for CALL
    SC_DEMO_FOR_WIFI                =  23,  //API test for WIFI
#if defined(FEATURE_SIMCOM_GPS) || defined(JACANA_GPS_SUPPORT)
    SC_DEMO_FOR_GNSS                =  24,  //API test for GNSS
    SC_DEMO_FOR_LCD                 =  25,  //API test for LCD
#else
    SC_DEMO_FOR_LCD                 =  24,  //API test for LCD
#endif
    SC_DEMO_FOR_RTC                 =  26,  //API test for RTC
    SC_DEMO_FOR_FLASH               =  27,  //API test for flash
#ifdef FEATURE_SIMCOM_FS_OLD
    SC_DEMO_FOR_FILE_SYSTEM_OLD     =  28,  //API test for File System of 1601
#endif
    SC_DEMO_FOR_SPI                 =  29,  //API test for SPI
    SC_DEMO_FOR_CAM                 =  30,  //API test for CAM
#ifdef BT_SUPPORT
    SC_DEMO_FOR_BLE                 =  31,  //API test for BLE
#endif
    SC_DEMO_FOR_SPI_NOR             =  32,  //API test for SPI
    SC_DEMO_FOR_APP_DOWNLOAD        =  33,  //API test for APP DOWNLOAD
    SC_DEMO_FOR_APP_UPDATE_FOR_NVM  =  34,  //API test for APP UPDATE FOR NVM
    SC_DEMO_FOR_PWM                 =  35,  //API test for PWM
#ifdef FEATURE_SIMCOM_POC
    SC_DEMO_FOR_POC                 =  36,  //API test for POC
#endif
/*begin added byxiaobing.fang for jira-A76801606-1884 20221027	*/
    SC_DEMO_FOR_WTD                 =  37,  //API test for WTD
/*end added byxiaobing.fang for jira-A76801606-1884 20221027	*/

}SC_DEMO_TYPE;


sMsgQRef simcomUI_msgq;
sTaskRef simcomUIProcesser;
#ifdef FEATURE_SIMCOM_MQTT
extern void MqttDemo(void);
#endif
#ifdef FEATURE_SIMCOM_NTP
extern void NtpDemo(void);
#endif
#ifdef FEATURE_SIMCOM_HTP
extern void HtpDemo(void);
#endif
extern void NetWorkDemo(void);
extern void FsDemo(void);
extern void TcpipDemo(void);
#if 0
extern void NtpDemo(void);
extern void HtpDemo(void);
extern void FsDemo(void);
extern void CALLDemo(void);
extern void WIFIDemo(void);
extern void Fs2Demo(void);
#endif
#ifdef SIMCOM_LCD_SUPPORT
extern void LcdDemo(void);
#endif
#ifdef SIMCOM_CAMERA_SUPPORT
extern void CamDemo(void);
#endif
#ifdef FEATURE_SIMCOM_AUDIO
extern void AudioDemo(void);
#endif
#ifdef FEATURE_SIMCOM_TTS
extern void TTSDemo(void);
#endif
extern void FlashRWdemo(void);
extern void SpiDemo(void);
extern void SpiNorDemo(void);
extern void GNSSDemo(void);
extern void UartDemo(void);
extern void I2cDemo(void);
extern void GpioDemo(void);
#ifdef FEATURE_SIMCOM_CALL
extern void CALLDemo(void);
#endif
extern void SMSDemo(void);
extern void SimcardDemo(void);
extern void PMUDemo(void);
extern void FotaDemo(void);
extern void SslDemo(void);
extern void AppDownloadDemo(void);
extern void FtpsDemo(void);
extern void HttpsDemo(void);
extern void AppUpdateDemo(void);
extern void PwmDemo(void);
extern void WIFIDemo(void);
#ifdef BT_SUPPORT
extern void BLEDemo(void);
#endif
#ifdef FEATURE_SIMCOM_POC
extern void POCDemo(void);
#endif
extern void LbsDemo(void);
extern void RTCDemo(void);
/*begin added byxiaobing.fang for jira-A76801606-1884 20221027  */
extern void WTDDemo(void);
/*end added byxiaobing.fang for jira-A76801606-1884 20221027  */

/**
  * @brief  Print string to UART1 or USB AT port.
  * @param  format,data pointer
  * @note   Please define SIMCOM_UI_DEMO_TO_UART1_PORT or SIMCOM_UI_DEMO_TO_USB_AT_PORT in advance.
  * @retval void
  */
void PrintfResp(char* format)
{
    UINT32 length = strlen(format);
#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
#if (defined SIMCOM_A7680C_V5_01) || (defined SIMCOM_A7670C_V7_01)
    sAPI_UartWrite(SC_UART4,(UINT8*)format,length);
#else
    sAPI_UartWrite(SC_UART,(UINT8*)format,length);
#endif
#else
    sAPI_UsbVcomWrite((UINT8*)format,length);
#endif
}

/**
  * @brief  Print operation menu.
  * @param  options_list,operation list
  * @param  array_size,size of options_list
  * @note   Please define SIMCOM_UI_DEMO_TO_UART1_PORT or SIMCOM_UI_DEMO_TO_USB_AT_PORT in advance.
  * @retval void
  */
void PrintfOptionMenu(char* options_list[], int array_size)
{
    UINT32 i = 0;
    sAPI_Debug("array_size = [%d]",array_size);
    char menu[80] = {0};
    PrintfResp("\r\n************************************************************\r\n");
    for(i = 0;i < (array_size/2);i++)
    {
        memset(menu, 0, 80);
        snprintf(menu, 80, "%-30s%-30s", options_list[2*i], options_list[2*i+1]);
        PrintfResp(menu);
        PrintfResp("\r\n");
    }

    if(array_size%2 != 0)
    {
        memset(menu, 0, 80);
        snprintf(menu, 80, "%s", options_list[array_size-1]);
        PrintfResp(menu);
        PrintfResp("\r\n");
    }
    PrintfResp("************************************************************\r\n");

}

/**
  * @brief  Get input operation parameter from UI.
  * @param  void
  * @note   Blocking API,suspend until there is data received from UI shell(UART).
  * @retval void
  */
SIM_MSG_T GetParamFromUart(void)
{
    SIM_MSG_T optionMsg ={0,0,0,NULL};
    sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);

    return optionMsg;
}

/**
  * @brief  SIMCom UI demo processer.
  * @param  arg
  * @note   Please select demo according to CLI.
  * @retval void
  */
void sTask_SimcomUIProcesser(void * arg)
{
    SIM_MSG_T optionMsg ={0,0,0,NULL};
    UINT32 opt = 0;
    char *note = "Please select an option to test from the items listed below.\n";
    char *options_list[] = {
    "1. NETWORK",
    "2. SIMCARD",
    "3. SMS",
    "4. UART",
#ifndef AT_COMMAND_SUPPORT
    "5. USB",
#endif
    "6. GPIO",
    "7. PMU",
    "8. I2C",
#ifdef FEATURE_SIMCOM_AUDIO
    "9. AUDIO",
#endif
    "10. FILE SYSTEM",
    "11. TCPIP",
    "12. HTTP",
    "13. FTP",
    "14. MQTT",
    "15. SSL",
    "16. FOTA",
    "17. LBS",
#ifdef FEATURE_SIMCOM_NTP
    "18. NTP",
#endif
#ifdef FEATURE_SIMCOM_HTP
    "19. HTP",
#endif
    "20. INTERNET SERVICE",
#ifdef FEATURE_SIMCOM_TTS
    "21. TTS",
#endif
    "22. CALL",
    "23. WIFI",
#if defined(FEATURE_SIMCOM_GPS) || defined(JACANA_GPS_SUPPORT)
    "24. GNSS",
    "25. LCD",
#else
    "24. LCD",
#endif
    "26. RTC",
    "27. FLASH",

#ifdef FEATURE_SIMCOM_FS_OLD
    "28. FILE SYSTEM Compatible API",
#endif
    "29. SPI",
    "30. CAM",
#ifdef BT_SUPPORT
    "31. BLE",
#endif
    "32. SPI NOR",
    "33. APP DOWNLOAD",
    "34. APP UPDATE FOR NVM",
    "35. PWM",
#ifdef FEATURE_SIMCOM_POC
    "36. POC",
#endif
    "37.WTD"
    };

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

        sAPI_Debug("arg3 = [%p]",optionMsg.arg3);
        opt = atoi(optionMsg.arg3);
        sAPI_Free(optionMsg.arg3);

        switch(opt)
        {
            case SC_DEMO_FOR_NETWORK:
                sAPI_Debug("Come to the NetWork demo!");
                NetWorkDemo();
                break;

            case SC_DEMO_FOR_SIMCARD:
                SimcardDemo();
                PrintfResp("Come to the SIMCARD demo\r\n");
                break;

            case SC_DEMO_FOR_SMS:
                sAPI_Debug("Come to the SMS demo!");
                SMSDemo();
                break;

            case SC_DEMO_FOR_UART:
                sAPI_Debug("Come to the Uart demo!");
                UartDemo();
                break;
#ifndef AT_COMMAND_SUPPORT
            case SC_DEMO_FOR_USB:
                PrintfResp("\r\nThere's no UI demo for USB.\r\n");
                break;
#endif
            case SC_DEMO_FOR_GPIO:
                sAPI_Debug("Come to the GPIO demo!");
                GpioDemo();
                break;

            case SC_DEMO_FOR_PMU:
                PrintfResp("\r\nCome to the PMU demo!\r\n");
				PMUDemo();
                break;

            case SC_DEMO_FOR_I2C:
                sAPI_Debug("Come to the I2C demo!");
                I2cDemo();
                break;
#ifdef FEATURE_SIMCOM_AUDIO
            case SC_DEMO_FOR_AUDIO:
                sAPI_Debug("Come to the FileSystem demo!");
                AudioDemo();
                break;
#endif
            case SC_DEMO_FOR_FILE_SYSTEM:
                sAPI_Debug("Come to the FileSystem demo!");
                FsDemo();
                break;

            case SC_DEMO_FOR_TCP_IP:
                PrintfResp("Come to the Tcpip demo!");
                TcpipDemo();
                break;

#ifdef FEATURE_SIMCOM_HTTP
            case SC_DEMO_FOR_HTTP_HTTPS:
                sAPI_Debug("Come to the Http demo!");
                HttpsDemo();
                break;
#endif
#ifdef FEATURE_SIMCOM_FTPS
            case SC_DEMO_FOR_FTP_FTPS:
                sAPI_Debug("Come to the Ftp demo!");
                FtpsDemo();
                break;
#endif
#ifdef FEATURE_SIMCOM_MQTT
            case SC_DEMO_FOR_MQTT_MQTTS:
                sAPI_Debug("Come to the MQTT demo!");
                MqttDemo();
                break;
#endif
#ifdef FEATURE_SIMCOM_MSSL
            case SC_DEMO_FOR_SSL:
                sAPI_Debug("Come to the ssl demo!");
                SslDemo();
                break;
#endif

            case SC_DEMO_FOR_OTA:
                PrintfResp("\r\nCome to FOTA demo.\r\n");
                FotaDemo();
                break;

            case SC_DEMO_FOR_LBS:
                sAPI_Debug("Come to the LBS demo!");
                LbsDemo();
                break;
#ifdef FEATURE_SIMCOM_NTP
            case SC_DEMO_FOR_NTP:
                sAPI_Debug("Come to the NTP demo!");
                NtpDemo();
                break;
#endif
#ifdef FEATURE_SIMCOM_HTP
            case SC_DEMO_FOR_HTP:
                sAPI_Debug("Come to the HTP demo!");
                HtpDemo();
                break;
#endif
#ifdef FEATURE_SIMCOM_TTS
            case SC_DEMO_FOR_TTS:
                sAPI_Debug("Come to the TTS demo!");
                TTSDemo();
                break;
#endif
            case SC_DEMO_FOR_CALL:
                #ifdef FEATURE_SIMCOM_CALL
                sAPI_Debug("Come to the CALL demo!");
                CALLDemo();
                #endif
                break;

            case SC_DEMO_FOR_WIFI:
                sAPI_Debug("Come to the WIFI demo!");
                WIFIDemo();
                break;
#if defined(FEATURE_SIMCOM_GPS) || defined(JACANA_GPS_SUPPORT)
            case SC_DEMO_FOR_GNSS:
                sAPI_Debug("Come to the GNSS demo!");
                GNSSDemo();
                break;

            case SC_DEMO_FOR_LCD:
                sAPI_Debug("Come to the LCD demo!");
#ifdef SIMCOM_LCD_SUPPORT
                LcdDemo();
#endif
                break;
#else
            case SC_DEMO_FOR_LCD:
                sAPI_Debug("Come to the LCD demo!");
#ifdef SIMCOM_LCD_SUPPORT
                LcdDemo();
#endif
                break;
#endif
            case SC_DEMO_FOR_RTC:
                sAPI_Debug("Come to the RTC demo!");
                RTCDemo();
                break;
            case SC_DEMO_FOR_FLASH:
                sAPI_Debug("Come to the FLASH demo!");
                FlashRWdemo();
				break;
#ifdef FEATURE_SIMCOM_FS_OLD
            case SC_DEMO_FOR_FILE_SYSTEM_OLD:
                sAPI_Debug("Come to the FileSystem Compatible API demo!");
                //Fs2Demo();
                break;
#endif
            case SC_DEMO_FOR_SPI:
                sAPI_Debug("Come to the SPI demo!");
                SpiDemo();
                break;
            case SC_DEMO_FOR_CAM:
                sAPI_Debug("Come to the CAM demo!");
#ifdef SIMCOM_CAMERA_SUPPORT
                CamDemo();
#endif
                break;
#ifdef BT_SUPPORT
            case SC_DEMO_FOR_BLE:
                BLEDemo();
                break;
#endif
            case SC_DEMO_FOR_SPI_NOR:
                sAPI_Debug("Come to the SPI Nor flash demo!");
                SpiNorDemo();
                break;

            case SC_DEMO_FOR_APP_DOWNLOAD:
                sAPI_Debug("Come to the app download demo!");
                AppDownloadDemo();
                break;

            case SC_DEMO_FOR_APP_UPDATE_FOR_NVM:
                sAPI_Debug("Come to the app update for nvm demo!");
                AppUpdateDemo();
                break;

            case SC_DEMO_FOR_PWM:
                sAPI_Debug("Come to the app pwm demo!");
                PwmDemo();
                break;

#ifdef FEATURE_SIMCOM_POC
            case SC_DEMO_FOR_POC:
                sAPI_Debug("Come to the app poc demo!");
                POCDemo();
                break;
#endif
/*begin added byxiaobing.fang for jira-A76801606-1884 20221027	*/
            case SC_DEMO_FOR_WTD:
                sAPI_Debug("Come to the WTD demo!");
                WTDDemo();
                break;
/*end added byxiaobing.fang for jira-A76801606-1884 20221027	*/

            default :
                break;
        }
    }
}

/**
  * @brief  Create SIMCom UI demo task.
  * @param  void
  * @note   UI demo based on message(queue) with blocking method.
  * @retval void
  */
void sAPP_SimcomUIDemo(void)
{
    SC_STATUS status;
    status = sAPI_MsgQCreate(&simcomUI_msgq, "simcomUI_msgq", sizeof(SIM_MSG_T), 12, SC_FIFO);
    if(SC_SUCCESS != status)
    {
        sAPI_Debug("msgQ create fail");
    }
#ifdef FEATURE_SIMCOM_MQTT
	status = sAPI_MsgQCreate(&urc_mqtt_msgq_1, "urc_mqtt_msgq_1", (sizeof(SIM_MSG_T)), 4, SC_FIFO);        //msgQ for subscribed data transfer
     if(status != SC_SUCCESS)
    {
        sAPI_Debug("message queue creat err!\n");
    }
    printf("%s,sTask_SimcomUIProcesser",__func__);
#endif

    void *simcomUIProcesserStack = (void *)malloc(30 * 1024);
    if (!simcomUIProcesserStack)
    {
        printf("malloc simcomUIProcesserStack fail!");
        return;
    }
    status = sAPI_TaskCreate(&simcomUIProcesser,simcomUIProcesserStack,1024 * 30,100,"simcomUIProcesser",sTask_SimcomUIProcesser,(void *)0);
    if(SC_SUCCESS != status)
    {
        sAPI_Debug("task create fail");
    }
}

