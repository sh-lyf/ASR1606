/**
  ******************************************************************************
  * @file    demo_cam_driver.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of camera driver.
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
#include "simcom_cam_common.h"
#include "sc_gpio.h"

//demo for custom add camera driver

//#define CAM_POW_CTRL_GPIO  //for A7670 series te board
#define CAM_POW_CTRL_LDO  //for A7630 series  te board

const framesize_info_t framesize[FRAMESIZE_INVALID] = {
    {   96,   96  }, /* 96x96 */
    {  160,  120  }, /* QQVGA */
    {  176,  144  }, /* QCIF  */
    {  240,  176  }, /* HQVGA */
    {  240,  240  }, /* 240x240 */
    {  320,  240  }, /* QVGA  */
    {  400,  296  }, /* CIF   */
    {  480,  320  }, /* HVGA  */
    {  640,  480  }, /* VGA   */
    {  800,  600  }, /* SVGA  */
    { 1024,  768  }, /* XGA   */
    { 1280,  720  }, /* HD    */
    { 1280, 1024  }, /* SXGA  */
    { 1600, 1200  }, /* UXGA  */
};

#ifdef CAM_POW_CTRL_GPIO
#define USER_CAM_PWDN  69
#define USER_CAM_RST   70
#define USER_CAM_AVDD  77
#define USER_CAM_IOVDD 124
#endif

#ifdef CAM_POW_CTRL_LDO
#define USER_CAM_PWDN  28
#define USER_CAM_RST   31
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#endif

#define H8(v) ((v)>>8)
#define L8(v) ((v)&0xff)

//yuv422 640 * 480 spi 2 lan sdr
static const struct cam_regval_tab gc032a_init_yuv422_spi2lan_nocrc_sdr[] = {
    {0xf3, 0x83},   /*System*/
    {0xf5, 0x0c},
    {0xf7, 0x13},
    {0xf8, 0x0f},
    {0xf9, 0x4e},
    {0xfa, 0x31},
    {0xfc, 0x02},
    {0xfe, 0x02},
    {0x81, 0x03},
    {0xfe, 0x00},   /*Analog&Cisctl*/
    {0x03, 0x01},
    {0x04, 0xc2},
    {0x05, 0x01},
    {0x06, 0xa3},
    {0x07, 0x00},
    {0x08, 0x08},
    {0x0a, 0x04},
    {0x0c, 0x04},
    {0x0d, 0x01},
    {0x0e, 0xe8},
    {0x0f, 0x02},
    {0x10, 0x88},
    {0x17, 0x54},
    {0x19, 0x04},
    {0x1a, 0x0a},
    {0x1f, 0x40},
    {0x20, 0x30},
    {0x2e, 0x80},
    {0x2f, 0x2b},
    {0x30, 0x1a},
    {0xfe, 0x02},
    {0x03, 0x02},   //[4:0]post_tx_width
    {0x06, 0x60},   //[5:4]stsbp_mode
    {0x05, 0xd7},   //drv
    {0x12, 0x89},   //[7:6]init_ramp_mode
    {0xfe, 0x03},   /*SPI*/
    {0x51, 0x00},   //stream off
    {0x52, 0xda},
    {0x53, 0x24},   //no crc 0x24     crc 0xa4
    {0x54, 0x20},
    {0x55, 0x00},
    {0x59, 0x10},
    {0x5a, 0x00},   //0x01  type
    {0x5b, 0x80},
    {0x5c, 0x02},
    {0x5d, 0xe0},
    {0x5e, 0x01},
    {0x64, 0x06},
    {0xfe, 0x00},   /*blk*/
    {0x18, 0x02},
    {0xfe, 0x02},
    {0x40, 0x22},
    {0x45, 0x00},
    {0x46, 0x00},
    {0x49, 0x20},
    {0x4b, 0x3c},
    {0x50, 0x20},
    {0x42, 0x10},
    {0xfe, 0x01},   /*isp*/
    {0x0a, 0xc5},
    {0x45, 0x00},   //[6]darksun_en
    {0xfe, 0x00},
    {0x40, 0xff},
    {0x41, 0x25},
    {0x42, 0xcf},
    {0x43, 0x10},
    {0x44, 0x83},
    {0x46, 0x26},
    {0x49, 0x03},
    {0x4f, 0x01},   //[0]AEC_en
    {0xde, 0x84},
    {0xfe, 0x02},
    {0x22, 0xf6},   //CISCTL_SUN_TH_R
    {0xfe, 0x01},   /*Shading*/
    {0xc1, 0x3c},
    {0xc2, 0x50},
    {0xc3, 0x00},
    {0xc4, 0x32},
    {0xc5, 0x24},
    {0xc6, 0x16},
    {0xc7, 0x08},
    {0xc8, 0x08},
    {0xc9, 0x00},
    {0xca, 0x20},
    {0xdc, 0x8a},
    {0xdd, 0xa0},
    {0xde, 0xa6},
    {0xdf, 0x75},
    {0xfe, 0x01},   /*AWB*/
    {0x7c, 0x09},
    {0x65, 0x06},
    {0x7c, 0x08},
    {0x56, 0xf4},
    {0x66, 0x0f},
    {0x67, 0x84},
    {0x6b, 0x80},
    {0x6d, 0x12},
    {0x6e, 0xb0},
    {0x86, 0x00},
    {0x87, 0x00},
    {0x88, 0x00},
    {0x89, 0x00},
    {0x8a, 0x00},
    {0x8b, 0x00},
    {0x8c, 0x00},
    {0x8d, 0x00},
    {0x8e, 0x00},
    {0x8f, 0x00},
    {0x90, 0xef},
    {0x91, 0xe1},
    {0x92, 0x0c},
    {0x93, 0xef},
    {0x94, 0x65},
    {0x95, 0x1f},
    {0x96, 0x0c},
    {0x97, 0x2d},
    {0x98, 0x20},
    {0x99, 0xaa},
    {0x9a, 0x3f},
    {0x9b, 0x2c},
    {0x9c, 0x5f},
    {0x9d, 0x3e},
    {0x9e, 0xaa},
    {0x9f, 0x67},
    {0xa0, 0x60},
    {0xa1, 0x00},
    {0xa2, 0x00},
    {0xa3, 0x0a},
    {0xa4, 0xb6},
    {0xa5, 0xac},
    {0xa6, 0xc1},
    {0xa7, 0xac},
    {0xa8, 0x55},
    {0xa9, 0xc3},
    {0xaa, 0xa4},
    {0xab, 0xba},
    {0xac, 0xa8},
    {0xad, 0x55},
    {0xae, 0xc8},
    {0xaf, 0xb9},
    {0xb0, 0xd4},
    {0xb1, 0xc3},
    {0xb2, 0x55},
    {0xb3, 0xd8},
    {0xb4, 0xce},
    {0xb5, 0x00},
    {0xb6, 0x00},
    {0xb7, 0x05},
    {0xb8, 0xd6},
    {0xb9, 0x8c},
    {0xfe, 0x01},   /*CC*/
    {0xd0, 0x40},   //3a
    {0xd1, 0xf8},
    {0xd2, 0x00},
    {0xd3, 0xfa},
    {0xd4, 0x45},
    {0xd5, 0x02},
    {0xd6, 0x30},
    {0xd7, 0xfa},
    {0xd8, 0x08},
    {0xd9, 0x08},
    {0xda, 0x58},
    {0xdb, 0x02},
    {0xfe, 0x00},
    {0xfe, 0x00},   /*Gamma*/
    {0xba, 0x00},
    {0xbb, 0x04},
    {0xbc, 0x0a},
    {0xbd, 0x0e},
    {0xbe, 0x22},
    {0xbf, 0x30},
    {0xc0, 0x3d},
    {0xc1, 0x4a},
    {0xc2, 0x5d},
    {0xc3, 0x6b},
    {0xc4, 0x7a},
    {0xc5, 0x85},
    {0xc6, 0x90},
    {0xc7, 0xa5},
    {0xc8, 0xb5},
    {0xc9, 0xc2},
    {0xca, 0xcc},
    {0xcb, 0xd5},
    {0xcc, 0xde},
    {0xcd, 0xea},
    {0xce, 0xf5},
    {0xcf, 0xff},
    {0xfe, 0x00},   /*Auto Gamma*/
    {0x5a, 0x08},
    {0x5b, 0x0f},
    {0x5c, 0x15},
    {0x5d, 0x1c},
    {0x5e, 0x28},
    {0x5f, 0x36},
    {0x60, 0x45},
    {0x61, 0x51},
    {0x62, 0x6a},
    {0x63, 0x7d},
    {0x64, 0x8d},
    {0x65, 0x98},
    {0x66, 0xa2},
    {0x67, 0xb5},
    {0x68, 0xc3},
    {0x69, 0xcd},
    {0x6a, 0xd4},
    {0x6b, 0xdc},
    {0x6c, 0xe3},
    {0x6d, 0xf0},
    {0x6e, 0xf9},
    {0x6f, 0xff},
    {0xfe, 0x00},   /*Gain*/
    {0x70, 0x50},
    {0xfe, 0x00},   /*AEC*/
    {0x4f, 0x01},
    {0xfe, 0x01},
    {0x44, 0x04},
    {0x1f, 0x30},
    {0x20, 0x40},
    {0x26, 0x4e},
    {0x27, 0x01},
    {0x28, 0xd4},
    {0x29, 0x03},
    {0x2a, 0x0c},
    {0x2b, 0x03},
    {0x2c, 0xe9},
    {0x2d, 0x07},
    {0x2e, 0xd2},
    {0x2f, 0x0b},
    {0x30, 0x6e},
    {0x31, 0x0e},
    {0x32, 0x70},
    {0x33, 0x12},
    {0x34, 0x0c},
    {0x3c, 0x10},   //[5:4] Max level setting
    {0x3e, 0x20},
    {0x3f, 0x2d},
    {0x40, 0x40},
    {0x41, 0x5b},
    {0x42, 0x82},
    {0x43, 0xb7},
    {0x04, 0x0a},
    {0x02, 0x79},
    {0x03, 0xc0},
    {0xcc, 0x08},   /*measure window*/
    {0xcd, 0x08},
    {0xce, 0xa4},
    {0xcf, 0xec},
    {0xfe, 0x00},   /*DNDD*/
    {0x81, 0xb8},   //f8
    {0x82, 0x12},
    {0x83, 0x0a},
    {0x84, 0x01},
    {0x86, 0x50},
    {0x87, 0x18},
    {0x88, 0x10},
    {0x89, 0x70},
    {0x8a, 0x20},
    {0x8b, 0x10},
    {0x8c, 0x08},
    {0x8d, 0x0a},
    {0xfe, 0x00},   /*Intpee*/
    {0x8f, 0xaa},
    {0x90, 0x9c},
    {0x91, 0x52},
    {0x92, 0x03},
    {0x93, 0x03},
    {0x94, 0x08},
    {0x95, 0x44},
    {0x97, 0x00},
    {0x98, 0x00},
    {0xfe, 0x00},   /*ASDE*/
    {0xa1, 0x30},
    {0xa2, 0x41},
    {0xa4, 0x30},
    {0xa5, 0x20},
    {0xaa, 0x30},
    {0xac, 0x32},
    {0xfe, 0x00},   /*YCP*/
    {0xd1, 0x3c},
    {0xd2, 0x3c},
    {0xd3, 0x38},
    {0xd6, 0xf4},
    {0xd7, 0x1d},
    {0xdd, 0x73},
    {0xde, 0x84},
};

static const struct cam_regval_tab GC032a_id[] = {
    {0xf0, 0x23},
    {0xf1, 0x2a},
};

static const struct cam_regval_tab GC032a_stream_on[] = {
    {0xfe, 0x03},
    {0x51, 0x01},
};

static const struct cam_regval_tab GC032a_stream_off[] = {
    {0xfe, 0x03},
    {0x51, 0x00},
};

static struct cam_i2c_attr GC032a_i2c_attr[] = {
    {
        SC_I2C_8BIT,
        SC_I2C_8BIT,
        0x21,//0x42,
    }
};

static struct cam_resolution GC032a_resolution[] = {
    {
        SC_SENSOR_FMT_YUV422,
        SC_MEDIA_BUS_FMT_YVYU8_2X8,
        640,
        480,
        0x0708,
        0x0d06,
        30,
        15,
        SC_SENSOR_RES_BINING1,
        {
            gc032a_init_yuv422_spi2lan_nocrc_sdr,
            ARRAY_SIZE(gc032a_init_yuv422_spi2lan_nocrc_sdr),
        },
    },
};

/**
  * @brief  GC032a camera power control
  * @param  on-- switch on or off camera by writing register
  * @note
  * @retval 0
  */
int GC032a_s_power(int on)
{
#if defined(CAM_POW_CTRL_GPIO)
    sAPI_GpioSetDirection(USER_CAM_PWDN,1);
    sAPI_GpioSetValue(USER_CAM_PWDN, 0);

    sAPI_GpioSetDirection(USER_CAM_AVDD,1);
    sAPI_GpioSetDirection(USER_CAM_IOVDD,1);

    if (on > 0) {
        sAPI_CamMclkOnOff(0);
        sAPI_TaskSleep(1); //delay 5ms
        sAPI_GpioSetValue(USER_CAM_IOVDD, 1);
        sAPI_TaskSleep(1);
        sAPI_GpioSetValue(USER_CAM_AVDD, 1);
        sAPI_TaskSleep(1);
        sAPI_CamMclkOnOff(1);
        sAPI_TaskSleep(1);
        sAPI_GpioSetValue(USER_CAM_PWDN, 1);
        sAPI_TaskSleep(2);
        sAPI_GpioSetValue(USER_CAM_PWDN, 0);
        sAPI_TaskSleep(2);
    } else {
        sAPI_GpioSetValue(USER_CAM_PWDN, 1);
        sAPI_TaskSleep(1);
        sAPI_CamMclkOnOff(0);
        sAPI_TaskSleep(1);
        sAPI_GpioSetValue(USER_CAM_AVDD, 0);
        sAPI_TaskSleep(2);
        sAPI_GpioSetValue(USER_CAM_IOVDD, 0);
        sAPI_TaskSleep(1);
        sAPI_GpioSetValue(USER_CAM_PWDN, 0);
    }

#elif defined(CAM_POW_CTRL_LDO)
    sAPI_GpioSetDirection(USER_CAM_PWDN,1);
    sAPI_GpioSetValue(USER_CAM_PWDN, 1);

    sAPI_GpioSetDirection(USER_CAM_PWDN,1);
    sAPI_GpioSetValue(USER_CAM_PWDN, 0);

    if (on > 0) {
        sAPI_GpioSetValue(USER_CAM_PWDN, 0);
        sAPI_TaskSleep(2);
        sAPI_CamLdoOnOff(0,SC_CAM_AVDD);
        sAPI_TaskSleep(2);
        sAPI_CamLdoOnOff(0,SC_CAM_IOVDD);
        sAPI_TaskSleep(2);
        sAPI_CamMclkOnOff(0);
        sAPI_TaskSleep(1); //delay 5ms
        sAPI_CamLdoOnOff(1,SC_CAM_IOVDD);
        sAPI_TaskSleep(1);
        sAPI_CamLdoOnOff(1,SC_CAM_AVDD);
        sAPI_TaskSleep(1);
        sAPI_CamMclkOnOff(1);
        sAPI_TaskSleep(1);
        sAPI_GpioSetValue(USER_CAM_PWDN, 1);
        sAPI_TaskSleep(2);
        sAPI_GpioSetValue(USER_CAM_PWDN, 0);
        sAPI_TaskSleep(2);
    }else {
        sAPI_GpioSetValue(USER_CAM_PWDN, 1);
        sAPI_TaskSleep(1);
        sAPI_CamMclkOnOff(0);
        sAPI_TaskSleep(1);
        sAPI_CamLdoOnOff(0,SC_CAM_AVDD);
        sAPI_TaskSleep(2);
        sAPI_CamLdoOnOff(0,SC_CAM_IOVDD);
        sAPI_TaskSleep(1);
        sAPI_GpioSetValue(USER_CAM_PWDN, 0);
    }
#endif
    return 0;
}

/**
  * @brief  GC032a set YUV effect
  * @param  sensor_ctrl---sensor control configuration
  * @param  level---effect level
  * @note
  * @retval 0
  */
int GC032a_YUV_set_effect(sc_cam_ctrl_t *sensor_ctrl, UINT16 level)
{
    int ret = 0;
    int cur_i2c_index = 0;
    struct cam_i2c_attr *i2c_attr = NULL;
    sc_cam_global_config_t *global_cam_config = NULL;
    sAPI_Debug("GC032a_YUV_set_effect mode %d", level);

    cur_i2c_index = sensor_ctrl->sensor_data->cur_i2c_index;
    i2c_attr = &(sensor_ctrl->sensor_data->i2c_attr[cur_i2c_index]);

    switch (level){
        case HAL_EFFECT_NORMAL:
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x00);
            sAPI_CamWriteReg(i2c_attr,0x43, 0x10);
            break;
        case HAL_EFFECT_COLORINV:
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x00);
            sAPI_CamWriteReg(i2c_attr,0x43, 0x11);
            break;
        case HAL_EFFECT_BLACKBOARD:
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x00);
            sAPI_CamWriteReg(i2c_attr,0x43, 0x15);
            break;
        case HAL_EFFECT_WHITEBOARD:
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x00);
            sAPI_CamWriteReg(i2c_attr,0x43, 0x14);
            break;
        case HAL_EFFECT_ANTIQUE:
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x00);
            sAPI_CamWriteReg(i2c_attr,0x43, 0x12);
            sAPI_CamWriteReg(i2c_attr,0xda, 0x90);
            sAPI_CamWriteReg(i2c_attr,0xdb, 0x30);
            break;
        case HAL_EFFECT_RED:
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x00);
            sAPI_CamWriteReg(i2c_attr,0x43, 0x12);
            sAPI_CamWriteReg(i2c_attr,0xda, 0x12);
            sAPI_CamWriteReg(i2c_attr,0xdb, 0x12);
            break;
        case HAL_EFFECT_GREEN:
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x00);
            sAPI_CamWriteReg(i2c_attr,0x43, 0x12);
            sAPI_CamWriteReg(i2c_attr,0xda, 0xc0);
            sAPI_CamWriteReg(i2c_attr,0xdb, 0xc0);
            break;
        case HAL_EFFECT_BLUE:
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x00);
            sAPI_CamWriteReg(i2c_attr,0x43, 0x12);
            sAPI_CamWriteReg(i2c_attr,0xda, 0x60);
            sAPI_CamWriteReg(i2c_attr,0xdb, 0xc0);
            break;
        case HAL_EFFECT_BLACKWHITE:
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x00);
            sAPI_CamWriteReg(i2c_attr,0x43, 0x12);
            sAPI_CamWriteReg(i2c_attr,0xda, 0x00);
            sAPI_CamWriteReg(i2c_attr,0xdb, 0x00);
            break;
        case HAL_EFFECT_NEGATIVE:
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x00);
            sAPI_CamWriteReg(i2c_attr,0x43, 0x11);
            break;
        default:
            sAPI_Debug("invalid effect mode %d", level);
            ret = -1;
    }
    if(0 == ret)
    {
        global_cam_config = sAPI_CamGetGlobalConfig();
        global_cam_config->cam_effect= level;
    }
    return ret;

}

/**
  * @brief  GC032a set YUV contrast
  * @param  sensor_ctrl---sensor control configuration
  * @param  level---contrast level
  * @note
  * @retval 0
  */
int GC032a_YUV_set_contrast(sc_cam_ctrl_t *sensor_ctrl, UINT16 level)
{
    int ret = 0;
    int cur_i2c_index = 0;
    struct cam_i2c_attr *i2c_attr = NULL;
    sc_cam_global_config_t *global_cam_config = NULL;
    sAPI_Debug("GC032a_YUV_set_contrast level %d", level);

    cur_i2c_index = sensor_ctrl->sensor_data->cur_i2c_index;
    i2c_attr = &(sensor_ctrl->sensor_data->i2c_attr[cur_i2c_index]);
    sAPI_CamWriteReg(i2c_attr,0xfe, 0x00);

    switch (level){
        case CAM_CONTRAST_HIGH:
            sAPI_CamWriteReg(i2c_attr,0xd3, 0x48);
            break;
        case CAM_CONTRAST_MEDIUM:
            sAPI_CamWriteReg(i2c_attr,0xd3, 0x40);
            break;
        case CAM_CONTRAST_LOW:
            sAPI_CamWriteReg(i2c_attr,0xd3, 0x38);
            break;
        default:
            sAPI_Debug("invalid contrast level %d", level);
            ret = -1;
    }
    if(0 == ret)
    {
        global_cam_config = sAPI_CamGetGlobalConfig();
        global_cam_config->cam_contrast = level;
    }
    return ret;

}

/**
  * @brief  GC032a set YUV exposure
  * @param  sensor_ctrl---sensor control configuration
  * @param  level---exposure level
  * @note
  * @retval 0
  */
int GC032a_YUV_set_exposure(sc_cam_ctrl_t *sensor_ctrl, UINT16 level)
{
    int ret = 0;
    int cur_i2c_index = 0;
    struct cam_i2c_attr *i2c_attr = NULL;
    sc_cam_global_config_t *global_cam_config = NULL;
    sAPI_Debug("GC032a_YUV_set_exposure level %d", level);

    cur_i2c_index = sensor_ctrl->sensor_data->cur_i2c_index;
    i2c_attr = &(sensor_ctrl->sensor_data->i2c_attr[cur_i2c_index]);
    switch (level){
        case CAM_EV_NEG_4_3:
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x01);
            sAPI_CamWriteReg(i2c_attr,0x13, 0x20);
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x00);
            sAPI_CamWriteReg(i2c_attr,0xd5, 0xc0);
            break;
        case CAM_EV_NEG_3_3:
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x01);
            sAPI_CamWriteReg(i2c_attr,0x13, 0x30);
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x00);
            sAPI_CamWriteReg(i2c_attr,0xd5, 0xd0);
            break;
        case CAM_EV_NEG_2_3:
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x01);
            sAPI_CamWriteReg(i2c_attr,0x13, 0x40);
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x00);
            sAPI_CamWriteReg(i2c_attr,0xd5, 0xe0);
            break;
        case CAM_EV_NEG_1_3:
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x01);
            sAPI_CamWriteReg(i2c_attr,0x13, 0x50);
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x00);
            sAPI_CamWriteReg(i2c_attr,0xd5, 0xf0);
            break;
        case CAM_EV_ZERO:
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x01);
            sAPI_CamWriteReg(i2c_attr,0x13, 0x60);
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x00);
            sAPI_CamWriteReg(i2c_attr,0xd5, 0x00);
            break;
        case CAM_EV_POS_1_3:
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x01);
            sAPI_CamWriteReg(i2c_attr,0x13, 0x70);
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x00);
            sAPI_CamWriteReg(i2c_attr,0xd5, 0x10);
            break;
        case CAM_EV_POS_2_3:
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x01);
            sAPI_CamWriteReg(i2c_attr,0x13, 0x80);
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x00);
            sAPI_CamWriteReg(i2c_attr,0xd5, 0x20);
            break;
        case CAM_EV_POS_3_3:
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x01);
            sAPI_CamWriteReg(i2c_attr,0x13, 0x90);
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x00);
            sAPI_CamWriteReg(i2c_attr,0xd5, 0x30);
            break;
        case CAM_EV_POS_4_3:
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x01);
            sAPI_CamWriteReg(i2c_attr,0x13, 0xa0);
            sAPI_CamWriteReg(i2c_attr,0xfe, 0x00);
            sAPI_CamWriteReg(i2c_attr,0xd5, 0x40);
            break;
        default:
            sAPI_Debug("invalid exposure level %d", level);
            ret = -1;
    }
    if(0 == ret)
    {
        global_cam_config = sAPI_CamGetGlobalConfig();
        global_cam_config->cam_exposure = level;
    }

    return ret;

}

/**
  * @brief  GC032a set YUV framesize
  * @param  sensor_ctrl---sensor control configuration
  * @param  level---framesize level
  * @note
  * @retval 0
  */
int GC032a_YUV_set_framesize(sc_cam_ctrl_t *sensor_ctrl, UINT16 level)
{
    int ret = 0;
    int cur_i2c_index = 0;
    UINT16 w,h,row_s,col_s;
    struct cam_i2c_attr *i2c_attr = NULL;
    sc_cam_global_config_t *global_cam_config = NULL;
    sAPI_Debug("GC032a_YUV_set_framesize level %d", level);
    if (level > FRAMESIZE_VGA) {
        sAPI_Debug("Invalid framesize");
        level = FRAMESIZE_VGA;
    }

    w = framesize[level].cam_width;
    h = framesize[level].cam_height;
    row_s = (framesize[FRAMESIZE_VGA].cam_height - h) / 2;
    col_s = (framesize[FRAMESIZE_VGA].cam_width - w) / 2;

    cur_i2c_index = sensor_ctrl->sensor_data->cur_i2c_index;
    i2c_attr = &(sensor_ctrl->sensor_data->i2c_attr[cur_i2c_index]);

    sAPI_CamWriteReg(i2c_attr, 0xfe, 0x00);
    sAPI_CamWriteReg(i2c_attr, 0x09, H8(row_s)); // Row_start[8]
    sAPI_CamWriteReg(i2c_attr, 0x0a, L8(row_s)); // Row_start[7:0]
    sAPI_CamWriteReg(i2c_attr, 0x0b, H8(col_s)); // Column_start[9:8]
    sAPI_CamWriteReg(i2c_attr, 0x0c, L8(col_s)); // Column_start[7:0]
    sAPI_CamWriteReg(i2c_attr, 0x0d, H8(h + 8)); // window_height [8]
    sAPI_CamWriteReg(i2c_attr, 0x0e, L8(h + 8)); // window_height [7:0]
    sAPI_CamWriteReg(i2c_attr, 0x0f, H8(w + 8)); // window_width [9:8]
    sAPI_CamWriteReg(i2c_attr, 0x10, L8(w + 8)); // window_width [7:0]

    sAPI_CamWriteReg(i2c_attr, 0x50, 0x01);
    sAPI_CamWriteReg(i2c_attr, 0x55, H8(h));
    sAPI_CamWriteReg(i2c_attr, 0x56, L8(h));
    sAPI_CamWriteReg(i2c_attr, 0x57, H8(w));
    sAPI_CamWriteReg(i2c_attr, 0x58, L8(w));

    sAPI_CamWriteReg(i2c_attr, 0xfe, 0x03);
    sAPI_CamWriteReg(i2c_attr, 0x5b, L8(w));
    sAPI_CamWriteReg(i2c_attr, 0x5c, H8(w));
    sAPI_CamWriteReg(i2c_attr, 0x5d, L8(h));
    sAPI_CamWriteReg(i2c_attr, 0x5e, H8(h));

    sAPI_CamIspsizeSet(w,h,w,h);

    global_cam_config = sAPI_CamGetGlobalConfig();
    global_cam_config->cam_image_size = level;

    return ret;
}

/**
  * @brief  GC032a set YUV mirror
  * @param  sensor_ctrl---sensor control configuration
  * @param  level---mirror level
  * @note
  * @retval 0
  */
int GC032a_YUV_set_mirror(sc_cam_ctrl_t *sensor_ctrl, UINT16 level)
{
    int ret = 0;
    int cur_i2c_index = 0;
    struct cam_i2c_attr *i2c_attr = NULL;
    sc_cam_global_config_t *global_cam_config = NULL;
    sAPI_Debug("GC032a_YUV_set_mirror level %d", level);

    cur_i2c_index = sensor_ctrl->sensor_data->cur_i2c_index;
    i2c_attr = &(sensor_ctrl->sensor_data->i2c_attr[cur_i2c_index]);

    sAPI_CamWriteReg(i2c_attr, 0xfe, 0x00);
    switch (level){
        case HAL_MIRROR_H:
            sAPI_CamWriteReg(i2c_attr,0x17, 0x55);
            break;
        case HAL_MIRROR_V:
            sAPI_CamWriteReg(i2c_attr,0x17, 0x56);
            break;
        case HAL_MIRROR_HV:
            sAPI_CamWriteReg(i2c_attr,0x17, 0x57);
            break;
        case HAL_MIRROR_NORMAL:
            sAPI_CamWriteReg(i2c_attr,0x17, 0x54);
            break;
        default:
            sAPI_Debug("invalid mirror val %d", level);
            ret = -1;
    }
    if(0 == ret)
    {
        global_cam_config = sAPI_CamGetGlobalConfig();
        global_cam_config->cam_mirror = level;
    }

    return ret;
}

struct cam_sensor_spec_ops GC032a_ops = {
    NULL,
    NULL,
    NULL,
    GC032a_s_power,
    NULL,
    NULL,
    GC032a_YUV_set_effect,
    NULL,
    GC032a_YUV_set_contrast,
    NULL,
    NULL,
    NULL,
    GC032a_YUV_set_exposure,
    GC032a_YUV_set_framesize,
    GC032a_YUV_set_mirror,
};

sc_cam_data_t CAM_GC032a_spi = {
    "gc032a_spi",
    &GC032a_ops,
    SC_CAMERA_BACK,
    GC032a_i2c_attr,
    ARRAY_SIZE(GC032a_i2c_attr),
    {
        GC032a_id,
        ARRAY_SIZE(GC032a_id),
    },
    GC032a_resolution,
    ARRAY_SIZE(GC032a_resolution),
    {
        GC032a_stream_on,
        ARRAY_SIZE(GC032a_stream_on),
    },
    {
        GC032a_stream_off,
        ARRAY_SIZE(GC032a_stream_off),
    },
    SC_CAM_INF_SPI,
    SC_SPI_2_LAN,
    0,   //0x0:sdr   0x1:ddr spi_sdr
    0,   //0x0:no crc   0x1:crc,.spi_crc
    100, //.reset_delay
    0,   //auto set .cur_i2c_index
    0,   //int cur_res_index
};
#endif