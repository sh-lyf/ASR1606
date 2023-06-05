/**
  ******************************************************************************
  * @file    mqtt_OneNET.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source code for mqtt OneNet cloud service.
  
	  1,clientID:device_name;
	  user_name:product_ID;
	  password:sign(token algorithm);

	  version:2018-10-31
	  res:products/product_ID/devices/device_name
	  et:Fatsys_time
	  method:sha1,sha256,md
	  sign:

	  StringForSignature:StringForSignature = et + '\n' + method + '\n' + res+ '\n' + version
	2,accessKey:prduct secret;
	  sign = base64(hmac_<method>(base64decode(accessKey), utf-8(StringForSignature)))
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "simcom_os.h"
#include "token.h"
#include "simcom_debug.h"
#include "token.h"
//#include "simcom_ntp_client.h"
/**
  * @brief  Generate URL according to input OneNET configuration parameters.
  * @param  product_ID
  * @param  device_name
  * @param  device_secret
  * @param  cfg_client_id
  * @param  cfg_host_addr
  * @param  cfg_username
  * @param  cfg_password
  * @note   
  * @retval void
  */
void Cfg_mqtt_service(char *product_ID,char *device_name,char *device_secret,  char *cfg_client_id,char *cfg_host_addr,char *cfg_username, char *cfg_password)
{   
/*username*/
    snprintf(cfg_username,20,"%s",product_ID);
    sAPI_Debug("func[%s] line[%d] MQTT get cfg cfg_username:%s,product_ID is:%s", __FUNCTION__,__LINE__,cfg_username,product_ID);

/*host_addr*/
    snprintf(cfg_host_addr,100,"tcp://183.230.40.96:1883");//URL
    sAPI_Debug("func[%s] line[%d] MQTT get cfg cfg_host_addr:%s", __FUNCTION__,__LINE__,cfg_host_addr);

/*client_id*/
    snprintf(cfg_client_id,20,"%s",device_name);//clientID
    sAPI_Debug("func[%s] line[%d] MQTT get cfg client_id:%s,device name is:%s", __FUNCTION__,__LINE__,cfg_client_id,device_name);

/*password*/
    /*version*/
    char * version;
    version = (char*)sAPI_Malloc(20);
    snprintf(version,20,"2018-10-31");
    sAPI_Debug( "func[%s] line[%d] MQTT get cfg version:%s", __FUNCTION__,__LINE__,version);
    
    /*res*/
    char *ten_res;
    int res_len = strlen(product_ID)+strlen(device_name)+25;
    ten_res = (char *)sAPI_Malloc(res_len);
    snprintf(ten_res,res_len,"products/%s/devices/%s",product_ID,device_name);
    sAPI_Debug( "func[%s] line[%d] MQTT get ten_res:%s", __FUNCTION__,__LINE__,ten_res);
#if 0
    /*et*/
    SCsysTime_t currUtcTime;
    sAPI_GetSysLocalTime(&currUtcTime);
    int time = ((currUtcTime.tm_year - 1970)*365*24*60*60) + (currUtcTime.tm_mon*30*24*3600) + (currUtcTime.tm_mday *24*3600) +(currUtcTime.tm_hour*3600) +(currUtcTime.tm_min*60)+currUtcTime.tm_sec;
#endif
    long time = 1608336000;
    unsigned int et = (time + 60*60*24*365);
    sAPI_Debug( "func[%s] line[%d] MQTT get et:%d", __FUNCTION__,__LINE__,et);

    /*method*/
    char *method;
    method = (char *)sAPI_Malloc(10);
    snprintf(method,10,"sha1");
    sAPI_Debug( "func[%s] line[%d] MQTT get method:%s", __FUNCTION__,__LINE__,method);

    /*sign&password*/
    char * password;
    password = (char *)sAPI_Malloc(500);
    //size_t src_len = strlen(device_secret);
    unsigned short password_lenth = 400;
   // et=1630165395;
    OTA_Authorization(version, ten_res, et, device_secret, password, password_lenth); //get sign,construct the password(before URL),and then change the format to URL  
    //snprintf(cfg_password,strlen(password)+1,"%s",password);
    memcpy(cfg_password,password,strlen(password));
    if(version != NULL)
        sAPI_Free(version);
    if(ten_res != NULL)
        sAPI_Free(ten_res);
    if(method != NULL)
        sAPI_Free(method);
    if(password != NULL)
        sAPI_Free(password);
}



