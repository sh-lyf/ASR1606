/**
  ******************************************************************************
  * @file    demo_helloworld.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of helloworld.
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
#include "simcom_debug.h"
#include "stdio.h"


sTaskRef helloWorldProcesser;
static UINT8 helloWorldProcesserStack[1024];
#if 0
/**
  * @brief  simcom printf
  * @param  pointer *format
  * @note   The message will be output by sAPI_Debug and captured by CATSTUDIO tool.
  * @retval void
  */
void simcom_printf(const char *format,...){
    char tmpstr[200];

    va_list args;

    memset(tmpstr,0,sizeof(tmpstr));

    va_start(args,format);
    sAPI_Vsnprintf(tmpstr,sizeof(tmpstr),format,args);
    va_end(args);

    sAPI_Debug("simcom_printf [%s]",tmpstr);

}
#endif
/**
  * @brief  helloworld task processor
  * @param  pointer *argv
  * @note   
  * @retval void
  */
void sTask_HelloWorldProcesser(void* argv)
{
    sAPI_Debug("Task runs successfully");

    //simcom_printf("%s  %d %f","simcom",2020,10.23);

}

/**
  * @brief  helloworld task initial
  * @param  void
  * @note   
  * @retval void
  */
void sAPP_HelloWorldDemo(void)
{
    SC_STATUS status = SC_SUCCESS;

    status = sAPI_TaskCreate(&helloWorldProcesser, helloWorldProcesserStack, 1024, 150, "helloWorldProcesser",sTask_HelloWorldProcesser,(void *)0);
    if(SC_SUCCESS != status)
    {
        sAPI_Debug("Task create fail,status = [%d]",status);
    }
}
