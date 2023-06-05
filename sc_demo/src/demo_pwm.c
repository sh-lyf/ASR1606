#include "simcom_pwm.h"
#include "simcom_debug.h"
#include "simcom_os.h"
#include "simcom_common.h"
#include "stdlib.h"

extern sMsgQRef simcomUI_msgq;
extern void PrintfResp(char* format);
extern void PrintfOptionMenu(char* options_list[], int array_size);

typedef enum{
    SC_PWM_DEMO_OPEN       = 1,
    SC_PWM_DEMO_CLOSE      = 2,
    SC_PWM_DEMO_MAX        = 99
}SC_PWM_DEMO_TYPE;

typedef enum{
    SC_PWM_DEMO_DEVICE_1      = 1,
    SC_PWM_DEMO_DEVICE_2      = 2,
    SC_PWM_DEMO_DEVICE_3      = 3,
    SC_PWM_DEMO_DEVICE_4      = 4,
    SC_PWM_DEMO_DEVICE_BACK   = 99,

    SC_PWM_DEVICE_MAX

}SC_PWM_DEMO_DEVICE_NUM;

/**
  * @brief  PWM operation demo
  * @param  void
  * @note
  *      ---------------------------theory calculation formula----------------------
  *                          period = frq_div*total_cnt/32000       (s)             
  *                          frequency = 32000/(frq_div*total_cnt)  (Hz)            
  *                          duty = high_level_cnt/total_cnt                        
  *                                                                                 
  *                                                                                 
  * @retval void
  */
void PwmDemo(void)
{
#if 1
    SIM_MSG_T optionMsg ={0,0,0,NULL};
    UINT32 opt = 0;
    UINT8 pwmDeviceNum = 0;
    char *note = "\r\nPlease select an option to test from the items listed below, demo just for PWM.\r\n";
    char *options_list[] = {
        "1. Open pwm",
        "2. Close pwm",
        "99. back",
    };

    sAPI_Debug("%s, enter",__func__);
    while (1)
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
            case SC_PWM_DEMO_OPEN:
            {
                //sAPI_PWMConfig(SC_PWM_DEVICE_1,PWM_ON,64,7,14);
                PrintfResp("\r\nPlease choose which PWM to enable:\r\n");
reSelectPwmOpenDevice:
                PrintfResp("\r\n1.PWM1  2.PWM2  3.PWM3  4.PWM4  99.back\r\n");
                sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                if(SRV_UART != optionMsg.msg_id)
                {
                    sAPI_Debug("%s,msg_id is error!!",__func__);
                    break;
                }
                pwmDeviceNum = atoi(optionMsg.arg3);
                free(optionMsg.arg3);
                sAPI_Debug("%s,pwmDeviceNum = %d",__func__,pwmDeviceNum);
                switch(pwmDeviceNum)
                {
                    case SC_PWM_DEMO_DEVICE_1:
                        sAPI_PWMConfig(SC_PWM_DEVICE_1,PWM_ON,64,7,14); //PWM1: 34.15 Hz in test and 35.71 Hz in theory value
                        break;
                    case SC_PWM_DEMO_DEVICE_2:
                        sAPI_PWMConfig(SC_PWM_DEVICE_2,PWM_ON,64,7,14); //PWM2: 34.15 Hz in test and 35.71 Hz in theory value
                        break;
                    case SC_PWM_DEMO_DEVICE_3:
                        sAPI_PWMConfig(SC_PWM_DEVICE_3,PWM_ON,64,7,14); //PWM3: 34.15 Hz in test and 35.71 Hz in theory value
                        break;
                    case SC_PWM_DEMO_DEVICE_4:
                        sAPI_PWMConfig(SC_PWM_DEVICE_4,PWM_ON,64,7,14); //PWM4: 34.15 Hz in test and 35.71 Hz in theory value
                        break;
                    case SC_PWM_DEMO_DEVICE_BACK:
                        break;
                    default:
                        PrintfResp("\r\nPlease choose which PWM to enable:\r\n");
                        goto reSelectPwmOpenDevice;
                        break;
                }
                PrintfResp(note);
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                goto reSelect;
                break;
            }

            case SC_PWM_DEMO_CLOSE:
            {
                PrintfResp("\r\nPlease choose which PWM to close:\r\n");
reSelectPwmCloseDevice:
                PrintfResp("\r\n1.PWM1  2.PWM2  3.PWM3  4.PWM4  99.back\r\n");
                sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                if(SRV_UART != optionMsg.msg_id)
                {
                    sAPI_Debug("%s,msg_id is error!!",__func__);
                    break;
                }
                pwmDeviceNum = atoi(optionMsg.arg3);
                free(optionMsg.arg3);
                switch(pwmDeviceNum)
                {
                    case SC_PWM_DEMO_DEVICE_1:
                        sAPI_PWMConfig(SC_PWM_DEVICE_1,PWM_OFF,64,7,14); //PWM1: 34.15 Hz in test and 35.71 Hz in theory value
                        break;
                    case SC_PWM_DEMO_DEVICE_2:
                        sAPI_PWMConfig(SC_PWM_DEVICE_2,PWM_OFF,64,7,14); //PWM2: 34.15 Hz in test and 35.71 Hz in theory value
                        break;
                    case SC_PWM_DEMO_DEVICE_3:
                        sAPI_PWMConfig(SC_PWM_DEVICE_3,PWM_OFF,64,7,14); //PWM3: 34.15 Hz in test and 35.71 Hz in theory value
                        break;
                    case SC_PWM_DEMO_DEVICE_4:
                        sAPI_PWMConfig(SC_PWM_DEVICE_4,PWM_OFF,64,7,14); //PWM4: 34.15 Hz in test and 35.71 Hz in theory value
                        break;
                    case SC_PWM_DEMO_DEVICE_BACK:
                        break;
                    default:
                        PrintfResp("\r\nPlease choose which PWM to close:\r\n");
                        goto reSelectPwmCloseDevice;
                        break;
                }
                PrintfResp(note);
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                goto reSelect;
                break;
            }

            case SC_PWM_DEMO_MAX:
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
#else
    /**************** pwm configuration method 1 ****************/
    /*In this way, you can choose to configure the clock source.*/
    /************************************************************/
    /************************************************************/
    SC_PWM_CONFIG pwmDev;

    /* set pwm channel */
    pwmDev.pwm_channel = SC_PWM_DEVICE_1;
    /*set pwm switch*/
    pwmDev.pwm_switch = PWM_ON;
    /*set pwm clock*/
    pwmDev.pwm_clock = PWM_CLK_SRC_13M;
    /*set pwm clock frequency divisor*/
    pwmDev.frq_div = 64;
    /*set pwm high_level_cnt*/
    pwmDev.high_level_cnt = 7;
    /*set pwm total_cnt*/
    pwmDev.total_cnt = 14;

    if(PWM_RC_OK != sAPI_PWMConfigEx(&pwmDev)) //PWM1: 16.1 KHz in test and 14.5 KHz in theory value
    {
        sAPI_Debug("%s,PWM config init fail!",__func__);
    }
    else
    {
        sAPI_Debug("%s,PWM config init successful!",__func__);
    }
    /**************** pwm configuration method 2 ****************/
    /*In this way, you can only set the clock source to 32K as the default*/
    /************************************************************/
    /************************************************************/
    //sAPI_PWMConfig(SC_PWM_DEVICE_1,PWM_ON,2,5,10); //PWM1: 1.49 KHz in test and 1.6 KHz in theory value
    //sAPI_PWMConfig(SC_PWM_DEVICE_2,PWM_ON,64,7,14); //PWM2: 34.15 Hz in test and 35.71 Hz in theory value
    return;
#endif
}