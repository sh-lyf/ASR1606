/**
  ******************************************************************************
  * @file    demo_lcd.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of lcd configuration operation.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#ifdef SIMCOM_LCD_SUPPORT
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "simcom_system.h"
#include "simcom_os.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "simcom_lcd.h"
#include "demo_lcd.h"

typedef enum{
    SC_LCD_DEMO_LCD_OPEN            = 1,
    SC_LCD_DEMO_LCD_CLOSE           = 2,
    SC_LCD_DEMO_LCD_SHOW_FONT       = 3,
    SC_LCD_DEMO_LCD_CLEAR_SCREEN    = 4,
    SC_LCD_DEMO_LCD_SET_BRIGHTNESS  = 5,
    SC_LCD_DEMO_LCD_MAX             = 99
}SC_LCD_DEMO_TYPE;

extern sMsgQRef simcomUI_msgq;
extern SIM_MSG_T GetParamFromUart(void);
extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(char *format);

/**
  * @brief  LCD show 16x16 character
  * @param  x--x begin position
  * @param  y--y begin position
  * @param  fc--font color
  * @param  bc--background color
  * @param  s--string pointer
  * @note   This demo will show how to full fill fxixed color to whole screen
  * @retval void
  */
void show_font_16x16(unsigned short x, unsigned short y, unsigned short fc, unsigned short bc, char *s)
{
    unsigned char len = strlen((void *)s);
    unsigned short *pBuf = NULL;
    if (pBuf == NULL)
    {
        pBuf = (unsigned short *)sAPI_Malloc(8*16*2*len);
        if (pBuf == NULL)
        {
            sAPI_Debug("wrong malloc");
            return;
        }
    }
    unsigned char Xb = x;
    unsigned char Yb = y;
    unsigned char Lenb = len;
    unsigned char i,j;
    unsigned short k,x0;
    x0=x;
    while(len)
    {
        while(*s)
        {
            if((*s) < 128)
            {
                k=*s;
                if (k==13)
                {
                    x=x0;
                    y+=16;
                }
                else
                {
                    if(k>32)
                        k-=32;
                    else
                        k=0;
                    for(i=0;i<16;i++)
                    {
                        for(j=0;j<8;j++)
                        {
                            if(ASCII_8X16[k*16+i]&(0x80>>j))
                                pBuf[(x+j-Xb)+(y+i-Yb)*8*Lenb]=fc;
                            else
                                pBuf[(x+j-Xb)+(y+i-Yb)*8*Lenb]=bc;
                        }
                    }
                    x+=8;
                }
                s++;
                len-=1;
            }
            else
            {
                for(k=0;k<GB_16_num;k++)
                {
                    if((GB_16[k].Index[0]==*(s))&&(GB_16[k].Index[1]==*(s+1)))
                    {
                        for(i=0;i<16;i++)
                        {
                            for(j=0;j<8;j++)
                            {
                                if(GB_16[k].Msk[i*2]&(0x80>>j))
                                    pBuf[(x+j-Xb)+(y+i-Yb)*8*Lenb]=fc;
                                else
                                    if((bc!=0)&&(fc!=bc)) pBuf[(x+j-Xb)+(y+i-Yb)*8*Lenb]=bc;
                            }
                            for(j=0;j<8;j++)
                            {
                                if(GB_16[k].Msk[i*2+1]&(0x80>>j))
                                    pBuf[(x+j+8-Xb)+(y+i-Yb)*8*Lenb]=fc;
                                else
                                    if ((bc!=0)&&(fc!=bc)) pBuf[(x+j+8-Xb)+(y+i-Yb)*8*Lenb]=bc;
                            }
                        }
                    }
                }
                s+=2;
                x+=16;
                len-=2;
            }
        }
    }
    sAPI_LcdWrite((unsigned char *)pBuf, Xb,Yb,Xb+(8*Lenb),Yb+16);
    sAPI_Free(pBuf);
}

/**
  * @brief  LCD clean screen to fixed colr
  * @param  bg_color--background color
  * @param  lcd--lcd info
  * @note   This demo will show how to full fill fxixed color to whole screen
  * @retval void
  */
static void lcd_clean_screen(unsigned short bg_color,sc_lcd_info_t lcd)
{
    unsigned short *buf = NULL;
    unsigned int i;
    if(5 == lcd.lcd_type)
    {
        unsigned char m,n;
        unsigned char ComTable[]={7,6,5,4,3,2,1,0};
        for(m = 0;m < 8;m++)
        {
            sAPI_LcdWriteCmd(0xB0|ComTable[m]);
            sAPI_LcdWriteCmd(0x10);
            sAPI_LcdWriteCmd(0x01);
            for(n = 0;n < 128;n++)
            sAPI_LcdWriteData((unsigned char *)&bg_color,1);
        }
    }
    else
    {
        if(0 == lcd.lcd_width || 0 == lcd.lcd_height)
        {
            sAPI_Debug("%s,lcd width or lcd height err!",__func__);
            return;
        }
        buf = sAPI_Malloc(lcd.lcd_width * lcd.lcd_height * 2);
        for(i = 0; i < lcd.lcd_width * lcd.lcd_height; i++)
            buf[i] = bg_color;
        sAPI_LcdClearScreen(buf);
        sAPI_TaskSleep(30);
        sAPI_Free(buf);
    }
}

/**
  * @brief  LCD show font
  * @param  void
  * @note   This demo will show TN_GB_16 font
  * @retval void
  */
void tn_lcd_show_font(void)
{
    unsigned char page, i;
    unsigned char *dat = 0;
    unsigned char s_col = 0;

    for(i=0; i < sizeof(TN_GB_16)/sizeof(typFNT_GB16_t); i++)
    {
        dat = (unsigned char *)TN_GB_16[i].Msk;
        s_col = (i*8);
        for(page = 1; page < 3; page++)
        {
            sAPI_LcdWriteCmd(0xb0 + page);
            sAPI_LcdWriteCmd(0x10|((s_col>>4)&0x0f));
            sAPI_LcdWriteCmd(0x00|(s_col&0x0f));
            sAPI_LcdWriteData(dat+(page - 1)*16, 16);
        }
    }
}

/**
  * @brief  get LCD info
  * @param  void
  * @note   This demo will get LCD info
  * @retval void
  */
void get_lcd_info(sc_lcd_info_t *lcd)
{
    switch(lcd->lcd_type)
    {
        case 1:
            lcd->lcd_width = 128;
            lcd->lcd_height = 128;
            break;
        case 2:
            lcd->lcd_width = 128;
            lcd->lcd_height = 160;
            break;
        case 3:
            lcd->lcd_width  = 240;
            lcd->lcd_height = 240;
            break;
        case 4:
            lcd->lcd_width  = 240;
            lcd->lcd_height = 320;
            break;
        default:
            lcd->lcd_width  = 240;
            lcd->lcd_height = 320;
            break;
    }
}

/**
  * @brief  LCD Demo
  * @param  void
  * @note   This demo will show how to operate LCD panal.
  * @retval void
  */
void LcdDemo(void)
{
    SIM_MSG_T optionMsg ={0,0,0,NULL};
    unsigned int opt = 0;
    char *str1="ª∂”≠ π”√";
    char *str2="SIMCom";
    unsigned short x_pos,y_pos;
    sc_lcd_info_t lcd = {0};
    char *note = "\r\nPlease select an option to test from the items listed below, demo just for LCD.\r\n";
    char *options_list[] = {
        "1. LCD open",
        "2. LCD close",
        "3. LCD show font",
        "4. LCD clear screen",
        "5. LCD set brightness",
        "99. back",
    };
#ifdef SIMCOM_A7670C_V6_02
    sAPI_LcdPinConfig(1,13,1,4,1);//just for simcom a7670c_v601 TE board
#endif
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
        sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
        opt = atoi(optionMsg.arg3);
        sAPI_Free(optionMsg.arg3);

        switch(opt)
        {
            case SC_LCD_DEMO_LCD_OPEN:
            {
                if(SRV_UART != optionMsg.msg_id)
                {
                    sAPI_Debug("%s,msg_id is error!!",__func__);
                    break;
                }
                PrintfResp("\r\nPlease input LCD TYPE num\r\n");
                PrintfResp(" 1:ST7735S(128*128)\r\n 2:ST7735S(128*160)\r\n 3:ST7789V(240*240)\r\n 4:ST7789V(240*320)\r\n 5:ST7567A(7565R)(128*64)\r\n");
                optionMsg = GetParamFromUart();
                strcpy(&lcd.lcd_type,optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);
                lcd.lcd_type = atoi(&lcd.lcd_type);
                get_lcd_info(&lcd);
                sAPI_LcdOpen(lcd_list[lcd.lcd_type-1]);
                sAPI_LcdSetBLPWM(5);
                PrintfResp("\r\nsAPI_LcdOpen...\r\n");
                break;
            }
            case SC_LCD_DEMO_LCD_CLOSE:
            {
                sAPI_LcdSetBLPWM(0);
                sAPI_LcdClose();
                PrintfResp("\r\nsAPI_LcdClose...\r\n");
                break;
            }
            case SC_LCD_DEMO_LCD_SHOW_FONT:
            {
                if(5 == lcd.lcd_type)
                    tn_lcd_show_font();
                else
                {
                    //Chinese
                    x_pos = (lcd.lcd_width - strlen(str1)*8)/2;
                    y_pos = (lcd.lcd_height - 16)/2;
                    show_font_16x16(x_pos,y_pos, BLACK, WHITE, str1);
                    //Ascii
                    x_pos = (lcd.lcd_width - strlen(str2)*8)/2;
                    y_pos += 16;
                    show_font_16x16(x_pos,y_pos, BLACK, WHITE, str2);
                }
                PrintfResp("\r\nsAPI_LcdShowFont...\r\n");
                break;
            }
            case SC_LCD_DEMO_LCD_CLEAR_SCREEN:
            {
                lcd_clean_screen(WHITE,lcd);
                PrintfResp("\r\nsAPI_LcdClearScreen...\r\n");
                break;
            }
            case SC_LCD_DEMO_LCD_SET_BRIGHTNESS:
            {
                PrintfResp("\r\nPlease input LCD BRIGHTNESS level(0-5)\r\n");
                optionMsg = GetParamFromUart();
                strcpy(&lcd.lcd_bri,optionMsg.arg3);
                sAPI_Free(optionMsg.arg3);
                lcd.lcd_bri = atoi(&lcd.lcd_bri);
                sAPI_LcdSetBLPWM((unsigned int)lcd.lcd_bri);
                PrintfResp("\r\nsAPI_LcdSetBrightness...\r\n");
                break;
            }
            case SC_LCD_DEMO_LCD_MAX:
            {
                sAPI_LcdSetBLPWM(0);
                sAPI_LcdClose();
                sAPI_Debug("Return to the previous menu!");
                PrintfResp("\r\nReturn to the previous menu!\r\n");
                return;
            }
            default:
                PrintfResp("\r\nPlease select again:\r\n");
                break;
        }
    }
}
#endif