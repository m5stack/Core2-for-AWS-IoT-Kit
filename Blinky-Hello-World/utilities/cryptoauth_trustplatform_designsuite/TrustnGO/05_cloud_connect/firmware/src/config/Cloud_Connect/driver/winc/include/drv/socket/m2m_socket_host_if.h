/*******************************************************************************
  BSD compatible socket interface internal types.

  File Name:
    m2m_socket_host_if.h

  Summary:

  Description:

*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

#ifndef __M2M_SOCKET_HOST_IF_H__
#define __M2M_SOCKET_HOST_IF_H__


#ifdef  __cplusplus
extern "C" {
#endif

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
INCLUDES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#include "socket.h"

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
MACROS
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#define SSL_MAX_OPT_LEN                     HOSTNAME_MAX_SIZE


#define SOCKET_CMD_INVALID                  0x00
/*!<
	Invalid Socket command value.
*/


#define SOCKET_CMD_BIND                     0x41
/*!<
    Socket Binding command value.
*/


#define SOCKET_CMD_LISTEN                   0x42
/*!<
    Socket Listening command value.
*/


#define SOCKET_CMD_ACCEPT                   0x43
/*!<
    Socket Accepting command value.
*/


#define SOCKET_CMD_CONNECT                  0x44
/*!<
    Socket Connecting command value.
*/


#define SOCKET_CMD_SEND                     0x45
/*!<
    Socket send command value.
*/


#define SOCKET_CMD_RECV                     0x46
/*!<
	Socket Receive command value.
*/


#define SOCKET_CMD_SENDTO                   0x47
/*!<
    Socket sendTo command value.
*/


#define SOCKET_CMD_RECVFROM                 0x48
/*!<
	Socket ReceiveFrom command value.
*/


#define SOCKET_CMD_CLOSE                    0x49
/*!<
    Socket Close command value.
*/


#define SOCKET_CMD_DNS_RESOLVE              0x4A
/*!<
    Socket DNS Resolve command value.
*/


#define SOCKET_CMD_SSL_CONNECT              0x4B
/*!<
    SSL-Socket Connect command value.
*/


#define SOCKET_CMD_SSL_SEND                 0x4C
/*!<
    SSL-Socket Send command value.
*/


#define SOCKET_CMD_SSL_RECV                 0x4D
/*!<
	SSL-Socket Receive command value.
*/


#define SOCKET_CMD_SSL_CLOSE                0x4E
/*!<
    SSL-Socket Close command value.
*/


#define SOCKET_CMD_SET_SOCKET_OPTION        0x4F
/*!<
    Set Socket Option command value.
*/


#define SOCKET_CMD_SSL_CREATE               0x50
/*!<
*/


#define SOCKET_CMD_SSL_SET_SOCK_OPT         0x51
/*!<
*/


#define SOCKET_CMD_PING                     0x52
/*!<
*/


#define SOCKET_CMD_SSL_SET_CS_LIST          0x53
/*!<
    Recommend instead using @ref M2M_SSL_REQ_SET_CS_LIST and
    associated response @ref M2M_SSL_RESP_SET_CS_LIST
*/


#define SOCKET_CMD_SSL_BIND                 0x54
/*!<
*/


#define SOCKET_CMD_SSL_EXP_CHECK            0x55
/*!<
*/



#define PING_ERR_SUCCESS                    0
#define PING_ERR_DEST_UNREACH               1
#define PING_ERR_TIMEOUT                    2

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
DATA TYPES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/


/*!
*  @brief
*/
typedef struct{
    uint16_t        u16Family;
    uint16_t        u16Port;
    uint32_t        u32IPAddr;
}tstrSockAddr;


typedef int8_t          SOCKET;
typedef tstrSockAddr    tstrUIPSockAddr;



/*!
@struct \
    tstrDnsReply

@brief
    DNS Reply, contains hostName and HostIP.
*/
typedef struct{
    char        acHostName[HOSTNAME_MAX_SIZE];
    uint32_t    u32HostIP;
}tstrDnsReply;


/*!
@brief
*/
typedef struct{
    tstrSockAddr    strAddr;
    SOCKET          sock;
    uint8_t         u8Void;
    uint16_t        u16SessionID;
}tstrBindCmd;


/*!
@brief
*/
typedef struct{
    SOCKET      sock;
    int8_t      s8Status;
    uint16_t    u16SessionID;
}tstrBindReply;


/*!
*  @brief
*/
typedef struct{
    SOCKET      sock;
    uint8_t     u8BackLog;
    uint16_t    u16SessionID;
}tstrListenCmd;


/*!
@struct \
    tstrSocketRecvMsg

@brief  Socket recv status.

    It is passed to the APPSocketEventHandler with SOCKET_MSG_RECV or SOCKET_MSG_RECVFROM message type
    in a response to a user call to the recv or recvfrom.
    If the received data from the remote peer is larger than the USER Buffer size (given at recv call), the data is
    delivered to the user in a number of consecutive chunks according to the USER Buffer size.
*/
typedef struct{
    SOCKET      sock;
    int8_t      s8Status;
    uint16_t    u16SessionID;
}tstrListenReply;


/*!
*  @brief
*/
typedef struct{
    tstrSockAddr    strAddr;
    SOCKET          sListenSock;
    SOCKET          sConnectedSock;
    uint16_t        u16AppDataOffset;
    /*!<
        In further packet send requests the host interface should put the user application
        data at this offset in the allocated shared data packet.
    */
}tstrAcceptReply;


/*!
*  @brief
*/
typedef struct{
    tstrSockAddr    strAddr;
    SOCKET          sock;
    uint8_t         u8SslFlags;
    uint16_t        u16SessionID;
}tstrConnectCmd;


/*!
@struct \
    tstrConnectReply

@brief
    Connect Reply, contains sock number and error value
*/
typedef struct{
    SOCKET      sock;
    int8_t      s8Error;
    uint16_t    u16AppDataOffset;
    /*!<
        In further packet send requests the host interface should put the user application
        data at this offset in the allocated shared data packet.
    */
}tstrConnectReply;


/*!
@brief
*/
typedef struct{
    SOCKET          sock;
    uint8_t         u8Void;
    uint16_t        u16DataSize;
    tstrSockAddr    strAddr;
    uint16_t        u16SessionID;
    uint16_t        u16Void;
}tstrSendCmd;


/*!
@struct \
    tstrSendReply

@brief
    Send Reply, contains socket number and number of sent bytes.
*/
typedef struct{
    SOCKET      sock;
    uint8_t     u8Void;
    int16_t     s16SentBytes;
    uint16_t    u16SessionID;
    uint16_t    u16Void;
}tstrSendReply;


/*!
*  @brief
*/
typedef struct{
    uint32_t    u32Timeoutmsec;
    SOCKET      sock;
    uint8_t     u8Void;
    uint16_t    u16SessionID;
}tstrRecvCmd;


/*!
@struct
@brief
*/
typedef struct{
    tstrSockAddr    strRemoteAddr;
    int16_t         s16RecvStatus;
    uint16_t        u16DataOffset;
    SOCKET          sock;
    uint8_t         u8Void;
    uint16_t        u16SessionID;
}tstrRecvReply;


/*!
*  @brief
*/
typedef struct{
    uint32_t    u32OptionValue;
    SOCKET      sock;
    uint8_t     u8Option;
    uint16_t    u16SessionID;
}tstrSetSocketOptCmd;


typedef struct{
    SOCKET      sslSock;
    uint8_t     __PAD24__[3];
}tstrSSLSocketCreateCmd;


/*!
*  @brief
*/
typedef struct{
    SOCKET      sock;
    uint8_t     u8Option;
    uint16_t    u16SessionID;
    uint32_t    u32OptLen;
    uint8_t     au8OptVal[SSL_MAX_OPT_LEN];
}tstrSSLSetSockOptCmd;


/*!
*/
typedef struct{
    uint32_t    u32DestIPAddr;
    uint32_t    u32CmdPrivate;
    uint16_t    u16PingCount;
    uint8_t     u8TTL;
    uint8_t     __PAD8__;
}tstrPingCmd;


typedef struct{
    uint32_t    u32IPAddr;
    uint32_t    u32CmdPrivate;
    uint32_t    u32RTT;
    uint16_t    u16Success;
    uint16_t    u16Fail;
    uint8_t     u8ErrorCode;
    uint8_t     __PAD24__[3];
}tstrPingReply;

/*!
@struct\
    tstrSslCertExpSettings

@brief  SSL Certificate Expiry Validation Settings

@sa     tenuSslCertExpSettings
*/
typedef struct{
    uint32_t    u32CertExpValidationOpt;
    /*!<
        See @tenuSslCertExpSettings for possible values.
    */
}tstrSslCertExpSettings;


#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __M2M_SOCKET_HOST_IF_H__ */
