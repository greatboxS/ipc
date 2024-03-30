#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "OSAL.h"
namespace gbs {
namespace osal {
__DLL_DECLSPEC__ SOCKET_T SOCKET_Create(int32_t sockettype, int32_t socketmode, int blockmode = SOCKET_BLOCKING_MODE, int addrfamily = SOCKET_ADDR_V4);
__DLL_DECLSPEC__ int SOCKET_Close(SOCKET_T &sk);

__DLL_DECLSPEC__ int SOCKET_Connect(SOCKET_T &sk, const char *remoteip, uint16_t remoteport);
__DLL_DECLSPEC__ int SOCKET_Disconnect(SOCKET_T &sk);

__DLL_DECLSPEC__ int SOCKET_Listen(SOCKET_T &sk, uint32_t connection);
__DLL_DECLSPEC__ int SOCKET_Bind(SOCKET_T &sk, uint16_t port);
__DLL_DECLSPEC__ SOCKET_T SOCKET_Accept(SOCKET_T &sk);

__DLL_DECLSPEC__ int SOCKET_Send(SOCKET_T &sk, const char *buff, size_t size);
__DLL_DECLSPEC__ int SOCKET_Recv(SOCKET_T &sk, char *buff, size_t size);

__DLL_DECLSPEC__ int SOCKET_SendTo(SOCKET_T &sk, SOCKADDR_T &sendaddr, const char *buff, size_t size);
__DLL_DECLSPEC__ int SOCKET_RecvFrom(SOCKET_T &sk, SOCKADDR_T &recvaddr, char *buff, size_t size);

__DLL_DECLSPEC__ int SOCKET_SendMulticast(SOCKET_T &sk, const char *groupip, uint16_t port, const char *buff, size_t size);

__DLL_DECLSPEC__ int SOCKET_SetRecvBuffer(SOCKET_T &sk, uint32_t size);
__DLL_DECLSPEC__ int SOCKET_SetSendBuffer(SOCKET_T &sk, uint32_t size);

__DLL_DECLSPEC__ int SOCKET_SetRecvTimeout(SOCKET_T &sk, uint32_t ms);
__DLL_DECLSPEC__ int SOCKET_SetSendTimeout(SOCKET_T &sk, uint32_t ms);

__DLL_DECLSPEC__ int SOCKET_SetBlockingMode(SOCKET_T &sk, int32_t mode);

__DLL_DECLSPEC__ int SOCKET_SetOption(SOCKET_T &sk, SOCKET_OPT &opt);
__DLL_DECLSPEC__ int SOCKET_GetOption(SOCKET_T &sk, SOCKET_OPT &opt);

__DLL_DECLSPEC__ SOCKADDR_T SOCKET_GetAddressV4(const char *ip, uint16_t port);
__DLL_DECLSPEC__ SOCKADDR_T SOCKET_GetAddressV6(const char *ip, uint16_t port);

__DLL_DECLSPEC__ const char *SOCKET_Ipv4ToString(SOCKADDR_T &addr);
__DLL_DECLSPEC__ const char *SOCKET_Ipv6ToString(SOCKADDR_T &addr);

__DLL_DECLSPEC__ SOCKADDR_T SOCKET_GetSocketName(SOCKET_T &sk);
}; // namespace osal
} // namespace gbs
#endif // __SOCKET_H__