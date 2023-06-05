/*begin added byxiaobing.fang for jira-A76801606-1884 20221027  */

#include "simcom_common.h"
#include "simcom_os.h"
#include "simcom_system.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "simcom_wtd.h"
#include "simcom_debug.h"

//#include "simcom_api.h"



extern SIM_MSG_T GetParamFromUart(void);
extern sMsgQRef simcomUI_msgq;
extern void PrintfOptionMenu(char* options_list[], int array_size);
extern void PrintfResp(char* format);


enum SC_WTD_OPS
{
    SC_WTD_ENABLE = 0,
    SC_WTD_DISABLE,
    SC_WTD_FEED,
    SC_WTD_BACK = 99
};

void  WTDDemo(void)
{
    SIM_MSG_T optionMsg ={0,0,0,NULL};
    UINT32 opt = 0;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] = {
        "0. wtd open",
        "1. wtd close",
        "2. feed dog",
        "99 back ",
    };
    int ret = 1;
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

        opt = atoi(optionMsg.arg3);
        sAPI_Free(optionMsg.arg3);

        switch(opt)
            {
            case SC_WTD_ENABLE:
                {

                    ret = sAPI_SetWtdTimeOutPeriod(0x04);//feed dog 16S Period

                    ret |=sAPI_FalutWakeEnable(1);

                    ret |=sAPI_SoftWtdEnable(1);

                    ret |=sAPI_FeedWtd(); //After starting the WDT,the dog needs to be fed once before the timeout set by the register 0x11 will load

                    if(ret){
                        PrintfResp("\r\n open watchdog opration failed\r\n");
                        }else{
                            PrintfResp("\r\n open watchdog opration successful!\r\n");
                    }
                break;
            }
            case SC_WTD_DISABLE:
                {
                    ret  = sAPI_FalutWakeEnable(0);
                    ret |= sAPI_SoftWtdEnable(0);
                    if(ret){
                        PrintfResp("\r\n open watchdog opration failed\r\n");
                    }else{
                        PrintfResp("\r\n open watchdog opration successful!\r\n");
                    }
                break;
            }
            case SC_WTD_FEED:
                {
                    ret = sAPI_FeedWtd();
                    if(ret){
                        PrintfResp("\r\n open watchdog opration failed\r\n");
                    }else{
                        PrintfResp("\r\n open watchdog opration successful!\r\n");
                    }
                break;
            }
            case SC_WTD_BACK:
                {
                return;
            }
        }

    }

}
/*end added byxiaobing.fang for jira-A76801606-1884 20221027  */



