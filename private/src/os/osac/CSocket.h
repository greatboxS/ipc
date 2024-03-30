/**
 * @file CSocket.h
 * @author greatboxsS (greatboxS@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef __CSOCKET_H__
#define __CSOCKET_H__

#include "common/Typedef.h"
#include "osal/OSAL.h"
#include "osac/CMutex.h"

namespace gbs
{
    namespace osac
    {
#define SOCKET_IP_ADDR_ANY           "0.0.0.0"
#define MULTICAST_ON_SUBNET_IP_GROUP "224.0.0.1"

        class __DLL_DECLSPEC__ CSocket
        {
        public:
            enum class Type : int32_t {
                SocketHost = 0,
                SocketTcp,
                SocketUdp,
            };

            enum class Mode : int32_t {
                Client = 0,
                Server,
            };

        protected:
            int32_t m_s32IsAcceptedSocket;
            int32_t m_s32IsOpen;
            int32_t m_s32IsConnected;
            SOCKET_T m_stSk;
            SOCKADDR_T m_stRemoteAddr;
            CMutex *m_poSocketSync;

            explicit CSocket(SOCKET_T &socket);

        public:
            explicit CSocket();
            virtual ~CSocket();

            /**
             * @fn Open
             * @brief Open a socket
             *
             * @param sockettype	TCP/UDP type
             * @param mode 			Client/Server mode
             * @return int 			0 if success, otherwise -1
             */
            int Open(int32_t sockettype, int32_t mode);

            /**
             * @fn Close
             * @brief Close this socket
             *
             * @return int 			0 if success, otherwise -1
             */
            int Close();

            /**
             * @fn IsOpen
             * @brief
             *
             * @return int
             */
            int IsOpen();

            /**
             * @fn IsConnected
             * @brief
             *
             * @return int
             */
            int IsConnected();

            /**
             * @fn Connect
             * @brief Connect to remote host, using for connection socket
             *
             * @param remoteip 		Remote host ip
             * @param remoteport 	Remote host port
             * @return int 			0 if success, otherwise -1
             */
            int Connect(const char *remoteip, uint16_t remoteport);

            /**
             * @fn Disconnect
             * @brief Disconect from host
             *
             * @return int 			0 if success, otherwise -1
             */
            int Disconnect();

            /**
             * @fn Bind
             * @brief Bind this soket with ip and port
             *
             * @param ip 			Bind ip
             * @param port 			Bind port
             * @return int 			0 if success, otherwise -1
             */
            int Bind(const char *ip, uint16_t port);

            /**
             * @fn Listen
             * @brief Listen on this socket, using for TCP server mode
             *
             * @param connection 	Maxinum of incoming connections to be established by this socket
             * @return int 			0 if success, otherwise -1
             */
            int Listen(uint32_t connection);

            /**
             * @fn Accept
             * @brief Get connected socket of this TCP server socket
             *
             * @return CSocket* New remote socket, user must destroys it.
             */
            CSocket *Accept();

            /**
             * @fn Send
             * @brief Send to this socket, used for connection or connectionless
             *
             * @param buff 			Pointer to buffer
             * @param size 			Buffer size
             * @return int 			0 if success, otherwise -1
             */
            int Send(const char *buff, size_t size);

            /**
             * @fn Receive
             * @brief Receive data within socket
             *
             * @param buff 			Pointer to buffer
             * @param size 			Buffer size
             * @return int 			0 if success, otherwise -1
             */
            int Receive(char *buff, size_t size);

            /**
             * @fn SendTo
             * @brief Send data to remote host
             *
             * @param ip 			Remote Ip address
             * @param port 			Remote port
             * @param buff 			Pointer to buffer
             * @param size 			Buffer size
             * @return int 			0 if success, otherwise -1
             */
            int SendTo(const char *ip, uint16_t port, const char *buff, size_t size);

            /**
             * @fn ReceiveFrom
             * @brief Receive data from specific host
             *
             * @param buff 			Pointer to buffer
             * @param size 			Buffer size
             * @param addr 			Target host address that this socket is received from
             * @return int 			0 if success, otherwise -1
             */
            int ReceiveFrom(char *buff, size_t size, SOCKADDR_T &addr);

            /**
             * @fn SendMulticast
             * @brief Send a socket UDP multicast
             *
             * @param groupip 		Multicast group Ip
             * @param port 			Groupt port
             * @param buff 			Pointer to buffer
             * @param size 			Buffer size
             * @return int 			0 if success, otherwise -1
             */
            int SendMulticast(const char *groupip, uint16_t port, const char *buff, size_t size);

            /**
             * @fn SetRecvBufferSize
             * @brief Set the socket receive buffer size
             *
             * @param size 			Recv Buffer size
             * @return int 			0 if success, otherwise -1
             */
            int SetRecvBufferSize(uint32_t size);

            /**
             * @fn SetSendBufferSize
             * @brief Set the socket send buffer size
             *
             * @param size 			Send buffer size
             * @return int 			0 if success, otherwise -1
             */
            int SetSendBufferSize(uint32_t size);

            /**
             * @fn SetBlockingMode
             * @brief Set the socket block mode
             *
             * @param mode 			Set socket block mode (1 - block(default) and 0 - non-block)
             * @return int 			0 if success, otherwise -1
             */
            int SetBlockingMode(int mode);

            /**
             * @fn SetRecvTimeout
             * @brief Set socket receive timeout in ms
             *
             * @param ms 			Timeout
             * @return int 			0 if success, otherwise -1
             */
            int SetRecvTimeout(uint32_t ms);

            /**
             * @fn SetSendTimeout
             * @brief Set socket send timeout in ms
             *
             * @param ms 			Timeout
             * @return int 			0 if success, otherwise -1
             */
            int SetSendTimeout(uint32_t ms);

            /**
             * @fn SetOption
             * @brief Set option to this socket
             *
             * @param opt 			Option name
             * @param level			Option level (linux SOL_SOCKET/ IPPROTO_IP)
             * @param optvalue 		Pointer to option buffer
             * @param optsize 		Option size
             * @return int 			0 if success, otherwise -1
             */
            int SetOption(int32_t opt, int32_t level, void *optvalue, size_t optsize);

            /**
             * @fn GetOption
             * @brief Get option to this socket
             *
             * @param opt 			Option name
             * @param level			Option level (linux SOL_SOCKET/ IPPROTO_IP)
             * @param optbuff 		Pointer to option buffer
             * @param optsize 		Option size
             * @return int 			0 if success, otherwise -1
             */
            int GetOption(int32_t opt, int32_t level, void *optbuff, size_t optsize);

            /**
             * @fn GetAddress
             * @brief Get the Address string
             *
             * @return const char*
             */
            const char *GetAddress();

            /**
             * @fn GetSocketAddr
             * @brief Get the Socket address
             *
             * @return SOCKADDR_T
             */
            SOCKADDR_T GetSocketAddr();

            /**
             * @fn GetError
             * @brief Get the error value
             *
             * @return int
             */
            int GetError() { return m_stSk.s32Error; }

            /**
             * @fn Ipv4ToString
             * @brief
             *
             * @param addr
             * @return const char*
             */
            static const char *Ipv4ToString(SOCKADDR_T &addr);

            /**
             * @fn Ipv6ToString
             * @brief
             *
             * @param addr
             * @return const char*
             */
            static const char *Ipv6ToString(SOCKADDR_T &addr);
        };
    }; // namespace osac
} // namespace gbs
#endif // __CSOCKET_H__