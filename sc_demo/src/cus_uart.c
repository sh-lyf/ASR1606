/**
  ******************************************************************************
  * @file    cus_uart.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of uart task.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/


#include "simcom_uart.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "string.h"
#include "stdlib.h"

#define UART_RX_BUFFER_SIZE     128 // RX buffer can not more than 2048
#define UART_TX_BUFFER_SIZE     128

extern sMsgQRef simcomUI_msgq;
/**
  * @brief  Send operation message to UI demo task.
  * @param  UartMsg struct message sent to UI demo task.
  * @note   non-blocking
  * @retval void
  */
void sendMsgToUIDemo(SIM_MSG_T UartMsg)
{
    SC_STATUS status = SC_SUCCESS;
    SIM_MSG_T optionMsg = {0,0,0,NULL};
    optionMsg.msg_id = UartMsg.msg_id;
    optionMsg.arg2 = UartMsg.arg2;

    optionMsg.arg3 = malloc(optionMsg.arg2+1);
    memset(optionMsg.arg3, 0, optionMsg.arg2+1);
    memcpy(optionMsg.arg3, UartMsg.arg3, UartMsg.arg2);

    status = sAPI_MsgQSend(simcomUI_msgq,&optionMsg);
    if(status != SC_SUCCESS)
    {
        free(optionMsg.arg3);
        sAPI_Debug("send msg error,status = [%d]",status);
    }
}


/**
  * @brief  Callback for uart1 receiving event, read received data, if define SIMCOM_UI_DEMO_TO_UART1_PORT then send received data to UI demo task.
  * @param  portNumber
  * @param  pointer para
  * @note   Please do not run delay\sleep or blocking API inside this callback.
  * @retval void
  */
void UartCBFunc(SC_Uart_Port_Number portNumber, void *para)
{
    int readLen = 0;
    char *uartData = malloc(UART_RX_BUFFER_SIZE);
#ifndef SIMCOM_A7680C_V5_01
#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
    SIM_MSG_T uartMsg = {0,0,0,NULL};
#endif
#endif

    readLen = sAPI_UartRead(portNumber, (UINT8 *)uartData, UART_RX_BUFFER_SIZE);
    sAPI_Debug("%s, portNumber is %d, readLen[%d].",__func__, portNumber, readLen);

#ifndef SIMCOM_A7680C_V5_01
#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
    uartMsg.msg_id = SRV_UART;
    uartMsg.arg2 = readLen;
    uartMsg.arg3 = uartData;
    sendMsgToUIDemo(uartMsg);
#endif
#endif

    free((void*)uartData);
    return;
}

/**
  * @brief  Extension Callback for uart1 receiving event, read fixed length of data, if define SIMCOM_UI_DEMO_TO_UART1_PORT then send received data to UI demo task.
  * @param  portNumber
  * @param  len for data to receive
  * @param  pointer para
  * @note   Please do not run delay\sleep or blocking API inside this callback.
  * @retval void
  */
void UartCBFuncEx(SC_Uart_Port_Number portNumber, int len, void *para)
{
    int readLen = 0;
    char *uartData = malloc(len);
#ifndef SIMCOM_A7680C_V5_01
#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
    SIM_MSG_T uartMsg = {0,0,0,NULL};
#endif
#endif

    readLen = sAPI_UartRead(portNumber, (UINT8 *)uartData, len);
    sAPI_Debug("%s, portNumber is %d, readLen[%d].",__func__, portNumber, readLen);

#ifndef SIMCOM_A7680C_V5_01
#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
    uartMsg.msg_id = SRV_UART;
    uartMsg.arg2 = readLen;
    uartMsg.arg3 = uartData;
    sendMsgToUIDemo(uartMsg);
#endif
#endif

    free((void*)uartData);

    return;
}

/**
  * @brief  Callback for uart2 receiving event, read received data.
  * @param  portNumber
  * @param  pointer para
  * @note   Please do not run delay\sleep or blocking API inside this callback.
  * @retval void
  */
void Uart2CBFunc(SC_Uart_Port_Number portNumber, void *para)
{
    int readLen = 0;
    UINT8 *uart2data = malloc(UART_RX_BUFFER_SIZE);

    readLen = sAPI_UartRead(portNumber, uart2data, UART_RX_BUFFER_SIZE);
    sAPI_Debug("%s, portNumber is %d, readLen[%d].",__func__, portNumber, readLen);

    free((void*)uart2data);
    return;
}

/**
  * @brief  Extension Callback for uart2 receiving event, read fixed length of data.
  * @param  portNumber
  * @param  len for data to receive
  * @param  pointer para
  * @note   Please do not run delay\sleep or blocking API inside this callback.
  * @retval void
  */
void Uart2CBFuncEx(SC_Uart_Port_Number portNumber, int len, void *para)
{
    int readLen = 0;
    UINT8 *uart2data = malloc(len);

    readLen = sAPI_UartRead(portNumber, uart2data, len);
    sAPI_Debug("%s, portNumber is %d, readLen[%d].",__func__, portNumber, readLen);

    free((void*)uart2data);
    return;
}

/**
  * @brief  Callback for uart3 receiving event, read received data.
  * @param  portNumber
  * @param  pointer para
  * @note   Please do not run delay\sleep or blocking API inside this callback.
  * @retval void
  */
void Uart3CBFunc(SC_Uart_Port_Number portNumber, void *reserve)
{
    int readLen = 0;
    UINT8 *uart3data = malloc(UART_RX_BUFFER_SIZE);

    readLen = sAPI_UartRead(portNumber, uart3data, UART_RX_BUFFER_SIZE);
    sAPI_Debug("%s, portNumber is %d, readlen[%d].",__func__, portNumber, readLen);

    free((void*)uart3data);
    return;
}

/**
  * @brief  Extension Callback for uart3 receiving event, read fixed length of data.
  * @param  portNumber
  * @param  len for data to receive
  * @param  pointer para
  * @note   Please do not run delay\sleep or blocking API inside this callback.
  * @retval void
  */
void Uart3CBFuncEx(SC_Uart_Port_Number portNumber, int len, void *reserve)
{
    int readLen = 0;
    UINT8 *uart3data = malloc(len);

    readLen = sAPI_UartRead(portNumber, uart3data, len);
    sAPI_Debug("%s, portNumber is %d, readlen[%d].",__func__, portNumber, readLen);

    free((void*)uart3data);
    return;
}

void Uart4CBFunc(SC_Uart_Port_Number portNumber, void *para)
{
#if (defined SIMCOM_A7680C_V5_01) || (defined SIMCOM_A7670C_V7_01)
    int readLen = 0;
    char *uart4Data = malloc(UART_RX_BUFFER_SIZE);

#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
    SIM_MSG_T uart4Msg = {0,0,0,NULL};
#endif

    readLen = sAPI_UartRead(portNumber, (UINT8 *)uart4Data, UART_RX_BUFFER_SIZE);
    sAPI_Debug("%s, portNumber is %d, readLen[%d].",__func__, portNumber, readLen);

#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
    uart4Msg.msg_id = SRV_UART;
    uart4Msg.arg2 = readLen;
    uart4Msg.arg3 = uart4Data;
    sendMsgToUIDemo(uart4Msg);
#endif

    free((void*)uart4Data);
#endif
    return;
}

void Uart4CBFuncEx(SC_Uart_Port_Number portNumber, int len, void *para)
{
#if (defined SIMCOM_A7680C_V5_01) || (defined SIMCOM_A7670C_V7_01)
    int readLen = 0;
    char *uart4Data = malloc(len);
#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
    SIM_MSG_T uart4Msg = {0,0,0,NULL};
#endif


    readLen = sAPI_UartRead(portNumber, (UINT8 *)uart4Data, len);
    sAPI_Debug("%s, portNumber is %d, readLen[%d].",__func__, portNumber, readLen);


#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
    uart4Msg.msg_id = SRV_UART;
    uart4Msg.arg2 = readLen;
    uart4Msg.arg3 = uart4Data;
    sendMsgToUIDemo(uart4Msg);
#endif


    free((void*)uart4Data);
#endif
    return;
}

/**
  * @brief  Configure 3 Uarts and register related callback.
  * @param  void
  * @note   SC_UART is from UART1 of module, it will be the CLI interface for the SIMCom UI demo
  * @retval void
  */
void sAPP_UartTask(void)
{
    SCuartConfiguration uartConfig,uart2Config,uart3Config,uart4Config;

    /*************************Configure UART again*********************************/
    /*******The user can modify the initialization configuratin of UART in here.***/ 
    /******************************************************************************/
    uartConfig.BaudRate  =  SC_UART_BAUD_115200;
    uartConfig.DataBits  =  SC_UART_WORD_LEN_8;
    uartConfig.ParityBit =  SC_UART_NO_PARITY_BITS;
    uartConfig.StopBits  =  SC_UART_ONE_STOP_BIT;
    if(sAPI_UartSetConfig(SC_UART,&uartConfig) == SC_UART_RETURN_CODE_ERROR)
    {
        sAPI_Debug("%s: Configure UART failure!!",__func__);
    }

    /*************************Configure UART2 again*********************************/
    /*******The user can modify the initialization configuratin of UART2 in here.***/
    /*******************************************************************************/
    uart2Config.BaudRate  =  SC_UART_BAUD_115200;
    uart2Config.DataBits  =  SC_UART_WORD_LEN_8;
    uart2Config.ParityBit =  SC_UART_NO_PARITY_BITS;
    uart2Config.StopBits  =  SC_UART_ONE_STOP_BIT;
    if(sAPI_UartSetConfig(SC_UART2,&uart2Config) == SC_UART_RETURN_CODE_ERROR)
    {
        sAPI_Debug("%s: Configure UART2 failure!!",__func__);
    }

    /*************************Configure UART3 again*********************************/
    /*******The user can modify the initialization configuratin of UART3 in here.***/ 
    /*******************************************************************************/
    uart3Config.BaudRate  =  SC_UART_BAUD_115200;
    uart3Config.DataBits  =  SC_UART_WORD_LEN_8;
    uart3Config.ParityBit =  SC_UART_NO_PARITY_BITS;
    uart3Config.StopBits  =  SC_UART_ONE_STOP_BIT;
    if(sAPI_UartSetConfig(SC_UART3,&uart3Config) == SC_UART_RETURN_CODE_ERROR)
    {
        sAPI_Debug("%s: Configure UART3 failure!!",__func__);
    }

    /*************************Configure UART4 again*********************************/
    /*******The user can modify the initialization configuratin of UART4 in here.***/ 
    /*******************************************************************************/
    uart4Config.BaudRate  =  SC_UART_BAUD_115200;
    uart4Config.DataBits  =  SC_UART_WORD_LEN_8;
    uart4Config.ParityBit =  SC_UART_NO_PARITY_BITS;
    uart4Config.StopBits  =  SC_UART_ONE_STOP_BIT;
    if(sAPI_UartSetConfig(SC_UART4,&uart4Config) == SC_UART_RETURN_CODE_ERROR)
    {
        sAPI_Debug("%s: Configure UART4 failure!!",__func__);
    }

    sAPI_Debug("%s: UART Configuration is complete!!\n",__func__);


    sAPI_UartRegisterCallback(SC_UART,  UartCBFunc);
    sAPI_UartRegisterCallback(SC_UART2, Uart2CBFunc);
    sAPI_UartRegisterCallbackEX(SC_UART3, Uart3CBFuncEx, (void *)"Uart3CBFuncEx");
    sAPI_UartRegisterCallback(SC_UART4,  Uart4CBFunc);

    sAPI_Debug("%s Task creation completed!!\n",__func__);
}


