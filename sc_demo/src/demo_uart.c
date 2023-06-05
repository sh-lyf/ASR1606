/**
  ******************************************************************************
  * @file    demo_uart.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of uart operation.
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
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "simcom_debug.h"
#include "simcom_uart.h"
#include "simcom_common.h"
#include "simcom_system.h"


#define uart_config_size (256)
#define frame_format_buf_size    (30)
#if (defined SIMCOM_A7680C_V5_01) || (defined SIMCOM_A7670C_V7_01)
#define SC_UART    SC_UART4
#else
#define SC_UART    SC_UART
#endif

typedef enum{
    SC_UART_DEMO_SET_BAUDRATE      = 1,
    SC_UART_DEMO_SET_DATABITS      = 2,
    SC_UART_DEMO_SET_PARITYBIT     = 3,
    SC_UART_DEMO_SET_STOPBITS      = 4,
    SC_UART_DEMO_GET_CONFIGRATION  = 5,
    SC_UART_DEMO_GET_RX_STATUS     = 6,
    SC_UART_DEMO_SET_SLEEP         = 7,
    SC_UART_DEMO_GET_SLEEP_STATUS  = 8,
    SC_UART_DEMO_OPEN_CLOSE        = 9,
    SC_UART_DEMO_ALARM_TO_WAKEUP   = 10,
    SC_UART_DEMO_INTERNAL_AT_CMD   = 11,
    SC_UART_DEMO_RS485_DE_PIN_ASSIGN =12,
    SC_UART_DEMO_SET_SLEEPEX =13,
    SC_UART_DEMO_GET_SLEEPEX_STATUS =14,


    SC_UART_DEMO_MAX           = 99
}SC_UART_DEMO_TYPE;

typedef enum{
    SC_UART_DEMO_BAUD_4800      = 1,
    SC_UART_DEMO_BAUD_9600      = 2,
    SC_UART_DEMO_BAUD_19200     = 3,
    SC_UART_DEMO_BAUD_38400     = 4,
    SC_UART_DEMO_BAUD_57600     = 5,
    SC_UART_DEMO_BAUD_115200    = 6,
    SC_UART_DEMO_BAUD_BACK      = 99,

    SC_UART_DEMO_BAUD_MAX

}SC_UART_DEMO_BAUD_RATE;

typedef enum{
    SC_UART_DEMO_DATABITS_5      = 1,
    SC_UART_DEMO_DATABITS_6      = 2,
    SC_UART_DEMO_DATABITS_7      = 3,
    SC_UART_DEMO_DATABITS_8      = 4,
    SC_UART_DEMO_DATABITS_BACK   = 99,

    SC_UART_DEMO_DATABITS_MAX
}SC_UART_DEMO_DATA_BITS;

typedef enum{
    SC_UART_DEMO_PARITYBIT_NO        = 1,
    SC_UART_DEMO_PARITYBIT_ODD       = 2,
    SC_UART_DEMO_PARITYBIT_EVEN      = 3,
    SC_UART_DEMO_PARITYBIT_BACK      = 99,

    SC_UART_DEMO_PARITYBITS_MAX
}SC_UART_DEMO_PARITY_BITS;

typedef enum{
    SC_UART_DEMO_STOPBITS_ONE                   = 1,
    SC_UART_DEMO_STOPBITS_ONE_HALF_OR_TWO       = 2,
    SC_UART_DEMO_STOPBITS_BACK                  = 99,

    SC_UART_DEMO_STOPBITS_MAX
}SC_UART_DEMO_STOP_BITS;

typedef enum  /* The status of Rx */
{
    SC_UART_DEMO_RX_IDEL                       = 0,
    SC_UART_DEMO_RX_BUSY                       = 1,
    SC_UART_DEMO_RX_STATUS_MAX                 = -1,

    SC_UART_DEMO_RX_STATUS_BACK                = 99,
}SC_UART_DEMO_RX_STATUS;

typedef enum
{
    SC_UART_DEMO_OPEN             = 1,
    SC_UART_DEMO_CLOSE            = 2,
}SC_UART_DEMO_CONTROL;

extern sMsgQRef simcomUI_msgq;
SCuartConfiguration config={SC_UART_BAUD_115200,SC_UART_WORD_LEN_8,SC_UART_ONE_STOP_BIT,SC_UART_NO_PARITY_BITS};
extern void PrintfOptionMenu(char* options_list[], int array_size);
extern void PrintfResp(char* format);
extern SIM_MSG_T GetParamFromUart(void);

/**
  * @brief  UI Demo.
  * @param  void
  * @note   Please select operation id accodring to index.
  * @retval void
  */
void UartDemo(void)
{
    SCuartConfiguration getconfig;
    SC_UART_DEMO_CONTROL input;
    SC_Uart_Control ctrl;
    SIM_MSG_T optionMsg ={0,0,0,NULL};
    UINT32 opt = 0;
    unsigned int gpioNum;
    SC_UartRs485DeActiveLevel DeActiveLevel;
    SC_Uart_Port_Number portNumber;
    SC_GPIOReturnCode ret = 0;
    int retResult;
    UINT8 baudRateOpt = 0;
    UINT8 dataBitOpt = 0;
    UINT8 parityBitOpt = 0;
    UINT8 stopBitOpt = 0;
    UINT8 rxStatus;
    char configbuf[uart_config_size];
    char paritybitbuf[frame_format_buf_size];
    char stopbitbuf[frame_format_buf_size];
    char databitbuf[frame_format_buf_size];
    char *note = "\r\nPlease select an option to test from the items listed below, demo just for UART.\r\n";
    char *options_list[] = {
        "1. Set baud rate",
        "2. Set data bits",
        "3. Set parity bit",
        "4. Set stop bits",
        "5. Get UART configration",
        "6. Get UART status",
        "7. Set sleep",
        "8. Get sleep status",
        "9. UART open/close",
        "10. alarm clock to wakeup",
        "11. Internal AT command",
        "12. RS485 DE pin assign",
        "13. Set SleepEx",
        "14. Get SleepEx",
        "99. back",
    };


    while(1)
    {
       PrintfResp(note);
       PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
reSelect:
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
            case SC_UART_DEMO_SET_BAUDRATE:
            {
                PrintfResp("\r\nPlease select baud rate:\r\n");
reSelectBaudRate:
                PrintfResp("\r\n1.4800  2.9600  3.19200  4.38400  5.57600  6.115200  99.back\r\n");
                sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                if(SRV_UART != optionMsg.msg_id)
                {
                    sAPI_Debug("%s,msg_id is error!!",__func__);
                    break;
                }
                baudRateOpt = atoi(optionMsg.arg3);
                free(optionMsg.arg3);
                switch(baudRateOpt)
                {
                    case SC_UART_DEMO_BAUD_4800:
                        config.BaudRate = SC_UART_BAUD_4800;
                        break;
                    case SC_UART_DEMO_BAUD_9600:
                        config.BaudRate = SC_UART_BAUD_9600;
                        break;
                    case SC_UART_DEMO_BAUD_19200:
                        config.BaudRate = SC_UART_BAUD_19200;
                        break;
                    case SC_UART_DEMO_BAUD_38400:
                        config.BaudRate = SC_UART_BAUD_38400;
                        break;
                    case SC_UART_DEMO_BAUD_57600:
                        config.BaudRate = SC_UART_BAUD_57600;
                        break;
                    case SC_UART_DEMO_BAUD_115200:
                        config.BaudRate = SC_UART_BAUD_115200;
                        break;
                    case SC_UART_DEMO_BAUD_BACK:
                        break;
                    default:
                        PrintfResp("\r\nPlease select baud rate again:\r\n");
                        goto reSelectBaudRate;
                        break;

                }
                PrintfResp(note);
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                if(sAPI_UartSetConfig(SC_UART,&config) == SC_UART_RETURN_CODE_ERROR)
                {
                   PrintfResp("\r\nSet baud rate failure.\r\n");
                   sAPI_Debug("%s: Configure baud rate failure!!",__func__);
                }
                goto reSelect;
                break;
            }
            case SC_UART_DEMO_SET_DATABITS:
                PrintfResp("\r\nPlease select data bits:\r\n");
reSelectDataBit:
                PrintfResp("\r\n1.5bits  2.6bits  3.7bits  4.8bits  99.back\r\n");
                sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                if(SRV_UART != optionMsg.msg_id)
                {
                    sAPI_Debug("%s,msg_id is error!!",__func__);
                    break;
                }
                dataBitOpt = atoi(optionMsg.arg3);
                free(optionMsg.arg3);
                switch(dataBitOpt)
                {
                    case SC_UART_DEMO_DATABITS_5:
                        config.DataBits = SC_UART_WORD_LEN_5;
                        break;
                    case SC_UART_DEMO_DATABITS_6:
                        config.DataBits = SC_UART_WORD_LEN_6;
                        break;
                    case SC_UART_DEMO_DATABITS_7:
                        config.DataBits = SC_UART_WORD_LEN_7;
                        break;
                    case SC_UART_DEMO_DATABITS_8:
                        config.DataBits = SC_UART_WORD_LEN_8;
                        break;
                    case SC_UART_DEMO_DATABITS_BACK:
                        break;
                    default:
                        PrintfResp("\r\nPlease select data bits again:\r\n");
                        goto reSelectDataBit;
                        break;

                }
                PrintfResp(note);
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                if(sAPI_UartSetConfig(SC_UART,&config) == SC_UART_RETURN_CODE_ERROR)
                {
                   PrintfResp("\r\nSet data bits failure.\r\n");
                   sAPI_Debug("%s: Configure data bits failure!!",__func__);
                }
                goto reSelect;
                break;
            case SC_UART_DEMO_SET_PARITYBIT:
                PrintfResp("\r\nPlease select parity bit:\r\n");
reSelectParityBit:
                PrintfResp("\r\n1.no parity bit  2.odd parity bit  3.even parity bit  99.back\r\n");
                sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                if(SRV_UART != optionMsg.msg_id)
                {
                    sAPI_Debug("%s,msg_id is error!!",__func__);
                    break;
                }
                parityBitOpt = atoi(optionMsg.arg3);
                free(optionMsg.arg3);
                switch(parityBitOpt)
                {
                    case SC_UART_DEMO_PARITYBIT_NO:
                        config.ParityBit = SC_UART_NO_PARITY_BITS;
                        break;
                    case SC_UART_DEMO_PARITYBIT_EVEN:
                        config.ParityBit = SC_UART_EVEN_PARITY_SELECT;
                        break;
                    case SC_UART_DEMO_PARITYBIT_ODD:
                        config.ParityBit = SC_UART_ODD_PARITY_SELECT;
                        break;
                    case SC_UART_DEMO_PARITYBIT_BACK:
                        break;
                    default:
                        PrintfResp("\r\nPlease select parity bits again:\r\n");
                        goto reSelectParityBit;
                        break;

                }
                PrintfResp(note);
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                if(sAPI_UartSetConfig(SC_UART,&config) == SC_UART_RETURN_CODE_ERROR)
                {
                   PrintfResp("\r\nSet parity bit failure.\r\n");
                   sAPI_Debug("%s: Configure parity bit failure!!",__func__);
                }
                goto reSelect;
                break;

            case SC_UART_DEMO_SET_STOPBITS:
                PrintfResp("\r\nPlease select stop bits:\r\n");
reSelectStopBit:
                PrintfResp("\r\n1. 1bits  2. 1.5bits or 2bits  99.back\r\n");
                sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                if(SRV_UART != optionMsg.msg_id)
                {
                    sAPI_Debug("%s,msg_id is error!!",__func__);
                    break;
                }
                stopBitOpt = atoi(optionMsg.arg3);
                free(optionMsg.arg3);
                switch(stopBitOpt)
                {
                    case SC_UART_DEMO_STOPBITS_ONE:
                        config.StopBits = SC_UART_ONE_STOP_BIT;
                        break;
                    case SC_UART_DEMO_STOPBITS_ONE_HALF_OR_TWO:
                        config.StopBits = SC_UART_ONE_HALF_OR_TWO_STOP_BITS;
                        break;
                    case SC_UART_DEMO_STOPBITS_BACK:
                        break;
                    default:
                        PrintfResp("\r\nPlease select stop bits again:\r\n");
                        goto reSelectStopBit;
                        break;

                }
                PrintfResp(note);
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                if(sAPI_UartSetConfig(SC_UART,&config) == SC_UART_RETURN_CODE_ERROR)
                {
                   PrintfResp("\r\nSet stop bits failure.\r\n");
                   sAPI_Debug("%s: Configure stop bits failure!!",__func__);
                }
                goto reSelect;
                break;

            case SC_UART_DEMO_GET_CONFIGRATION:
            {
                PrintfResp("\r\nThe configration of UART:\r\n");
                if(sAPI_UartGetConfig(SC_UART,&getconfig) == SC_UART_RETURN_CODE_ERROR)
                {
                    PrintfResp("\r\nGet configration failure.\r\n");
                    sAPI_Debug("%s: Get configration failure.",__func__);
                    break;
                }
                switch(getconfig.DataBits)
                {
                     case SC_UART_WORD_LEN_5:
                        strcpy(databitbuf,"5bits");
                        break;
                    case SC_UART_WORD_LEN_6:
                        strcpy(databitbuf,"6bits");
                        break;
                    case SC_UART_WORD_LEN_7:
                        strcpy(databitbuf,"7bits");
                        break;
                    case SC_UART_WORD_LEN_8:
                        strcpy(databitbuf,"8bits");
                    default:
                        break;
                }
                switch(getconfig.ParityBit)
                {
                    case SC_UART_NO_PARITY_BITS:
                        strcpy(paritybitbuf,"no parity");
                        break;
                    case SC_UART_EVEN_PARITY_SELECT:
                        strcpy(paritybitbuf,"even parity");
                        break;
                    case SC_UART_ODD_PARITY_SELECT:
                        strcpy(paritybitbuf,"odd parity");
                        break;
                    default:
                        break;
                }
                switch(getconfig.StopBits)
                {
                    case SC_UART_ONE_STOP_BIT:
                        strcpy(stopbitbuf, "1bits");
                        break;
                    case SC_UART_ONE_HALF_OR_TWO_STOP_BITS:
                        strcpy(stopbitbuf, "1.5bits or 2bits");
                        break;
                    default:
                        break;
                }
                memset(configbuf, 0, sizeof(configbuf));
                snprintf(configbuf, sizeof(configbuf),"\r\nbaud rate is: %d\r\ndata bits is: %s\r\nparity bit is: %s\r\nstop bits is: %s\r\n",  \
                    (int)getconfig.BaudRate,databitbuf,paritybitbuf,stopbitbuf);
                PrintfResp(configbuf);

                break;
            }

            case SC_UART_DEMO_GET_RX_STATUS:
            {
                rxStatus = (SC_UART_DEMO_RX_STATUS)sAPI_UartRxStatus(SC_UART3,50);
                switch (rxStatus)
                {
                    case SC_UART_DEMO_RX_IDEL:
                        PrintfResp("\r\nUART3 is idel.\r\n");
                        break;
                    case SC_UART_DEMO_RX_BUSY:
                        PrintfResp("\r\nUART3 is busy.\r\n");
                        break;
                    default:
                        PrintfResp("\r\nPort is invalid.\r\n");
                        break;
                }
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                goto reSelect;
                break;
            }
            case SC_UART_DEMO_SET_SLEEP:
            {
                PrintfResp("\r\nPlease pull down the DTR pin to wake up the module.\r\n");
                PrintfResp("\r\nPlease unplug the USB to put the module into sleep mode.\r\n");
                if((SC_Uart_Return_Code)sAPI_SystemSleepSet(SC_SYSTEM_SLEEP_ENABLE) == SC_UART_RETURN_CODE_ERROR)
                {
                    PrintfResp("\r\nSet sleep mode error.\r\n");
                }
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                goto reSelect;
                break;
            }
            case SC_UART_DEMO_GET_SLEEP_STATUS:
            {
                char tmp[40];
                unsigned char sleepStatus;

                sleepStatus = sAPI_SystemSleepGet();
                sprintf(tmp,"\r\nSleep status is %d \r\n", sleepStatus);
                PrintfResp(tmp);
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                goto reSelect;
                break;
            }
            case SC_UART_DEMO_SET_SLEEPEX:
            {
                PrintfResp("\r\nPlease pull down the DTR pin to wake up the module.\r\n");
                PrintfResp("\r\nPlease unplug the USB to put the module into sleep mode.\r\n");
                if((SC_Uart_Return_Code)sAPI_SystemSleepExSet(SC_SYSTEM_SLEEP_ENABLE,3) == SC_UART_RETURN_CODE_ERROR)
                {
                    PrintfResp("\r\nSet sleep mode error.\r\n");
                }
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                goto reSelect;
                break;
            }
            case SC_UART_DEMO_GET_SLEEPEX_STATUS:
            {
            #if 0
                char tmp[40];
                SC_SleepEx_str sleepExStatus;

                sleepExStatus = sAPI_SystemSleepExGet();
                sprintf(tmp,"\r\nSleep status is %d,%d \r\n", sleepExStatus.sleep_flag, sleepExStatus.time);
                PrintfResp(tmp);
            #endif
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                goto reSelect;
                break;
            }
            case SC_UART_DEMO_OPEN_CLOSE:
            {
                PrintfResp("\r\nOnly valid for UART3:\r\n");
reSelectCtrl:
                PrintfResp("\r\n1. open  2. close\r\n");
                sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                if(SRV_UART != optionMsg.msg_id)
                {
                    sAPI_Debug("%s,msg_id is error!!",__func__);
                    break;
                }
                input = atoi(optionMsg.arg3);
                free(optionMsg.arg3);
                switch(input)
                {
                    case SC_UART_DEMO_OPEN:
                        ctrl = SC_UART_OPEN;
                        break;
                    case SC_UART_DEMO_CLOSE:
                        ctrl = SC_UART_CLOSE;
                        break;
                    default:
                        PrintfResp("\r\nPlease select again:\r\n");
                        goto reSelectCtrl;
                        break;

                }
                PrintfResp(note);
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                if(sAPI_UartControl(SC_UART3,ctrl) == SC_UART_RETURN_CODE_ERROR)
                {
                   PrintfResp("\r\nControl UART3 failed.\r\n");
                   sAPI_Debug("%s: Control UART3 failed!",__func__);
                }
                goto reSelect;
                break;
            }
            case SC_UART_DEMO_ALARM_TO_WAKEUP:
            {
                PrintfResp("\r\nPlease pull down the DTR pin to wake up the module.\r\n");
                PrintfResp("\r\nPlease unplug the USB to put the module into sleep mode.\r\n");
                if((SC_Uart_Return_Code)sAPI_SystemAlarmClock2Wakeup(120000) == SC_UART_RETURN_CODE_ERROR)
                {
                    PrintfResp("\r\nSet sleep mode error.\r\n");
                    sAPI_Debug("%s: Set sleep mode error!!",__func__);
                }
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                goto reSelect;
                break;
            }

            case SC_UART_DEMO_INTERNAL_AT_CMD:
            {
                /*Channel 10 is TEL_AT_CMD_ATP_10, user is recommended to use channel 10*/
                PrintfResp("\r\nInternal AT command send: AT+CPIN?\r\n");
                char respStr[20];
                retResult = sAPI_SendATCMDWaitResp(10, "AT+CPIN?\r", 150, "+CPIN", 1, NULL, respStr, sizeof(respStr));
                {
                    if(retResult != 0)
                    {
                        PrintfResp("\r\nSend AT CMD error.\r\n");
                        sAPI_Debug("%s: Send AT CMD error!!, Status = %d",__func__,retResult);
                    }

                }
                PrintfResp(respStr);
                PrintfResp("\r\n");
                return;
            }

            case SC_UART_DEMO_RS485_DE_PIN_ASSIGN:
            {
                PrintfResp("\r\nPlease input uart number.\r\n");
                PrintfResp("\r\n1:SC_UART  2:SC_UART2  3:SC_UART3\r\n");

                optionMsg = GetParamFromUart();
                portNumber = atoi(optionMsg.arg3);
                free(optionMsg.arg3);

                if(portNumber < SC_UART || portNumber > SC_UART3)
                {
                    PrintfResp("\r\nincorrect uart port.\r\n");
                    break;
                }

                PrintfResp("\r\nPlease input gpio number.\r\n");
                optionMsg = GetParamFromUart();
                gpioNum = atoi(optionMsg.arg3);
                free(optionMsg.arg3);

                PrintfResp("\r\nPlease input DE active level.\r\n");
                optionMsg = GetParamFromUart();
                DeActiveLevel = atoi(optionMsg.arg3);
                free(optionMsg.arg3);

                if(DeActiveLevel < SC_UART_RS485_DE_LOW_LEVEL || DeActiveLevel > SC_UART_RS485_DE_HIGH_LEVEL)
                {
                    PrintfResp("\r\nincorrect DE active level.\r\n");
                    break;
                }

                ret = sAPI_UartRs485DePinAssignEx(portNumber, gpioNum, DeActiveLevel);
                if(ret != SC_GPIORC_OK)
                    sAPI_Debug("sAPI_UartRs485DePinAssignEx failed.");
                else
                    sAPI_Debug("sAPI_UartRs485DePinAssignEx success.");

                PrintfResp("\r\noperation successful!\r\n");
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                goto reSelect;
                break;
            }

            case SC_UART_DEMO_MAX:
            {
                sAPI_Debug("Return to the previous menu!");
                PrintfResp("\r\nReturn to the previous menu!\r\n");
                return;
            }

            default:
                PrintfResp("\r\nPlease select again:\r\n");
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                goto reSelect;
                break;
       }
    }
}
