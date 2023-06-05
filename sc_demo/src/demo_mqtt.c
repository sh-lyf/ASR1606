/**
  ******************************************************************************
  * @file    demo_mqtt.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of MQTT operation.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#ifdef FEATURE_SIMCOM_MQTT
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "simcom_mqtts_client.h"
//#include "simcom_network.h"
#include "simcom_debug.h"
#include "simcom_uart.h"
#include "simcom_common.h"
#include "core_auth.h"
#include "simcom_network.h"

typedef enum{
    SC_MQTTS_DEMO_ALIYUN      = 1,
    SC_MQTTS_DEMO_TENCENT     = 2,
    SC_MQTTS_DEMO_ONENET      = 3,
    SC_MQTTS_DEMO_TWING       = 4,
    SC_MQTTS_DEMO_MAX         = 99
}SC_MQTTS_DEMO_CLOUD_TYPE;

typedef enum{
	SC_MQTTS_DEMO_START       = 1,
	SC_MQTTS_DEMO_CONNECT     = 2,
	SC_MQTTS_DEMO_SUBSCRIBE   = 3,
	SC_MQTTS_DEMO_PUBLISH     = 4,
	SC_MQTTS_DEMO_UNSUBSCRIBE = 5,
	SC_MQTTS_DEMO_DISCONNECT  = 6,
	SC_MQTTS_DEMO_RELEASE     = 7,
	SC_MQTTS_DEMO_STOP        = 8,
}SC_MQTTS_DEMO_YUNPLATFORM_TYPE;

#if 0
typedef enum{
    SC_MQTTS_DEMO_START       = 1,
    SC_MQTTS_DEMO_ACCQ        = 2,
    SC_MQTTS_DEMO_CCF         = 3,
    SC_MQTTS_DEMO_CONN        = 4,
    SC_MQTTS_DEMO_SUB         = 5,
    SC_MQTTS_DEMO_PUB         = 6,
    SC_MQTTS_DEMO_UNSUB       = 7,
    SC_MQTTS_DEMO_DISCONN     = 8,
    SC_MQTTS_DEMO_REL         = 9,
    SC_MQTTS_DEMO_STOP        = 10,
    SC_MQTTS_DEMO_MAX         = 99
}SC_MQTTS_DEMO_TYPE;
#endif
#if 0
static sTaskRef gMqttProcessTask = NULL;
static UINT8 gMqttProcessTaskStack[SC_DEFAULT_THREAD_STACKSIZE*2] = {0xA5};
#endif

static sTaskRef gMqttRecvTask = NULL;
static UINT8 gMqttRecvTaskStack[SC_DEFAULT_THREAD_STACKSIZE*2];


extern sMsgQRef urc_mqtt_msgq_1;

sMsgQRef uart_mqtt_msgq_1;

extern sMsgQRef simcomUI_msgq;
extern void PrintfOptionMenu(char* options_list[], int array_size);
extern void PrintfResp(char* format);

/**
  * @brief notify MQTT lost connection
  * @param  int index, int cause;
  * @note   *1.Advice: using it before connection every time,
         since it will be cleared after using sAPI_MqttRel().
            *2.Must not call function of reconnection directly,
         if did it will cause receiving data of mqtt fail.
         Advice: Using sAPI_MsgQSend() send a MsgQ to notify
         other task of reconnection.
  * @retval void
*/
static void app_MqttLostConnCb(int index, int cause)
{
    int pGreg = 0;
    char notice[100]={0};
    UINT32 i = 0;
    snprintf(notice ,sizeof(notice)-1, "\r\nMQTT LostConn Notice:index_%d,cause_%d\r\n", index, cause);
    PrintfResp(notice);
    memset(notice, 0, sizeof(notice));

    //Advice: Using sAPI_MsgQSend() send a MsgQ to notify other task of reconnection.
    //sAPI_MsgQSend(...);
    
/*MUST!!! other task to do it. --Start--*/
#if 1
    /*Reconnection*/
    //network?
    while(1)
    {
        sAPI_NetworkGetCgreg(&pGreg);
        //network ->> OK
        if(1 == pGreg || 5 == pGreg)
        {
            sAPI_Debug("MQTT NETWORK STATUS IS NORMAL");
            break;
        }
        //network ->> ERROE,sleep 10s
        else
        {
            sAPI_Debug("MQTT NETWORK STATUS IS [%d]",pGreg);
            sAPI_TaskSleep(10*200);
        }
        i++;
    }

    /*to reconnect*/
    if(i==0)
    {
        //when network ok, call sAPI_MqttConnect
        snprintf(notice ,sizeof(notice)-1, "\r\nMQTT NETWORK STATUS IS NORMAL\r\n");
        PrintfResp(notice);
    }
    else
    {
        //when netword ERROR -> OK, must release and stop MQTT(you used),then restart MQTT
        /*example:
        sAPI_MqttRel(0/1);//(you used)
        sAPI_MqttStop();

        sAPI_MqttStart(-1);
        sAPI_MqttAccq(...);
        sAPI_MqttConnect(...);
        ...
        */
        snprintf(notice ,sizeof(notice)-1, "\r\nMQTT NETWORK ERROR -> OK,close then open MQTT\r\n");
        PrintfResp(notice);
    }
#endif 
/*MUST!!! other task to do it. --End--*/
}

/**
  * @brief  MQTT task processor
  * @param  void
  * @note   
  * @retval void
  */
 void sTask_MqttProcesser(void)                 //thread of control
 {
    int ret = 0;
    int branch = 0;
    //INT32 pGreg = 0;
	SIM_MSG_T optionMsg ={0,0,0,NULL};
	INT8* rxbuf=NULL;
    int readsize=0;
#if 0
    while(1)
    {
        sAPI_NetworkGetCgreg(&pGreg);
        if(0 != pGreg)
        {
            sAPI_Debug("MQTT NETWORK STATUS IS [%d]",pGreg);
            sAPI_TaskSleep(10*300);
        }
        else
        {
            sAPI_Debug("MQTT NETWORK STATUS IS NORMAL");
            break;
        }
    }
#endif
    while(1)
    {
        switch(branch){
            case 0:    
                ret = sAPI_MqttStart(-1);
                sAPI_Debug("1-----ret = %d", ret);
                
                ret= sAPI_MqttAccq(0, NULL, 0, "614393569", 0, urc_mqtt_msgq_1);
                sAPI_Debug("2-----ret = %d", ret);
                
                ret= sAPI_MqttCfg(0, NULL, 0, 0, 0);
                sAPI_Debug("2.5-----ret = %d", ret);

                //ret = sAPI_MqttSslCfg();
                sAPI_MqttConnLostCb(app_MqttLostConnCb);
                ret= sAPI_MqttConnect(0, NULL, 0, "tcp://183.230.40.39:6002", 60, 1, "364072", "productA");
                sAPI_Debug("3-----ret = %d", ret);
                branch =1;
                break;


            case 1:
                ret= sAPI_MqttSubTopic(0, "$sys/{pid}/{device-name}/dp/post/json/accepted", 7, 1);
                sAPI_Debug("4-----ret = %d", ret);
            
                ret= sAPI_MqttSub(0, NULL, 0, 0, 0);
                sAPI_Debug("4.5-----ret = %d", ret);

                ret = sAPI_MqttSub(0, "apitest1", 8, 0, 0);
                sAPI_Debug("4.6-----ret = %d", ret);
                
                ret = sAPI_MqttSub(0, "apitest2", 8, 0, 0);
                sAPI_Debug("4.7-----ret = %d", ret);
                branch = 2;
                break;



            case 2:    
                ret= sAPI_MqttTopic(0, "$sys/{pid}/{device-name}/dp/post/json/accepted", 7);
                sAPI_Debug("5-----ret = %d", ret);

                sAPI_MsgQRecv(uart_mqtt_msgq_1,&optionMsg,SC_SUSPEND);
				rxbuf = optionMsg.arg3;
				readsize = optionMsg.arg2;

                ret= sAPI_MqttPayload(0, (char *)rxbuf, readsize);
                sAPI_Debug("6-----ret = %d", ret);
                
                ret= sAPI_MqttPub(0, 1, 60, 0, 0);
                sAPI_Debug("7-----ret = %d", ret);
                branch = 2;
				sAPI_Free(optionMsg.arg3);
                break;



            case 3:    
                sAPI_TaskSleep((300) * 200);           //sleep 30 sec before unsub, receive messages published by other clients
                
                ret = sAPI_MqttUnsub(0, "apitest1", 8, 0);
                sAPI_Debug("7.5-----ret = %d", ret);

                sAPI_TaskSleep((30) * 200);           //sleep 30 sec before unsub
                ret = sAPI_MqttUNSubTopic(0, "apitest2", 8);
                sAPI_Debug("7.6-----ret = %d", ret);
                
                ret = sAPI_MqttUnsub(0, NULL, 0, 0);
                sAPI_Debug("7.7-----ret = %d", ret);
                branch = 4;
                break;

            case 4:
                sAPI_TaskSleep((30) * 200);           //sleep 30 sec before disconn
                ret= sAPI_MqttDisConnect(0,NULL, 0, 60);
                sAPI_Debug("8-----ret = %d", ret);
                
                ret= sAPI_MqttRel(0);
                sAPI_Debug("9-----ret = %d", ret);
                
                ret= sAPI_MqttStop();
                sAPI_Debug("10-----ret = %d", ret);
                branch = -1;
				break;
        }
        
        if( -1 == branch)
            break;
    }
    
    sAPI_Debug("-------MQTT TEST-----, finish");
}

/**
  * @brief  MQTT receiving task processor
  * @param  void
  * @note   
  * @retval void
  */    
 void sTask_MqttRecvProcesser(void* arg)                    //thread of sub topic data process
{
    while(1)
    {
        SIM_MSG_T msgQ_data_recv = {SIM_MSG_INIT, 0, -1, NULL};                 //NULL pointer for msgQ_data_recv.arg3 is necessary!
        SCmqttData *sub_data = NULL;
        /*=======================================================================================================================
        *
        *   NOTE: if this data reception cycle too long may cause data loss(data processing slower than data receive from server)
        *
        **=======================================================================================================================*/
        //recv the subscribed topic data, from the message queue: urc_mqtt_msgq_1, it is set buy sAPI_MqttAccq                                          //*===================*//
        sAPI_MsgQRecv(urc_mqtt_msgq_1, &msgQ_data_recv, SC_SUSPEND);                                                                                    //                     //
        if((SC_SRV_MQTT != msgQ_data_recv.msg_id) || (0 != msgQ_data_recv.arg1) || (NULL == msgQ_data_recv.arg3))   //wrong msg received                //                     //
            continue;                                                                                                                                   //                     //
        sub_data = (SCmqttData *)(msgQ_data_recv.arg3);                                                                                                  //                     //
        /*this part just for test, the payload msg just ascii string*/                                                                                  //                     //          
        sAPI_Debug("MQTT TEST----------index: [%d]; tpoic_len: [%d]; tpoic: [%s]", sub_data->client_index, sub_data->topic_len, sub_data->topic_P);     //                     //
                                                                                                                                                        //                     //
        sAPI_Debug("MQTT TEST----------payload_len: [%d]", sub_data->payload_len);                                                                      //   rception cycle    //
		sAPI_UartWrite(SC_UART,(UINT8*)"recieve topic: ",strlen("recieve topic: "));
		sAPI_UartWrite(SC_UART,(UINT8*)sub_data->topic_P,sub_data->topic_len);
		sAPI_UartWrite(SC_UART,(UINT8*)"\r\n",2);
		sAPI_UartWrite(SC_UART,(UINT8*)"recieve payload: ",strlen("recieve payload: "));
		sAPI_UartWrite(SC_UART,(UINT8*)sub_data->payload_P,sub_data->payload_len);                                                                              //                     //    
        sAPI_UartWrite(SC_UART,(UINT8*)"\r\n",2);                                                                                                               //                     //
        /*************************************************************************************/                                                         //                     //
        /*************************************************************************************/                                                         //                     //
        /*these msg pointer must be free after using, don not change the free order*/                                                                   //                     //
            sAPI_Free(sub_data->topic_P);                                                                                                               //                     //
            sAPI_Free(sub_data->payload_P);                                                                                                             //                     //
            sAPI_Free(sub_data);                                                                                                                        //                     //
        /*************************************************************************************/                                                         //                     //
        /*************************************************************************************/                                                         //*===================*//
    }


}
 

 
 #if 0
 void sAPP_MqttTaskDemo(void)
 {
     SC_STATUS status;    
     sAPI_Debug("enter: api_mqtts_demo_init");
     
     if ((NULL != gMqttProcessTask) || (NULL != gMqttRecvTask))
     {
         return;
     }
     
     /*creat msgq*/     
     /*==================================================================================================================
     *  urc_mqtt_msgq_1: queue for receiving the subsribed topic data, data is cached in the queue
     *
     *  MQTT_MSGQ_QUEUE_SIZE: Maximum cached messages in the queue
     *
     *  NOTE: If the data receiving speed is less than the sending speed, the following data will be lost. 
     *        If the reading "-----MSG SEND ERROR-----, The message queue of client-[0/1] is full" in the log record, 
     *        please increase the length of the queue(normal value is 4) or speed up the receiving speed 
     **==================================================================================================================*/
     status = sAPI_MsgQCreate(&urc_mqtt_msgq_1, "urc_mqtt_msgq_1", (sizeof(SIM_MSG_T)), 4, SC_FIFO);        //msgQ for subscribed data transfer
     if(status != SC_SUCCESS)
        sAPI_Debug("message queue creat err!\n");

	 status = sAPI_MsgQCreate(&uart_mqtt_msgq_1, "uart_mqtt_msgq_1", (sizeof(SIM_MSG_T)), 4, SC_FIFO);		//msgQ for subscribed data transfer
	 if(status != SC_SUCCESS)
		sAPI_Debug("message queue creat err!\n");
	 
     /*creat a thread*/
     if(sAPI_TaskCreate(&gMqttProcessTask, gMqttProcessTaskStack, SC_DEFAULT_THREAD_STACKSIZE*2, SC_DEFAULT_TASK_PRIORITY, (char*)"mqttProcesser", sTask_MqttProcesser,(void *)0)) //init the demo task
     {
        gMqttProcessTask = NULL;
        sAPI_Debug("Task Create error!\n");
     }
     
     if(sAPI_TaskCreate(&gMqttRecvTask, gMqttRecvTaskStack, SC_DEFAULT_THREAD_STACKSIZE*2, SC_DEFAULT_TASK_PRIORITY, (char*)"mqttProcesser", sTask_MqttRecvProcesser,(void *)0)) //init the demo task
     {
        gMqttRecvTask = NULL;
        sAPI_Debug("Task Create error!\n");
     }
 }
#endif

extern void Get_Cfg_Parameters(char *product_id,char *device_name,char *device_secret,char *region,  char *cfg_client_id,char *cfg_host_addr,char *cfg_username, char *cfg_password);
/**
  * @brief  Tencet cloud connection demo
  * @param  void
  * @note   
  * @retval void
  */  
 void TencentDemo(void)
 {
     char topic[1025] = {0};
     INT32 topic_len = 0;
     char payload[10241] = {0};
     INT32 payload_len = 0;
     INT32 ret;
     SIM_MSG_T optionMsg ={0,0,0,NULL};
     UINT32 opt = 0;
     char productkey[100];
     char devicename[100];
     char devicesecret[100];
     char host[100] = {0};
     char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
     char *options_list[] = {
         "1. Mqtt Start",
         "2. Connect to tencent cloud",
         "3. Subscribe",
         "4. Publish",
         "5. Unsubscribe",
         "6. Disconnect to tencent cloud",
         "7. Release a client",
         "8. Mqtt Stop",
         "99. back"
     };
 
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
             case SC_MQTTS_DEMO_START:
                 ret = sAPI_MqttStart(-1);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("Init SUCCESS");
                     PrintfResp("\r\nMQTT Init Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("Init FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT Init Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_CONNECT:
                #if 0
                 PrintfResp("\r\nPlease input productkey\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                 topic_len = strlen(optionMsg.arg3);
                 memset(productkey, 0, sizeof(productkey));
                 memcpy(productkey, optionMsg.arg3, strlen(optionMsg.arg3));
                 sAPI_Free(optionMsg.arg3);
 
                 PrintfResp("\r\nPlease input devicename\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                 topic_len = strlen(optionMsg.arg3);
                 memset(devicename, 0, sizeof(productkey));
                 memcpy(devicename, optionMsg.arg3, strlen(optionMsg.arg3));
                 sAPI_Free(optionMsg.arg3);
 
                 PrintfResp("\r\nPlease input devicesecret\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                 topic_len = strlen(optionMsg.arg3);
                 memset(devicesecret, 0, sizeof(productkey));
                 memcpy(devicesecret, optionMsg.arg3, strlen(optionMsg.arg3));
                 sAPI_Free(optionMsg.arg3);
#endif
                 memset(devicename , 0, sizeof(devicename));
                 snprintf(devicename,100,"%s","cxvcxv");
                 memset(devicesecret , 0, sizeof(devicesecret));
                 snprintf(devicesecret,100,"%s","+OFD4uq+l+PtTxypN0yiEg==");
                 memset(productkey , 0, sizeof(productkey));
                 snprintf(productkey,100,"%s","B8KY2ENZ58");


                 char region[20] = "china";
                 char * cfg_host_addr;
                 cfg_host_addr = sAPI_Malloc(64);
                 char * cfg_username;
                 cfg_username = sAPI_Malloc(500);
                 char * cfg_password;
                 cfg_password = sAPI_Malloc(51);
                 char * cfg_client_id;
                 cfg_client_id = sAPI_Malloc(81);
                 Get_Cfg_Parameters(productkey, devicename, devicesecret, region, cfg_client_id, cfg_host_addr, cfg_username, cfg_password);
                 sAPI_Debug( "func[%s] line[%d] MQTT get cfg client_id:%s,%p", __FUNCTION__,__LINE__,cfg_client_id,cfg_client_id);
                 sAPI_Debug( "func[%s] line[%d] MQTT get cfg host_addr:%s,%p", __FUNCTION__,__LINE__,cfg_host_addr,cfg_host_addr);
                 sAPI_Debug( "func[%s] line[%d] MQTT get cfg username:%s,%p", __FUNCTION__,__LINE__,cfg_username,cfg_username);
                 sAPI_Debug( "func[%s] line[%d] MQTT get cfg password:%s,%p", __FUNCTION__,__LINE__,cfg_password,cfg_password);
               
                 
                 memset(host , 0, sizeof(host));
                 sprintf(host, "%s", cfg_host_addr);
                 ret= sAPI_MqttAccq(0, NULL, 0, cfg_client_id, 0, urc_mqtt_msgq_1);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("Init SUCCESS");
                     sAPI_Debug("\r\nMQTT accquire Successful!\r\n");
                     
                 }
                 else
                 {
                     sAPI_Debug("Init FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT accquire Fail!\r\n");
                     break;
                 }
                 sAPI_MqttConnLostCb(app_MqttLostConnCb);
                 ret= sAPI_MqttConnect(0, NULL, 0, host, 60, 0, cfg_username, cfg_password);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("connect SUCCESS");
                     PrintfResp("\r\nMQTT connect Successful!\r\n");
                     sAPI_Free(cfg_host_addr);
                     sAPI_Free(cfg_username);
                     sAPI_Free(cfg_password);
                     sAPI_Free(cfg_client_id);
                     break;
                 }
                 else
                 {
                     sAPI_Debug("connect FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT connect Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_SUBSCRIBE:
                 PrintfResp("\r\nPlease input subtopic\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                 topic_len = strlen(optionMsg.arg3);
                 memset(topic, 0, sizeof(topic));
                 memcpy(topic, optionMsg.arg3, strlen(optionMsg.arg3));
                 sAPI_Free(optionMsg.arg3);
                 ret = sAPI_MqttSub(0, topic, topic_len, 0, 0);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("subscribe SUCCESS");
                     PrintfResp("\r\nMQTT subscribe Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("subscribe FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT subscribe Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_PUBLISH:
                 PrintfResp("\r\nPlease input pubtopic\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                 topic_len = strlen(optionMsg.arg3);
                 memset(topic, 0, sizeof(topic));
                 memcpy(topic, optionMsg.arg3, strlen(optionMsg.arg3));
                 sAPI_Free(optionMsg.arg3);
 
                 ret= sAPI_MqttTopic(0, topic, topic_len);
 
                 PrintfResp("\r\nPlease input payload\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                 payload_len = strlen(optionMsg.arg3);
                 memset(payload, 0, sizeof(payload));
                 memcpy(payload, optionMsg.arg3, strlen(optionMsg.arg3));
                 sAPI_Free(optionMsg.arg3);
                 ret= sAPI_MqttPayload(0, payload, payload_len);
 
                 ret= sAPI_MqttPub(0, 1, 60, 0, 0);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("publish SUCCESS");
                     PrintfResp("\r\nMQTT publish Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("publish FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT publish Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_UNSUBSCRIBE:
                 PrintfResp("\r\nPlease input unsubscribe pubtopic\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                 topic_len = strlen(optionMsg.arg3);
                 memset(topic, 0, sizeof(topic));
                 memcpy(topic, optionMsg.arg3, strlen(optionMsg.arg3));
                 sAPI_Free(optionMsg.arg3);
                 ret = sAPI_MqttUnsub(0, topic, topic_len, 0);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("unsubscribe SUCCESS");
                     PrintfResp("\r\nMQTT unsubscribe Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("unsubscribe FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT unsubscribe Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_DISCONNECT:
                 ret= sAPI_MqttDisConnect(0, NULL, 0, 60);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("disconnect SUCCESS");
                     PrintfResp("\r\nMQTT disconnect Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("disconnect FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT disconnect Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_RELEASE:
                 ret= sAPI_MqttRel(0);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("release SUCCESS");
                     PrintfResp("\r\nMQTT release Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("release FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT release Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_STOP:
                 ret= sAPI_MqttStop();
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("Stop SUCCESS");
                     PrintfResp("\r\nMQTT Stop Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("Stop FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT Stop Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_MAX:
                 sAPI_Debug("Return to the previous menu!");
                 PrintfResp("\r\nReturn to the previous menu!\r\n");
                 ret= sAPI_MqttDisConnect(0,NULL, 0, 60);
                 ret= sAPI_MqttDisConnect(0,NULL, 1, 60);
                 ret= sAPI_MqttRel(0);
                 ret= sAPI_MqttRel(1);
                 ret= sAPI_MqttStop();
                 return;
 
             default:
                 break;
         }
     }
  }

extern void Cfg_mqtt_service(char *product_ID,char *device_name,char *device_secret,  char *cfg_client_id,char *cfg_host_addr,char *cfg_username, char *cfg_password);
/**
  * @brief  OneNet cloud connection demo
  * @param  void
  * @note   
  * @retval void
  */  
 void OnenetDemo(void)
 {
     char topic[1025] = {0};
     INT32 topic_len = 0;
     char payload[10241] = {0};
     INT32 payload_len = 0;
     INT32 ret;
     SIM_MSG_T optionMsg ={0,0,0,NULL};
     UINT32 opt = 0;
     char productkey[100];
     char devicename[100];
     char devicesecret[100];
     char host[100] = {0};
     char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
     char *options_list[] = {
         "1. Mqtt Start",
         "2. Connect to onenet",
         "3. Subscribe",
         "4. Publish",
         "5. Unsubscribe",
         "6. Disconnect to onenet",
         "7. Release a client",
         "8. Mqtt Stop",
         "99. back"
     };
 
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
             case SC_MQTTS_DEMO_START:
                 ret = sAPI_MqttStart(-1);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("Init SUCCESS");
                     PrintfResp("\r\nMQTT Init Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("Init FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT Init Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_CONNECT:
                #if 0
                 PrintfResp("\r\nPlease input productkey\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                 topic_len = strlen(optionMsg.arg3);
                 memset(productkey, 0, sizeof(productkey));
                 memcpy(productkey, optionMsg.arg3, strlen(optionMsg.arg3));
                 sAPI_Free(optionMsg.arg3);
 
                 PrintfResp("\r\nPlease input devicename\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                 topic_len = strlen(optionMsg.arg3);
                 memset(devicename, 0, sizeof(productkey));
                 memcpy(devicename, optionMsg.arg3, strlen(optionMsg.arg3));
                 sAPI_Free(optionMsg.arg3);
 
                 PrintfResp("\r\nPlease input devicesecret\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                 topic_len = strlen(optionMsg.arg3);
                 memset(devicesecret, 0, sizeof(productkey));
                 memcpy(devicesecret, optionMsg.arg3, strlen(optionMsg.arg3));
                 sAPI_Free(optionMsg.arg3);
                 #endif
                 memset(devicename , 0, sizeof(devicename));
                 snprintf(devicename,100,"%s","asr_1601");
                 memset(devicesecret , 0, sizeof(devicesecret));
                 snprintf(devicesecret,100,"%s","ceu9anCCNYKZcYh+ZL283YGSeojVhi+tn7v2nmBrfGI=");
                 memset(productkey , 0, sizeof(productkey));
                 snprintf(productkey,100,"%s","367735");
                 
                char * cfg_host_addr;
                cfg_host_addr = (char *)sAPI_Malloc(100);
                char * cfg_username;
                cfg_username = (char *)sAPI_Malloc(100);
                char * cfg_password;
                cfg_password = (char *)sAPI_Malloc(256+1);
                char * cfg_client_id;
                cfg_client_id = (char *)sAPI_Malloc(100);
                Cfg_mqtt_service(productkey, devicename, devicesecret, cfg_client_id, cfg_host_addr, cfg_username, cfg_password);
                
                sAPI_Debug( "func[%s] line[%d] MQTT get cfg client_id:%s,%p", __FUNCTION__,__LINE__,cfg_client_id,cfg_client_id);
                sAPI_Debug( "func[%s] line[%d] MQTT get cfg host_addr:%s,%p", __FUNCTION__,__LINE__,cfg_host_addr,cfg_host_addr);
                sAPI_Debug( "func[%s] line[%d] MQTT get cfg username:%s,%p", __FUNCTION__,__LINE__,cfg_username,cfg_username);
                sAPI_Debug( "func[%s] line[%d] MQTT get cfg password:%s,%p", __FUNCTION__,__LINE__,cfg_password,cfg_password);
                
                 //_core_mqtt_sign_clean(mqtt_handle);
                 
               
                 
                 memset(host , 0, sizeof(host));
                 sprintf(host, "%s", cfg_host_addr);
                 ret= sAPI_MqttAccq(0, NULL, 0, cfg_client_id, 0, urc_mqtt_msgq_1);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("Init SUCCESS");
                     sAPI_Debug("\r\nMQTT accquire Successful!\r\n");
                     
                 }
                 else
                 {
                     sAPI_Debug("Init FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT accquire Fail!\r\n");
                     break;
                 }
                 sAPI_MqttConnLostCb(app_MqttLostConnCb);
                 ret= sAPI_MqttConnect(0, NULL, 0, host, 60, 1, cfg_username, cfg_password);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("connect SUCCESS");
                     PrintfResp("\r\nMQTT connect Successful!\r\n");
                     sAPI_Free(cfg_username);
                     sAPI_Free(cfg_password);
                     sAPI_Free(cfg_client_id);
                     break;
                 }
                 else
                 {
                     sAPI_Debug("connect FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT connect Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_SUBSCRIBE:
                 PrintfResp("\r\nPlease input subtopic\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                 topic_len = strlen(optionMsg.arg3);
                 memset(topic, 0, sizeof(topic));
                 memcpy(topic, optionMsg.arg3, strlen(optionMsg.arg3));
                 sAPI_Free(optionMsg.arg3);
                 ret = sAPI_MqttSub(0, topic, topic_len, 0, 0);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("subscribe SUCCESS");
                     PrintfResp("\r\nMQTT subscribe Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("subscribe FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT subscribe Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_PUBLISH:
                 PrintfResp("\r\nPlease input pubtopic\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                 topic_len = strlen(optionMsg.arg3);
                 memset(topic, 0, sizeof(topic));
                 memcpy(topic, optionMsg.arg3, strlen(optionMsg.arg3));
                 sAPI_Free(optionMsg.arg3);
 
                 ret= sAPI_MqttTopic(0, topic, topic_len);
 
                 PrintfResp("\r\nPlease input payload\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                 payload_len = strlen(optionMsg.arg3);
                 memset(payload, 0, sizeof(payload));
                 memcpy(payload, optionMsg.arg3, strlen(optionMsg.arg3));
                 sAPI_Free(optionMsg.arg3);
                 ret= sAPI_MqttPayload(0, payload, payload_len);
 
                 ret= sAPI_MqttPub(0, 1, 60, 0, 0);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("publish SUCCESS");
                     PrintfResp("\r\nMQTT publish Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("publish FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT publish Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_UNSUBSCRIBE:
                 PrintfResp("\r\nPlease input unsubscribe pubtopic\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                 topic_len = strlen(optionMsg.arg3);
                 memset(topic, 0, sizeof(topic));
                 memcpy(topic, optionMsg.arg3, strlen(optionMsg.arg3));
                 sAPI_Free(optionMsg.arg3);
                 ret = sAPI_MqttUnsub(0, topic, topic_len, 0);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("unsubscribe SUCCESS");
                     PrintfResp("\r\nMQTT unsubscribe Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("unsubscribe FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT unsubscribe Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_DISCONNECT:
                 ret= sAPI_MqttDisConnect(0, NULL, 0, 60);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("disconnect SUCCESS");
                     PrintfResp("\r\nMQTT disconnect Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("disconnect FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT disconnect Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_RELEASE:
                 ret= sAPI_MqttRel(0);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("release SUCCESS");
                     PrintfResp("\r\nMQTT release Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("release FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT release Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_STOP:
                 ret= sAPI_MqttStop();
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("Stop SUCCESS");
                     PrintfResp("\r\nMQTT Stop Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("Stop FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT Stop Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_MAX:
                 sAPI_Debug("Return to the previous menu!");
                 PrintfResp("\r\nReturn to the previous menu!\r\n");
                 ret= sAPI_MqttDisConnect(0,NULL, 0, 60);
                 ret= sAPI_MqttDisConnect(0,NULL, 1, 60);
                 ret= sAPI_MqttRel(0);
                 ret= sAPI_MqttRel(1);
                 ret= sAPI_MqttStop();
                 return;
 
             default:
                 break;
         }
     }
  }


/**
  * @brief  Twing cloud connection demo
  * @param  void
  * @note   
  * @retval void
  */  
 void TwingDemo(void)
 {
     char topic[1025] = {0};
     INT32 topic_len = 0;
     char payload[10241] = {0};
     INT32 payload_len = 0;
     INT32 ret;
     SIM_MSG_T optionMsg ={0,0,0,NULL};
     UINT32 opt = 0;
     //char productkey[100];
     //char devicename[100];
     //char devicesecret[100];
     char *client_id = NULL;
     char *usrName = NULL;
     char *usrPwd = NULL;
     char host[100] = {0};
     char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
     char *options_list[] = {
         "1. Mqtt Start",
         "2. Connect to twing",
         "3. Subscribe",
         "4. Publish",
         "5. Unsubscribe",
         "6. Disconnect to twing",
         "7. Release a client",
         "8. Mqtt Stop",
         "99. back"
     };
 
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
             case SC_MQTTS_DEMO_START:
                 ret = sAPI_MqttStart(-1);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("Init SUCCESS");
                     PrintfResp("\r\nMQTT Init Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("Init FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT Init Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_CONNECT:
                #if 0
                 PrintfResp("\r\nPlease input productkey\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                 topic_len = strlen(optionMsg.arg3);
                 memset(productkey, 0, sizeof(productkey));
                 memcpy(productkey, optionMsg.arg3, strlen(optionMsg.arg3));
                 sAPI_Free(optionMsg.arg3);
 
                 PrintfResp("\r\nPlease input devicename\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                 topic_len = strlen(optionMsg.arg3);
                 memset(devicename, 0, sizeof(productkey));
                 memcpy(devicename, optionMsg.arg3, strlen(optionMsg.arg3));
                 sAPI_Free(optionMsg.arg3);
 
                 PrintfResp("\r\nPlease input devicesecret\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s],arg3_lenth:%d",optionMsg.arg3,strlen(optionMsg.arg3));
                 
                 topic_len = strlen(optionMsg.arg3);
                 memset(devicesecret, 0, sizeof(productkey));
                 memcpy(devicesecret, optionMsg.arg3, strlen(optionMsg.arg3));
                 if(strlen(optionMsg.arg3)>=32)
                 {
                     sAPI_Free(optionMsg.arg3);
                     

                 }
                 
                 memset(devicesecret, 0, sizeof(productkey));
                 do
                 {
                     sAPI_Free(optionMsg.arg3);
                     sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,(30*200));
                     if(SRV_UART != optionMsg.msg_id)
                     {
                         sAPI_Debug("%s,msg_id is error!!",__func__);
                         break;
                     }
                     
                     sAPI_Debug("arg3 = [%s],arg3_lenth:%d",optionMsg.arg3,strlen(optionMsg.arg3));
                     
                     topic_len = strlen(optionMsg.arg3);
                     
                     memcpy(devicesecret, optionMsg.arg3, strlen(optionMsg.arg3));
                         
                 }while(strlen(optionMsg.arg3)>=32)
                 #endif
                 usrName = sAPI_Malloc(300);
                 usrPwd = sAPI_Malloc(300);
                 client_id = sAPI_Malloc(300);
                 
                 memset(usrName , 0, 300);
                 snprintf(usrName,100,"%s","simcom");
                 memset(usrPwd , 0, 300);
                 snprintf(usrPwd,100,"%s","VJJf0tFfqqtB1Tu3npE96iraL24NZPz9QnbF1Zzo7k4");
                 memset(client_id , 0, 300);
                 snprintf(client_id,100,"%s","100931661");
                
                 memset(host , 0, sizeof(host));
                 sprintf(host,"%s","tcp://mqtt.ctwing.cn:1883");
                 sAPI_Debug("usrName:%s,usrPwd:%s,client_id:%s,host:%s",usrName,usrPwd,client_id,host);
                 ret= sAPI_MqttAccq(0, NULL, 0, client_id, 0, urc_mqtt_msgq_1);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("Init SUCCESS");
                     sAPI_Debug("\r\nMQTT accquire Successful!\r\n");
                 }
                 else
                 {
                     sAPI_Debug("Init FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT accquire Fail!\r\n");
                     break;
                 }

                 sAPI_MqttConnLostCb(app_MqttLostConnCb);
                 ret= sAPI_MqttConnect(0, NULL, 0, host, 60, 1, usrName, usrPwd);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("connect SUCCESS");
                     PrintfResp("\r\nMQTT connect Successful!\r\n");
                     sAPI_Free(usrName);
                     sAPI_Free(usrPwd);
                     sAPI_Free(client_id);
                     break;
                 }
                 else
                 {
                     sAPI_Debug("connect FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT connect Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_SUBSCRIBE:
                 PrintfResp("\r\nPlease input subtopic\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                 topic_len = strlen(optionMsg.arg3);
                 memset(topic, 0, sizeof(topic));
                 memcpy(topic, optionMsg.arg3, strlen(optionMsg.arg3));
                 sAPI_Free(optionMsg.arg3);
                 ret = sAPI_MqttSub(0, topic, topic_len, 0, 0);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("subscribe SUCCESS");
                     PrintfResp("\r\nMQTT subscribe Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("subscribe FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT subscribe Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_PUBLISH:
                 PrintfResp("\r\nPlease input pubtopic\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                 topic_len = strlen(optionMsg.arg3);
                 memset(topic, 0, sizeof(topic));
                 memcpy(topic, optionMsg.arg3, strlen(optionMsg.arg3));
                 sAPI_Free(optionMsg.arg3);
 
                 ret= sAPI_MqttTopic(0, topic, topic_len);
 
                 PrintfResp("\r\nPlease input payload\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                 payload_len = strlen(optionMsg.arg3);
                 memset(payload, 0, sizeof(payload));
                 memcpy(payload, optionMsg.arg3, strlen(optionMsg.arg3));
                 sAPI_Free(optionMsg.arg3);
                 ret= sAPI_MqttPayload(0, payload, payload_len);
 
                 ret= sAPI_MqttPub(0, 1, 60, 0, 0);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("publish SUCCESS");
                     PrintfResp("\r\nMQTT publish Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("publish FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT publish Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_UNSUBSCRIBE:
                 PrintfResp("\r\nPlease input unsubscribe pubtopic\r\n");
                 sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                 if(SRV_UART != optionMsg.msg_id)
                 {
                     sAPI_Debug("%s,msg_id is error!!",__func__);
                     break;
                 }
 
                 sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
                 topic_len = strlen(optionMsg.arg3);
                 memset(topic, 0, sizeof(topic));
                 memcpy(topic, optionMsg.arg3, strlen(optionMsg.arg3));
                 sAPI_Free(optionMsg.arg3);
                 ret = sAPI_MqttUnsub(0, topic, topic_len, 0);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("unsubscribe SUCCESS");
                     PrintfResp("\r\nMQTT unsubscribe Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("unsubscribe FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT unsubscribe Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_DISCONNECT:
                 ret= sAPI_MqttDisConnect(0, NULL, 0, 60);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("disconnect SUCCESS");
                     PrintfResp("\r\nMQTT disconnect Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("disconnect FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT disconnect Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_RELEASE:
                 ret= sAPI_MqttRel(0);
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("release SUCCESS");
                     PrintfResp("\r\nMQTT release Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("release FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT release Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_STOP:
                 ret= sAPI_MqttStop();
                 if(SC_MQTT_RESULT_SUCCESS == ret)
                 {
                     sAPI_Debug("Stop SUCCESS");
                     PrintfResp("\r\nMQTT Stop Successful!\r\n");
                     break;
                 }
                 else
                 {
                     sAPI_Debug("Stop FAIL,ERRCODE = [%d]",ret);
                     PrintfResp("\r\nMQTT Stop Fail!\r\n");
                     break;
                 }
 
             case SC_MQTTS_DEMO_MAX:
                 sAPI_Debug("Return to the previous menu!");
                 PrintfResp("\r\nReturn to the previous menu!\r\n");
                 ret= sAPI_MqttDisConnect(0,NULL, 0, 60);
                 ret= sAPI_MqttDisConnect(0,NULL, 1, 60);
                 ret= sAPI_MqttRel(0);
                 ret= sAPI_MqttRel(1);
                 ret= sAPI_MqttStop();
                 return;
 
             default:
                 break;
         }
     }
  }
#if 0
 void MqttDemo(void)
 {
	 INT32 ret;
	 INT32 client_index = 0;
	 INT8 clientID[256+1] = { 0 };
	 INT32 serverType = 0;
	 INT32 config_type = 0;
	 INT32 config_value = 0;
	 INT8 serverAdd[256+1] = { 0 };
	 INT8 usrName[256+1] = { 0 };
	 INT8 usrPwd[256+1] = { 0 };
	 INT8 topic[1025] = {0};
	 INT32 topic_len = 0;
	 INT8 payload[10241] = {0};
	 INT32 payload_len = 0;
	 
	 SIM_MSG_T optionMsg ={0,0,0,NULL};
	 UINT32 opt = 0;
	 INT8 *note = "\r\nPlease select an option to test from the items listed below.\r\n";
	 INT8 *options_list[] = {
		 "1. Mqtt Start",
		 "2. Accquire",
		 "3. Sslccf",
		 "4. Connect the server",
		 "5. Subscribe",
		 "6. Publish",
		 "7. Unsubscribe",
		 "8. Disconnect from the server",
		 "9. Release a client",
		 "10. Mqtt Stop",
		 "99. back",
	 };

	 if (NULL == gMqttRecvTask)
     {
         if(sAPI_TaskCreate(&gMqttRecvTask, gMqttRecvTaskStack, SC_DEFAULT_THREAD_STACKSIZE*2, SC_DEFAULT_TASK_PRIORITY, (char*)"mqttProcesser", sTask_MqttRecvProcesser,(void *)0)) //init the demo task
     	 {
        	 gMqttRecvTask = NULL;
        	 sAPI_Debug("Task Create error!\n");
     	 }
     }

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
		 	case SC_MQTTS_DEMO_START:
				ret = sAPI_MqttStart(-1);
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("Init SUCCESS");
                    PrintfResp("\r\nMQTT Init Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("Init FAIL,ERRCODE = [%d]",ret);
                    PrintfResp("\r\nMQTT Init Fail!\r\n");
                    break;
                }
			case SC_MQTTS_DEMO_ACCQ:
				PrintfResp("\r\nPlease input client_index\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
		 		if(SRV_UART != optionMsg.msg_id)
		 		{
			 		sAPI_Debug("%s,msg_id is error!!",__func__);
			 		break;
		 		}
				
				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
		 		client_index = atoi(optionMsg.arg3);
				sAPI_Free(optionMsg.arg3);

				PrintfResp("\r\nPlease input clientid\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
		 		if(SRV_UART != optionMsg.msg_id)
		 		{
			 		sAPI_Debug("%s,msg_id is error!!",__func__);
			 		break;
		 		}
				
				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
		 		memcpy(clientID, optionMsg.arg3, strlen(optionMsg.arg3));
				sAPI_Free(optionMsg.arg3);

				PrintfResp("\r\nPlease input server_type\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
		 		if(SRV_UART != optionMsg.msg_id)
		 		{
			 		sAPI_Debug("%s,msg_id is error!!",__func__);
			 		break;
		 		}
				
				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
		 		serverType = atoi(optionMsg.arg3);
				sAPI_Free(optionMsg.arg3);

				ret= sAPI_MqttAccq(0, NULL, client_index, clientID, serverType, urc_mqtt_msgq_1);
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("Init SUCCESS");
                    PrintfResp("\r\nMQTT accquire Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("Init FAIL,ERRCODE = [%d]",ret);
                    PrintfResp("\r\nMQTT accquire Fail!\r\n");
                    break;
                }

			case SC_MQTTS_DEMO_CCF:
				PrintfResp("\r\nPlease input client_index\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
		 		if(SRV_UART != optionMsg.msg_id)
		 		{
			 		sAPI_Debug("%s,msg_id is error!!",__func__);
			 		break;
		 		}
				
				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
		 		client_index = atoi(optionMsg.arg3);
				sAPI_Free(optionMsg.arg3);
				
				PrintfResp("\r\nPlease input config_type\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
		 		if(SRV_UART != optionMsg.msg_id)
		 		{
			 		sAPI_Debug("%s,msg_id is error!!",__func__);
			 		break;
		 		}
				
				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
		 		config_type = atoi(optionMsg.arg3);
				sAPI_Free(optionMsg.arg3);

				PrintfResp("\r\nPlease input config_value\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
		 		if(SRV_UART != optionMsg.msg_id)
		 		{
			 		sAPI_Debug("%s,msg_id is error!!",__func__);
			 		break;
		 		}
				
				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
		 		config_value = atoi(optionMsg.arg3);
				sAPI_Free(optionMsg.arg3);

				ret= sAPI_MqttCfg(0, NULL, client_index, config_type, config_value);
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("config SUCCESS");
                    PrintfResp("\r\nMQTT config Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("config FAIL,ERRCODE = [%d]",ret);
                    PrintfResp("\r\nMQTT config Fail!\r\n");
                    break;
                }

			case SC_MQTTS_DEMO_CONN:
				PrintfResp("\r\nPlease input client_index\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
		 		if(SRV_UART != optionMsg.msg_id)
		 		{
			 		sAPI_Debug("%s,msg_id is error!!",__func__);
			 		break;
		 		}
				
				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
		 		client_index = atoi(optionMsg.arg3);
				sAPI_Free(optionMsg.arg3);
				
				PrintfResp("\r\nPlease input serverAdd\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
		 		if(SRV_UART != optionMsg.msg_id)
		 		{
			 		sAPI_Debug("%s,msg_id is error!!",__func__);
			 		break;
		 		}
				
				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
		 		memcpy(serverAdd, optionMsg.arg3, strlen(optionMsg.arg3));
				sAPI_Free(optionMsg.arg3);

				PrintfResp("\r\nPlease input usrName\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
		 		if(SRV_UART != optionMsg.msg_id)
		 		{
			 		sAPI_Debug("%s,msg_id is error!!",__func__);
			 		break;
		 		}
				
				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
		 		memcpy(usrName, optionMsg.arg3, strlen(optionMsg.arg3));
				sAPI_Free(optionMsg.arg3);

				PrintfResp("\r\nPlease input usrPwd\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
		 		if(SRV_UART != optionMsg.msg_id)
		 		{
			 		sAPI_Debug("%s,msg_id is error!!",__func__);
			 		break;
		 		}
				
				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
		 		memcpy(usrPwd, optionMsg.arg3, strlen(optionMsg.arg3));
				sAPI_Free(optionMsg.arg3);
                sAPI_MqttConnLostCb(app_MqttLostConnCb);
				ret= sAPI_MqttConnect(0, NULL, client_index, serverAdd, 60, 1, usrName, usrPwd);
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("connect SUCCESS");
                    PrintfResp("\r\nMQTT connect Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("connect FAIL,ERRCODE = [%d]",ret);
                    PrintfResp("\r\nMQTT connect Fail!\r\n");
                    break;
                }

			case SC_MQTTS_DEMO_SUB:
				PrintfResp("\r\nPlease input client_index\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
		 		if(SRV_UART != optionMsg.msg_id)
		 		{
			 		sAPI_Debug("%s,msg_id is error!!",__func__);
			 		break;
		 		}
				
				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
		 		client_index = atoi(optionMsg.arg3);
				sAPI_Free(optionMsg.arg3);
				
				PrintfResp("\r\nPlease input subtopic\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
		 		if(SRV_UART != optionMsg.msg_id)
		 		{
			 		sAPI_Debug("%s,msg_id is error!!",__func__);
			 		break;
		 		}
				
				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
				topic_len = strlen(optionMsg.arg3);
		 		memcpy(topic, optionMsg.arg3, strlen(optionMsg.arg3));
				sAPI_Free(optionMsg.arg3);

				ret = sAPI_MqttSub(client_index, topic, topic_len, 0, 0);
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("subscribe SUCCESS");
                    PrintfResp("\r\nMQTT subscribe Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("subscribe FAIL,ERRCODE = [%d]",ret);
                    PrintfResp("\r\nMQTT subscribe Fail!\r\n");
                    break;
                }
			
			case SC_MQTTS_DEMO_PUB:
				PrintfResp("\r\nPlease input client_index\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
		 		if(SRV_UART != optionMsg.msg_id)
		 		{
			 		sAPI_Debug("%s,msg_id is error!!",__func__);
			 		break;
		 		}
				
				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
		 		client_index = atoi(optionMsg.arg3);
				sAPI_Free(optionMsg.arg3);
				
				PrintfResp("\r\nPlease input pubtopic\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
		 		if(SRV_UART != optionMsg.msg_id)
		 		{
			 		sAPI_Debug("%s,msg_id is error!!",__func__);
			 		break;
		 		}
				
				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
				topic_len = strlen(optionMsg.arg3);
		 		memcpy(topic, optionMsg.arg3, strlen(optionMsg.arg3));
				sAPI_Free(optionMsg.arg3);
				
				ret= sAPI_MqttTopic(client_index, topic, topic_len);

                PrintfResp("\r\nPlease input payload\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
		 		if(SRV_UART != optionMsg.msg_id)
		 		{
			 		sAPI_Debug("%s,msg_id is error!!",__func__);
			 		break;
		 		}
				
				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
				payload_len = strlen(optionMsg.arg3);
		 		memcpy(payload, optionMsg.arg3, strlen(optionMsg.arg3));
				sAPI_Free(optionMsg.arg3);
				
                ret= sAPI_MqttPayload(client_index, payload, payload_len);
                
                ret= sAPI_MqttPub(client_index, 1, 60, 0, 0);
                if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("publish SUCCESS");
                    PrintfResp("\r\nMQTT publish Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("publish FAIL,ERRCODE = [%d]",ret);
                    PrintfResp("\r\nMQTT publish Fail!\r\n");
                    break;
                }

			case SC_MQTTS_DEMO_UNSUB:
				PrintfResp("\r\nPlease input client_index\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
		 		if(SRV_UART != optionMsg.msg_id)
		 		{
			 		sAPI_Debug("%s,msg_id is error!!",__func__);
			 		break;
		 		}
				
				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
		 		client_index = atoi(optionMsg.arg3);
				sAPI_Free(optionMsg.arg3);
				
				PrintfResp("\r\nPlease input unsubscribe pubtopic\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
		 		if(SRV_UART != optionMsg.msg_id)
		 		{
			 		sAPI_Debug("%s,msg_id is error!!",__func__);
			 		break;
		 		}
				
				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
				topic_len = strlen(optionMsg.arg3);
		 		memcpy(topic, optionMsg.arg3, strlen(optionMsg.arg3));
				sAPI_Free(optionMsg.arg3);
				ret = sAPI_MqttUnsub(client_index, topic, topic_len, 0);
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("unsubscribe SUCCESS");
                    PrintfResp("\r\nMQTT unsubscribe Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("unsubscribe FAIL,ERRCODE = [%d]",ret);
                    PrintfResp("\r\nMQTT unsubscribe Fail!\r\n");
                    break;
                }

			case SC_MQTTS_DEMO_DISCONN:
				PrintfResp("\r\nPlease input client_index\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
		 		if(SRV_UART != optionMsg.msg_id)
		 		{
			 		sAPI_Debug("%s,msg_id is error!!",__func__);
			 		break;
		 		}
				
				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
		 		client_index = atoi(optionMsg.arg3);
				sAPI_Free(optionMsg.arg3);
				
				ret= sAPI_MqttDisConnect(0,NULL, client_index, 60);
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("disconnect SUCCESS");
                    PrintfResp("\r\nMQTT disconnect Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("disconnect FAIL,ERRCODE = [%d]",ret);
                    PrintfResp("\r\nMQTT disconnect Fail!\r\n");
                    break;
                }

			case SC_MQTTS_DEMO_REL:
				PrintfResp("\r\nPlease input client_index\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
		 		if(SRV_UART != optionMsg.msg_id)
		 		{
			 		sAPI_Debug("%s,msg_id is error!!",__func__);
			 		break;
		 		}
				
				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
		 		client_index = atoi(optionMsg.arg3);
				sAPI_Free(optionMsg.arg3);
				
				ret= sAPI_MqttRel(client_index);
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("release SUCCESS");
                    PrintfResp("\r\nMQTT release Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("release FAIL,ERRCODE = [%d]",ret);
                    PrintfResp("\r\nMQTT release Fail!\r\n");
                    break;
                }

			case SC_MQTTS_DEMO_STOP:
				ret= sAPI_MqttStop();
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("Stop SUCCESS");
                    PrintfResp("\r\nMQTT Stop Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("Stop FAIL,ERRCODE = [%d]",ret);
                    PrintfResp("\r\nMQTT Stop Fail!\r\n");
                    break;
                }

			case SC_MQTTS_DEMO_MAX:
				sAPI_Debug("Return to the previous menu!");
                PrintfResp("\r\nReturn to the previous menu!\r\n");
				ret= sAPI_MqttDisConnect(0,NULL, 0, 60);
				ret= sAPI_MqttDisConnect(0,NULL, 1, 60);
				ret= sAPI_MqttRel(0);
				ret= sAPI_MqttRel(1);
				ret= sAPI_MqttStop();
				return;

			default:
				break;
		 }
	 }
 }
 #endif

/**
  * @brief  Ali cloud connection demo
  * @param  void
  * @note   
  * @retval void
  */  
void AyliunDemo(void)
{
	char topic[1025] = {0};
	INT32 topic_len = 0;
	char payload[10241] = {0};
	INT32 payload_len = 0;
	INT32 ret;
	SIM_MSG_T optionMsg ={0,0,0,NULL};
	UINT32 opt = 0;
	char productkey[100];
	char devicename[100];
	char devicesecret[100];
	char *client_id = NULL;
	char *usrName = NULL;
	char *usrPwd = NULL;
	char *secure_mode = "3";
	char *url = "iot-as-mqtt.cn-shanghai.aliyuncs.com:443";
	char host[256] = {0};
	char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
	char *options_list[] = {
		"1. Mqtt Start",
		"2. Connect to aliyun",
		"3. Subscribe",
		"4. Publish",
		"5. Unsubscribe",
		"6. Disconnect to aliyun",
		"7. Release a client",
		"8. Mqtt Stop",
		"99. back"
	};

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
			case SC_MQTTS_DEMO_START:
				ret = sAPI_MqttStart(-1);
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("Init SUCCESS");
                    PrintfResp("\r\nMQTT Init Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("Init FAIL,ERRCODE = [%d]",ret);
                    PrintfResp("\r\nMQTT Init Fail!\r\n");
                    break;
                }

			case SC_MQTTS_DEMO_CONNECT:
				PrintfResp("\r\nPlease input productkey\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
				if(SRV_UART != optionMsg.msg_id)
				{
					sAPI_Debug("%s,msg_id is error!!",__func__);
					break;
				}

				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
				topic_len = strlen(optionMsg.arg3);
				memset(productkey, 0, sizeof(productkey));
				memcpy(productkey, optionMsg.arg3, strlen(optionMsg.arg3));
				sAPI_Free(optionMsg.arg3);

				PrintfResp("\r\nPlease input devicename\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
				if(SRV_UART != optionMsg.msg_id)
				{
					sAPI_Debug("%s,msg_id is error!!",__func__);
					break;
				}

				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
				topic_len = strlen(optionMsg.arg3);
				memset(devicename, 0, sizeof(productkey));
				memcpy(devicename, optionMsg.arg3, strlen(optionMsg.arg3));
				sAPI_Free(optionMsg.arg3);

				PrintfResp("\r\nPlease input devicesecret\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
				if(SRV_UART != optionMsg.msg_id)
				{
					sAPI_Debug("%s,msg_id is error!!",__func__);
					break;
				}

				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
				topic_len = strlen(optionMsg.arg3);
				memset(devicesecret, 0, sizeof(productkey));
				memcpy(devicesecret, optionMsg.arg3, strlen(optionMsg.arg3));
				sAPI_Free(optionMsg.arg3);

				if ((core_auth_mqtt_username(&usrName, productkey, devicename)) < 0 ||
					(core_auth_mqtt_password(&usrPwd, productkey, devicename, devicesecret)) < 0 ||
					(core_auth_mqtt_clientid(&client_id, productkey, devicename, secure_mode, NULL)) < 0)
				{
					//_core_mqtt_sign_clean(mqtt_handle);
					if(usrName != NULL)
					{
						sAPI_Free(usrName);
					}
					if(usrPwd != NULL)
					{
						sAPI_Free(usrPwd);
					}
					if(client_id != NULL)
					{
						sAPI_Free(client_id);
					}
					break;
				}
				memset(host , 0, sizeof(host));
				sprintf(host, "tcp://%s.%s", productkey, url);
				ret= sAPI_MqttAccq(0, NULL, 0, client_id, 0, urc_mqtt_msgq_1);
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("Init SUCCESS");
                    sAPI_Debug("\r\nMQTT accquire Successful!\r\n");
                }
                else
                {
                    sAPI_Debug("Init FAIL,ERRCODE = [%d]",ret);
                    PrintfResp("\r\nMQTT accquire Fail!\r\n");
                    break;
                }
                sAPI_MqttConnLostCb(app_MqttLostConnCb);
				ret= sAPI_MqttConnect(0, NULL, 0, host, 60, 0, usrName, usrPwd);
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("connect SUCCESS");
                    PrintfResp("\r\nMQTT connect Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("connect FAIL,ERRCODE = [%d]",ret);
                    PrintfResp("\r\nMQTT connect Fail!\r\n");
                    break;
                }

			case SC_MQTTS_DEMO_SUBSCRIBE:
				PrintfResp("\r\nPlease input subtopic\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
				if(SRV_UART != optionMsg.msg_id)
				{
					sAPI_Debug("%s,msg_id is error!!",__func__);
					break;
				}

				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
				topic_len = strlen(optionMsg.arg3);
				memset(topic, 0, sizeof(topic));
				memcpy(topic, optionMsg.arg3, strlen(optionMsg.arg3));
				sAPI_Free(optionMsg.arg3);
				ret = sAPI_MqttSub(0, topic, topic_len, 0, 0);
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("subscribe SUCCESS");
                    PrintfResp("\r\nMQTT subscribe Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("subscribe FAIL,ERRCODE = [%d]",ret);
                    PrintfResp("\r\nMQTT subscribe Fail!\r\n");
                    break;
                }

			case SC_MQTTS_DEMO_PUBLISH:
				PrintfResp("\r\nPlease input pubtopic\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
				if(SRV_UART != optionMsg.msg_id)
				{
					sAPI_Debug("%s,msg_id is error!!",__func__);
					break;
				}

				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
				topic_len = strlen(optionMsg.arg3);
				memset(topic, 0, sizeof(topic));
				memcpy(topic, optionMsg.arg3, strlen(optionMsg.arg3));
				sAPI_Free(optionMsg.arg3);

				ret= sAPI_MqttTopic(0, topic, topic_len);

				PrintfResp("\r\nPlease input payload\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
				if(SRV_UART != optionMsg.msg_id)
				{
					sAPI_Debug("%s,msg_id is error!!",__func__);
					break;
				}

				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
				payload_len = strlen(optionMsg.arg3);
				memset(payload, 0, sizeof(payload));
				memcpy(payload, optionMsg.arg3, strlen(optionMsg.arg3));
				sAPI_Free(optionMsg.arg3);
				ret= sAPI_MqttPayload(0, payload, payload_len);

                ret= sAPI_MqttPub(0, 1, 60, 0, 0);
                if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("publish SUCCESS");
                    PrintfResp("\r\nMQTT publish Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("publish FAIL,ERRCODE = [%d]",ret);
                    PrintfResp("\r\nMQTT publish Fail!\r\n");
                    break;
                }

			case SC_MQTTS_DEMO_UNSUBSCRIBE:
				PrintfResp("\r\nPlease input unsubscribe pubtopic\r\n");
				sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
				if(SRV_UART != optionMsg.msg_id)
				{
					sAPI_Debug("%s,msg_id is error!!",__func__);
					break;
				}

				sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
				topic_len = strlen(optionMsg.arg3);
				memset(topic, 0, sizeof(topic));
				memcpy(topic, optionMsg.arg3, strlen(optionMsg.arg3));
				sAPI_Free(optionMsg.arg3);
				ret = sAPI_MqttUnsub(0, topic, topic_len, 0);
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("unsubscribe SUCCESS");
                    PrintfResp("\r\nMQTT unsubscribe Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("unsubscribe FAIL,ERRCODE = [%d]",ret);
                    PrintfResp("\r\nMQTT unsubscribe Fail!\r\n");
                    break;
                }

			case SC_MQTTS_DEMO_DISCONNECT:
				ret= sAPI_MqttDisConnect(0, NULL, 0, 60);
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("disconnect SUCCESS");
                    PrintfResp("\r\nMQTT disconnect Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("disconnect FAIL,ERRCODE = [%d]",ret);
                    PrintfResp("\r\nMQTT disconnect Fail!\r\n");
                    break;
                }

			case SC_MQTTS_DEMO_RELEASE:
				ret= sAPI_MqttRel(0);
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("release SUCCESS");
                    PrintfResp("\r\nMQTT release Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("release FAIL,ERRCODE = [%d]",ret);
                    PrintfResp("\r\nMQTT release Fail!\r\n");
                    break;
                }

			case SC_MQTTS_DEMO_STOP:
				ret= sAPI_MqttStop();
				if(SC_MQTT_RESULT_SUCCESS == ret)
                {
					sAPI_Debug("Stop SUCCESS");
                    PrintfResp("\r\nMQTT Stop Successful!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("Stop FAIL,ERRCODE = [%d]",ret);
                    PrintfResp("\r\nMQTT Stop Fail!\r\n");
                    break;
                }

			case SC_MQTTS_DEMO_MAX:
				sAPI_Debug("Return to the previous menu!");
                PrintfResp("\r\nReturn to the previous menu!\r\n");
				ret= sAPI_MqttDisConnect(0,NULL, 0, 60);
				ret= sAPI_MqttDisConnect(0,NULL, 1, 60);
				ret= sAPI_MqttRel(0);
				ret= sAPI_MqttRel(1);
				ret= sAPI_MqttStop();
				return;

			default:
				break;
		}
	}
 }

/**
  * @brief  Cloud demo connection with different cloud in China
  * @param  void
  * @note   
  * @retval void
  */  
 void MqttDemo(void)
 {
	 SIM_MSG_T optionMsg ={0,0,0,NULL};
	 UINT32 opt = 0;
	 char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
	 char *options_list[] = {
          "1. Connect to aliyun platform",
          "2. Connect to tencent platform",
          "3. Connect to onenet platform",
          "4. Connect to twing platform",
          "99. back",
      };

	 if (NULL == gMqttRecvTask)
     {
         if(sAPI_TaskCreate(&gMqttRecvTask, gMqttRecvTaskStack, SC_DEFAULT_THREAD_STACKSIZE*2, SC_DEFAULT_TASK_PRIORITY, (char*)"mqttProcesser", sTask_MqttRecvProcesser,(void *)0)) //init the demo task
		 {
			gMqttRecvTask = NULL;
			sAPI_Debug("Task Create error!\n");
		 }
     }

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
			case SC_MQTTS_DEMO_ALIYUN:
				AyliunDemo();
				break;

			case SC_MQTTS_DEMO_TENCENT:
                 TencentDemo();
                 break;

            case SC_MQTTS_DEMO_ONENET:
                 OnenetDemo();
                 break;
                
            case SC_MQTTS_DEMO_TWING:
                 TwingDemo();
                 break;

			case SC_MQTTS_DEMO_MAX:
				sAPI_Debug("Return to the previous menu!");
                PrintfResp("\r\nReturn to the previous menu!\r\n");
				return;

			default:
				break;
		 }
	 }
 }
#endif



