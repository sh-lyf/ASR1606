#ifndef __A7630C_LANS_GPIO_H__
#define __A7630C_LANS_GPIO_H__


/* A7630_LANS GPIO Resource Sheet  */
typedef enum
{
    SC_MODULE_GPIO_NOT_ASSIGNED = -1,

    SC_MODULE_GPIO_00 = 71,  /*  pin1, pad name: GPIO_0/SSP_CLK  */
    SC_MODULE_GPIO_01 = 73,  /*  pin2, pad name: GPIO_1/SSP_MISO  */
    SC_MODULE_GPIO_02 = 74,  /*  pin3, pad name: GPIO_2/SSP_MOSI  */
    SC_MODULE_GPIO_03 = 72,  /*  pin4, pad name: GPIO_3/SSP_CS  */
    SC_MODULE_GPIO_04 = 21,  /*  pin10, pad name: GPIO_4  */

    SC_MODULE_GPIO_05 = 25,  /*  pin11, pad name: GPIO_5  */
    SC_MODULE_GPIO_06 = 26,  /*  pin12, pad name: GPIO_6  */
    SC_MODULE_GPIO_07 = 22,  /*  pin13, pad name: GPIO_7  */
    SC_MODULE_GPIO_08 = 24,  /*  pin14, pad name: GPIO_8  */
    SC_MODULE_GPIO_09 = 23,  /*  pin15, pad name: GPIO_9  */

    SC_MODULE_GPIO_10 = 78,  /*  pin16, pad name: GPIO_10  */
    SC_MODULE_GPIO_12 = 20,  /*  pin62, pad name: GPIO_12  */
    SC_MODULE_GPIO_13 = 14,  /*  pin63, pad name: GPIO_13  */
    SC_MODULE_GPIO_14 = 77,  /*  pin64, pad name: GPIO_14  */
    SC_MODULE_GPIO_15 = 13,  /*  pin65, pad name: GPIO_15  */

    SC_MODULE_GPIO_16 = 15,  /*  pin66, pad name: GPIO_16  */
    SC_MODULE_GPIO_17 = 12,  /*  pin67, pad name: GPIO_17  */
    SC_MODULE_GPIO_19 = 7,   /*  pin69, pad name: GPIO_19  */
    SC_MODULE_GPIO_20 = 80,  /*  pin70, pad name: GPIO_20  */
    SC_MODULE_GPIO_21 = 0,   /*  pin41, pad name: GPIO_21  */

    SC_MODULE_GPIO_22 = 1,   /*  pin42, pad name: GPIO_22  */
    SC_MODULE_GPIO_25 = 32,  /*  pin48, pad name: GPIO_25  */
    SC_MODULE_GPIO_26 = 3,   /*  pin49, pad name: GPIO_26  */
    SC_MODULE_GPIO_27 = 4,   /*  pin50, pad name: GPIO_27  */
    SC_MODULE_GPIO_28 = 6,   /*  pin52, pad name: GPIO_28  */

    SC_MODULE_GPIO_29 = 2,   /*  pin53, pad name: GPIO_29  */
    SC_MODULE_GPIO_30 = 79,  /*  pin54, pad name: GPIO_30  */
    SC_MODULE_GPIO_31 = 17,  /*  pin58, pad name: GPIO_31  */
    SC_MODULE_GPIO_32 = 19,  /*  pin59, pad name: GPIO_32  */
    SC_MODULE_GPIO_33 = 18,  /*  pin60, pad name: GPIO_33  */

    SC_MODULE_GPIO_34 = 16,  /*  pin61, pad name: GPIO_34  */
    SC_MODULE_GPIO_35 = 9,   /*  pin55, pad name: GPIO_35/USB_BOOT/PWM4  */
    SC_MODULE_GPIO_36 = 11,  /*  pin56, pad name: GPIO_36/I2C_2_SDA/MK_IN5  */
    SC_MODULE_GPIO_37 = 10,  /*  pin57, pad name: GPIO_37/I2C_2_SCL/MK_OUT5  */
#ifdef SIMCOM_A7630C_ST
    SC_MODULE_GPIO_38 = 54,   /*  pin33 UART3 TXD /UART_RTS */
    SC_MODULE_GPIO_39 = 53,   /*  pin34 UART3 RXD /UART_CTS  */
#endif
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_40 = 70,   /*  pin39, pad name: DTR  */
#endif
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_41 = 69,   /*  pin40, pad name: RI  */
#endif

    SC_MODULE_GPIO_MAX = 128,
} SC_Module_GPIONumbers;

#endif