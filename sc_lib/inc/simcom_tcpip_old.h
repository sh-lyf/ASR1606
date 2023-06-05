/**
  ******************************************************************************
  * @file    simcom_tcpip.h
  * @author  SIMCom OpenSDK Team
  * @brief   Header file of internal tcp\ip stack.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */
#ifndef _SIMCOM_TCPIP_OLD_H_
#define _SIMCOM_TCPIP_OLD_H_

#include "simcom_os.h"
#include "scfw_socket.h"
#include "scfw_netdb.h"
#include "scfw_inet.h"

#define SC_FD_SETSIZE      128

#define SC_SOL_SOCKET      0xfff    /* options for socket level */
#define SC_SO_ERROR        0x1007


#define SC_ECONNRESET      104
#define SC_ECONNABORTED    103  /* Software caused connection abort */

#define SC_AF_INET         2
#define SC_AF_INET6        10
#define SC_INADDR_ANY      ((INT32)0x00000000)

/* Socket protocol types (TCP/UDP/RAW) */
#define SC_SOCK_STREAM     1
#define SC_SOCK_DGRAM      2
#define SC_SOCK_RAW        3

#define SC_INVALID_SOCKET  (0xFFFFFFFFL)
#define SC_SOCKET_ERROR    (0xFFFFFFFFL)

#define SC_SO_ACCEPTCONN   0x0002 /* socket has had listen() */
#define SC_SO_REUSEADDR    0x0004 /* Allow local address reuse */
#define SC_SO_KEEPALIVE    0x0008 /* keep connections alive */

#define SC_TCP_NODELAY         0x01    /* don't delay send to coalesce packets */
#define SC_TCP_KEEPALIVE       0x02    /* send KEEPALIVE probes when idle for pcb->keep_idle milliseconds */
#define SC_TCP_KEEPIDLE        0x03    /* set pcb->keep_idle  - Same as TCP_KEEPALIVE, but use seconds for get/setsockopt */
#define SC_TCP_KEEPINTVL       0x04    /* set pcb->keep_intvl - Use seconds for get/setsockopt */
#define SC_TCP_KEEPCNT         0x05    /* set pcb->keep_cnt   - Use number of probes sent for get/setsockopt */
#define SC_TCP_TIMEROUT        0x06    /* set pcb->TCP_TIMEROUT   - Use number of probes sent for get/setsockopt */
#define SC_TCP_MSS_VALUE       0x07    /* set pcb->mss   - Use number of probes only for get/setsockopt */
#define SC_TCP_RTO_VALUE       0x08    /* set pcb->rto  - Use number of probes only for get/setsockopt */
#define SC_TCP_TXB_UNACK       0x09    /* set (pcb->snd_lbb - pcb->lastack) - how much byte remaind in buffer for unacked */
#define SC_TCP_TXB_REST        0x0A    /* set (pcb->snd_buf) - how much byte remaind in tx buffer can writen by app */
#define SC_TCP_TXB_ACKED       0x0B    /* set (pcb->acked_sum) - the number of accumulation of acked */
#define SC_TCP_CLOSE_TIMEROUT  0x0C    /* set (pcb->close_timeout) - the timerout set of tcp close all */
#define SC_TCP_RST_TRIGGER     0x0D    /* set reset man trigger - call tcp_reset_man_trigger() */
#define SC_TCP_INFO            0x0E    /* get tcp_info */

/*
 * Option flags per-socket.
 */
#define SC_SO_DEBUG        0x0001 /* Unimplemented: turn on debugging info recording */
#define SC_SO_ACCEPTCONN   0x0002 /* socket has had listen() */
#define SC_SO_REUSEADDR    0x0004 /* Allow local address reuse */
#define SC_SO_KEEPALIVE    0x0008 /* keep connections alive */
#define SC_SO_DONTROUTE    0x0010 /* Unimplemented: just use interface addresses */
#define SC_SO_BROADCAST    0x0020 /* permit to send and to receive broadcast messages (see IP_SOF_BROADCAST option) */
#define SC_SO_USELOOPBACK  0x0040 /* Unimplemented: bypass hardware when possible */
#define SC_SO_LINGER       0x0080 /* linger on close if data present */
#define SC_SO_DONTLINGER   ((int)(~SC_SO_LINGER))
#define SC_SO_OOBINLINE    0x0100 /* Unimplemented: leave received OOB data in line */
#define SC_SO_REUSEPORT    0x0200 /* Unimplemented: allow local address & port reuse */
#define SC_SO_IPSEC        0x0400 /* denote ipsec using */

#define SC_SO_SNDBUF       0x1001  /* Unimplemented: send buffer size */
#define SC_SO_RCVBUF       0x1002  /* receive buffer size */
#define SC_SO_SNDLOWAT     0x1003  /* Unimplemented: send low-water mark */
#define SC_SO_RCVLOWAT     0x1004  /* Unimplemented: receive low-water mark */
#define SC_SO_SNDTIMEO     0x1005  /* Unimplemented: send timeout */
#define SC_SO_RCVTIMEO     0x1006  /* receive timeout */
#define SC_SO_ERROR        0x1007  /* get error status and clear */
#define SC_SO_TYPE         0x1008  /* get socket type */
#define SC_SO_CONTIMEO     0x1009  /* Unimplemented: connect timeout */
#define SC_SO_NO_CHECK     0x100a  /* don't create UDP checksum */
#define SC_SO_BINDTODEVICE 0x100b /* bind to device */
#define SC_SO_BIO          0x100c  /* set socket into blocking mode */
#define SC_SO_NBIO         0x100d  /* set socket into non-blocking mode */
#define SC_SO_NONBLOCK     0x100e  /* set/get socket blocking mode via optval param*/


/** 255.255.255.255 */
#define SC_IPADDR_NONE         ((UINT32)0xffffffffUL)

#define SC_IPPROTO_IP      0
#define SC_IPPROTO_TCP     6
#define SC_IPPROTO_UDP     17

/* Flags we can use with sAPI_TcpipSend and sAPI_TcpipRecv. */
#define SC_MSG_PEEK       0x01    /* Peeks at an incoming message */
#define SC_MSG_WAITALL    0x02    /* Unimplemented: Requests that the function block until the full amount of data requested can be returned */
#define SC_MSG_OOB        0x04    /* Unimplemented: Requests out-of-band data. The significance and semantics of out-of-band data are protocol-specific */
#define SC_MSG_DONTWAIT   0x08    /* Nonblocking i/o for this operation only */
#define SC_MSG_MORE       0x10    /* Sender will send more */

#define SC_TCPIP_SUCCESS      0
#define SC_TCPIP_FAIL        -1

/*
 * howto arguments for shutdown(2), specified by Posix.1g.
 */
enum
{
  SC_SHUT_RD	=0,		/* shut down the reading side */
  SC_SHUT_WR,			/* shut down the writing side */
  SC_SHUT_RDWR			/* shut down both sides */
};		

typedef struct
{
    INT32 tv_sec;        /* seconds */
    INT32 tv_usec;       /* and microseconds */
} SCtimeval;
#if 0
enum SCnetType
{
    TCPIP_PDP_IPV4 = 1,
    TCPIP_PDP_IPV6,
    TCPIP_PDP_IPV4V6,
    TCPIP_PDP_INVALID
};

struct SCipInfo
{
    enum SCnetType type;
    UINT32 ip4;
    UINT32 ip6[4];
};
#endif
typedef struct
{
    UINT32 s_addr;
} SCinAddr;

typedef struct
{
    UINT8 sin_len;
    UINT8 sin_family;
    UINT16 sin_port;
    SCinAddr sin_addr;
#define SC_SIN_ZERO_LEN 8
    INT8 sin_zero[SC_SIN_ZERO_LEN];
} SCsockAddrIn;



typedef struct
{
    UINT8 fd_bits [(SC_FD_SETSIZE * 2 + 7) / 8];
} SCfdSet;


typedef struct
{
    INT8  *h_name;      /* Official name of the host. */
    INT8 **h_aliases;   /* A pointer to an array of pointers to alternative host names,
                           terminated by a null pointer. */
    INT32    h_addrtype;  /* Address type. */
    INT32    h_length;    /* The length, in bytes, of the address. */
    INT8 **h_addr_list; /* A pointer to an array of pointers to network addresses (in
                           network byte order) for the host, terminated by a null pointer. */
#define h_addr h_addr_list[0] /* for backward compatibility */
} SChostent;

typedef void   *sMsgQRef;


typedef struct
{
    UINT8 sa_len;
    UINT8 sa_family;
#if LWIP_IPV6
    UINT8 sa_data[22];
#else /* LWIP_IPV6 */
    UINT8 sa_data[14];
#endif /* LWIP_IPV6 */
} SCsockAddr;

struct SCaddrinfo {
    int               ai_flags;      /* Input flags. */
    int               ai_family;     /* Address family of socket. */
    int               ai_socktype;   /* Socket type. */
    int               ai_protocol;   /* Protocol of socket. */
    UINT32         ai_addrlen;    /* Length of socket address. */
    SCsockAddr *ai_addr;       /* Socket address of socket. */
    char             *ai_canonname;  /* Canonical name of service location. */
    struct SCaddrinfo  *ai_next;       /* Pointer to next in list. */
};

#define SC_IOCPARM_MASK    0x7fU           /* parameters must be < 128 bytes */
#define SC_IOC_VOID        0x20000000UL    /* no parameters */
#define SC_IOC_OUT         0x40000000UL    /* copy out parameters */
#define SC_IOC_IN          0x80000000UL    /* copy in parameters */
#define SC_IOC_INOUT       (SC_IOC_IN|SC_IOC_OUT)
/* 0x20000000 distinguishes new &
   old ioctl's */
#define SC_IO(x,y)        (SC_IOC_VOID|((x)<<8)|(y))

#define SC_IOR(x,y,t)     (SC_IOC_OUT|(((UINT32)sizeof(t)&SC_IOCPARM_MASK)<<16)|((x)<<8)|(y))

#define SC_IOW(x,y,t)     (SC_IOC_IN|(((UINT32)sizeof(t)&SC_IOCPARM_MASK)<<16)|((x)<<8)|(y))

#define SC_FIONBIO     SC_IOW('f', 126, UINT32) /* set/clear non-blocking i/o */

#define SC_SOCKET_OFFSET  (0) /*note: must set to 0*/

#define SC_MEMP_NUM_NETCONN 128

#define SC_FDSETSAFESET(n, code)  do {\
    if ((((int)(n) - SC_SOCKET_OFFSET) < (SC_MEMP_NUM_NETCONN * 2)) \
       && (((int)(n) - SC_SOCKET_OFFSET) >= 0)) {code;}\
    } while(0)

#define SC_FDSETSAFEGET(n, code) (((((int)(n) - SC_SOCKET_OFFSET) < (SC_MEMP_NUM_NETCONN * 2)) \
       && (((int)(n) - SC_SOCKET_OFFSET) >= 0)) ? (code) : 0)

#define SC_FD_SET(n, p)  SC_FDSETSAFESET(n, (p)->fd_bits[((n) - SC_SOCKET_OFFSET)/8] |=  (1 << (((n) - SC_SOCKET_OFFSET) & 7)))
#define SC_FD_CLR(n, p)  SC_FDSETSAFESET(n, (p)->fd_bits[((n) - SC_SOCKET_OFFSET)/8] &= ~(1 << (((n) - SC_SOCKET_OFFSET) & 7)))
#define SC_FD_ISSET(n,p) SC_FDSETSAFEGET(n, (p)->fd_bits[((n) - SC_SOCKET_OFFSET)/8] &   (1 << (((n) - SC_SOCKET_OFFSET) & 7)))
#define SC_FD_ZERO(p)    memset((void*)(p),0,sizeof(*(p)))


enum
{
    SC_SOCKET_NONE,
    SC_SOCKET_WRITE,
    SC_SOCKET_LISTEN,
    SC_SOCKET_CONNECT,
    SC_SOCKET_CLOSE
};

/** Use to inform the callback function about changes */
enum 
{
  SC_NETCONN_EVT_RCVPLUS,
  SC_NETCONN_EVT_RCVMINUS,
  SC_NETCONN_EVT_SENDPLUS,
  SC_NETCONN_EVT_SENDMINUS,
  SC_NETCONN_EVT_CONNECTED,
  SC_NETCONN_EVT_ACCEPTPLUS,
  SC_NETCONN_EVT_ERROR_CLSD,
  SC_NETCONN_EVT_ERROR_RST,
  SC_NETCONN_EVT_ERROR_ABRT,
  SC_NETCONN_EVT_CLOSE_WAIT,
  SC_NETCONN_EVT_SENDACKED,
  SC_NETCONN_EVT_CLOSE_NORMAL,
  SC_NETCONN_EVT_ERROR
};
/*****************************************************************************
* FUNCTION
*  sAPI_TcpipSocket
*
* DESCRIPTION
*  Allocates a socket.
*  The parameters specify what type of socket is requested. Since this socket API implementation is concerned only with network sockets,
*  these are the only socket type that is supported. Also, TCP (SOCK STREAM) sockets can be used.
*
* PARAMETERS
*   domain	  Specifies the protocol family of the created socket.
*   type      socket protocol types.
*   protocol  Type of protocol.
*
* RETURNS
*  Upon  successful  completion, sAPI_TcpipSocket() shall return a non-negative integer, the socket file descriptor.
*  -1 - Failure
*
* NOTE
*
*****************************************************************************/
INT32 sAPI_TcpipSocket(INT32 domain, INT32 type, INT32 protocol);

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipBind
*
* DESCRIPTION
*   Associates a local address with a socket.
*   This fucntion is used on an unconnected datagram or stream socket, before a subsequent sAPI_TcpipConnect() or sAPI_TcpipListen().
*
* PARAMETERS
*    sockfd	Specifies the index of the socket.
*    addr	Pointer to sockaddr structure that identify connection.
*    addrlen	Size name structure.
*
* RETURNS
*    Upon  successful  completion, sAPI_TcpipSocket() shall return a non-negative integer, the socket file descriptor.
*    -1 - Failure
*
* NOTE
*
*****************************************************************************/
INT32 sAPI_TcpipBind(INT32 sockfd, const SCsockAddr *addr, UINT32 addrlen);

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipListen
*
* DESCRIPTION
*   Sets a socket into listen mode.
*   The socket may not have been used for another connection previously.
* PARAMETERS
*    s	        Specifies the index of the socket.
*    backlog	the maximum length to which the queue of pending connections for sockfd may grow.  
* RETURNS
*    0 - OK
*    -1 - Failure
*
* NOTE
*
*****************************************************************************/
INT32 sAPI_TcpipListen(INT32 sockfd, INT32 backlog);

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipAccept
*
* DESCRIPTION
*   Accepts a connection on a socket.
*   The sAPI_TcpipAccept() call is used to wait for incoming connections on a TCP socket that previously
*   has been set into LISTEN state by a call to sAPI_TcpipListen(). The call to sAPI_TcpipAccept() blocks until a 
*   connection has been established with a remote host. The arguments to listen are result parameters
*   that are set by the call to sAPI_TcpipAccept(). These are filled with the address of the remote host. 
*   When the new connection has been established, a new socket identifier is allocated and returned.
* PARAMETERS
*    sockfd	Specifies the index of the socket.
*    addr	Pointer to sockaddr structure that identify connection.
*    addrlen	Size name structure. 
* RETURNS
*    New socket
*    -1 - Failure
*
* NOTE
*
*****************************************************************************/
INT32 sAPI_TcpipAccept(INT32 sockfd, SCsockAddr *addr, UINT32 *addrlen);

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipConnect
*
* DESCRIPTION
*   Attempts to make a connection on a socket.
* PARAMETERS
*    sockfd	Specifies the index of the socket.
*    addr	Pointer to sockaddr structure that identify connection.
*    addrlen	Size name structure. 
* RETURNS
*    0 - OK
*    -1 - Failure
*
* NOTE
*
*****************************************************************************/
INT32 sAPI_TcpipConnect(INT32 sockfd, const SCsockAddr *addr, UINT32 addrlen);

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipSend
*
* DESCRIPTION
*   Sends bytes.
*   This function is used in TCP connection for sending data. Before a call to sAPI_TcpipSend() the receiver of the data must have been set up using sAPI_TcpipConnect().
* PARAMETERS
*    s	Specifies the index of the socket.
*    data	Data buffer to send.
*    size	Size of the data.
*    flags	User can use this flags:
*        0x00 - No Flags
*        SC_MSG_PEEK - Peeks at an incoming message
*        SC_MSG_DONTWAIT - Nonblocking i/o for this operation only
*        SC_MSG_MORE - Sender will send more
* RETURNS
*    0 - OK
*    -1 - Failure
*
* NOTE
*
*****************************************************************************/
INT32 sAPI_TcpipSend(INT32 sockfd, const void *buf, INT32 len, INT32 flags);

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipRecv
*
* DESCRIPTION
*   Received bytes.
*   the sAPI_TcpipRecv() calls are used on a connected socket to receive data. 
*   A number of flags can be passed by the call to sAPI_TcpipRecv(). If the received
*   message is larger than the supplied memory area, the excess data is silently discarded.
* PARAMETERS
*    sockfd	Specifies the index of the socket.
*    buf	Memory buffer to store received data.
*    len	Length to data to receiving.
*    flags	User shall use one of followings:
*        0x00 - No Flags
*        SC_MSG_PEEK - Peeks at an incoming message
*        SC_MSG_DONTWAIT - Nonblocking i/o for this operation only
*        SC_MSG_MORE - Wait for more than one message
* RETURNS
*    0 - OK
*    -1 - Failure
*
* NOTE
*
*****************************************************************************/
INT32 sAPI_TcpipRecv(INT32 sockfd, void *buf, INT32 len, INT32 flags);

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipSendto
*
* DESCRIPTION
*   Sends bytes.
*   This function is used in TCP connection for sending data. Before a call to sAPI_TcpipSend() the receiver of the data must have been set up using sAPI_TcpipConnect().
* PARAMETERS
*    s	Specifies the index of the socket.
*    data	Data buffer to send.
*    size	Size of the data.
*    flags	User can use this flags:
*        0x00 - No Flags
*        SC_MSG_PEEK - Peeks at an incoming message
*        SC_MSG_DONTWAIT - Nonblocking i/o for this operation only
*        SC_MSG_MORE - Sender will send more
*    addr	the addr struture with the remote network parameter like port and IP address.
*    addrlen Size name structure.
* RETURNS
*    0 - OK
*    -1 - Failure
*
* NOTE
*
*****************************************************************************/
INT32 sAPI_TcpipSendto(INT32 sockfd, const void *buf, INT32 len, INT32 flags,
                       const SCsockAddr *dest_addr, UINT32 addrlen);

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipRecvfrom
*
* DESCRIPTION
*   Received bytes.
*   used to receive messages from a socket, and may be used to receive data on a
*   socket whether or not it is connection-oriented.
* PARAMETERS
*    sockfd	Specifies the index of the socket.
*    buf	Memory buffer to store received data.
*    len	Length to data to receiving.
*    flags	User shall use one of followings:
*        0x00 - No Flags
*        SC_MSG_PEEK - Peeks at an incoming message
*        SC_MSG_DONTWAIT - Nonblocking i/o for this operation only
*        SC_MSG_MORE - Wait for more than one message
*    addr	fill the addr struture with the remote network parameter like port and IP address.
*    addrlen Size name structure.
* RETURNS
*    0 - OK
*    -1 - Failure
*
* NOTE
*
*****************************************************************************/
INT32 sAPI_TcpipRecvfrom(INT32 sockfd, void *buf, INT32 len, INT32 flags,
                         SCsockAddr *src_addr, UINT32 *addrlen);

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipClose
*
* DESCRIPTION
*   Closes a socket.
* PARAMETERS
*    fd	Specifies the index of the socket.
* RETURNS
*    0 - OK
*    -1 - Failure
*
* NOTE
*
*****************************************************************************/
INT32 sAPI_TcpipClose(INT32 fd);
/*****************************************************************************
* FUNCTION
*  sAPI_TcpipShutdown
*
* DESCRIPTION
*   shut down socket send and receive operations
* PARAMETERS
*    fd	Specifies the index of the socket.
*    how    Specifies the type of shutdown. The values are as follows:
*       SC_SHUT_RD
*              Disables further receive operations.
*
*       SC_SHUT_WR
*              Disables further send operations.
*
*       SC_SHUT_RDWR
*              Disables further send and receive operations.
* RETURNS
*    0 - OK
*    -1 - Failure
*
* NOTE
*
*****************************************************************************/
INT32 sAPI_TcpipShutdown(INT32 sockfd, INT32 how);

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipGetsockname
*
* DESCRIPTION
*   Check local network parameters.
*   This function fill the addr struture with the local network parameter like port and IP address.
* PARAMETERS
*    sockfd	Specifies the index of the socket.
*    addr	Pointer to sockaddr structure that identify connection.
*    addrlen	Size name structure.
* RETURNS
*    0 - OK
*    -1 - Failure
*
* NOTE
*
*****************************************************************************/
INT32 sAPI_TcpipGetsockname(INT32 sockfd, SCsockAddr *addr, UINT32 *addrlen);

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipGetpeername
*
* DESCRIPTION
*   The sAPI_TcpipGetpeername() function retrieves the peer address of the specified socket,
*   stores this address in the sockaddr structure pointed to by the addr argument,
*   and stores the length of this address in the object pointed to by the addrlen argument.
* PARAMETERS
*    sockfd	Specifies the index of the socket.
*    addr	Pointer to sockaddr structure that identify connection.
*    addrlen	Size name structure.
* RETURNS
*    0 - OK
*    -1 - Failure
*
* NOTE
*
*****************************************************************************/
INT32 sAPI_TcpipGetpeername(INT32 sockfd, SCsockAddr *addr, UINT32 *addrlen);

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipGetsockopt
*
* DESCRIPTION
*   get the socket options
* PARAMETERS
*    sockfd	 Specifies the index of the socket.
*    level   specifies the protocol level at which the option resides
*    optname specifies a single option to be retrieved
*    optval	 option value
*    optlen  size of the option value
* RETURNS
*    0 - OK
*    -1 - Failure
*
* NOTE
*   If the size of the option value is greater than option_len, the value stored 
*   in the object pointed to by the option_value argument will be silently truncated. 
*   Otherwise, the object pointed to by the option_len argument will be modified to
*   indicate the actual length of the value.
*****************************************************************************/
INT32 sAPI_TcpipGetsockopt(INT32 sockfd, INT32 level, INT32 optname,
                           void *optval, UINT32 *optlen);

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipSetsockopt
*
* DESCRIPTION
*   set the socket options
* PARAMETERS
*    sockfd	 Specifies the index of the socket.
*    level   specifies the protocol level at which the option resides
*    optname specifies a single option to be retrieved
*    optval	 option value
*    optlen  size of the option value
* RETURNS
*    0 - OK
*    -1 - Failure
*
* NOTE
*
*****************************************************************************/
INT32 sAPI_TcpipSetsockopt(INT32 sockfd, INT32 level, INT32 optname,
                           const void *optval, UINT32 optlen);


/*****************************************************************************
* FUNCTION
*  sAPI_TcpipIoctlsocket
*
* DESCRIPTION
*   set the socket options
* PARAMETERS
*    fd	    Specifies the index of the socket.
*    level  device-dependent request code
*    argp   untyped pointer to memory
* RETURNS
*    0 - OK
*    -1 - Failure
*
* NOTE
*
*****************************************************************************/
INT32 sAPI_TcpipIoctlsocket(INT32 fd, INT32 level, void *argp);

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipIoctlsocket
*
* DESCRIPTION
*   Returns an entry containing addresses of address family AF_INET for the host 
*   with name name. Due to sAPI_TcpipGethostbyname limitations, only one address is returned.
* PARAMETERS
*    name	The hostname to resolve
* RETURNS
*    An entry containing addresses of address family AF_INET for the host with name name
*    NULL - Failure
*
* NOTE
*
*****************************************************************************/
 SChostent *sAPI_TcpipGethostbyname(INT8 *name);

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipSelect
*
* DESCRIPTION
*   synchronous I/O multiplexing
* PARAMETERS
*    nfds	specifies the range of descriptors to be tested.
*    readfds If  the  readfds  argument  is  not a null pointer, it points to an object of type fd_set that on
*            input specifies the file descriptors to be checked for being ready to read, and on  output  indi‐
*            cates which file descriptors are ready to read.   
*    writefds If  the  writefds  argument  is not a null pointer, it points to an object of type fd_set that on
*             input specifies the file descriptors to be checked for being ready to write, and on output  indi‐
*             cates which file descriptors are ready to write.
*    exceptfds If  the  errorfds  argument  is not a null pointer, it points to an object of type fd_set that on
*              input specifies the file descriptors to be checked for error conditions pending,  and  on  output
*              indicates which file descriptors have error conditions pending.
* RETURNS
*    Upon successful completion, the  sAPI_TcpipSelect() functions shall return the total number of
*    bits set in the bit masks. Otherwise, -1 shall be returned,If  the  writefds  argument
*    is not a null pointer Returns 0 if the socket has not changed within the specified time
*
* NOTE
*
*****************************************************************************/
 INT32 sAPI_TcpipSelect(INT32 nfds, SCfdSet *readfds, SCfdSet *writefds,
                       SCfdSet *exceptfds, SCtimeval *timeout);     

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipInet_addr
*
* DESCRIPTION
*   Ascii internet address interpretation routine. The value returned is in network order.
* PARAMETERS
*    cp	IP address in ascii represenation (e.g. "127.0.0.1")
* RETURNS
*    ip address in network order
*
* NOTE
*
*****************************************************************************/
UINT32 sAPI_TcpipInet_addr(INT8 *cp);

static inline  UINT32 sAPI_TcpipInetAddr(INT8 *cp)
{
    return sAPI_TcpipInet_addr(cp);
}
/*****************************************************************************
* FUNCTION
*  sAPI_TcpipHtons
*
* DESCRIPTION
*   Convert an UINT16 from host to network byte order.
* PARAMETERS
*    n	UINT16 in host byte order
* RETURNS
*    in network byte order
*
* NOTE
*
*****************************************************************************/    
UINT16 sAPI_TcpipHtons(UINT16 hostshort);

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipNtohs
*
* DESCRIPTION
*   Convert an UINT16 from network to host byte order.
* PARAMETERS
*    hostshort	UINT16 in network byte order
* RETURNS
*    in host byte order
*
* NOTE
*
*****************************************************************************/    
UINT16 sAPI_TcpipNtohs(UINT16 hostshort);

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipInet_ntoa
*
* DESCRIPTION
*   Convert numeric IP address into decimal dotted ASCII representation.
*   returns ptr to static buffer; not reentrant!
* PARAMETERS
*    in	IP address in network order to convert
* RETURNS
*    Pointer to a global static (!) buffer that holds the ASCII represenation of addr
*
* NOTE
*
*****************************************************************************/   
INT8 *sAPI_TcpipInet_ntoa(UINT32 in);

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipInet_ntop
*
* DESCRIPTION
*  This  function converts the network address structure src in the af address family into a charac‐
*  ter string.  The resulting string is copied to the buffer pointed to by dst, which must be a non-NULL  pointer. 
*  The caller specifies the number of bytes available in this buffer in the argument size.
* PARAMETERS
*     af User shall use one of followings:
*        AF_INET
*              src points to a struct in_addr (in network byte order) which is converted to an IPv4  net‐
*              work  address  in the dotted-decimal format, "ddd.ddd.ddd.ddd".  The buffer dst must be at
*              least INET_ADDRSTRLEN bytes long.
*        AF_INET6
*              src points to a struct in6_addr (in network byte order) which is converted to a  represen‐
*              tation  of  this  address  in  the  most  appropriate IPv6 network address format for this
*              address.  The buffer dst must be at least INET6_ADDRSTRLEN bytes long.
*     src network  address structure 
*     dst resulting string is copied to the buffer pointed to by dst
*     size The caller  specifies the number of bytes available in this buffer in the argument size
* RETURNS
*    On success, sAPI_TcpipInet_ntop() returns a non-NULL pointer to dst.  
*    NULL is returned if there was an error.
*
* NOTE
*
*****************************************************************************/   
INT8 *sAPI_TcpipInet_ntop(INT32 af, const void *src, INT8 *dst,UINT32 size);

static inline  INT8 *sAPI_TcpipInetNtop(INT32 af, const void *src, INT8 *dst,UINT32 size)
{
    return sAPI_TcpipInet_ntop(af,src,dst,size);

}
/*****************************************************************************
* FUNCTION
*  sAPI_TcpipInet_pton
*
* DESCRIPTION
*  This  function  converts  the  character  string  src  into a network address structure in the af
*  address family, then copies the network address structure to dst.
* PARAMETERS
*     af User shall use one of followings:
*        AF_INET
*              src points to a struct in_addr (in network byte order) which is converted to an IPv4  net‐
*              work  address  in the dotted-decimal format, "ddd.ddd.ddd.ddd".  The buffer dst must be at
*              least INET_ADDRSTRLEN bytes long.
*        AF_INET6
*              src points to a struct in6_addr (in network byte order) which is converted to a  represen‐
*              tation  of  this  address  in  the  most  appropriate IPv6 network address format for this
*              address.  The buffer dst must be at least INET6_ADDRSTRLEN bytes long.
*     src character  string 
*     dst network address structure
* RETURNS
*   sAPI_TcpipInet_pton() returns 1 on success (network address was successfully converted).  0 is returned if src does  not  
*   contain  a  character string representing a valid network address in the specified address family.  If af does not 
*   contain a valid address family, -1 is returned
*
* NOTE
*
*****************************************************************************/                            
INT32 sAPI_TcpipInet_pton(INT32 af, const void *src, void *dst);

static inline  INT32 sAPI_TcpipInetPton(INT32 af, const void *src, void *dst)
{
    return sAPI_TcpipInet_pton(af,src,dst);
}
/*****************************************************************************
* FUNCTION
*  sAPI_TcpipGetSocketErrno
*
* DESCRIPTION
*    Get the error code of the specified sockfd
* PARAMETERS
*    sockfd Specifies the index of the socket.
Returns
*    -1 - Failure
*    error code
* NOTE
*
*****************************************************************************/   
INT32 sAPI_TcpipGetSocketErrno(int sockfd);

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipHtonl
*
* DESCRIPTION
*    Convert an UINT32 from host to network byte order.
* PARAMETERS
*    hostlong UINT32 in host byte order
Returns
*    in network byte order
* NOTE
*
*****************************************************************************/      
UINT32 sAPI_TcpipHtonl(UINT32 hostlong);

/*****************************************************************************
* FUNCTION
*  sAPI_TcpipGetaddrinfo
*
* DESCRIPTION
*    network address and service translation
* PARAMETERS
*    nodename which identify an Internet host
*    servname which identify an Internet service
*    hints The  hints  argument  points  to  an  addrinfo structure that specifies 
*          criteria for selecting the socket address structures returned in the list pointed to by res.
*    res the socket address structures returned in the list pointed to by res.
Returns
*    0 - OK
*    -1 - Failure
* NOTE
*   If the function call is successful, you need to call sAPI_TcpipFreeaddrinfo to release resources
*****************************************************************************/     
int sAPI_TcpipGetaddrinfo(const char *nodename, const char *servname,const struct SCaddrinfo *hints, struct SCaddrinfo **res);


/*****************************************************************************
* FUNCTION
*  sAPI_TcpipGetaddrinfo_with_pcid
*
* DESCRIPTION
*    The network address and service translation are obtained by specifying the pcid
* PARAMETERS
*    nodename which identify an Internet host
*    servname which identify an Internet service
*    hints The  hints  argument  points  to  an  addrinfo structure that specifies 
*          criteria for selecting the socket address structures returned in the list pointed to by res.
*    res the socket address structures returned in the list pointed to by res.
*    pcid Specifies the index of the pcid.
Returns
*    0 - OK
*    -1 - Failure
* NOTE
*    If the function call is successful, you need to call sAPI_TcpipFreeaddrinfo to release resources
*****************************************************************************/ 
int sAPI_TcpipGetaddrinfo_with_pcid(const char *nodename, const char *servname,
                                    const struct addrinfo *hints, struct addrinfo **res, UINT8 pcid);


/*****************************************************************************
* FUNCTION
*  sAPI_TcpipFreeaddrinfo
*
* DESCRIPTION
*    Free addrinfo storage
* PARAMETERS
*    ai structure to be freed
Returns
*
* NOTE
*
*****************************************************************************/ 
void sAPI_TcpipFreeaddrinfo(struct SCaddrinfo *ai);


/*****************************************************************************
* FUNCTION
*  sAPI_TcpipSocket_with_callback
*
* DESCRIPTION
*  Allocates a socket.
*  The parameters specify what type of socket is requested. Since this socket API implementation is concerned only with network sockets,
*  these are the only socket type that is supported. Also, TCP (SOCK STREAM) sockets can be used.
*
* PARAMETERS
*   domain	  Specifies the protocol family of the created socket.
*   type      socket protocol types.
*   protocol  Type of protocol.
*   callback  A callback is triggered when the state of this socket changes
* RETURNS
*  Upon  successful  completion, sAPI_TcpipSocket_with_callback() shall return a non-negative integer, the socket file descriptor.
*  -1 - Failure
*
* NOTE
*  Do not call blocking interfaces in callbacks
*****************************************************************************/
int sAPI_TcpipSocket_with_callback(int domain, int type, int protocol,socket_callback callback);

#endif