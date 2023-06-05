#ifdef FEATURE_SIMCOM_MQTT

#include "string.h"
#include "stdlib.h"
#include <stdio.h>

#include "simcom_mqtts_client.h"
//#include "simcom_common.h"
#include "simcom_os.h"
//#include "simcom_network.h"


#define PrintfLog printf

sMsgQRef urc_mqtt_msgq_1_autotest = NULL;
sMsgQRef notice_mqtt_offline = NULL;
static sTaskRef gMqttRecvTask = NULL;
static UINT8 gMqttRecvTaskStack[SC_DEFAULT_THREAD_STACKSIZE*2];
static sTaskRef gMqttdemo = NULL;
static UINT8 gMqttdemoStack[SC_DEFAULT_THREAD_STACKSIZE*2];

void mqtt_notice_offline(int index, int cause)
{
    SIM_MSG_T msgQ_notice_offline_send = {0, 0, -1, NULL}; 
    
    PrintfLog("mqtt offline,index:%d,cause:%d", index, cause);
    msgQ_notice_offline_send.msg_id = 11;//just test
    msgQ_notice_offline_send.arg1 = index;
    msgQ_notice_offline_send.arg2 = cause;
    sAPI_MsgQSend(notice_mqtt_offline, &msgQ_notice_offline_send);
}


 void sTask_MqttProcesser_auto(void* arg)                 //thread of control
 {
    int ret = 0;
    SC_STATUS status = SC_SUCCESS;
    SIM_MSG_T msgQ_notice_offline = {0, 0, -1, NULL}; 

    PrintfLog("enter mqtt test");

#if 0
    while(1)
    {
 
        sAPI_NetworkGetCgreg(&pGreg);
        if(0 != pGreg)
        {
            PrintfLog("MQTT NETWORK STATUS IS [%d]",pGreg);
            sAPI_TaskSleep(10*300);
        }
        else
        {
            PrintfLog("MQTT NETWORK STATUS IS NORMAL");
            break;
        }
    }
#endif
    //delay 10s wait network ok
    sAPI_TaskSleep(200*10);
    while(1)
    {
        ret = sAPI_MqttStart(-1);
        if(ret != SC_MQTT_RESULT_SUCCESS)
        {
            PrintfLog("ret=%d,sAPI_MqttStart fail,wait 10s", ret);
            sAPI_TaskSleep(200*10);
            continue;
        }
        PrintfLog("1-----ret = %d", ret);
        ret= sAPI_MqttAccq(0, NULL, 0, "614393569", 0, urc_mqtt_msgq_1_autotest);
        PrintfLog("2-----ret = %d", ret);


        ret= sAPI_MqttCfg(0, NULL, 0, 0, 0);
        PrintfLog("2.5-----ret = %d", ret);

        sAPI_MqttConnLostCb(mqtt_notice_offline);
        ret= sAPI_MqttConnect(0, NULL, 0, "tcp://47.108.134.22:1883", 60, 1, "znn", "znn11");
        PrintfLog("3-----ret = %d", ret);

        ret= sAPI_MqttSubTopic(0, "topic11", 7, 1);
        PrintfLog("4-----ret = %d", ret);

        ret= sAPI_MqttSub(0, NULL, 0, 0, 0);
        PrintfLog("4.5-----ret = %d", ret);

        ret = sAPI_MqttSub(0, "apitest1", 8, 0, 0);
        PrintfLog("4.6-----ret = %d", ret);

        ret = sAPI_MqttSub(0, "apitest2", 8, 0, 0);
        PrintfLog("4.7-----ret = %d", ret);

        ret= sAPI_MqttTopic(0, "topic11", 7);
        PrintfLog("5-----ret = %d", ret);


        ret= sAPI_MqttPayload(0, "aMsgsend123", strlen("aMsgsend123"));
        PrintfLog("6-----ret = %d", ret);

        ret= sAPI_MqttPub(0, 1, 60, 0, 0);

        ret = sAPI_MqttUnsub(0, "apitest1", 8, 0);
        PrintfLog("7.5-----ret = %d", ret);

        sAPI_TaskSleep((10) * 200);           //sleep 10 sec before unsub
        ret = sAPI_MqttUNSubTopic(0, "apitest2", 8);
        PrintfLog("7.6-----ret = %d", ret);

        ret = sAPI_MqttUnsub(0, NULL, 0, 0);
        PrintfLog("7.7-----ret = %d", ret);

        //sAPI_MsgQRecv(notice_mqtt_offline, &msgQ_notice_offline, SC_SUSPEND); 
        //receive offline msgQ should use in another task,now just test
        status = sAPI_MsgQRecv(notice_mqtt_offline, &msgQ_notice_offline, (6*60*200)); //set timeout 6min for test
        PrintfLog("notice_mqtt_offline, msgid[%ld],index[%d],cause[%d]", msgQ_notice_offline.msg_id, msgQ_notice_offline.arg1, msgQ_notice_offline.arg2);
        //msgQ_notice_offline.arg, no use no free
        PrintfLog("sAPI_MsgQRecv status:%d", status);
        if(status == SC_SUCCESS)//receive offline msgQ
        {
            PrintfLog("8-----receive callback notice offline donnot need use sAPI_MqttDisConnect()");

            ret= sAPI_MqttRel(0);
            PrintfLog("9-----ret = %d", ret);

            ret= sAPI_MqttStop();
            PrintfLog("10-----ret = %d", ret);
        }
        else
        {
            ret= sAPI_MqttDisConnect(0,NULL, 0, 60);
            PrintfLog("8-----ret = %d", ret);

            ret= sAPI_MqttRel(0);
            PrintfLog("9-----ret = %d", ret);

            ret= sAPI_MqttStop();
            PrintfLog("10-----ret = %d", ret);
        }

        break;

    }
    
    PrintfLog("-------MQTT TEST-----, finish");
}
    
 void sTask_MqttRecvProcesser_auto(void* arg)                    //thread of sub topic data process
{
    while(1)
    {
        SIM_MSG_T msgQ_data_recv = {0, 0, -1, NULL};                 //NULL pointer for msgQ_data_recv.arg3 is necessary!
        SCmqttData *sub_data = NULL;
        /*=======================================================================================================================
        *
        *   NOTE: if this data reception cycle too long may cause data loss(data processing slower than data receive from server)
        *
        **=======================================================================================================================*/
        //recv the subscribed topic data, from the message queue: urc_mqtt_msgq_1_autotest, it is set buy sAPI_MqttAccq                                          //*===================*//
        sAPI_MsgQRecv(urc_mqtt_msgq_1_autotest, &msgQ_data_recv, SC_SUSPEND);                                                                                    //                     //
        if(/*(SC_SRV_MQTT != msgQ_data_recv.msg_id) ||*/ (0 != msgQ_data_recv.arg1) || (NULL == msgQ_data_recv.arg3))   //wrong msg received                //                     //
            continue;                                                                                                                                   //                     //
        PrintfLog("recv:msgid[%ld]", msgQ_data_recv.msg_id);
        sub_data = (SCmqttData *)(msgQ_data_recv.arg3);                                                                                                  //                     //
        /*this part just for test, the payload msg just ascii string*/                                                                                  //                     //          
        PrintfLog("MQTT TEST----------index: [%d]; tpoic_len: [%d]; tpoic: [%s]", sub_data->client_index, sub_data->topic_len, sub_data->topic_P);     //                     //
#if 0                                                                                                                                                        //                     //
        PrintfLog("MQTT TEST----------payload_len: [%d]", sub_data->payload_len);                                                                      //   rception cycle    //
		sAPI_UartWrite(SC_UART,(UINT8*)"recieve topic: ",strlen("recieve topic: "));
		sAPI_UartWrite(SC_UART,(UINT8*)sub_data->topic_P,sub_data->topic_len);
		sAPI_UartWrite(SC_UART,(UINT8*)"\r\n",2);
		sAPI_UartWrite(SC_UART,(UINT8*)"recieve payload: ",strlen("recieve payload: "));
		sAPI_UartWrite(SC_UART,(UINT8*)sub_data->payload_P,sub_data->payload_len);                                                                              //                     //    
        sAPI_UartWrite(SC_UART,(UINT8*)"\r\n",2);                                                                                                               //                     //
#endif
        /*************************************************************************************/                                                         //                     //
        /*************************************************************************************/                                                         //                     //
        /*these msg pointer must be free after using, don not change the free order*/                                                                   //                     //
            free(sub_data->topic_P);                                                                                                               //                     //
            free(sub_data->payload_P);                                                                                                             //                     //
            free(sub_data);                                                                                                                        //                     //
        /*************************************************************************************/                                                         //                     //
        /*************************************************************************************/                                                         //*===================*//
    }


}
 
//Add demo_mqtt_test_init() in userSpace_Main(), running a mqtt test
void demo_mqtt_test_init(void)
{
    SC_STATUS status;

    PrintfLog("%s, enter", __FUNCTION__);
    status = sAPI_MsgQCreate(&urc_mqtt_msgq_1_autotest, "urc_mqtt_msgq_1_autotest",sizeof(SIM_MSG_T),4, SC_FIFO);        //msgQ for subscribed data transfer
    if(status != SC_SUCCESS)
    {
        PrintfLog("message queue creat err!\n");
    }
    status = sAPI_MsgQCreate(&notice_mqtt_offline, "notice_mqtt_offline",sizeof(SIM_MSG_T),4, SC_FIFO);        //msgQ for notice offline
    if(status != SC_SUCCESS)
    {
        PrintfLog("message queue creat err!\n");
    }
    if (NULL == gMqttRecvTask)
    {
        if((sAPI_TaskCreate(&gMqttRecvTask, gMqttRecvTaskStack, SC_DEFAULT_THREAD_STACKSIZE*2, SC_DEFAULT_TASK_PRIORITY, (char*)"mqttProcesser", sTask_MqttRecvProcesser_auto,(void *)0)) != SC_SUCCESS) //init the demo task
        {
            gMqttRecvTask = NULL;
            PrintfLog("Task Create error!\n");
        }
    }
    if (NULL == gMqttdemo)
    {
        if((sAPI_TaskCreate(&gMqttdemo, gMqttdemoStack, SC_DEFAULT_THREAD_STACKSIZE*2, SC_DEFAULT_TASK_PRIORITY, (char*)"gMqttdemo", sTask_MqttProcesser_auto, (void *)0)) != SC_SUCCESS) //init the demo task
        {
            gMqttdemo = NULL;
            PrintfLog("Task Create error!\n");
        }
    }
}
#endif
