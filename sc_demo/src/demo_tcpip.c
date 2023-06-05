/**
  ******************************************************************************
  * @file    demo_tcpip.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of tcpip stack operation.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "simcom_os.h"
#include "simcom_tcpip.h"
#include "simcom_tcpip_old.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "scfw_socket.h"
#include "scfw_netdb.h"
#include "scfw_inet.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "stdio.h"
#include "stdarg.h"

#define SC_TCPIP_TEST_STACK_SIZE (1024*4)
#define SC_TCPIP_TEST_TASK_PRO 80
#define SC_MAX_STRING_LEN  64
#define SC_REMOTE_PORT_MIN               0
#define SC_REMOTE_PORT_MAX               65535
extern sMsgQRef simcomUI_msgq;
extern void PrintfOptionMenu(char* options_list[], int array_size);
extern void PrintfResp(char* format);



typedef enum{
    SC_TCPIP_DEMO_INIT                  = 1,
    SC_TCPIP_DEMO_TCP_CLIENT_OPEN       = 2,
    SC_TCPIP_DEMO_TCP_CLIENT_SEND       = 3,
    SC_CLIENT_CLOSE                     = 4,
    SC_TCPIP_DEMO_UDP_CLIENT_OPEN       = 5,
    SC_TCPIP_DEMO_UDP_CLIENT_SEND       = 6,
    SC_TCPIP_DEMO_UDP_CLIENT_CLOSE      = 7,
    SC_TCPIP_DEMO_TCP_SERVER_OPEN       = 8,
    SC_TCPIP_DEMO_TCP_SERVER_CLOSE      = 9,
    SC_TCPIP_DEMO_DEINIT                = 10,
    SC_TCPIP_DEMO_MAX                   = 99
}SC_FTPS_DEMO_TYPE;


char *options_list[] = {
    " 1. Init",
    " 2. TCP client open",
    " 3. TCP client send",
    " 4. socket close",
    " 5. UDP client open",
    " 6. UDP client send",
    " 8. TCP server open",
    " 9. TCP server close",
    "10. DeInit",
    "99. back",
};

sMsgQRef tcpipUIResp_msgq;
INT32 pdp_id = 1;
int tcpserver_localport= 0;
int tcp_serverfd = 0;
struct SCipInfo localip_info = {TCPIP_PDP_INVALID, 0, {0}};
sTaskRef tcpserverRef;

static sMsgQRef gClientMsgQueue = NULL;
static sTaskRef clientProcesser;

static sMsgQRef gServerMsgQueue = NULL;
static sTaskRef serverProcesser;

static sMutexRef sockMutexRef = NULL;
int g_tcp_udp_client_sockfd = -1;

/**
  * @brief  Get the ip address of the module and convert it into a struct sockaddr structure
  * @param  type
  *            AF_INET： Get ipv4 address
  *            AF_INET6：Get ipv6 address
  *         local_addr struct to hold addresses
  * @note   
  * @retval 0 is sucess,other is fail
  */
static int get_ipaddr(int type,struct sockaddr *local_addr)
{
	struct sockaddr_in ipv4sa;
	struct sockaddr_in6 ipv6sa;

    struct in_addr addr1;
    int isIpv6 = 0;
    int ret = -1;
    int result = -1;
    struct SCipInfo ipinfo;

    if(type == AF_INET6)
        isIpv6 = 1;

    ret = sAPI_TcpipGetSocketPdpAddr(1,1,&ipinfo);
    if(ret != SC_TCPIP_SUCCESS)
        goto exit;

    if(ipinfo.type == TCPIP_PDP_IPV4)
    {
        addr1.s_addr = ipinfo.ip4;
        sAPI_Debug("ipinfo.type[%d]",ipinfo.type);
      
        if(isIpv6 == 1)
        {
            sAPI_Debug("can't get ipv6 addr");
            goto exit;
        }

        sAPI_Debug("\r\nPDPCTX type: IPV4,\r\nADDR: %s\r\n",inet_ntoa(addr1));
        memset(&ipv4sa,0,sizeof(ipv4sa));
        ipv4sa.sin_family = AF_INET;
    
        memcpy(&ipv4sa.sin_addr,&ipinfo.ip4,sizeof(ipv4sa.sin_addr));
        memcpy(local_addr,&ipv4sa,sizeof(ipv4sa));
    }
    else if(ipinfo.type == TCPIP_PDP_IPV6)
    {
        if(isIpv6 == 0)
        {
            sAPI_Debug("can't get ipv4 addr");
            goto exit;
        }
        char dststr[100] = {0};
        inet_ntop(AF_INET6, ipinfo.ip6, dststr, sizeof(dststr));
        sAPI_Debug("\r\nPDPCTX type: IPV6,\r\nADDR: [%s]\r\n", dststr);
        memset(&ipv6sa,0,sizeof(ipv6sa));
        ipv6sa.sin6_family = AF_INET6;

        memcpy(&ipv6sa.sin6_addr,&ipinfo.ip6,sizeof(ipv6sa.sin6_addr));
        memcpy(local_addr,&ipv6sa,sizeof(ipv6sa));

    }
    else if(TCPIP_PDP_IPV4V6 == ipinfo.type)
    {

        char dststr[100] = {0};
        inet_ntop(AF_INET6, ipinfo.ip6, dststr, sizeof(dststr));
        addr1.s_addr = ipinfo.ip4;
        sAPI_Debug("\r\nPDPCTX type: IPV4V6,\r\nADDR: %s\r\nADDR: [%s]\r\n", inet_ntoa(addr1),dststr);
        if(isIpv6 == 1)
        {
            memset(&ipv6sa,0,sizeof(ipv6sa));
            ipv6sa.sin6_family = AF_INET6;
            memcpy(&ipv6sa.sin6_addr,&ipinfo.ip6,sizeof(ipv6sa.sin6_addr));
            memcpy(local_addr,&ipv6sa,sizeof(ipv6sa));
        }
        else
        {
            memset(&ipv4sa,0,sizeof(ipv4sa));
            ipv4sa.sin_family = AF_INET;   
            memcpy(&ipv4sa.sin_addr,&ipinfo.ip4,sizeof(ipv4sa.sin_addr));
            memcpy(local_addr,&ipv4sa,sizeof(ipv4sa));
        }

    }

    result = 0;
    

exit:

    return result;
}
/**
  * @brief  Convert struct sockaddr to string
  * @param  
  * @note   
  * @retval 0 is sucess,other is fail
  */
int Inet_ntop(const struct sockaddr *sa,
                  socklen_t salen,
                  char str[128],
                  UINT16 *port)
{


    switch (sa->sa_family)
    {
        case AF_INET:
        {
            struct sockaddr_in  *sin = (struct sockaddr_in *) sa;

            if (inet_ntop(AF_INET, &sin->sin_addr, str, 128) == NULL)
                return -1;

            *port = ntohs(sin->sin_port);
            return 0;
        }
        case AF_INET6:
        {
            struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;

            if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, 128) == NULL)
                return (-1);

            *port = ntohs(sin6->sin6_port);
            return 0;
        }
    }

    return -1;
}

void ui_print(const char *format,...){
    char tmpstr[200];
    va_list args;

    memset(tmpstr,0,sizeof(tmpstr));
    va_start(args,format);
    vsnprintf(tmpstr,sizeof(tmpstr),format,args);
    va_end(args);
    PrintfResp("\r\n");
    PrintfResp(tmpstr);
    PrintfResp("\r\n");
}
/**
  * @brief  initiate a connection on a socket
  * @param  
  * @note   
  * @retval 0 is sucess,other is fail
  */
static int Connect(int* socketfd,int cid,
            const char *host,
            unsigned short port)
{
    #define PORT_MAX_LEN   6
    int ret = -1;
    int fd = -1;
    char portstr[PORT_MAX_LEN] = {0};
    struct addrinfo hints;
    struct addrinfo *addr_list = NULL, *rp = NULL;
    if (socketfd == NULL || host == NULL)
        return -1;

    sAPI_Debug("host[%s] port[%d]",host,port);
    snprintf(portstr,sizeof(portstr),"%d",port);
    memset(&hints, 0x0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family   = AF_UNSPEC/*AF_INET*/;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(host, portstr, &hints, &addr_list) != 0)
    {
        sAPI_Debug("getaddrinfo error");
        return -1;
    }

    for (rp = addr_list; rp != NULL; rp = rp->ai_next){
        if((fd = socket(rp->ai_family, rp->ai_socktype, 0)) < 0)
        {
            continue;
        }
        if((ret = connect(fd, rp->ai_addr, rp->ai_addrlen)) == 0)
        {
            *socketfd = fd;
            sAPI_Debug("connect server sucess");
            break;
        }
        close(fd);
    }

    freeaddrinfo(addr_list);
    return ret;
}

/**
  * @brief  send a message on a socket
  * @param  
  * @note   
  * @retval 
  */
static int Send(int fd,
                   char *data,
                   int len,
                   int flag,
                   const struct sockaddr *destcAddr)
{
    int bytes = 0;
    int index = 0;

    while (len)
    {
        if (destcAddr == NULL)
        {
            bytes = send(fd, data + index, len, flag);
        }
        else
        {
            bytes = sendto(fd, data + index, len, flag, destcAddr, sizeof(struct sockaddr));
        }
        if (bytes < 0)
        {
            return -1;
        }
        else
        {
            len = len - bytes;
            index = index + bytes;
        }
    }

    return index;
}



void set_tcp_udp_client_sockfd(int sockfd)
{
    sAPI_MutexLock(sockMutexRef,SC_SUSPEND);
    g_tcp_udp_client_sockfd = sockfd;
    sAPI_MutexUnLock(sockMutexRef);
}
int get_tcp_udp_client_sockfd(void)
{
    int sockfd = -1;
    sAPI_MutexLock(sockMutexRef,SC_SUSPEND);
    sockfd = g_tcp_udp_client_sockfd;
    sAPI_MutexUnLock(sockMutexRef);
    return sockfd;
    
}
/**
  * @brief  tcp or udp recv task
  * @param  
  * @note   
  * @retval 
  */
static void tcp_udp_client_recv(int fd)
{
    fd_set master, read_fds;
    int fdmax = 0;
    int ret = -1;
    struct timeval tv;
    int socket_errno = 0;
    socklen_t addr_len;
    struct sockaddr addr;
    int i = 0;
    char recvbuf[1024];
    char ipstr[128] = {0};
    UINT16 port = 0;
    
    if(fd < 0)
    {
        return;
    }
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_SET(fd, &master);

    tv.tv_sec = 5;
    tv.tv_usec = 0;
    while(1)
    {
        read_fds = master;
        fdmax = fd + 1;
        ret = select(fdmax, &read_fds, NULL, NULL,&tv);
        if(ret > 0)
        {
            for(i = 0;i < fd + 1;i++)
            {
				if (!FD_ISSET(i, &read_fds))
				{
					continue;
				}
				if (i == fd)
				{
					memset(recvbuf,0,sizeof(recvbuf));
                    addr_len = sizeof(struct sockaddr);
                    memset(&addr,0,sizeof(addr));
					ret = recvfrom(fd,recvbuf,sizeof(recvbuf),0,&addr,&addr_len);
					if(ret > 0){
                        memset(ipstr,0,sizeof(ipstr));
                        if(Inet_ntop(&addr,addr_len,ipstr,&port) != 0)
                        {
                            ui_print("\r\n converts  the  network  address fail\r\n");
                        }
                        ui_print("recv ipstr[%s:%d] size[%d] [%s]",ipstr,port,ret,recvbuf);
						
					}else{
						socket_errno = lwip_getsockerrno(fd);

						if(socket_errno != EAGAIN /*&& errno != EWOULDBLOCK*/)
						{
							ui_print("recv fail errno[%d]",socket_errno);
							goto exit;
						}
					}
				}
			}
        }
        else if(ret == 0)
        {
            ui_print("select timeout");
            continue;
        }
        else
        {
            ui_print("select fail");
            goto exit;
            
        }
    }
exit:
    close(fd);
    set_tcp_udp_client_sockfd(-1);
    return;
}

static void server_recv(int fd)
{
    while(1)
    {
        

    }

}

static void tcp_udp_process(void *arg)
{
    SIM_MSG_T msg ={0,0,0,NULL};
    SC_STATUS status;
    int sockfd = 0;
    while(1)
    {
        status = sAPI_MsgQRecv(gClientMsgQueue,&msg,SC_SUSPEND);
        if(status == SC_SUCCESS)
        {
            sockfd = msg.arg1;

            tcp_udp_client_recv(sockfd);
        }
    }


}
static void server_process(void *arg)
{
   SIM_MSG_T msg ={0,0,0,NULL};
    SC_STATUS status;
    int sockfd = -1;
    while(1)
    {
        status = sAPI_MsgQRecv(gServerMsgQueue,&msg,SC_SUSPEND);
        if(status == SC_SUCCESS)
        {
            sockfd = msg.arg1;
            server_recv(sockfd);
        }
    }
}



void tcp_demo_task_init(void)
{
    static int inited = 0;
    if(inited == 0)
    {
        sAPI_MutexCreate(&sockMutexRef,SC_FIFO);
        sAPI_MsgQCreate(&gClientMsgQueue, "srvqueue", sizeof(SIM_MSG_T), 1, SC_FIFO);
        sAPI_TaskCreate(&clientProcesser,NULL,4096,150,"client1",tcp_udp_process,NULL);

        sAPI_MsgQCreate(&gServerMsgQueue, "srvqueue", sizeof(SIM_MSG_T), 1, SC_FIFO);
        sAPI_TaskCreate(&serverProcesser,NULL,4096,150,"server",server_process,NULL);
        inited = 1;
    }
   
}

int udp_send(int sockfd,const char *remoteIP,unsigned short port,void *data,int datelen)
{
    int socket_errno = -1;
    char portstr[PORT_MAX_LEN] = {0};
    struct addrinfo hints;
    struct addrinfo *addr_list = NULL;

    int ret = -1;
    if(sockfd < 0){
        ui_print("udp not't open [%d]",sockfd);
        return -1;
    }
    
    snprintf(portstr,sizeof(portstr),"%d",port);
    memset(&hints, 0x0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family   = AF_UNSPEC/*AF_INET*/;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(remoteIP, portstr, &hints, &addr_list) != 0)
    {
        ui_print("getaddrinfo error");
        return -1;
    }

    ret = Send(sockfd,data,datelen,0,addr_list->ai_addr);
    freeaddrinfo(addr_list);
    if(ret == datelen)
    {
        ui_print("send sucess");
    }
    else
    {
        socket_errno = lwip_getsockerrno(sockfd);
        ui_print("send fail socket_errno[%d]",socket_errno);
        if(socket_errno != EAGAIN /*&& errno != EWOULDBLOCK*/)
        {
            ret = 0;
        }
        else
        {
            ret = -1;
        }
    }

    return ret;
}

/**
  * @brief  TCP Demo.
  * @param  void
  * @note   
  * @retval void
  */
void TcpipDemo(void)
{
    SC_STATUS status;
    int opt = 0;
    int ret = -1;
    SIM_MSG_T optionMsg ={0,0,0,NULL};
    struct sockaddr local_addr;
    int sockfd = -1;
    int socket_errno = -1;
    SIM_MSG_T msg ={0,0,0,NULL};
    tcp_demo_task_init();
    while(1)
    {
        ui_print("Please select an option to test from the items listed below.");
        PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
        status = sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
        if(status != SC_SUCCESS && SRV_UART != optionMsg.msg_id)
        {
            sAPI_Debug("%s,msg_id is error!!",__func__);
            break;
        }
        sAPI_Debug("arg3 = [%s]",optionMsg.arg3);
        opt = atoi(optionMsg.arg3);
        sAPI_Free(optionMsg.arg3);
        switch(opt)
        {
            case SC_TCPIP_DEMO_INIT:
            {
                if (SC_TCPIP_SUCCESS == sAPI_TcpipPdpActive(pdp_id, 1))
                {
                    char ipstr[128] = {0};
                    UINT16 port = 0;

                    sAPI_Debug("pdp active successful");
                    if(get_ipaddr(AF_INET,&local_addr) != 0)
                    {
                        ui_print("\r\ncan't get ip address\r\n");
                        break;
                    }
                    memset(ipstr,0,sizeof(ipstr));
                    if(Inet_ntop(&local_addr,sizeof(local_addr),ipstr,&port) != 0)
                    {
                        ui_print("\r\n converts  the  network  address fail\r\n");
                    }
                    ui_print("ipstr[%s] port[%d]",ipstr,port);

                }
                else
                {
                    ui_print("TCPIP Init Fail !");
                }

                break;  
            }
              

            case SC_TCPIP_DEMO_TCP_CLIENT_OPEN:
            {
                char *serverIP = "47.108.134.223";
                unsigned short port = 8888;
                /*
                The demo only supports one tcp or one udp client, and the module itself can support 64-way tcp
                */
                if(get_tcp_udp_client_sockfd() >= 0){
                    ui_print("socket is busy sockfd[%d]",sockfd);
                    break;
                }
                ret = Connect(&sockfd,1,serverIP,port);
                if(ret == 0)
                {
                    ui_print("connect [%s:%d] sucess",serverIP,port);
                    ui_print("sockfd is [%d]",sockfd);
                    set_tcp_udp_client_sockfd(sockfd);
                    memset(&msg,0,sizeof(msg));
                    msg.arg1 = sockfd;
                
                    status = sAPI_MsgQSend(gClientMsgQueue,&msg);
                    if(status != SC_SUCCESS)
                    {
                        ui_print("tcp recv thread busy status[%d]",status);
                        close(sockfd);
                        set_tcp_udp_client_sockfd(-1);
                        sockfd = -1;
                    }

                }
                else
                {
                    ui_print("connect [%s:%d] fail",serverIP,port);
                }
                break;
            }
            case SC_TCPIP_DEMO_TCP_CLIENT_SEND:
            {
                 
                if(get_tcp_udp_client_sockfd() < 0)
                {
                    ui_print("\r\nYou haven't opened the tcp client !\r\n");
                }
                else
                {
                    char *sendstr = "hello";
                    ret = Send(sockfd,sendstr,strlen(sendstr),0,NULL);
                    if(ret == strlen(sendstr))
                    {
                        ui_print("send sucess");
                    }
                    else
                    {
                        socket_errno = lwip_getsockerrno(sockfd);
                        ui_print("send fail socket_errno[%d]",socket_errno);
                        if(socket_errno != EAGAIN /*&& errno != EWOULDBLOCK*/)
                        {
                            ret = close(sockfd);
                            set_tcp_udp_client_sockfd(-1);
                           
                        }
                    }
                }
                
                break;
            }
            

            case SC_CLIENT_CLOSE:
              {
                  if(get_tcp_udp_client_sockfd() < 0)
                  {
                      ui_print("no tcp client and udp open");
                  }
                  else
                  {
                      ret = close(sockfd);
                      if(ret == 0)
                      {
                        ui_print("close client sucess");
                        set_tcp_udp_client_sockfd(-1);
                      }
                      else
                      {
                        errno = lwip_getsockerrno(sockfd);
                        ui_print("close client fail errno[%d]",errno);

                      }
                  }
              }
              break;

            case SC_TCPIP_DEMO_UDP_CLIENT_OPEN:
            {
                  /*
                The demo only supports one tcp or one udp client, and the module itself can support 64-way tcp
                */
                if(get_tcp_udp_client_sockfd() >= 0){
                    ui_print("socket is busy sockfd[%d]",sockfd);
                    break;
                }

                sockfd = socket(AF_INET,SOCK_DGRAM, 0);
                if(sockfd < 0)
                {
                    ui_print("\r\nUDP client fail !\r\n");
                }
                else
                {
                    set_tcp_udp_client_sockfd(sockfd);
                    ui_print("\r\nUDP client open success !\r\n");
                }
                break;
            }
            

            case SC_TCPIP_DEMO_UDP_CLIENT_SEND:
            {
                char *remoteIP = "47.108.134.223";
                unsigned short port = 8888;
                char *sendstr = "hello";

                if(get_tcp_udp_client_sockfd() < 0){
                    ui_print("socket is busy sockfd[%d]",sockfd);
                    break;
                }
              
                ret = udp_send(get_tcp_udp_client_sockfd(),remoteIP,port,sendstr,strlen(sendstr));
                if(ret > 0)
                {
                    ui_print("send to [%s:%d] sucess",remoteIP,port);
                }
                else if(ret == 0)
                {
                    ui_print("send fail try again");
                }
                else
                {
                    ui_print("send fail");
                    close(get_tcp_udp_client_sockfd());
                    set_tcp_udp_client_sockfd(-1);
                }
                break;
            }
            case SC_TCPIP_DEMO_TCP_SERVER_OPEN:
            {

                break;
            }
            case SC_TCPIP_DEMO_TCP_SERVER_CLOSE:
            {

                break;
            }
            case SC_TCPIP_DEMO_DEINIT:
            {
                if(get_tcp_udp_client_sockfd() >= 0)
                {
                    close(get_tcp_udp_client_sockfd());
                    set_tcp_udp_client_sockfd(-1);
                    break;
                }
                if (SC_TCPIP_SUCCESS == sAPI_TcpipPdpDeactive(pdp_id, 1))
                {
                    ui_print("pdp deavtive sucess");
                }
                else
                {
                    ui_print("TCPIP Init Fail !");
                }
                break;
            }
            case SC_TCPIP_DEMO_MAX:
            {
                return;
            }
            default :
            {
                ui_print("Input err !");
                break;
            }
        }
        
    }
  
}




