#ifndef __A7680C_LANS_GPIO_H__
#define __A7680C_LANS_GPIO_H__

/* A7680C_LANS GPIO Resource Sheet  */
typedef enum
{
    SC_MODULE_GPIO_00 = 73,  /*  pin3, pad name: GPIO_00/UART4_CTS */
    SC_MODULE_GPIO_01 = 74,  /*  pin4, pad name: GPIO_01/UART4_RTS */
    SC_MODULE_GPIO_02 = 124, /*  pin5, pad name: GPIO_02  */
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_03 = 0,   /*  pin6, pad name: GPIO_03  */
#endif
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_04 = 10,  /*  pin7, pad name: GPIO_04  */
#endif
    SC_MODULE_GPIO_05 = 9,   /*  pin20, pad name: GPIO_05  */
    SC_MODULE_GPIO_06 = 36,  /*  pin22, pad name: GPIO_06/UART3_TX/NC  */
    SC_MODULE_GPIO_07 = 35,  /*  pin23, pad name: GPIO_07/UART3_RX/NC  */
    SC_MODULE_GPIO_08 = 121, /*  pin41, pad name: GPIO_08  */
    SC_MODULE_GPIO_09 = 06,  /*  pin42, pad name: GPIO_09/PWM3  */
    SC_MODULE_GPIO_10 = 78,  /*  pin43, pad name: GPIO_10  */
    SC_MODULE_GPIO_11 = 1,   /*  pin44, pad name: GPIO_11  */

    SC_MODULE_GPIO_12 = 4,   /*  pin57, pad name: GPIO_12  */
    SC_MODULE_GPIO_13 = 17,  /*  pin62, pad name: GPIO_13/SPI0_FRM  */
    SC_MODULE_GPIO_14 = 16,  /*  pin63, pad name: GPIO_14/SPI0_LCK  */
    SC_MODULE_GPIO_15 = 18,  /*  pin64, pad name: GPIO_15/SPI0_MISO  */
    SC_MODULE_GPIO_16 = 19,  /*  pin65, pad name: GPIO_16/SPI0_MOSI  */
    SC_MODULE_GPIO_17 = 50,  /*  pin68, pad name: GPIO_17/CI2C_SDA  */
    SC_MODULE_GPIO_18 = 49,  /*  pin69, pad name: GPIO_18/CI2C_SCL  */


    SC_MODULE_GPIO_MAX = 128,
} SC_Module_GPIONumbers;

#endif