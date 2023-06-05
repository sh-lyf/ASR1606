/**
  ******************************************************************************
  * @file    demo_cam.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of camera operation.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */
#ifdef SIMCOM_CAMERA_SUPPORT
/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "simcom_system.h"
#include "simcom_os.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "simcom_lcd.h"
#include "simcom_cam.h"
#include "simcom_file.h"
#include "demo_lcd.h"

#ifdef CAM_DECODE
#include "tiny_scanner.h"
#endif

#ifdef USER_CAM_DRIVER
extern sc_cam_data_t CAM_GC032a_spi;
#endif

typedef enum{
    SC_CAM_DEMO_CAM_TAKE_PICTURES       = 1,
    SC_CAM_DEMO_CAM_SHOW_LCD            = 2,
    SC_CAM_DEMO_CAM_SHOW_PICTURES       = 3,
#ifdef CAM_DECODE
    SC_CAM_DEMO_CAM_DECODE_NOLCD        = 4,
    SC_CAM_DEMO_CAM_DECODE_ONLCD        = 5,
#endif
    SC_CAM_DEMO_CAM_SET_PARA            = 6,
    SC_CAM_DEMO_CAM_SET_IMAGESIZE       = 7,
    SC_CAM_DEMO_CAM_SET_MIRROR          = 8,
    SC_CAM_DEMO_CAM_SCAN_BARCODE_ONCE   = 9,
    SC_CAM_DEMO_CAM_SCAN_BARCODE_REPEAT = 10,
    SC_CAM_DEMO_CAM_MAX                 = 99
}SC_LCD_DEMO_TYPE;

typedef struct {
    char cam_type;
    unsigned short cam_width;
    unsigned short cam_height;
    int set_para[9][2];
}sc_cam_info_t;

extern sMsgQRef simcomUI_msgq;
extern SIM_MSG_T GetParamFromUart(void);
extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(char *format);
extern void show_font_16x16(unsigned short x, unsigned short y, unsigned short fc, unsigned short bc, char *s);

int gc032a_set_para[9][2] = {{1,5},{1,9},{1,0},{4,5},{4,8},{5,0},{5,1},{5,2},{5,3}};
int gc6153_set_para[9][2] = {{1,5},{1,9},{1,0},{4,4},{4,5},{5,0},{5,1},{5,2},{5,3}};
int bf30a2_set_para[9][2] = {{1,5},{1,9},{1,0},{4,1},{4,5},{5,0},{5,1},{5,2},{5,3}};
unsigned int lcd_w,lcd_h,cam_w,cam_h;

void get_cam_info(sc_cam_info_t *cam)
{
    switch(cam->cam_type)
    {
        case 1:
            cam->cam_width = 640;
            cam->cam_height = 480;
            memcpy(cam->set_para,gc032a_set_para,sizeof(gc032a_set_para));
            break;
        case 2:
            cam->cam_width = 240;
            cam->cam_height = 320;
            memcpy(cam->set_para,bf30a2_set_para,sizeof(bf30a2_set_para));
            break;
        case 3:
            cam->cam_width = 640;
            cam->cam_height = 480;
            memcpy(cam->set_para,gc6153_set_para,sizeof(gc6153_set_para));
            break;
        default:
            cam->cam_width = 640;
            cam->cam_height = 480;
            memcpy(cam->set_para,gc032a_set_para,sizeof(gc032a_set_para));
            break;
    }
}

/**
  * @brief  Save picture to file system
  * @param  buf-- image buffer from camera
  * @note
  * @retval void
  */
void save_picture(void *buf)
{
    SCFILE *file_hdl = NULL;
    unsigned long long buff_data_len = 0;
    unsigned long long actul_write_len = 0;
    char file_name[32] = {0};
    memcpy(file_name, "c:/image.nv12", strlen("c:/image.nv12"));
    file_hdl = sAPI_fopen(file_name, "wb");

    if(file_hdl == NULL)
        sAPI_Debug("CAM file open err");

    buff_data_len = cam_w*cam_h*3/2;
    actul_write_len = sAPI_fwrite(buf, buff_data_len,1,file_hdl);

    if(actul_write_len != buff_data_len)
        sAPI_Debug("cam image data write err,write length: %d\r\n", actul_write_len);

    sAPI_fclose(file_hdl);
}

/**
  * @brief  test zbar decode callback
  * @param  buf-- image buffer from camera
  * @note
  * @retval void
  */
void zbar_decode_test(void *buf)
{
    static unsigned char *lcd_buf_zbar = NULL;
    if(!lcd_buf_zbar)
        lcd_buf_zbar = sAPI_Malloc(lcd_w*lcd_h*2);
    if(sAPI_YUVtoRGBZoomOut((unsigned short *)lcd_buf_zbar,lcd_w,lcd_h,buf,cam_w,cam_h) < 0)
    {
        if(lcd_buf_zbar)
        {
            sAPI_Free(lcd_buf_zbar);
            lcd_buf_zbar = NULL;
        }
        return;
    }
    sAPI_LcdWrite((unsigned char *)lcd_buf_zbar, 0, 0, lcd_w,lcd_h);
    if(lcd_buf_zbar)
    {
        sAPI_Free(lcd_buf_zbar);
        lcd_buf_zbar = NULL;
    }
}

/**
  * @brief  test zbar decode result callback
  * @param  str-- decode result string
  * @note
  * @retval void
  */
void recv_decode_result(void *str)
{
    char camstr[256] = {0};
    memset(camstr,'\0',sizeof(camstr));
    unsigned char len = strlen(str);
    unsigned char xnum = lcd_w/8;
    unsigned char cnt = (len%xnum)?(len/xnum+1):(len/xnum);
    memcpy(camstr,str,len);
    for(int i = 0;i < cnt; i++)
        show_font_16x16(0,16*i, BLACK, WHITE, &camstr[i*xnum]);
    sAPI_TaskSleep(500);
    sAPI_LcdClose();
}

#ifdef CAM_DECODE
static unsigned char *cam_decode_heap = NULL;
static unsigned char decode_init_flag = 0;
PlatformFunctions Platform_Printf;

/**
  * @brief  camera decode image
  * @param  buf-- image buffer from camera
  * @note
  * @retval void
  */
void cam_decode(void *buf)
{
    int ret = 0;
    char result[256];
    int len = sizeof(result);
    int lcd_w = 128;
    int lcd_h = 128;

    static unsigned char *lcd_buf_decode = NULL;

    if(!lcd_buf_decode)
        lcd_buf_decode = sAPI_Malloc(lcd_w*lcd_h*2);

    ret = tiny_scanner_set_preview(lcd_buf_decode, cam_w, cam_h, &lcd_w, &lcd_h);
    if(0 == ret)
        PrintfResp("\r\nCAM decode preview init ok...\r\n");
    else
        PrintfResp("\r\nCAM decode preview init fail...\r\n");

    ret = tiny_scanner_read(buf, cam_w, cam_h, result, &len);
    if(ret>=0)
    {
        PrintfResp("\r\nCAM decode ok\r\n");
        PrintfResp(result);
        sAPI_Debug("\r\nCAM decode ok,and type = %d\r\n",ret);
        sAPI_Debug("sc","data:%s\r\n",result);
    }
    else
        PrintfResp("\r\nCAM decode fail\r\n");

    if(lcd_buf_decode)
    {
        sAPI_Free(lcd_buf_decode);
        lcd_buf_decode = NULL;
    }
}

/**
  * @brief  camera decode to LCD image
  * @param  buf-- image buffer from camera
  * @note
  * @retval void
  */
void cam_decode_with_lcd(void *buf)
{
    int ret = 0;
    char result[256];
    int len = sizeof(result);
    int preview_w = 80;
    int preview_h = 60;
    static unsigned char *lcd_buf_decode = NULL;

    if(!lcd_buf_decode)
        lcd_buf_decode = sAPI_Malloc(lcd_w*lcd_h*2);
    if(sAPI_YUVtoRGBZoomOut((unsigned short *)lcd_buf_decode,lcd_w,lcd_h,buf,cam_w,cam_h) < 0)
    {
        if(lcd_buf_decode)
        {
            sAPI_Free(lcd_buf_decode);
            lcd_buf_decode = NULL;
        }
        return;
    }
    sAPI_LcdWrite((unsigned char *)lcd_buf_decode, 0, 0, lcd_w,lcd_h);

    ret = tiny_scanner_set_preview(lcd_buf_decode, cam_w, cam_h, &preview_w, &preview_h);
    if(0 == ret)
        PrintfResp("\r\nCAM decode preview init ok...\r\n");
    else
        PrintfResp("\r\nCAM decode preview init fail...\r\n");

    ret = tiny_scanner_read(buf, cam_w, cam_h, result, &len);
    if(ret>=0)
    {
        PrintfResp("\r\nCAM decode ok\r\n");
        PrintfResp(result);
        sAPI_Debug("\r\nCAM decode ok,and type = %d\r\n",ret);
        sAPI_Debug("sc===data:%s\r\n",result);
    }
    else
    {
        PrintfResp("\r\nCAM decode fail\r\n");
        sAPI_Debug("\r\nCAM decode fail,and type = %d\r\n",ret);
    }
    if(lcd_buf_decode)
    {
        sAPI_Free(lcd_buf_decode);
        lcd_buf_decode = NULL;
    }
}

#endif

/**
  * @brief  camera demo operation
  * @param  void
  * @note
  * @retval void
  */
void CamDemo(void)
{
    SIM_MSG_T optionMsg ={0,0,0,NULL};
    unsigned int opt = 0;
    unsigned long long buff_data_len = 0;
    unsigned int actul_read_len = 0;
    void *cam_buf = NULL;
    unsigned short *lcd_buf = NULL;
    SCFILE *file_hdl = NULL;
    char file_name[32] = {0};
    int ret = 0;
    sc_lcd_info_t lcd = {0};
    sc_cam_info_t cam = {0};
    char *note = "\r\nPlease select an option to test from the items listed below, demo just for CAM.\r\n";
    char *options_list[] = {
        "1. CAM take pictures",
        "2. CAM show images on LCD",
        "3. CAM show the pictures",
        #ifdef CAM_DECODE
        "4. CAM decode no lcd",
        "5. CAM decode and show LCD",
        #endif
        "6. CAM set paras",
        "7. CAM set image size",
        "8. CAM set mirror",
        "9. CAM scan barcode once",
        "10. CAM scan barcode repeat",
        "99. back",
    };

#ifdef SIMCOM_A7670C_V6_02
    sAPI_LcdPinConfig(1,13,1,4,1);//just for simcom a7670c_v601 TE board
#endif
    memcpy(file_name, "c:/image.nv12", strlen("c:/image.nv12"));
    while(1)
    {
        PrintfResp("\r\nPlease select LCD and CAMERA first\r\n");
        PrintfResp(" 1:ST7735S(128*128)\r\n 2:ST7735S(128*160)\r\n 3:ST7789V(240*240)\r\n 4:ST7789V(240*320)\r\n");
        optionMsg = GetParamFromUart();
        strcpy(&lcd.lcd_type,optionMsg.arg3);
        sAPI_Free(optionMsg.arg3);
        lcd.lcd_type = atoi(&lcd.lcd_type);
        PrintfResp("\r\n 1:GC032A\r\n 2:BF30A2\r\n 3:GC6153\r\n");
        optionMsg = GetParamFromUart();
        strcpy(&cam.cam_type,optionMsg.arg3);
        sAPI_Free(optionMsg.arg3);
        cam.cam_type = atoi(&cam.cam_type);
        get_lcd_info(&lcd);
        get_cam_info(&cam);
        lcd_w = lcd.lcd_width;
        lcd_h = lcd.lcd_height;
        cam_w = cam.cam_width;
        cam_h = cam.cam_height;
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
        sAPI_Free(optionMsg.arg3);

        switch(opt)
        {
            case SC_CAM_DEMO_CAM_TAKE_PICTURES:
            {
                if(SRV_UART != optionMsg.msg_id)
                {
                    sAPI_Debug("%s,msg_id is error!!",__func__);
                    break;
                }
                sAPI_CamOpen();
                ret = sAPI_GetCamBuf(0,save_picture);
                if(ret != 1)
                {
                    sAPI_Debug("CAM get buf err");
                    PrintfResp("\r\nCAM get buf err...\r\n");
                    sAPI_CamClose();
                    break;
                }
                sAPI_CamClose();
                PrintfResp(note);
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                PrintfResp("\r\nCAM take pictures and save ok...\r\n");
                goto reSelect;
                break;
            }
            case SC_CAM_DEMO_CAM_SHOW_LCD:
            {
                if(SRV_UART != optionMsg.msg_id)
                {
                    sAPI_Debug("%s,msg_id is error!!",__func__);
                    break;
                }
                sAPI_CamOpen();
                sAPI_LcdOpen(lcd_list[lcd.lcd_type-1]);
                sAPI_LcdSetBLPWM(5);
                sAPI_StartShowLCD();
                sAPI_TaskSleep(2000);
                sAPI_StopShowLCD();
                sAPI_CamClose();
                sAPI_LcdSetBLPWM(0);
                sAPI_LcdClose();
                PrintfResp(note);
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                PrintfResp("\r\nCAM show images on lcd...\r\n");
                goto reSelect;
                break;
            }
            case SC_CAM_DEMO_CAM_SHOW_PICTURES:
            {
                if(SRV_UART != optionMsg.msg_id)
                {
                    sAPI_Debug("%s,msg_id is error!!",__func__);
                    break;
                }
                //do read images data form file system and show the pic on LCD;
                buff_data_len = 0;
                file_hdl = sAPI_fopen(file_name, "rb");
                if(file_hdl == NULL)
                {
                    sAPI_Debug("CAM open image data err");
                    PrintfResp("\r\nCAM open image data err...\r\n");
                    break;
                }
                buff_data_len = sAPI_fsize(file_hdl);
                sAPI_Debug("sAPI_fsize buff_data_len: %d",buff_data_len);
                cam_buf = sAPI_Malloc(cam.cam_width*cam.cam_height*3/2);
                if(!cam_buf)
                    sAPI_Debug("cam buf malloc err");
                memset(cam_buf, 0, cam.cam_width*cam.cam_height*3/2);
                actul_read_len = sAPI_fread(cam_buf,buff_data_len, 1, file_hdl);
                if(actul_read_len <= 0)
                {
                    sAPI_Debug("CAM image data read err");
                    PrintfResp("\r\nCAM image data read err...\r\n");
                    sAPI_Free(cam_buf);
                    sAPI_fclose(file_hdl);
                    break;
                }
                lcd_buf = sAPI_Malloc(lcd.lcd_width*lcd.lcd_height*2);
                if(!lcd_buf)
                    sAPI_Debug("lcd buf malloc err");
                sAPI_YUVtoRGBZoomOut(lcd_buf,lcd.lcd_width,lcd.lcd_height,cam_buf,cam.cam_width,cam.cam_height);
                sAPI_LcdOpen(lcd_list[lcd.lcd_type-1]);
                sAPI_LcdSetBLPWM(5);
                sAPI_LcdWrite((unsigned char *)lcd_buf, 0, 0, lcd.lcd_width,lcd.lcd_height);
                ret = sAPI_fclose(file_hdl);
                if(ret != 0)
                {
                    sAPI_Debug("CAM close image data err");
                    PrintfResp("\r\nCAM close image data err...\r\n");
                    sAPI_Free(cam_buf);
                    sAPI_Free(lcd_buf);
                    break;
                }else{
                    file_hdl = NULL;
                }
                sAPI_TaskSleep(1000);
                sAPI_LcdSetBLPWM(0);
                sAPI_Free(cam_buf);
                sAPI_Free(lcd_buf);
                sAPI_LcdClose();
                PrintfResp(note);
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                PrintfResp("\r\nCAM show the pictures taken...\r\n");
                goto reSelect;
                break;
            }
            #ifdef CAM_DECODE
            case SC_CAM_DEMO_CAM_DECODE_NOLCD:
            {
                Platform_Printf._printf = printf;
                if(!cam_decode_heap)
                    cam_decode_heap = sAPI_Malloc(120*1024);
                sAPI_CamOpen();
                if(!decode_init_flag)
                {
                    ret = tiny_scanner_init(&Platform_Printf, cam_decode_heap, 120*1024);
                    if(0 == ret)
                    {
                        decode_init_flag = 1;
                        PrintfResp("\r\nCAM decode init ok...\r\n");
                    }
                    else
                        PrintfResp("\r\nCAM decode init fail...\r\n");
                }
                sAPI_GetCamBuf(1,cam_decode);
                sAPI_TaskSleep(200);
                sAPI_CamClose();
                decode_init_flag = 0;
                if(cam_decode_heap)
                {
                    sAPI_Free(cam_decode_heap);
                    cam_decode_heap = NULL;
                }
                PrintfResp(note);
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                PrintfResp("\r\nCAM decode done...\r\n");
                goto reSelect;
                break;
            }
            case SC_CAM_DEMO_CAM_DECODE_ONLCD:
            {
                Platform_Printf._printf = printf;
                #if 0 //def CAM_GC032A
                PrintfResp("\r\nCAM decode can not support,not enough ram\r\n");
                #else
                if(!cam_decode_heap)
                    cam_decode_heap = sAPI_Malloc(120*1024);
                sAPI_CamOpen();
                if(!decode_init_flag)
                {
                    ret = tiny_scanner_init(&Platform_Printf, cam_decode_heap, 120*1024);
                    if(0 == ret)
                    {
                        decode_init_flag = 1;
                        PrintfResp("\r\nCAM decode init ok...\r\n");
                    }
                    else
                        PrintfResp("\r\nCAM decode init fail...\r\n");
                }
                sAPI_LcdOpen(lcd_list[lcd.lcd_type-1]);
                sAPI_LcdSetBLPWM(5);
                sAPI_GetCamBuf(1,cam_decode_with_lcd);
                sAPI_TaskSleep(1000);
                sAPI_LcdSetBLPWM(0);
                sAPI_CamClose();
                sAPI_LcdClose();
                decode_init_flag = 0;
                if(cam_decode_heap)
                {
                    sAPI_Free(cam_decode_heap);
                    cam_decode_heap = NULL;
                }
                #endif
                PrintfResp(note);
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                PrintfResp("\r\nCAM decode done...\r\n");
                goto reSelect;
                break;
            }
            #endif
            case SC_CAM_DEMO_CAM_SET_PARA:
            {
                sAPI_LcdOpen(lcd_list[lcd.lcd_type-1]);
                sAPI_LcdSetBLPWM(5);
                #ifdef USER_CAM_DRIVER
                sAPI_CamOpenEx(&CAM_GC032a_spi);
                #else
                sAPI_CamOpen();
                #endif
                sAPI_StartShowLCD();
                for(int i = 0;i < 3;i++)
                {
                    sAPI_CamParaSet(cam.set_para[i][0],&cam.set_para[i][1]);
                    sAPI_TaskSleep(1000);
                }
                sAPI_StopShowLCD();
                sAPI_CamClose();
                sAPI_LcdSetBLPWM(0);
                sAPI_LcdClose();
                PrintfResp(note);
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                PrintfResp("\r\nCAM set para demo...\r\n");
                goto reSelect;
                break;
            }
            case SC_CAM_DEMO_CAM_SET_IMAGESIZE:
            {
                for(int i = 3;i < 5;i++)
                {
                    sAPI_LcdOpen(lcd_list[lcd.lcd_type-1]);
                    sAPI_LcdSetBLPWM(5);
                    sAPI_CamParaSet(cam.set_para[i][0],&cam.set_para[i][1]);
                    #ifdef USER_CAM_DRIVER
                    sAPI_CamOpenEx(&CAM_GC032a_spi);
                    #else
                    sAPI_CamOpen();
                    #endif
                    sAPI_StartShowLCD();
                    sAPI_TaskSleep(1000);
                    sAPI_StopShowLCD();
                    sAPI_CamClose();
                    sAPI_LcdSetBLPWM(0);
                    sAPI_LcdClose();
                }
                PrintfResp(note);
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                PrintfResp("\r\nCAM set image size demo...\r\n");
                goto reSelect;
                break;
            }
            case SC_CAM_DEMO_CAM_SET_MIRROR:
            {
                sAPI_LcdOpen(lcd_list[lcd.lcd_type-1]);
                sAPI_LcdSetBLPWM(5);
                #ifdef USER_CAM_DRIVER
                sAPI_CamOpenEx(&CAM_GC032a_spi);
                #else
                sAPI_CamOpen();
                #endif
                sAPI_StartShowLCD();
                for(int i = 5;i < 9;i++)
                {
                    sAPI_CamParaSet(cam.set_para[i][0],&cam.set_para[i][1]);
                    sAPI_TaskSleep(1000);
                }
                sAPI_StopShowLCD();
                sAPI_CamClose();
                sAPI_LcdSetBLPWM(0);
                sAPI_LcdClose();
                PrintfResp(note);
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                PrintfResp("\r\nCAM set mirror demo...\r\n");
                goto reSelect;
                break;
            }
            case SC_CAM_DEMO_CAM_SCAN_BARCODE_ONCE:
            {
                if(SRV_UART != optionMsg.msg_id)
                {
                    sAPI_Debug("%s,msg_id is error!!",__func__);
                    break;
                }
                #ifdef USER_CAM_DRIVER
                sAPI_CamOpenEx(&CAM_GC032a_spi);
                #else
                sAPI_CamOpen();
                #endif
                sAPI_LcdOpen(lcd_list[lcd.lcd_type-1]);
                sAPI_LcdSetBLPWM(5);
                ret = sAPI_CamScanBarcode(0,zbar_decode_test,recv_decode_result);
                if(ret != 1)
                {
                    sAPI_Debug("CAM scan once err");
                    PrintfResp("\r\nCAM scan once err...\r\n");
                    PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                    sAPI_LcdSetBLPWM(0);
                    sAPI_LcdClose();
                    sAPI_CamClose();
                    goto reSelect;
                    break;
                }
                sAPI_CamClose();
                PrintfResp(note);
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                PrintfResp("\r\nCAM scan once ok...\r\n");
                goto reSelect;
                break;
            }
            case SC_CAM_DEMO_CAM_SCAN_BARCODE_REPEAT:
            {
                if(SRV_UART != optionMsg.msg_id)
                {
                    sAPI_Debug("%s,msg_id is error!!",__func__);
                    break;
                }
                #ifdef USER_CAM_DRIVER
                sAPI_CamOpenEx(&CAM_GC032a_spi);
                #else
                sAPI_CamOpen();
                #endif
                sAPI_LcdOpen(lcd_list[lcd.lcd_type-1]);
                sAPI_LcdSetBLPWM(5);
                ret = sAPI_CamScanBarcode(1,zbar_decode_test,NULL);
                if(ret != 1)
                {
                    sAPI_Debug("CAM scan repeat err");
                    PrintfResp("\r\nCAM scan repeat err...\r\n");
                    sAPI_LcdClose();
                    sAPI_CamClose();
                    break;
                }
                sAPI_TaskSleep(2000);
                sAPI_LcdSetBLPWM(0);
                sAPI_LcdClose();
                sAPI_CamClose();
                PrintfResp(note);
                PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
                PrintfResp("\r\nCAM scan repeat ok...\r\n");
                goto reSelect;
                break;
            }
            case SC_CAM_DEMO_CAM_MAX:
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
#endif
/*******************************************************************************************************
int sAPI_CamOpen(void);
DES: open camera              RETURN: 0:OK other:FAIL        PARA: NO

int sAPI_CamClose(void);
DES: close camera             RETURN: 0:OK other:FAIL        PARA: NO

int sAPI_StartShowLCD(void);
DES: show image data on lcd   RETURN: 0:OK other:FAIL        PARA: NO

int sAPI_StopShowLCD(void);
DES: stop show image data     RETURN: 0:OK other:FAIL        PARA: NO

int sAPI_GetCamBuf(int func,CamBufferHandle callbackfunc);
DES: get camera image data    RETURN: 1:OK 0:ERR             PARA1: 0:take picture 1:decode  PARA2:user image data handle

int sAPI_YUVtoRGB_ZoomOut(unsigned short *pRgbData,unsigned int rgbWidth,unsigned int rgbHeight,
                          char *pYuvData,unsigned int yuvWidth,unsigned int yuvHeight);
DES: YUV data to RBG565       RETURN: 0:OK -1:FAIL           PARA: ...

void sAPI_CamPinConfig(cam_user_pin_t *cam_pins);
DES: camera pinconfig         RETURN: none
PARA: typedef struct {
    unsigned char pin_pwdn;
    unsigned char pin_rst;
    unsigned char pin_iovdd;
    unsigned char pin_avdd;
}cam_user_pin_t;

void sAPI_CamPinConfigUninstall(void);
DES: camera pinconfig         RETURN: none                   PARA: NO

int sAPI_CamParaSet(unsigned char param_mode, void *pValue);
DES: set camera para          RETURN: 0:OK -1:FAIL
NOTE: just set the output image size of the camera before opening the camera
PARA1:
      =1:set camera effect
      PARA2:effect val
                0  HAL_EFFECT_NORMAL
                1  HAL_EFFECT_COLORINV
                2  HAL_EFFECT_BLACKBOARD
                3  HAL_EFFECT_WHITEBOARD
                4  HAL_EFFECT_ANTIQUE
                5  HAL_EFFECT_RED
                6  HAL_EFFECT_GREEN
                7  HAL_EFFECT_BLUE
                8  HAL_EFFECT_BLACKWHITE
                9  HAL_EFFECT_NEGATIVE
      =2:set camera contrast
      PARA2:contrastt val
                0  CAM_CONTRAST_HIGH
                1  CAM_CONTRAST_MEDIUM
                2  CAM_CONTRAST_LOW
      =3:set camera exposure
      PARA2:exposure val
                0  CAM_EV_NEG_4_3
                1  CAM_EV_NEG_3_3
                2  CAM_EV_NEG_2_3
                3  CAM_EV_NEG_1_3
                4  CAM_EV_ZERO
                5  CAM_EV_POS_1_3
                6  CAM_EV_POS_2_3
                7  CAM_EV_POS_3_3
                8  CAM_EV_POS_4_3
      =4:set camera output image size
      PARA2:image size val
              //0  {   96,   96  }, // 96x96
              //1  {  160,  120  }, // QQVGA
              //2  {  176,  144  }, // QCIF
              //3  {  240,  176  }, // HQVGA
              //4  {  240,  240  }, // 240x240
              //5  {  320,  240  }, // QVGA
              //6  {  400,  296  }, // CIF
              //7  {  480,  320  }, // HVGA
              //8  {  640,  480  }, // VGA
              //9  {  800,  600  }, // SVGA
              //10 { 1024,  768  }, // XGA
              //11 { 1280,  720  }, // HD
              //12 { 1280, 1024  }, // SXGA
              //13 { 1600, 1200  }, // UXGA
      =5:set camera mirror
      PARA2:mirror val
                0  HAL_MIRROR_H
                1  HAL_MIRROR_V
                2  HAL_MIRROR_HV
                3  HAL_MIRROR_NORMAL
*******************************************************************************************************/
