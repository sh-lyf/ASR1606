#ifdef FEATURE_SIMCOM_TTS
#include "simcom_tts_api.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

#define FALSE 0
#define TRUE 1

typedef enum{
    SC_TTS_DEMO_PLAY                 = 1,
    SC_TTS_DEMO_STOP                 = 2,
    SC_TTS_DEMO_PARA                 = 3,

    SC_AUDIO_DEMO_MAX                  = 99
}SC_TTS_DEMO_TYPE;

extern SIM_MSG_T GetParamFromUart(void);
extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(char *format);
extern sMsgQRef simcomUI_msgq;

void TTSDemo(void)
{
    SIM_MSG_T optionMsg = { 0 };
    char text[512] = { 0 };
    UINT32 opt, option, mode, volume, tts_volume, digit_mode, pitch, speed = 0;
    BOOL ret = FALSE;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] ={
        "1. Play",
        "2. Stop",
        "3. SetPara",
        "99. back",
    };

    while(1)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
        sAPI_MsgQRecv(simcomUI_msgq, &optionMsg, SC_SUSPEND);
        if (SRV_UART != optionMsg.msg_id)
        {
            sAPI_Debug("%s, msg_id is error!!!", __func__);
            break;
        }

        sAPI_Debug("arg3 = [%s]", optionMsg.arg3);
        opt = atoi(optionMsg.arg3);
        sAPI_Free(optionMsg.arg3);

        switch(opt)
        {
            case SC_TTS_DEMO_PLAY:
            {
                PrintfResp("\r\nPlease option.(1.UNICODE,2.ASCII)\r\n");
                optionMsg = GetParamFromUart();
                option = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);
                sAPI_Debug("sAPI_TTSPlay: option is %d ",option);

                PrintfResp("\r\nPlease input TTS text.\r\n");
                optionMsg = GetParamFromUart();
                strcpy(text,optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);
                sAPI_Debug("sAPI_TTSPlay: text is %s ",text);

                PrintfResp("\r\nPlease input play mode.(0.local,1.remote)\r\n");
                optionMsg = GetParamFromUart();
                mode = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);
                sAPI_Debug("sAPI_TTSPlay: playMode is %d ",mode);

                ret = sAPI_TTSPlay(option,text, mode);
                if(ret)
                    sAPI_Debug("sAPI_TTSPlay: playing  success ");
                else
                    sAPI_Debug("sAPI_TTSPlay: playing  failed");
                break;
            }

            case SC_TTS_DEMO_STOP:
            {
                ret = sAPI_TTSStop();
                if(ret)
                    sAPI_Debug("sAPI_TTSStop: stop success ");
                else
                    sAPI_Debug("sAPI_TTSStop: stop failed");
                break;
            }

            case SC_TTS_DEMO_PARA:
            {
                PrintfResp("\r\nPlease input system volme.\r\n");
                optionMsg = GetParamFromUart();
                volume = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                PrintfResp("\r\nPlease input tts volume.\r\n");
                optionMsg = GetParamFromUart();
                tts_volume = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                PrintfResp("\r\nPlease input digit mode.\r\n");
                optionMsg = GetParamFromUart();
                digit_mode = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                PrintfResp("\r\nPlease input pitch.\r\n");
                optionMsg = GetParamFromUart();
                pitch = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                PrintfResp("\r\nPlease input speed.\r\n");
                optionMsg = GetParamFromUart();
                speed = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                ret = sAPI_TTSSetParameters(volume, tts_volume, digit_mode,pitch,speed);
                if(ret){
                    PrintfResp("\r\nSet parameters success\r\n");
                    sAPI_Debug("sAPI_TTSSetParameters: set success");
                }
                else{
                    PrintfResp("\r\nSet parameters faild\r\n");
                    sAPI_Debug("sAPI_TTSSetParameters: set failed");
                }
                break;
            }

            case SC_AUDIO_DEMO_MAX:
            {
                return;
            }
        }


    }
}
#endif