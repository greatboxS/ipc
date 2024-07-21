#ifndef IPC_SOCKET_H
#define IPC_SOCKET_H

#include "osal.h"
namespace ipc::core {
__dll_declspec__ SOCKET_T socket_create(int32_t sockettype, int32_t socketmode, int blockmode = SOCKET_BLOCKING_MODE, int addrfamily = SOCKET_ADDR_V4);
__dll_declspec__ int socket_close(SOCKET_T &sk);

__dll_declspec__ int socket_connect(SOCKET_T &sk, const char *remoteip, uint16_t remoteport);
__dll_declspec__ int socket_disconnect(SOCKET_T &sk);

__dll_declspec__ int socket_listen(SOCKET_T &sk, uint32_t connection);
__dll_declspec__ int socket_bind(SOCKET_T &sk, uint16_t port);
__dll_declspec__ SOCKET_T socket_accept(SOCKET_T &sk);

__dll_declspec__ int socket_send(SOCKET_T &sk, const char *buff, size_t size);
__dll_declspec__ int socket_recv(SOCKET_T &sk, char *buff, size_t size);

__dll_declspec__ int socket_send_to(SOCKET_T &sk, SOCKADDR_T &sendaddr, const char *buff, size_t size);
__dll_declspec__ int socket_recv_from(SOCKET_T &sk, SOCKADDR_T &recvaddr, char *buff, size_t size);

__dll_declspec__ int socket_send_multicast(SOCKET_T &sk, const char *groupip, uint16_t port, const char *buff, size_t size);

__dll_declspec__ int socket_set_recv_buff(SOCKET_T &sk, uint32_t size);
__dll_declspec__ int socket_set_send_buff(SOCKET_T &sk, uint32_t size);

__dll_declspec__ int socket_set_recv_timeout(SOCKET_T &sk, uint32_t ms);
__dll_declspec__ int socket_set_send_timeout(SOCKET_T &sk, uint32_t ms);

__dll_declspec__ int socket_set_blocking_mode(SOCKET_T &sk, int32_t mode);

__dll_declspec__ int socket_set_option(SOCKET_T &sk, SOCKET_OPT &opt);
__dll_declspec__ int socket_get_option(SOCKET_T &sk, SOCKET_OPT &opt);

__dll_declspec__ SOCKADDR_T socket_get_addr_v4(const char *ip, uint16_t port);
__dll_declspec__ SOCKADDR_T socket_get_addr_v6(const char *ip, uint16_t port);

__dll_declspec__ const char *socket_ip_v4_to_string(SOCKADDR_T &addr);
__dll_declspec__ const char *socket_ip_v6_to_string(SOCKADDR_T &addr);

__dll_declspec__ SOCKADDR_T socket_get_name(SOCKET_T &sk);
}
#endif // IPC_SOCKET_H