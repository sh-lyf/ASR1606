#ifndef __A7670C_FANS_GPIO_H__
#define __A7670C_FANS_GPIO_H__

/* A7670C_FANS GPIO Resource Sheet  */
typedef enum
{
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_00 = 31,  /*  pin4, pad name: RI  */
#endif
    SC_MODULE_GPIO_01 = 32,  /*  pin5, pad name: GPIO_01 */
    SC_MODULE_GPIO_02 = 9,   /*  pin6, pad name: GPIO_02/PWM4  */
    SC_MODULE_GPIO_03 = 54,  /*  pin7, pad name: GPIO_03 */
    SC_MODULE_GPIO_04 = 53,   /*  pin8, pad name: GPIO_04  */
    SC_MODULE_GPIO_05 = 120, /*  pin19, pad name: GPIO_05  */
    SC_MODULE_GPIO_07 = 7,   /*  pin21, pad name: GPIO_07/MK_OUT_3  */
    SC_MODULE_GPIO_08 = 77,  /*  pin48, pad name: GPIO_08  */
    SC_MODULE_GPIO_09 = 28,  /*  pin52, pad name: GPIO_09  */
    SC_MODULE_GPIO_10 = 37,  /*  pin66, pad name: GPIO_10  */

    SC_MODULE_GPIO_11 = 11,  /*  pin67, pad name: GPIO_11/MK_OUT_5  */
    SC_MODULE_GPIO_12 = 10,  /*  pin68, pad name: GPIO_12/MK_IN_5/PWM3  */
    SC_MODULE_GPIO_13 = 13,  /*  pin35, pad name: GPIO_13/MK_OUT_6/I2C3_SDA  */
    SC_MODULE_GPIO_14 = 12,  /*  pin36, pad name: GPIO_14/MK_IN_6/I2C3_SCL  */
    SC_MODULE_GPIO_15 = 5,   /*  pin44, pad name: GPIO_15/MK_OUT_2  */
    SC_MODULE_GPIO_16 = 4,   /*  pin47, pad name: GPIO_16/MK_IN_2  */
    SC_MODULE_GPIO_17 = 124, /*  pin26, pad name: GPIO_17  */
    SC_MODULE_GPIO_18 = 118, /*  pin53, pad name: GPIO_18/PWM2  */
    SC_MODULE_GPIO_19 = 126, /*  pin101, pad name: GPIO_19/LCD_BL_PWM/PWM1  */
    SC_MODULE_GPIO_20 = 33,  /*  pin102, pad name: GPIO_20/LCD_SPI_CLK/SPI2_CLK */

    SC_MODULE_GPIO_21 = 36,  /*  pin103, pad name: GPIO_21/LCD_SPI_TXD/SPI2_TXD */
    SC_MODULE_GPIO_22 = 35,  /*  pin104, pad name: GPIO_22/LCD_SPI_RXD/SPI2_RXD */
    SC_MODULE_GPIO_23 = 34,  /*  pin105, pad name: GPIO_23/LCD_SPI_CS/SPI2_FRM */
    SC_MODULE_GPIO_24 = 78,  /*  pin106, pad name: GPIO_24/LCD_RST */
    SC_MODULE_GPIO_25 = 121, /*  pin107, pad name: GPIO_25/LCD_DCX */
    SC_MODULE_GPIO_26 = 26,  /*  pin117, pad name: GPIO_26/CAM_I2C_SCL  */
    SC_MODULE_GPIO_27 = 25,  /*  pin118, pad name: GPIO_27/CAM_I2C_SDA  */
    SC_MODULE_GPIO_28 = 69,  /*  pin119, pad name: GPIO_28/CAM_PWN  */
    SC_MODULE_GPIO_29 = 70,  /*  pin120, pad name: GPIO_29/CAM_RST  */
    SC_MODULE_GPIO_30 = 21,  /*  pin121, pad name: GPIO_30/CAM_MCLK  */

    SC_MODULE_GPIO_31 = 24, /*  pin122, pad name: GPIO_31/CAM_SPI_DO  */
    SC_MODULE_GPIO_32 = 23, /*  pin123, pad name: GPIO_32/CAM_SPI_DI  */
    SC_MODULE_GPIO_33 = 22, /*  pin124, pad name: GPIO_33/CAM_SPI_CLK  */
    SC_MODULE_GPIO_34 = 16, /*  pin11, pad name: GPIO_34/SSP0_CLK/PCM_CLK  */
    SC_MODULE_GPIO_35 = 17, /*  pin12, pad name: GPIO_35/SSP0_FRM/PCM_SYNC  */
    SC_MODULE_GPIO_36 = 19, /*  pin13, pad name: GPIO_36/SSP0_TXD/PCM_DIN  */
    SC_MODULE_GPIO_37 = 18, /*  pin14, pad name: GPIO_37/SSP0_RXD/PCM_DOUT  */
    SC_MODULE_GPIO_38 = 50, /*  pin37, pad name: GPIO_39/CI2C_SDA  */
    SC_MODULE_GPIO_39 = 49, /*  pin38, pad name: GPIO_40/CI2C_CLK  */

    SC_MODULE_GPIO_MAX = 128,
} SC_Module_GPIONumbers;

#endif