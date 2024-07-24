/**
 * @file csocket.h
 * @author greatboxsS (greatboxS@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef CSOCKET_H
#define CSOCKET_H

#include "osal/osal.h"
#include "osac/cmutex.h"

namespace ipc::core {
#define SOCKET_IP_ADDR_ANY           "0.0.0.0"
#define MULTICAST_ON_SUBNET_IP_GROUP "224.0.0.1"

class __dll_declspec__ csocket {
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
    int32_t m_sockettype;
    int32_t m_mode;
    int32_t m_s32IsAcceptedSocket;
    int32_t m_s32IsOpen;
    int32_t m_s32IsConnected;
    SOCKET_T m_stSk;
    SOCKADDR_T m_stRemoteAddr;
    cmutex *m_poSocketSync;

    explicit csocket(SOCKET_T &socket);

public:
    explicit csocket(int32_t sockettype, int32_t mode);
    virtual ~csocket();

    /**
     * @fn open
     * @brief open a socket
     *
     * @param sockettype	TCP/UDP type
     * @param mode 			Client/Server mode
     * @return int 			0 if success, otherwise -1
     */
    int open();

    /**
     * @fn close
     * @brief close this socket
     *
     * @return int 			0 if success, otherwise -1
     */
    int close();

    /**
     * @fn is_open
     * @brief
     *
     * @return int
     */
    int is_open();

    /**
     * @fn is_connected
     * @brief
     *
     * @return int
     */
    int is_connected();

    /**
     * @fn connect
     * @brief connect to remote host, using for connection socket
     *
     * @param remoteip 		Remote host ip
     * @param remoteport 	Remote host port
     * @return int 			0 if success, otherwise -1
     */
    int connect(const char *remoteip, uint16_t remoteport);

    /**
     * @fn disconnect
     * @brief Disconect from host
     *
     * @return int 			0 if success, otherwise -1
     */
    int disconnect();

    /**
     * @fn bind
     * @brief bind this soket with ip and port
     *
     * @param ip 			bind ip
     * @param port 			bind port
     * @return int 			0 if success, otherwise -1
     */
    int bind(const char *ip, uint16_t port);

    int bind(const char *path);

    /**
     * @fn listen
     * @brief listen on this socket, using for TCP server mode
     *
     * @param connection 	Maxinum of incoming connections to be established by this socket
     * @return int 			0 if success, otherwise -1
     */
    int listen(uint32_t connection);

    /**
     * @fn accept
     * @brief Get connected socket of this TCP server socket
     *
     * @return csocket* New remote socket, user must destroys it.
     */
    csocket *accept();

    /**
     * @fn send
     * @brief send to this socket, used for connection or connectionless
     *
     * @param buff 			Pointer to buffer
     * @param size 			Buffer size
     * @return int 			0 if success, otherwise -1
     */
    int send(const char *buff, size_t size);

    /**
     * @fn receive
     * @brief receive data within socket
     *
     * @param buff 			Pointer to buffer
     * @param size 			Buffer size
     * @return int 			0 if success, otherwise -1
     */
    int receive(char *buff, size_t size);

    /**
     * @fn send_to
     * @brief send data to remote host
     *
     * @param ip 			Remote Ip address
     * @param port 			Remote port
     * @param buff 			Pointer to buffer
     * @param size 			Buffer size
     * @return int 			0 if success, otherwise -1
     */
    int send_to(const char *ip, uint16_t port, const char *buff, size_t size);

    /**
     * @fn receive_from
     * @brief receive data from specific host
     *
     * @param buff 			Pointer to buffer
     * @param size 			Buffer size
     * @param addr 			Target host address that this socket is received from
     * @return int 			0 if success, otherwise -1
     */
    int receive_from(char *buff, size_t size, SOCKADDR_T &addr);

    /**
     * @fn send_multicast
     * @brief send a socket UDP multicast
     *
     * @param groupip 		Multicast group Ip
     * @param port 			Groupt port
     * @param buff 			Pointer to buffer
     * @param size 			Buffer size
     * @return int 			0 if success, otherwise -1
     */
    int send_multicast(const char *groupip, uint16_t port, const char *buff, size_t size);

    /**
     * @fn set_recv_buff_size
     * @brief Set the socket receive buffer size
     *
     * @param size 			Recv Buffer size
     * @return int 			0 if success, otherwise -1
     */
    int set_recv_buff_size(uint32_t size);

    /**
     * @fn set_send_buff_size
     * @brief Set the socket send buffer size
     *
     * @param size 			send buffer size
     * @return int 			0 if success, otherwise -1
     */
    int set_send_buff_size(uint32_t size);

    /**
     * @fn set_blocking_mode
     * @brief Set the socket block mode
     *
     * @param mode 			Set socket block mode (1 - block(default) and 0 - non-block)
     * @return int 			0 if success, otherwise -1
     */
    int set_blocking_mode(int mode);

    /**
     * @fn set_recv_timeout
     * @brief Set socket receive timeout in ms
     *
     * @param ms 			Timeout
     * @return int 			0 if success, otherwise -1
     */
    int set_recv_timeout(uint32_t ms);

    /**
     * @fn set_send_timeout
     * @brief Set socket send timeout in ms
     *
     * @param ms 			Timeout
     * @return int 			0 if success, otherwise -1
     */
    int set_send_timeout(uint32_t ms);

    /**
     * @fn set_option
     * @brief Set option to this socket
     *
     * @param opt 			Option name
     * @param level			Option level (linux SOL_SOCKET/ IPPROTO_IP)
     * @param optvalue 		Pointer to option buffer
     * @param optsize 		Option size
     * @return int 			0 if success, otherwise -1
     */
    int set_option(int32_t opt, int32_t level, void *optvalue, size_t optsize);

    /**
     * @fn get_option
     * @brief Get option to this socket
     *
     * @param opt 			Option name
     * @param level			Option level (linux SOL_SOCKET/ IPPROTO_IP)
     * @param optbuff 		Pointer to option buffer
     * @param optsize 		Option size
     * @return int 			0 if success, otherwise -1
     */
    int get_option(int32_t opt, int32_t level, void *optbuff, size_t optsize);

    /**
     * @fn get_addr_str
     * @brief Get the Address string
     *
     * @return const char*
     */
    const char *get_addr_str();

    /**
     * @fn get_socket_addr
     * @brief Get the Socket address
     *
     * @return SOCKADDR_T
     */
    SOCKADDR_T get_socket_addr();

    /**
     * @fn get_error
     * @brief Get the error value
     *
     * @return int
     */
    int get_error() { return m_stSk.s32Error; }

    /**
     * @fn ipv4_to_string
     * @brief
     *
     * @param addr
     * @return const char*
     */
    static const char *ipv4_to_string(SOCKADDR_T &addr);

    /**
     * @fn ipv6_to_string
     * @brief
     *
     * @param addr
     * @return const char*
     */
    static const char *ipv6_to_string(SOCKADDR_T &addr);
};
} // namespace ipc::core
#endif // CSOCKET_H