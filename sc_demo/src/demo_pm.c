/**
  ******************************************************************************
  * @file    demo_pm.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of power system management.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "sc_power.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "simcom_os.h"
#include "simcom_common.h"

extern sMsgQRef simcomUI_msgq;
extern SIM_MSG_T GetParamFromUart(void);
extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(char *format);

/**
  * @brief  Power management demo
  * @param  void
  * @note
  * @retval void
  */
void PMUDemo(void)
{
    SIM_MSG_T optionMsg = {0, 0, 0, NULL};
    int opt;
    char buff[100] = {0};

    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] = {
        "1. detect power key status",
        "2. onoff power key shutdown",
        "3. power up&down event",
        "4. ADC votalge",
        "5. VBAT voltage",
        "6. set VDD_AUX",
        "7. get reset key status",
        "8. POWER-OFF",
        "9. POWER-RESET",
        "99. back",
    };

    while (1) {
        PrintfResp(note);
        PrintfOptionMenu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        sAPI_MsgQRecv(simcomUI_msgq, &optionMsg, SC_SUSPEND);
        if (SRV_UART != optionMsg.msg_id) {
            printf("%s,msg_id is error!!", __func__);
            break;
        }

        printf("arg3 = [%s]", (char *)optionMsg.arg3);
        opt = atoi(optionMsg.arg3);
        free(optionMsg.arg3);

        switch (opt) {
        case 1: {
            int onoff = -1;

            onoff = sAPI_GetPowerKeyStatus();
            sprintf(buff, "\r\n PowerKeyStaus  = %d \r\n", onoff);
            PrintfResp(buff);
        }
        break;

        case 2: {
            POWER_KEY_STATUS onoff = -1;
            int ret;

            PrintfResp("\r\n select : \r\n");
            PrintfResp("\r\n  0: disable \r\n");
            PrintfResp("\r\n  1: open \r\n");
            PrintfResp("\r\nany: just return current status \r\n");

            optionMsg = GetParamFromUart();
            onoff = atoi(optionMsg.arg3);
            free(optionMsg.arg3);

            ret = sAPI_SetPowerKeyOffFunc(onoff, 50);   //50 x 100ms: about 5S
            sprintf(buff, "\r\n PowerKeyOffFunc %d \r\n", ret);
            PrintfResp(buff);
        }
        break;

        case 3: {
            sprintf(buff, "powerup = %d\r\n", sAPI_GetPowerUpEvent());
            PrintfResp(buff);

            sprintf(buff, "powerdown = %d\r\n", sAPI_GetPowerDownEvent());
            PrintfResp(buff);

            break;
        }

        case 4: {
            sprintf(buff, "ADC1 VOL = %dmv\r\nADC2 VOL = %dmv\r\n", sAPI_ReadAdc(1), sAPI_ReadAdc(2));
            PrintfResp(buff);

            break;
        }

        case 5: {
            sprintf(buff, "VBAT VOL = %dmv\r\n", sAPI_ReadVbat());
            PrintfResp(buff);

            break;
        }

        case 6: {
            unsigned int voltage = 0;
            PrintfResp("\r\nPlease input voltage.\r\n");
            optionMsg = GetParamFromUart();
            voltage = atoi(optionMsg.arg3);
            free(optionMsg.arg3);

            if (voltage > 3000 || voltage < 2500) {
                PrintfResp("\r\nPlease input correct voltage.\r\n");
                break;
            }

            if (sAPI_SetVddAux(voltage) == 0) {
                sprintf(buff, "set VDD_AUX %dmv success!\r\n", voltage);
                PrintfResp(buff);
            }

            break;
        }

        case 8:
            sAPI_SysPowerOff();
            break;

        case 9:
            sAPI_SysReset();
            break;

        case 99:
            return;

        default:
            break;
        }
    }
}



