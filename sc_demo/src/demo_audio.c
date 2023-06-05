/**
  ******************************************************************************
  * @file    demo_audio.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of audio operation,this demo will show how to control the audio component including play\record\set audio configuration\set TX\RX gain ,etc.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */
 
/* Includes ------------------------------------------------------------------*/
#ifdef FEATURE_SIMCOM_AUDIO
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "sc_dspgain.h"
#include "simcom_audio.h"
#include "simcom_os.h"
#include "simcom_file.h"
#include "simcom_common.h"
#include "sc_gpio.h"

extern SIM_MSG_T GetParamFromUart(void);
extern sMsgQRef simcomUI_msgq;
extern void PrintfOptionMenu(char* options_list[], int array_size);
extern void PrintfResp(char* format);

typedef enum{
    SC_AUDIO_DEMO_SET_SAMPLE_RATE       = 1,
    SC_AUDIO_DEMO_PLAY_FILE             = 2,
    SC_AUDIO_DEMO_STOP_PLAY_FILE        = 3,
    SC_AUDIO_DEMO_RECORD                = 4,
    SC_AUDIO_DEMO_RECORD_STOP           = 5,
    SC_AUDIO_DEMO_PLAY_PCM              = 6,
    SC_AUDIO_DEMO_STOP_PLAY_PCM         = 7,
    SC_AUDIO_DEMO_SET_VOLUME            = 8,
    SC_AUDIO_DEMO_GET_VOLUME            = 9,
    SC_AUDIO_DEMO_CUS_AUDREC            = 10,
    SC_AUDIO_DEMO_PLAY_MP3_CONT         = 11,
    SC_AUDIO_DEMO_SET_MICGAIN           = 12,
    SC_AUDIO_DEMO_GET_MICGAIN           = 13,
    SC_AUDIO_DEMO_PLAY_MP3_BUFFER       = 14,
    SC_AUDIO_DEMO_STOP_MP3_BUFFER       = 15,
    SC_AUDIO_DEMO_PA_CONFIG             = 16,
    SC_AUDIO_DEMO_PLAY_AMR_BUFFER       = 17,
    SC_AUDIO_DEMO_STOP_AMR_BUFFER       = 18,
    SC_AUDIO_DEMO_PLAY_AMR_CONT         = 19,
    SC_AUDIO_DEMO_PLAY_WAV_FILE         = 20,
    SC_AUDIO_DEMO_GET_AMR_FRAME         = 21,
    SC_AUDIO_DEMO_STOP_AMR_FRAME        = 22,
    SC_AUDIO_DEMO_AMR_ENCODE_RATE       = 23,
    SC_AUDIO_DEMO_PLAY_WAV_CONT         = 24,
    SC_EchoSuppressionParameter         = 25,
    SC_EchoParaModeSet                  = 26,
    SC_AUDIO_DEMO_MAX                   = 99
}SC_AUDIO_DEMO_TYPE;

#define MAX_VALID_USER_NAME_LENGTH  255-7

/**
  * @brief  Dummy API.
  * @param  status.
  * @note   
  * @retval void
  */
void recordCallBac_Test(UINT8 status)
{
    printf("%s status is %d!",__func__,status);
}

/**
  * @brief  Get amr file frame bytes.
  * @param  *buf pointer.
  * @param  file size.
  * @note   
  * @retval void
  */
void GetAmrFrame(const UINT8* buf, UINT32 size)
{
    for(int i = 0; i < size; i++)
    {
        printf("buf[%d] = %x", i, buf[i]);
    }
}

/**
  * @brief  Create audio demo.
  * @param  void
  * @note   This demo will show how to control audio component with SIMCom OpenSDK APIs.
  * @retval void
  */
void AudioDemo(void)
{
    SIM_MSG_T optionMsg ={0,0,0,NULL};
    char audioFile[26] = {0};
    char fileNameInfo[MAX_VALID_USER_NAME_LENGTH] = {0};
    SCFILE* fh = NULL;
    static char *buffer = NULL;
    char tempBuffer[64];
    char recording_file[26] = {0};
    int len, opt = 0;
    AUD_SampleRate sampleRate = 0;
    AUD_Volume volume;
    BOOL directPlay = 0;
    BOOL isSingle = 0;
    BOOL startplay = 0;
    BOOL frame = 0;
    BOOL ret = 0;
    int RateLevel;
    unsigned int gpioNum;
    UINT8 activeLevel;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *optionsList[] = {
        "1. SampleRate",
        "2. Play file",
        "3. Stop play file",
        "4. Record",
        "5. Stop record",
        "6. Play PCM",
        "7. Stop PCM",
        "8. Set volume",
        "9. Get volume",
        "10. slxk record",
        "11, Play MP3 cont",
        "12. Set micgain",
        "13. Get micgain",
        "14. play mp3 buffer",
        "15, stop MP3 buffer",
        "16. PA control config",
        "17. play amr buffer",
        "18, stop amr buffer",
        "19, Play amr cont",
        "20, Play WAV with high sampling rate",
        "21. get amr record frame",
        "22. stop amr frame",
        "23. set amr encoder rate",
        "24. Play Wav cont",
        "25. EchoSuppressionParameter",
        "26. EchoParaModeSet",
        "99. back",
    };

    buffer = sAPI_Malloc(100*1024);

    while(1)
    {
        PrintfResp(note);
        PrintfOptionMenu(optionsList,sizeof(optionsList)/sizeof(optionsList[0]));
        sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
        if(SRV_UART != optionMsg.msg_id)
        {
            printf("%s,msg_id is error!!",__func__);
            break;
        }

        printf("arg3 = [%s]",(char *)optionMsg.arg3);
        opt = atoi(optionMsg.arg3);
        sAPI_Free(optionMsg.arg3);

        switch(opt)
        {
            case SC_AUDIO_DEMO_SET_SAMPLE_RATE:
            {
                PrintfResp("\r\nPlease input sample rate, 0(8K) or 1(16K)\r\n");
                optionMsg = GetParamFromUart();
                sampleRate = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                ret = sAPI_AudioPlaySampleRate(sampleRate);
                if(ret)
                  printf("sAPI_AudioPlaySampleRate: %d success !", sampleRate);
                else
                  printf("sAPI_AudioPlaySampleRate: %d failed !", sampleRate);

                break;
            }

            case SC_AUDIO_DEMO_PLAY_FILE:
            {
                UINT8 path;
                PrintfResp("\r\nPlease input File Name, like c:/test.wav\r\n");
                optionMsg = GetParamFromUart();
                memset(audioFile, 0, sizeof(audioFile));
                strcpy(audioFile, optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                PrintfResp("\r\nPlease input direct or not, 1 or 0\r\n");
                optionMsg = GetParamFromUart();
                directPlay = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                PrintfResp("\r\nPlease input is single file palyback, 1 or 0\r\n");
                optionMsg = GetParamFromUart();
                isSingle = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                PrintfResp("\r\nPlease input playback path, 1(remote) or 0(local)\r\n");
                optionMsg = GetParamFromUart();
                path = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);
                ret = sAPI_AudioSetPlayPath(path);
                printf("sAPI_AudioSetPlayPath: play path set result %d ",ret);

                ret = sAPI_AudioPlay(audioFile, directPlay,isSingle);
                if(ret)
                  printf("sAPI_AudioPlay: playing %s success ", audioFile);
                else
                  printf("sAPI_AudioPlay: playing %s failed", audioFile);
                sAPI_TaskSleep(1);
                printf("sAPI_AudioStatus: %d", sAPI_AudioStatus());

                break;
            }

            case SC_AUDIO_DEMO_STOP_PLAY_FILE:
            {
                ret = sAPI_AudioStop();
                if(ret)
                  printf("sAPI_AudioStop: stop %s success ", audioFile);
                else
                  printf("sAPI_AudioStop: stop %s failed", audioFile);
                sAPI_TaskSleep(1);
                printf("sAPI_AudioStatus: %d", sAPI_AudioStatus());

                break;
            }

            case SC_AUDIO_DEMO_RECORD:
            {
                PrintfResp("\r\nPlease input File Name.\r\n");
                optionMsg = GetParamFromUart();
                strcpy(recording_file,optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                ret = sAPI_AudioRecord(1, REC_PATH_LOCAL, recording_file);
                if(ret)
                  printf("sAPI_AudioRecord: record to %s success ",recording_file);
                else
                  printf("sAPI_AudioRecord: record to %s failed",recording_file);
                sAPI_TaskSleep(1);
                printf("sAPI_AudioStatus: %d", sAPI_AudioStatus());

                break;
            }

            case SC_AUDIO_DEMO_RECORD_STOP:
            {
                PrintfResp("\r\nPlease input File Name.\r\n");
                optionMsg = GetParamFromUart();
                strcpy(recording_file,optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                ret = sAPI_AudioRecord(0, REC_PATH_LOCAL, recording_file);
                if(ret)
                  printf("sAPI_AudioRecord: stop success ");
                else
                  printf("sAPI_AudioRecord: stop failed");
                sAPI_TaskSleep(1);
                printf("sAPI_AudioStatus: %d", sAPI_AudioStatus());
                break;
            }

            case SC_AUDIO_DEMO_PLAY_PCM:
            {
                UINT8 path;
                PrintfResp("\r\nPlease input File Name to test, like test.wav\r\n");
                optionMsg = GetParamFromUart();
                memset(audioFile, 0, sizeof(audioFile));
                strcpy(audioFile,optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                PrintfResp("\r\nPlease input direct or not, 1 or 0\r\n");
                optionMsg = GetParamFromUart();
                directPlay = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);
                memset(fileNameInfo,0,sizeof(fileNameInfo));
                memcpy(fileNameInfo, "c:/", strlen("c:/"));
                strcat(fileNameInfo, audioFile);

                printf("fileNameInfo[%s]",fileNameInfo);

                PrintfResp("\r\nPlease input playback path, 1(remote) or 0(local)\r\n");
                optionMsg = GetParamFromUart();
                path = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);
                ret = sAPI_AudioSetPlayPath(path);
                printf("sAPI_AudioSetPlayPath: play path set result %d ",ret);

                fh = sAPI_fopen(fileNameInfo, "rb");
                //buffer = sAPI_Malloc(100*1024);
                if(buffer == NULL && fh != NULL)
                {
                    printf("sAPI_Malloc fail !");
                }
                else
                {
                    len = sAPI_fread(buffer, 1,100*1024, fh);
                    printf("sAPI_FsFileRead len:%d !", len);
                    if(len > 44)
                    {
                        ret = sAPI_AudioPcmPlay(&buffer[44], (len - 44), directPlay);
                        printf("sAPI_AudioPcmPlay ret:%d !", ret);
                    }
                }
                sAPI_fclose(fh);
                sAPI_TaskSleep(1);
                printf("sAPI_AudioStatus: %d", sAPI_AudioStatus());
                break;
            }

            case SC_AUDIO_DEMO_STOP_PLAY_PCM:
            {
                PrintfResp("\r\nPlease input any key.\r\n");
                optionMsg = GetParamFromUart();
                sAPI_Free(optionMsg.arg3);

                ret = sAPI_AudioPcmStop();
                if(ret)
                  printf("sAPI_AudioPcmStop: stop success !");
                else
                  printf("sAPI_AudioPcmStop: stop failed !");
                sAPI_TaskSleep(1);
                printf("sAPI_AudioStatus: %d", sAPI_AudioStatus());
                break;
            }

            case SC_AUDIO_DEMO_SET_VOLUME:
            {
                PrintfResp("\r\nPlease input volume: 0~11.\r\n");
                optionMsg = GetParamFromUart();
                volume = (AUD_Volume)atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                sAPI_AudioSetVolume(volume);
                printf("sAPI_AudioSetVolume !");
                break;
            }

            case SC_AUDIO_DEMO_GET_VOLUME:
            {
                volume = sAPI_AudioGetVolume();
                snprintf(tempBuffer, sizeof(tempBuffer), "\r\nvolume = %d\r\n", volume);

                PrintfResp(tempBuffer);
                printf("sAPI_AudioGetVolume: %d !", volume);
                break;
            }

            case SC_AUDIO_DEMO_CUS_AUDREC:
            {
                int duration,oper;
                PrintfResp("\r\nPlease input oper, 1(start record)/0(stop record)\r\n");
                optionMsg = GetParamFromUart();
                oper = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                PrintfResp("\r\nPlease input file name, like C:/test.amr\r\n");
                optionMsg = GetParamFromUart();
                memset(audioFile, 0, sizeof(audioFile));
                strcpy(audioFile, optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                PrintfResp("\r\nPlease input record time amr(1-180s),wav/pcm(1-15s)\r\n");
                optionMsg = GetParamFromUart();
                duration = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                ret = sAPI_AudRec(oper, audioFile,duration,recordCallBac_Test);
                if(ret)
                  printf("sAPI_AudRec: %d operate success", ret);
                else
                  printf("sAPI_AudRec: %d operate failed", ret);
                sAPI_TaskSleep(1);
                printf("sAPI_AudioStatus: %d", sAPI_AudioStatus());
                break;
            }

            case SC_AUDIO_DEMO_PLAY_MP3_CONT:
            {
                do
                {
                    PrintfResp("\r\nPlease input File Name, like c:/test.mp3\r\n");
                    optionMsg = GetParamFromUart();
                    memset(audioFile, 0, sizeof(audioFile));
                    strcpy(audioFile, optionMsg.arg3);
                    sAPI_Free(optionMsg.arg3);
                    
                    PrintfResp("\r\nPlease input startplay or not, 1 or 0\r\n");
                    optionMsg = GetParamFromUart();
                    startplay = atoi(optionMsg.arg3);
                    sAPI_Free(optionMsg.arg3);
                    
                    PrintfResp("\r\nPlease input is frame, 0 ~ 2\r\n");
                    optionMsg = GetParamFromUart();
                    frame = atoi(optionMsg.arg3);
                    sAPI_Free(optionMsg.arg3);

                    ret = sAPI_AudioPlayMp3Cont(audioFile, startplay,frame);
                    if(ret)
                      printf("sAPI_AudioPlayMp3Cont: load %s success ", audioFile);
                    else
                      printf("sAPI_AudioPlayMp3Cont: load %s failed", audioFile);

                }while(!startplay);
                    
                printf("sAPI_AudioPlayMp3Cont%s: start play!", audioFile);
                sAPI_TaskSleep(1);
                printf("sAPI_AudioStatus: %d", sAPI_AudioStatus());
                break;
            }

            case SC_AUDIO_DEMO_SET_MICGAIN:
            {
                int micgain;
                PrintfResp("\r\nPlease input micgain(0-7)\r\n");
                optionMsg = GetParamFromUart();
                micgain = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);
                if(micgain < 0 || micgain > 7)
                {
                    PrintfResp("\r\nparameter error\r\n");
                    break;
                }
                sAPI_AudioSetMicGain(micgain);
                printf("sAPI_AudioSetMicGain !");
                PrintfResp("\r\nset parameter OK\r\n");
                break;
            }

            case SC_AUDIO_DEMO_GET_MICGAIN:
            {
                int ret = sAPI_AudioGetMicGain();
                printf("sAPI_AudioGetMicGain ret = %d !", ret);
                break;
            }
            case SC_AUDIO_DEMO_PLAY_MP3_BUFFER:
            {
                PrintfResp("\r\nPlease input File Name to test, like c:/test.mp3\r\n");
                optionMsg = GetParamFromUart();
                memset(audioFile, 0, sizeof(audioFile));
                strcpy(audioFile,optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                PrintfResp("\r\nPlease input direct or not, 1 or 0\r\n");
                optionMsg = GetParamFromUart();
                directPlay = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                memcpy(fileNameInfo, audioFile, strlen(audioFile));
                printf("fileNameInfo[%s]",fileNameInfo);

                fh = sAPI_fopen(fileNameInfo, "rb");
                //buffer = sAPI_Malloc(100*1024);
                if(buffer == NULL || fh == NULL)
                {
                    printf("sAPI_Malloc fail !");
                }
                else
                {
                    len = sAPI_fread(buffer, 1,100*1024, fh);
                    printf("sAPI_FsFileRead len:%d !", len);
                    if(len > 0)
                    {
                        ret = sAPI_AudioMp3StreamPlay(buffer, len, directPlay);
                        printf("sAPI_AudioMp3StreamPlay ret:%d !", ret);
                    }
                }
                sAPI_fclose(fh);
                sAPI_TaskSleep(1);
                printf("sAPI_AudioStatus: %d", sAPI_AudioStatus());
                break;
            }

            case SC_AUDIO_DEMO_STOP_MP3_BUFFER:
            {
                ret = sAPI_AudioMp3StreamStop();
                printf("sAPI_AudioMp3StreamStop ret:%d !", ret);
                sAPI_TaskSleep(1);
                printf("sAPI_AudioStatus: %d", sAPI_AudioStatus());
                break;
            }

            case SC_AUDIO_DEMO_PA_CONFIG:
            {
                PrintfResp("\r\nPlease input gpio number.\r\n");
                optionMsg = GetParamFromUart();
                gpioNum = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                PrintfResp("\r\nPlease input active level.    0:low  1:high.\r\n");
                optionMsg = GetParamFromUart();
                activeLevel = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                ret = sAPI_AudioSetPaCtrlConfig(gpioNum, activeLevel);
                if(ret != SC_GPIORC_OK)
                    printf("sAPI_AudioSetPaCtrlConfig failed.");
                else
                    printf("sAPI_AudioSetPaCtrlConfig success.");

                PrintfResp("\r\noperation successful!\r\n");

                break;
            }
            case SC_AUDIO_DEMO_PLAY_AMR_BUFFER:
            {
                PrintfResp("\r\nPlease input File Name to test, like c:/test.amr\r\n");
                optionMsg = GetParamFromUart();
                memset(audioFile, 0, sizeof(audioFile));
                strcpy(audioFile,optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                PrintfResp("\r\nPlease input direct or not, 1 or 0\r\n");
                optionMsg = GetParamFromUart();
                directPlay = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                memcpy(fileNameInfo, audioFile, strlen(audioFile));
                printf("fileNameInfo[%s]",fileNameInfo);

                fh = sAPI_fopen(fileNameInfo, "rb");
                //buffer = sAPI_Malloc(100*1024);
                if(buffer == NULL || fh == NULL)
                {
                    printf("sAPI_Malloc fail !");
                }
                else
                {
                    len = sAPI_fread(buffer, 1,100*1024, fh);
                    printf("sAPI_FsFileRead len:%d !", len);
                    if(len > 0)
                    {
                        ret = sAPI_AudioAmrStreamPlay(buffer, len, directPlay);
                        printf("sAPI_AudioAmrStreamPlay ret:%d !", ret);
                    }
                }
                sAPI_fclose(fh);
                sAPI_TaskSleep(1);
                printf("sAPI_AudioStatus: %d", sAPI_AudioStatus());
                break;
            }

            case SC_AUDIO_DEMO_STOP_AMR_BUFFER:
            {
                ret = sAPI_AudioAmrStreamStop();
                printf("sAPI_AudioAmrStreamStop ret:%d !", ret);
                sAPI_TaskSleep(1);
                printf("sAPI_AudioStatus: %d", sAPI_AudioStatus());
                break;
            }

            case SC_AUDIO_DEMO_PLAY_AMR_CONT:
            {
                do
                {
                    PrintfResp("\r\nPlease input File Name, like c:/test.amr\r\n");
                    optionMsg = GetParamFromUart();
                    memset(audioFile, 0, sizeof(audioFile));
                    strcpy(audioFile, optionMsg.arg3);
                    sAPI_Free(optionMsg.arg3);
                    
                    PrintfResp("\r\nPlease input startplay or not, 1 or 0\r\n");
                    optionMsg = GetParamFromUart();
                    startplay = atoi(optionMsg.arg3);
                    sAPI_Free(optionMsg.arg3);

                    ret = sAPI_AudioPlayAmrCont(audioFile, startplay);
                    if(ret)
                      printf("sAPI_AudioPlayAmrCont: load %s success ", audioFile);
                    else
                      printf("sAPI_AudioPlayAmrCont: load %s failed", audioFile);
                }while(!startplay);
                    
                printf("sAPI_AudioPlayAmrCont %s: start play!", audioFile);
                sAPI_TaskSleep(1);
                printf("sAPI_AudioStatus: %d", sAPI_AudioStatus());
                break;
            }

            case SC_AUDIO_DEMO_PLAY_WAV_FILE:
            {
                PrintfResp("\r\nPlease input File Name, like c:/test.wav\r\n");
                optionMsg = GetParamFromUart();
                memset(audioFile, 0, sizeof(audioFile));
                strcpy(audioFile, optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                PrintfResp("\r\nPlease input direct or not, 1 or 0\r\n");
                optionMsg = GetParamFromUart();
                directPlay = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                ret = sAPI_AudioWavFilePlay(audioFile, directPlay);
                if(ret)
                  printf("sAPI_AudioWavFilePlay: playing %s success ", audioFile);
                else
                  printf("sAPI_AudioWavFilePlay: playing %s failed", audioFile);
                sAPI_TaskSleep(1);
                printf("sAPI_AudioStatus: %d", sAPI_AudioStatus());
                break;
            }

            case SC_AUDIO_DEMO_GET_AMR_FRAME:
            {
                ret = sAPI_AmrStreamRecord(GetAmrFrame);
                if(ret)
                    printf("sAPI_AmrStreamRecord: start record!");
                else
                    printf("sAPI_AmrStreamRecord: fail record!");
                break;
            }

            case SC_AUDIO_DEMO_STOP_AMR_FRAME:
            {
                ret = sAPI_AmrStopStreamRecord();
                if(ret)
                    printf("sAPI_AmrStreamRecord: stop record!");
                else
                    printf("sAPI_AmrStreamRecord: stop failed!");
                break;
            }

            case SC_AUDIO_DEMO_AMR_ENCODE_RATE:
            {
                PrintfResp("\r\nPlease input is amr encoder rate, 0 ~ 7\r\n");
                optionMsg = GetParamFromUart();
                RateLevel = atoi(optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);

                sAPI_AudioSetAmrRateLevel(RateLevel);
                printf("RateLevel = %d", RateLevel);
                break;
            }

            case SC_AUDIO_DEMO_PLAY_WAV_CONT:
            {
                do
                {
                    PrintfResp("\r\nPlease input File Name, like c:/test.WAV\r\n");
                    optionMsg = GetParamFromUart();
                    memset(audioFile, 0, sizeof(audioFile));
                    strcpy(audioFile, optionMsg.arg3);
                    sAPI_Free(optionMsg.arg3);

                    PrintfResp("\r\nPlease input startplay or not, 1 or 0\r\n");
                    optionMsg = GetParamFromUart();
                    startplay = atoi(optionMsg.arg3);
                    sAPI_Free(optionMsg.arg3);

                    ret = sAPI_AudioPlayWavCont(audioFile, startplay);
                    if(ret)
                      printf("sAPI_AudioPlayWavCont: load %s success ", audioFile);
                    else
                      printf("sAPI_AudioPlayWavCont: load %s failed", audioFile);
                }while(!startplay);

                printf("sAPI_AudioPlayWavCont%s: start play!", audioFile);
                sAPI_TaskSleep(1);
                printf("sAPI_AudioStatus: %d", sAPI_AudioStatus());
                break;
            }
#ifndef NO_AUDIO
            case SC_EchoSuppressionParameter:
            {
                SC_Dspconfiguration dspconfig;
                /*default Para*/
                dspconfig.resmode = 2;
                dspconfig.dtTresh = 40;
                dspconfig.gamma = 0x1500;
                dspconfig.nonlinear = 0x3266;
                dspconfig.noiseFloorDbov = -88;
                dspconfig.MaxSuppressDb = -18;
                dspconfig.PeakNoiseDb = -39;
                dspconfig.NoiseMatrix = 0xA53;
                dspconfig.txBoostMode = 1;
                dspconfig.txBoostGainDb = 3;
                dspconfig.txPeakGainDb = -3;
                dspconfig.txBoostNoiseGainDb = -3;
                dspconfig.txAgcMaxGainDb = 2;
                dspconfig.rxBoostMode = 1;
                dspconfig.rxBoostGainDb = 8;
                dspconfig.rxPeakGainDb = -2;
                dspconfig.rxBoostNoiseGainDb = 0;
                dspconfig.rxAgcMaxGainDb = 2;
                dspconfig.DAC_DigGain = 22;
                dspconfig.DAC_Gain = 1;
                dspconfig.RCV_Gain = 3;
                dspconfig.PGA_Stage1 = 8;
                dspconfig.PGA_Stage2 = 3;
                dspconfig.ADC_DIgGain = 6;
                dspconfig.sampleDelay = 0;
                dspconfig.numOfTaps = 40;
                dspconfig.convergenceSpeed = 7;
                dspconfig.ref2EchoPowerRatio = 0;
                unsigned char ret = 0;
                ret = sAPI_EchoSuppression_PARA(dspconfig);
                if (ret != TRUE)
                    printf("error audio echosupperssion");
                break;
            }
            case SC_EchoParaModeSet:
            {
                sAPI_EchoParaModeSet(1,1);
                break;
            }
            case 31:
            {
                SC_Dspconfiguration dspconfig;
                int mode = 1;
                char Mode[10]={0};
                sAPI_Get_EchoSuppression_PARA(&dspconfig,mode);
                if (mode == 1)
                {
                    memcpy(Mode, "16k", strlen("16k"));
                }
                else
                {
                    memcpy(Mode, "8k", strlen("8k"));
                }
                printf("%s dspconfig.resmode %d \r\n",Mode, dspconfig.resmode);
                printf("%s spconfig.dtTresh %d \r\n",Mode, dspconfig.dtTresh);
                printf("%s dspconfig.gamma 0x%x \r\n",Mode, dspconfig.gamma);
                printf("%s dspconfig.nonlinear 0x%x \r\n",Mode, dspconfig.nonlinear);
                printf("%s dspconfig.noiseFloorDbov %d \r\n",Mode, dspconfig.noiseFloorDbov);
                printf("%s dspconfig.MaxSuppressDb %d \r\n",Mode, dspconfig.MaxSuppressDb);
                printf("%s dspconfig.PeakNoiseDb %d \r\n",Mode, dspconfig.PeakNoiseDb);
                printf("%s dspconfig.NoiseMatrix 0x%x \r\n",Mode, dspconfig.NoiseMatrix);
                printf("%s dspconfig.txBoostMode %d \r\n",Mode, dspconfig.txBoostMode);
                printf("%s dspconfig.txPeakGainDb %d \r\n",Mode, dspconfig.txPeakGainDb);
                printf("%s dspconfig.txBoostNoiseGainDb %d \r\n",Mode, dspconfig.txBoostNoiseGainDb);
                printf("%s dspconfig.txAgcMaxGainDb %d \r\n",Mode, dspconfig.txAgcMaxGainDb);
                printf("%s dspconfig.rxBoostMode %d \r\n",Mode, dspconfig.rxBoostMode);
                printf("%s dspconfig.rxPeakGainDb %d \r\n",Mode, dspconfig.rxPeakGainDb);
                printf("%s dspconfig.rxBoostNoiseGainDb %d \r\n",Mode, dspconfig.rxBoostNoiseGainDb);
                printf("%s dspconfig.rxAgcMaxGainDb %d \r\n",Mode, dspconfig.rxAgcMaxGainDb);
                printf("dspconfig.DAC_DigGain %d \r\n",dspconfig.DAC_DigGain);
                printf("dspconfig.DAC_Gain %d \r\n",dspconfig.DAC_Gain);
                printf("dspconfig.RCV_Gain %d \r\n",dspconfig.RCV_Gain);
                printf("dspconfig.PGA_Stage1 %d \r\n",dspconfig.PGA_Stage1);
                printf("dspconfig.PGA_Stage2 %d \r\n",dspconfig.PGA_Stage2);
                printf("dspconfig.ADC_DIgGain %d \r\n",dspconfig.ADC_DIgGain);
                printf("%s dspconfig.sampleDelay %d \r\n",Mode, dspconfig.sampleDelay);
                printf("%s dspconfig.numOfTaps %d \r\n",Mode, dspconfig.numOfTaps);
                printf("%s dspconfig.convergenceSpeed %d \r\n",Mode, dspconfig.convergenceSpeed);
                printf("%s dspconfig.ref2EchoPowerRatio %d \r\n",Mode, dspconfig.ref2EchoPowerRatio);
                break;
            }
#endif

            case SC_AUDIO_DEMO_MAX:
            {
                return;
            }
        }
    }
}
#endif
