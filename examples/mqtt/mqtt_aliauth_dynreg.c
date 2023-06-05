#include "simcom_api.h"
#include "string.h"
#include "stdlib.h"

sMsgQRef mqtt_recv_msg;
sMsgQRef mqtt_conn_msgq;
sMsgQRef mqtt_disconn_msgq;

extern void PrintfResp(INT8* format);

typedef struct
{
    char *clientId;
    char *productKey;
    char *deviceName;
    char *deviceToken;
}mqtt_conn_msgq_params;

char *mqtt_dynreg_get_params(char *src_str, char *find_str, char **re_data)
{
    int find_len = 0;
    char *src_data = src_str;
    char *head_str = NULL;
    char *buf = NULL;
    int find_str_len = strlen(find_str);

    head_str = strstr(src_data, find_str);
    if(head_str == NULL)
    {
        return NULL;
    }
    while(*(head_str+find_str_len+find_len) != '\"')
    {
        find_len++;
    }
    buf = sAPI_Malloc(find_len+1);//must sAPI_Free
    memset(buf, 0, find_len+1);
    memcpy(buf, head_str+find_str_len, find_len);

    *re_data = buf;

    return buf;
}

void mqtt_dynreg_send_message_connect(char *data, INT32 len)
{
    SC_STATUS status = SC_SUCCESS;
    char *recv_data = data;
    char *clientId = NULL;
    char *productKey = NULL;
    char *deviceName = NULL;
    char *deviceToken = NULL;
    char *str_clientId = "\"clientId\":\"";
    char *str_productKey = "\"productKey\":\"";
    char *str_deviceName = "\"deviceName\":\"";
    char *str_deviceToken = "\"deviceToken\":\"";
    mqtt_conn_msgq_params *optionMsg = NULL;
    SIM_MSG_T send_data = {SIM_MSG_INIT, 0, -1, NULL}; 

    if(NULL == mqtt_dynreg_get_params(recv_data, str_clientId, &clientId))
    {
        sAPI_Debug("error clientId");
        return;
    }
    if(NULL == mqtt_dynreg_get_params(recv_data, str_productKey, &productKey))
    {
        sAPI_Debug("error productKey");
        return;
    }
    if(NULL == mqtt_dynreg_get_params(recv_data, str_deviceName, &deviceName))
    {
        sAPI_Debug("error deviceName");
        return;
    }
    if(NULL == mqtt_dynreg_get_params(recv_data, str_deviceToken, &deviceToken))
    {
        sAPI_Debug("error deviceToken");
        return;
    }
    optionMsg = sAPI_Malloc(sizeof(mqtt_conn_msgq_params));
    optionMsg->clientId = clientId;
    optionMsg->deviceName = deviceName;
    optionMsg->productKey = productKey;
    optionMsg->deviceToken = deviceToken;
    send_data.arg3 = (void *)optionMsg;
    status = sAPI_MsgQSend(mqtt_conn_msgq, &send_data);
    if(status != SC_SUCCESS)
    {
        sAPI_Free(clientId);
        sAPI_Free(deviceName);
        sAPI_Free(productKey);
        sAPI_Free(deviceToken);
        sAPI_Free(optionMsg);
        sAPI_Debug("send msg error,status = [%d]",status);
    }
}

BOOL mqtt_connect_server_get_connect_param(char *productkey, char *devicename, char *productsecret)
{
    SCmqttReturnCode ret = SC_MQTT_RESULT_SUCCESS;
    char host[256] = {0};
    
    {
        int random_num = 0;
        char random[11] = {0};
        char *auth_type = "regnwl";
        srand((unsigned) time(NULL) *10);
        random_num = rand();

        core_uint2str(random_num, random, NULL);
        sAPI_Debug("random = %s", random);
        
        char *url = "iot-as-mqtt.cn-shanghai.aliyuncs.com:443";
        char *client_src[] = {devicename, productkey, random, auth_type, NULL};
        char *username_fmt = "%s&%s";
        char *username_src[] = {devicename, productkey };
        char *sign_input_fmt = "deviceName%sproductKey%srandom%s";
        char *client_fmt = "%s.%s|random=%s,authType=%s,securemode=2,signmethod=hmacsha256|";
        char *client_id = NULL;
        char *usrName = NULL;
        char *sign_input = NULL;
        unsigned char sign_output[32] = {0};
        char auth_password[65] = {0};

        //clientId
        core_sprintf(&client_id, client_fmt, client_src, 4);
        sAPI_Debug("client_id = %s", client_id);
        //usrName
        core_sprintf(&usrName, username_fmt, username_src, sizeof(username_src) / sizeof(char *));
        sAPI_Debug("usrName = %s", usrName);
        //auth_password
        core_sprintf(&sign_input, sign_input_fmt, client_src, 3);
        sAPI_Debug("sign_input = %s", sign_input);
        core_hmac_sha256((const UINT8 *)sign_input, (UINT32)strlen(sign_input), (const UINT8 *)productsecret,
                 (UINT32)strlen(productsecret), sign_output);
        sAPI_Debug("sign_output = %s", sign_output);
        core_hex2str(sign_output, sizeof(sign_output), auth_password, 0);
        sAPI_Debug("auth_password = %s", auth_password);
        //host
        sprintf(host, "tcp://%s.%s", productkey, url);
        
        ret= sAPI_MqttAccq(0, NULL, 0, client_id, 1, mqtt_recv_msg);//must use SSL
        sAPI_Debug("sAPI_MqttAccq = %d", ret);
        
        ret= sAPI_MqttConnect(0, NULL, 0, host, 60, 1, usrName, auth_password);
        sAPI_Free(client_id);
        sAPI_Free(usrName);
        if(ret!= SC_MQTT_RESULT_SUCCESS)
        {
            sAPI_Debug("sAPI_MqttConnect= [%d]",ret);
            PrintfResp("\r\nMQTT connect Fail!\r\n");
            return FALSE;
        }
        sAPI_Debug("connect SUCCESS");
        PrintfResp("\r\nMQTT connect Successful!\r\n");
    }
    return TRUE;
}

BOOL mqtt_connect_server_final(mqtt_conn_msgq_params *param)
{
    SCmqttReturnCode ret = SC_MQTT_RESULT_SUCCESS;
    char *cliendId = NULL;
    char *cliendId_fmt = "%s%s";
    char *cliendId_part = "|securemode=-2,authType=connwl|";
    char *cliendId_src[] = {param->clientId, cliendId_part };
    char *usrName = NULL;
    char *username_fmt = "%s&%s";
    char *username_src[] = {param->deviceName, param->productKey };
    char *usrPassword = param->deviceToken;
    char *url = "iot-as-mqtt.cn-shanghai.aliyuncs.com:1883";
    char *host = NULL;
    char *host_fmt = "tcp://%s.%s";
    char *host_src[] = {param->productKey, url};
    
    core_sprintf(&cliendId, cliendId_fmt, cliendId_src, sizeof(cliendId_src) / sizeof(char *));
    sAPI_Debug("cliendId = %s", cliendId);

    core_sprintf(&usrName, username_fmt, username_src, sizeof(username_src) / sizeof(char *));
    sAPI_Debug("usrName = %s", usrName);

    sAPI_Debug("usrPassword = %s", usrPassword);

    core_sprintf(&host, host_fmt, host_src, sizeof(host_src) / sizeof(char *));
    sAPI_Debug("host = %s", host);

    ret= sAPI_MqttAccq(0, NULL, 0, cliendId, 1, mqtt_recv_msg);
    sAPI_Debug("sAPI_MqttAccq= [%d]",ret);
    ret= sAPI_MqttConnect(0, NULL, 0, host, 60, 1, usrName, usrPassword);
    sAPI_Free(cliendId);
    sAPI_Free(usrName);
    //sAPI_Free(usrPassword);//donot need
    sAPI_Free(host);
    if(ret!= SC_MQTT_RESULT_SUCCESS)
    {
        sAPI_Debug("sAPI_MqttConnect= [%d]",ret);
        PrintfResp("\r\nMQTT connect Fail!\r\n");
        return FALSE;
    }
    return TRUE;
}

void mqtt_disconn_notice_cb(int index, int cause)
{
    SC_STATUS status = SC_SUCCESS;
    SIM_MSG_T send_data = {SIM_MSG_INIT, 0, -1, NULL}; 

    sAPI_Debug("mqtt disconn:[%d,%d]", index, cause);
    send_data.arg1 = index;
    send_data.arg2 = cause;
    status = sAPI_MsgQSend(mqtt_disconn_msgq, &send_data);
    if(status != SC_SUCCESS)
    {
         sAPI_Debug("send msg error,mqtt_disconn_msgq");
    }
}

void mqtt_aliauth_dynreg(void *arg)
{
    int pGreg = 0;
    SCmqttReturnCode ret = SC_MQTT_RESULT_SUCCESS;
    mqtt_conn_msgq_params *recvConnMsg = NULL;
    SIM_MSG_T msgQ_data_recv = {SIM_MSG_INIT, 0, -1, NULL};
    BOOL connect_flag = TRUE;
    while(1)
    {
        sAPI_NetworkGetCgreg(&pGreg);
        if(1 != pGreg)
        {
            sAPI_Debug("MQTT NETWORK STATUS IS [%d]",pGreg);
            sAPI_TaskSleep(5*200);
        }
        else
        {
            sAPI_Debug("MQTT NETWORK STATUS IS NORMAL");
            break;
        }
    }

    ret = sAPI_MqttStart(-1);
    if(SC_MQTT_RESULT_SUCCESS == ret)
    {
        sAPI_Debug("Init SUCCESS");
        PrintfResp("\r\nMQTT Init Successful!\r\n");
    }
    else
    {
        sAPI_Debug("Init FAIL,ERRCODE = [%d]", ret);
        PrintfResp("\r\nMQTT Init Fail!\r\n");
        return;
    }

    //connect mqtt server get param of connection by using ssl
    {
        char *productkey = "a1WuzE5Q6PX";//use ALi productkey
        char *devicename = "deviceznn1";//can use IMEI to define different devices
        char *productsecret = "uZoQpgqhCf4Z9y7h";//use ALi productsecret
        if(!mqtt_connect_server_get_connect_param(productkey, devicename, productsecret))
        {
            sAPI_Debug("mqtt_connect_server_get_connect_param");
            PrintfResp("\r\nMQTT mqtt_connect_server_get_connect_param Fail!\r\n");
            return;
        }
    }
    
    //wait param of connection
    sAPI_MsgQRecv(mqtt_conn_msgq, &msgQ_data_recv, SC_SUSPEND);
    if(msgQ_data_recv.arg3 == NULL)
    {
        sAPI_Debug("sAPI_MsgQRecv NULL");
        PrintfResp("\r\nsAPI_MsgQRecv NULL!\r\n");
        return;
    }
    recvConnMsg = (mqtt_conn_msgq_params *)(msgQ_data_recv.arg3);
    //get param of connection, need to disconn and rel; reconnect mqtt use recvConnMsg
    ret= sAPI_MqttDisConnect(0, NULL, 0, 60);
    sAPI_Debug("sAPI_MqttDisConnect[%d]", ret);
    ret = sAPI_MqttRel(0);
    sAPI_Debug("sAPI_MqttRel[%d]", ret);

    sAPI_MqttConnLostCb(mqtt_disconn_notice_cb);//set callback notice offline
    //to connect mqtt server finaly
    //should save clientId,deviceName,productKey,deviceToken(save in file),next use them to connect.
    connect_flag = mqtt_connect_server_final(recvConnMsg);
    sAPI_Free(recvConnMsg->clientId);
    sAPI_Free(recvConnMsg->deviceName);
    sAPI_Free(recvConnMsg->productKey);
    sAPI_Free(recvConnMsg->deviceToken);
    sAPI_Free(recvConnMsg);
    if(!connect_flag)
    {
        sAPI_Debug("mqtt_connect_server_final Fail");
        PrintfResp("\r\nMQTT mqtt_connect_server_final Fail!\r\n");
        return;
    }
    PrintfResp("\r\nMQTT mqtt_connect_server_final succ!\r\n");
    
    while(1)
    {
        SIM_MSG_T msgQ_disconn = {SIM_MSG_INIT, 0, -1, NULL};
        int index = 0, cause = 0;
        
        sAPI_MsgQRecv(mqtt_disconn_msgq, &msgQ_disconn, SC_SUSPEND);
        index = msgQ_disconn.arg1;
        cause = msgQ_disconn.arg2;
        PrintfResp("\r\nmqtt disconnet\r\n");
        //you can reconnect
        //ret = sAPI_MqttRel(index);
        //ret = sAPI_MqttStop();
        //connect_flag = mqtt_connect_server_final(recvConnMsg);
        //...
    }
}

void sTask_MqttRecv(void *arg)                    //thread of sub topic data process
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
        sAPI_MsgQRecv(mqtt_recv_msg, &msgQ_data_recv, SC_SUSPEND);                                                                                    //                     //
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
        if(memcmp(sub_data->topic_P, "/ext/regnwl", sub_data->topic_len) == 0)
        {
            sAPI_Debug("receive dynamic message\n");
            mqtt_dynreg_send_message_connect(sub_data->payload_P, sub_data->payload_len);
        }
        /*************************************************************************************/                                                         //                     //
        /*these msg pointer must be free after using, don not change the free order*/                                                                   //                     //
        sAPI_Free(sub_data->topic_P);                                                                                                               //                     //
        sAPI_Free(sub_data->payload_P);                                                                                                             //                     //
        sAPI_Free(sub_data);                                                                                                                        //                     //
        /*************************************************************************************/                                                         //                     //
        /*************************************************************************************/                                                         //*===================*//
    }

}

void mqtt_aliauth_dynreg_demo(void)
{
    sTaskRef gMqttConn;
    sTaskRef mqttRecvTask;

    if(SC_SUCCESS != sAPI_MsgQCreate(&mqtt_recv_msg, "urc_mqtt_msgq_1", (sizeof(SIM_MSG_T)), 4, SC_FIFO))//msgQ for subscribed data transfer
    {
        sAPI_Debug("mqtt_aliauth message queue creat err!\n");
        return;
    }
    if(SC_SUCCESS != sAPI_MsgQCreate(&mqtt_conn_msgq, "mqtt_conn_msgq", (sizeof(SIM_MSG_T)), 4, SC_FIFO))//msgQ for waiting connnect param
    {
        sAPI_Debug("mqtt_conn_msgq message queue creat err!\n");
        return;
    }
    if(SC_SUCCESS != sAPI_MsgQCreate(&mqtt_disconn_msgq, "mqtt_disconn_msgq", (sizeof(SIM_MSG_T)), 4, SC_FIFO))//msgQ for waiting disconnect msg
    {
        sAPI_Debug("mqtt_disconn_msgq message queue creat err!\n");
        return;
    }
    if (SC_SUCCESS != sAPI_TaskCreate(&gMqttConn, NULL, SC_DEFAULT_THREAD_STACKSIZE*2, SC_DEFAULT_TASK_PRIORITY, (char *)"mqtconn", mqtt_aliauth_dynreg, (void *)0))
    {
        sAPI_Debug("mqtt_aliauth task create fail.\r\n");
        return;
    }

    if(SC_SUCCESS != sAPI_TaskCreate(&mqttRecvTask, NULL, SC_DEFAULT_THREAD_STACKSIZE*2, SC_DEFAULT_TASK_PRIORITY, (char *)"mqtrecv", sTask_MqttRecv, (void *)0))
    {
       sAPI_Debug("Task Create recv error!\n");
       return;
    }
    
    sAPI_Debug("mqtt_aliauth create success.\r\n");
}


