#include "csocket.h"
#include "osac.h"
#include "osal/ipc_mutex.h"
#include "osal/ipc_socket.h"

namespace ipc::core {
csocket::csocket(int32_t sockettype, int32_t mode) :
    m_sockettype(sockettype),
    m_mode(mode),
    m_stSk(),
    m_s32IsOpen(0),
    m_s32IsConnected(0),
    m_s32IsAcceptedSocket(0),
    m_stRemoteAddr(),
    m_poSocketSync(NULL) {
    m_poSocketSync = new cmutex();
    m_poSocketSync->create();
}

csocket::csocket(SOCKET_T &socket) :
    m_sockettype(-1),
    m_mode(-1),
    m_stSk(),
    m_s32IsOpen(0),
    m_s32IsConnected(0),
    m_s32IsAcceptedSocket(0),
    m_stRemoteAddr(),
    m_poSocketSync(NULL) {
    m_stSk = socket;
    m_poSocketSync = new cmutex();
    m_poSocketSync->create();
    m_s32IsAcceptedSocket = 1;
    m_s32IsConnected = 1;
    m_s32IsOpen = m_stSk.skHandle >= 0 ? 1 : 0;
}

csocket::~csocket() {
    if (is_open()) {
        close();
    }
    delete m_poSocketSync;
}

/**
 * @fn open
 * @brief open a socket
 *
 * @param sockettype	TCP/UDP type
 * @param mode 			Client/Server mode
 * @return int 			0 if success, otherwise -1
 */
int csocket::open() {
    if (m_s32IsAcceptedSocket) {
        OSAC_ERR("Can not open accepted socket\n");
        return -1;
    }
    if (is_open()) {
        OSAC_ERR("Socket is openning\n");
        return 0;
    }
    m_stSk = socket_create(m_sockettype);
    m_s32IsOpen = m_stSk.skHandle >= 0 ? 1 : 0;
    return m_s32IsOpen ? 0 : -1;
}

/**
 * @fn close
 * @brief close this socket
 *
 * @return int 			0 if success, otherwise -1
 */
int csocket::close() {
    if (is_open()) {
        m_poSocketSync->lock();
        m_s32IsOpen = 0;
        m_poSocketSync->unlock();
    }
    int ret = socket_close(m_stSk);
    return ret;
}

/**
 * @fn is_open
 * @brief
 *
 * @return int
 */
int csocket::is_open() {
    return m_s32IsOpen;
}

/**
 * @fn is_connected
 * @brief
 *
 * @return int
 */
int csocket::is_connected() {
    return m_s32IsConnected;
}

/**
 * @fn connect
 * @brief connect to remote host, using for connection socket
 *
 * @param remoteip 		Remote host ip
 * @param remoteport 	Remote host port
 * @return int 			0 if success, otherwise -1
 */
int csocket::connect(const char *remoteip, uint16_t remoteport) {
    m_poSocketSync->lock();
    int ret = socket_connect(m_stSk, remoteip, remoteport);
    m_s32IsConnected = (ret == 0 ? 1 : 0);
    m_poSocketSync->unlock();
    return ret;
}

/**
 * @fn disconnect
 * @brief Disconect from host
 *
 * @return int 			0 if success, otherwise -1
 */
int csocket::disconnect() {
    m_poSocketSync->lock();
    m_s32IsConnected = 0;
    int ret = socket_disconnect(m_stSk);
    m_poSocketSync->unlock();
    return ret;
}

/**
 * @fn bind
 * @brief bind this soket with ip and port
 *
 * @param ip 			bind ip
 * @param port 			bind port
 * @return int 			0 if success, otherwise -1
 */
int csocket::bind(const char *ip, uint16_t port) {
    m_stSk.stAddrInet.Ip = socket_get_addr_v4(ip, port);
    return socket_bind(m_stSk, port);
}

int csocket::bind(const char *path) {
    return socket_bind(m_stSk, 0, path);
}

/**
 * @fn listen
 * @brief listen on this socket, using for TCP server mode
 *
 * @param connection 	Maxinum of incoming connections to be established by this socket
 * @return int 			0 if success, otherwise -1
 */
int csocket::listen(uint32_t connection) { return socket_listen(m_stSk, connection); }

/**
 * @fn accept
 * @brief Get connected socket of this TCP server socket
 *
 * @return csocket* New remote socket, user must destroys it.
 */
csocket *csocket::accept() {
    csocket *poAcceptSk = NULL;
    m_poSocketSync->lock();
    auto sk = socket_accept(m_stSk);
    if (sk.skHandle > 0) {
        poAcceptSk = new csocket(sk);
    }
    m_poSocketSync->unlock();
    return poAcceptSk;
}

/**
 * @fn send
 * @brief send to this socket, used for connection or connectionless
 *
 * @param buff 			Pointer to buffer
 * @param size 			Buffer size
 * @return int 			0 if success, otherwise -1
 */
int csocket::send(const char *buff, size_t size) {
    int ret = -1;
    if (!is_connected()) {
        return ret;
    }
    m_poSocketSync->lock();
    ret = socket_send(m_stSk, buff, size);
    m_poSocketSync->unlock();
    return ret;
}

/**
 * @fn receive
 * @brief receive data within socket
 *
 * @param buff 			Pointer to buffer
 * @param size 			Buffer size
 * @return int 			0 if success, otherwise -1
 */
int csocket::receive(char *buff, size_t size) {
    int ret = -1;
    if (!is_connected()) {
        return ret;
    }
    m_poSocketSync->lock();
    ret = socket_recv(m_stSk, buff, size);
    m_poSocketSync->unlock();
    return ret;
}

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
int csocket::send_to(const char *ip, uint16_t port, const char *buff, size_t size) {
    int ret = 0;
    SOCKADDR_T stAddr = socket_get_addr_v4(ip, port);
    m_poSocketSync->lock();
    ret = socket_send_to(m_stSk, stAddr, buff, size);
    m_poSocketSync->unlock();
    return ret;
}

/**
 * @fn receive_from
 * @brief receive data from specific host
 *
 * @param buff 			Pointer to buffer
 * @param size 			Buffer size
 * @param addr 			Target host address that this socket is received from
 * @return int 			0 if success, otherwise -1
 */
int csocket::receive_from(char *buff, size_t size, SOCKADDR_T &addr) {
    int ret = 0;
    m_poSocketSync->lock();
    ret = socket_recv_from(m_stSk, addr, buff, size);
    m_poSocketSync->unlock();
    return ret;
}

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
int csocket::send_multicast(const char *groupip, uint16_t port, const char *buff, size_t size) {
    int ret = 0;
    m_poSocketSync->lock();
    ret = socket_send_multicast(m_stSk, groupip, port, buff, size);
    m_poSocketSync->unlock();
    return ret;
}

/**
 * @fn set_recv_buff_size
 * @brief Set the socket receive buffer size
 *
 * @param size 			Recv Buffer size
 * @return int 			0 if success, otherwise -1
 */
int csocket::set_recv_buff_size(uint32_t size) {
    int ret = 0;
    m_poSocketSync->lock();
    ret = socket_set_recv_buff(m_stSk, size);
    m_poSocketSync->unlock();
    return ret;
}

/**
 * @fn set_send_buff_size
 * @brief Set the socket send buffer size
 *
 * @param size 			send buffer size
 * @return int 			0 if success, otherwise -1
 */
int csocket::set_send_buff_size(uint32_t size) {
    int ret = 0;
    m_poSocketSync->lock();
    ret = socket_set_send_buff(m_stSk, size);
    m_poSocketSync->unlock();
    return ret;
}

/**
 * @fn set_blocking_mode
 * @brief Set the socket block mode
 *
 * @param mode 			Set socket block mode (1 - block(default) and 0 - non-block)
 * @return int 			0 if success, otherwise -1
 */
int csocket::set_blocking_mode(int mode) {
    int ret = 0;
    m_poSocketSync->lock();
    ret = socket_set_blocking_mode(m_stSk, mode);
    m_poSocketSync->unlock();
    return ret;
}

/**
 * @fn set_recv_timeout
 * @brief Set socket receive timeout in ms
 *
 * @param ms 			Timeout
 * @return int 			0 if success, otherwise -1
 */
int csocket::set_recv_timeout(uint32_t ms) {
    int ret = 0;
    m_poSocketSync->lock();
    ret = socket_set_recv_timeout(m_stSk, ms);
    m_poSocketSync->unlock();
    return ret;
}

/**
 * @fn set_send_timeout
 * @brief Set socket send timeout in ms
 *
 * @param ms 			Timeout
 * @return int 			0 if success, otherwise -1
 */
int csocket::set_send_timeout(uint32_t ms) {
    int ret = 0;
    m_poSocketSync->lock();
    ret = socket_set_send_timeout(m_stSk, ms);
    m_poSocketSync->unlock();
    return ret;
}

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
int csocket::set_option(int32_t opt, int32_t level, void *optvalue, size_t optsize) {
    int ret = 0;
    SOCKET_OPT stOpt = {level, opt, optvalue, optsize};
    m_poSocketSync->lock();
    ret = socket_set_option(m_stSk, stOpt);
    m_poSocketSync->unlock();
    return ret;
}

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
int csocket::get_option(int32_t opt, int32_t level, void *optbuff, size_t optsize) {
    int ret = 0;
    SOCKET_OPT stOpt = {level, opt, optbuff, optsize};
    m_poSocketSync->lock();
    ret = socket_get_option(m_stSk, stOpt);
    m_poSocketSync->unlock();
    return ret;
}

/**
 * @fn get_addr_str
 * @brief Get the Address string
 *
 * @return const char*
 */
const char *csocket::get_addr_str() {
    if (m_stSk.stAddrInet.s32AddrFamily == SOCKET_ADDR_V4) {
        return socket_ip_v4_to_string(m_stSk.stAddrInet.Ip);
    } else {
        return socket_ip_v6_to_string(m_stSk.stAddrInet.Ip);
    }
}

/**
 * @fn get_socket_addr
 * @brief Get the Socket address
 *
 * @return SOCKADDR_T
 */
SOCKADDR_T csocket::get_socket_addr() {
    return socket_get_name(m_stSk);
}

/**
 * @fn ipv4_to_string
 * @brief
 *
 * @param addr
 * @return const char*
 */
const char *csocket::ipv4_to_string(SOCKADDR_T &addr) { return socket_ip_v4_to_string(addr); }

/**
 * @fn ipv6_to_string
 * @brief
 *
 * @param addr
 * @return const char*
 */
const char *csocket::ipv6_to_string(SOCKADDR_T &addr) { return socket_ip_v6_to_string(addr); }
} // namespace ipc::core