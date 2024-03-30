#include "osal/Socket.h"
#include "dbg/Debug.h"
#include <arpa/inet.h>

namespace gbs {
namespace osal {
static inline int SOCKET_Valid(SOCKET_T &sk) {
    return (sk.skHandle == -1 ? 0 : 1);
}

SOCKET_T SOCKET_Create(int32_t sockettype, int32_t socketmode, int blockmode, int addrfamily) {
    SOCKET_T stSocket;
    int domain = (addrfamily == SOCKET_ADDR_V4 ? AF_INET : AF_INET6);
    int type = SOCK_DGRAM;

    memset(&stSocket, 0, sizeof(stSocket));
    stSocket.skHandle = -1;

    if (sockettype == eSOCKET_HOST) {
        domain = AF_LOCAL;
    } else if (sockettype == eSOCKET_TCP) {
        type = SOCK_STREAM;
    } else if (sockettype = eSOCKET_UDP) {
        type = SOCK_DGRAM;
    } else {
        LOG_OSAL_ERROR("[%s] Unsupport socket type %d\n", __FUNCTION__, sockettype);
        return stSocket;
    }

    if ((stSocket.skHandle = socket(domain, type, 0)) == INVALID_SOCKET) {
        stSocket.s32Error = __ERROR__;
        LOG_OSAL_ERROR("[%s] Create socket failed, %d\n", __FUNCTION__, stSocket.s32Error);
        return stSocket;
    }

    stSocket.s32SocketMode = socketmode;
    stSocket.s32SocketType = sockettype;
    stSocket.s32BlockMode = blockmode;
    stSocket.stAddrInet.s32AddrFamily = addrfamily;
    stSocket.stAddrInet.u32Size = (addrfamily == SOCKET_ADDR_V4 ? sizeof(SOCKADDR_V4) : sizeof(SOCKADDR_V6));
    return stSocket;
}

int SOCKET_Close(SOCKET_T &sk) {
    if (!SOCKET_Valid(sk)) {
        LOG_OSAL_ERROR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (close(sk.skHandle) < 0) {
        sk.s32Error = __ERROR__;
        LOG_OSAL_ERROR("[%s] Close socket failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

int SOCKET_Connect(SOCKET_T &sk, const char *remoteip, uint16_t remoteport) {
    void *socketaddr = NULL;
    SOCKADDR_V4 v4_addr;
    SOCKADDR_V6 v6_addr;
    socklen_t size = 0;

    if (!remoteip) {
        LOG_OSAL_ERROR("[%s] Remote ip is null\n", __FUNCTION__);
        return RET_ERR;
    }

    if (!SOCKET_Valid(sk)) {
        LOG_OSAL_ERROR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }

    if (sk.s32SocketType != eSOCKET_TCP) {
        LOG_OSAL_WARN("[%s] Can not connect a not streaming socket\n", __FUNCTION__);
        return RET_ERR;
    }

    if (sk.stAddrInet.s32AddrFamily == SOCKET_ADDR_V4) {
        size = sizeof(SOCKADDR_V4);
        v4_addr.sin_family = AF_INET;
        v4_addr.sin_port = htons(remoteport);
        if (inet_pton(AF_INET, remoteip, &v4_addr.sin_addr) <= 0) {
            sk.s32Error = __ERROR__;
            LOG_OSAL_ERROR("[%s] Can not convert remote ip to Ipv4 address\n", __FUNCTION__);
            return RET_ERR;
        }

        socketaddr = &v4_addr;
    } else {
        size = sizeof(SOCKADDR_V6);
        v6_addr.sin6_family = AF_INET6;
        v6_addr.sin6_port = htons(remoteport);
        if (inet_pton(AF_INET6, remoteip, &v6_addr.sin6_addr) <= 0) {
            sk.s32Error = __ERROR__;
            LOG_OSAL_ERROR("[%s] Can not convert remote ip to Ipv6 address\n", __FUNCTION__);
            return RET_ERR;
        }

        socketaddr = &v6_addr;
    }

    if (connect(sk.skHandle, (sockaddr *)socketaddr, size) < 0) {
        sk.s32Error = __ERROR__;
        LOG_OSAL_ERROR("[%s] Socket connect error, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

int SOCKET_Disconnect(SOCKET_T &sk) {
    if (shutdown(sk.skHandle, SHUT_RDWR) < 0) {
        sk.s32Error = __ERROR__;
        LOG_OSAL_ERROR("[%s] Shutdown failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

int SOCKET_Listen(SOCKET_T &sk, uint32_t connection) {
    if (!SOCKET_Valid(sk)) {
        LOG_OSAL_ERROR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (listen(sk.skHandle, connection) < 0) {
        sk.s32Error = __ERROR__;
        LOG_OSAL_ERROR("[%s] Listen failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

int SOCKET_Bind(SOCKET_T &sk, uint16_t port) {
    void *socketaddr = NULL;
    SOCKADDR_V4 v4_addr;
    SOCKADDR_V6 v6_addr;
    socklen_t size = 0;

    memset(&v4_addr, 0, sizeof(v4_addr));
    memset(&v6_addr, 0, sizeof(v6_addr));

    if (!SOCKET_Valid(sk)) {
        LOG_OSAL_ERROR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }

    if (sk.stAddrInet.s32AddrFamily == SOCKET_ADDR_V4) {
        size = sizeof(SOCKADDR_V4);
        v4_addr.sin_family = AF_INET;
        v4_addr.sin_port = htons(port);
        v4_addr.sin_addr.s_addr = sk.stAddrInet.Ip.v4.sin_addr.s_addr;
        socketaddr = &v4_addr;
    } else {
        size = sizeof(SOCKADDR_V6);
        v6_addr.sin6_family = AF_INET6;
        v6_addr.sin6_port = htons(port);
        v6_addr.sin6_addr = sk.stAddrInet.Ip.v6.sin6_addr;
        socketaddr = &v6_addr;
    }

#ifndef DISABLE_REUSE_BINDING_SOCKET
    int reuse = 1;
    SOCKET_OPT stOpt = {SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)};

    /**
     * After the SO_REUSEADDR option is active, the following situation is supported:
     * A server can bind() the same port multiple times as long as every invocation uses a different
     * local IP address and the wildcard address INADDR_ANY is used only one time per port.
     */
    SOCKET_SetOption(sk, stOpt);
#endif
    if (bind(sk.skHandle, (sockaddr *)socketaddr, size) < 0) {
        sk.s32Error = __ERROR__;
        LOG_OSAL_ERROR("[%s] Bind failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }

    return RET_OK;
}

/**
 * @fn SOCKET_Accept
 * @brief
 *
 * @param sk
 * @return SOCKET_T
 */
SOCKET_T SOCKET_Accept(SOCKET_T &sk) {
    SOCKET_T stSocket;

    memset(&stSocket, 0, sizeof(stSocket));
    stSocket.skHandle = -1;

    /* accept socket ip version will be inherited from host socket */
    stSocket.stAddrInet.s32AddrFamily = sk.stAddrInet.s32AddrFamily;
    if (stSocket.stAddrInet.s32AddrFamily == SOCKET_ADDR_V4) {
        stSocket.stAddrInet.u32Size = sizeof(SOCKADDR_V4);
    } else {
        stSocket.stAddrInet.u32Size = sizeof(SOCKADDR_V6);
    }

    if (!SOCKET_Valid(sk)) {
        LOG_OSAL_ERROR("[%s] Invalid socket\n", __FUNCTION__);
        return stSocket;
    }

    if ((stSocket.skHandle = accept(sk.skHandle, (sockaddr *)&stSocket.stAddrInet.Ip,
                                    (socklen_t *)&stSocket.stAddrInet.u32Size))
        < 0) {
        stSocket.s32Error = __ERROR__;
        // LOG_OSAL_ERROR("[%s] Accept failed, %s\n", __FUNCTION__, __ERROR_STR__);
    }
    return stSocket;
}

int SOCKET_Send(SOCKET_T &sk, const char *buff, size_t size) {
    int bytes = 0;

    if (!SOCKET_Valid(sk)) {
        LOG_OSAL_ERROR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (!buff || size == 0) {
        LOG_OSAL_ERROR("[%s] Invalid buffer\n", __FUNCTION__);
        return RET_ERR;
    }

    if ((bytes = send(sk.skHandle, buff, size, 0)) < 0) {
        sk.s32Error = __ERROR__;
        LOG_OSAL_ERROR("[%s] Send failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return bytes;
}

int SOCKET_Recv(SOCKET_T &sk, char *buff, size_t size) {
    int bytes = 0;

    if (!SOCKET_Valid(sk)) {
        LOG_OSAL_ERROR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (!buff || size == 0) {
        LOG_OSAL_ERROR("[%s] Invalid buffer\n", __FUNCTION__);
        return RET_ERR;
    }

    if ((bytes = recv(sk.skHandle, buff, size, 0)) < 0) {
        sk.s32Error = __ERROR__;
        // LOG_OSAL_ERROR("[%s] Receive failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return bytes;
}

int SOCKET_SendTo(SOCKET_T &sk, SOCKADDR_T &sendaddr, const char *buff, size_t size) {
    int bytes = 0;
    socklen_t addrSize = 0;

    if (!SOCKET_Valid(sk)) {
        LOG_OSAL_ERROR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (!buff || size == 0) {
        LOG_OSAL_ERROR("[%s] Invalid buffer\n", __FUNCTION__);
        return RET_ERR;
    }

    if (sk.stAddrInet.s32AddrFamily == SOCKET_ADDR_V4) {
        addrSize = sizeof(SOCKADDR_V4);
    } else {
        addrSize = sizeof(SOCKADDR_V6);
    }

    if ((bytes = sendto(sk.skHandle, (void *)buff, size, 0, (sockaddr *)&sendaddr, addrSize)) < 0) {
        sk.s32Error = __ERROR__;
        LOG_OSAL_ERROR("[%s] Send to failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return bytes;
}

int SOCKET_RecvFrom(SOCKET_T &sk, SOCKADDR_T &recvaddr, char *buff, size_t size) {
    int bytes = 0;
    socklen_t addrSize = 0;

    if (!SOCKET_Valid(sk)) {
        LOG_OSAL_ERROR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (!buff || size == 0) {
        LOG_OSAL_ERROR("[%s] Invalid buffer\n", __FUNCTION__);
        return RET_ERR;
    }

    if (sk.stAddrInet.s32AddrFamily == SOCKET_ADDR_V4) {
        addrSize = sizeof(SOCKADDR_V4);
    } else {
        addrSize = sizeof(SOCKADDR_V6);
    }

    if ((bytes = recvfrom(sk.skHandle, (void *)buff, size, 0, (sockaddr *)&recvaddr, &addrSize)) < 0) {
        sk.s32Error = __ERROR__;
        // LOG_OSAL_ERROR("[%s] Receive failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return bytes;
}

int SOCKET_SendMulticast(SOCKET_T &sk, const char *groupip, uint16_t port, const char *buff, size_t size) {
    int bytes = 0;
    socklen_t addrSize = 0;
    struct sockaddr_in stGroupSocket;
    char loopch = 0;

    if (!SOCKET_Valid(sk)) {
        LOG_OSAL_ERROR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (!buff || size == 0) {
        LOG_OSAL_ERROR("[%s] Invalid buffer\n", __FUNCTION__);
        return RET_ERR;
    }

    stGroupSocket.sin_family = AF_INET;
    stGroupSocket.sin_addr.s_addr = inet_addr(groupip);
    stGroupSocket.sin_port = htons(port);

    /*
     * Disable loopback so you do not receive your own datagrams.
     */
    if (setsockopt(sk.skHandle, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopch, sizeof(loopch)) < 0) {
        sk.s32Error = __ERROR__;
        LOG_OSAC_ERROR("[%s] setting IP_MULTICAST_LOOP: failed, %s", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    /*
     * Set local interface for outbound multicast datagrams.
     * The IP address specified must be associated with a local,
     * multicast-capable interface.
     */
    if (setsockopt(sk.skHandle, IPPROTO_IP, IP_MULTICAST_IF, &sk.stAddrInet.Ip.v4, sizeof(sk.stAddrInet.Ip.v4)) < 0) {
        sk.s32Error = __ERROR__;
        LOG_OSAC_ERROR("[%s] Setting local interface for multicast failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }

    /*
     * Send a message to the multicast group specified by the
     * stGroupSocket sockaddr structure.
     */
    if (sendto(sk.skHandle, buff, size, 0, (struct sockaddr *)&stGroupSocket, sizeof(stGroupSocket)) < 0) {
        sk.s32Error = __ERROR__;
        LOG_OSAC_ERROR("[%s] Sending datagram message failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

int SOCKET_SetRecvBuffer(SOCKET_T &sk, uint32_t size) {
    int32_t s32Size = static_cast<int32_t>(size);
    socklen_t len = sizeof(int32_t);

    if (!SOCKET_Valid(sk)) {
        LOG_OSAL_ERROR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (setsockopt(sk.skHandle, SOL_SOCKET, SO_RCVBUF, &s32Size, len) < 0) {
        sk.s32Error = __ERROR__;
        LOG_OSAL_ERROR("[%s] Set buffer failed\n", __FUNCTION__);
        return RET_ERR;
    }
    return RET_OK;
}

int SOCKET_SetSendBuffer(SOCKET_T &sk, uint32_t size) {
    int32_t s32Size = static_cast<int32_t>(size);
    socklen_t len = sizeof(int32_t);

    if (!SOCKET_Valid(sk)) {
        LOG_OSAL_ERROR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (setsockopt(sk.skHandle, SOL_SOCKET, SO_SNDBUF, &s32Size, len) < 0) {
        sk.s32Error = __ERROR__;
        LOG_OSAL_ERROR("[%s] Set buffer failed\n", __FUNCTION__);
        return RET_ERR;
    }
    return RET_OK;
}

int SOCKET_SetRecvTimeout(SOCKET_T &sk, uint32_t ms) {
    struct timeval pstTime;
    socklen_t len = sizeof(pstTime);

    pstTime.tv_sec = ms / 1000;
    pstTime.tv_usec = (ms * 1000) % 1000000;

    if (!SOCKET_Valid(sk)) {
        LOG_OSAL_ERROR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (setsockopt(sk.skHandle, SOL_SOCKET, SO_RCVTIMEO, &pstTime, len) < 0) {
        sk.s32Error = __ERROR__;
        LOG_OSAL_ERROR("[%s] Set receiving timeout %d failed, %s\n", __FUNCTION__, ms, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

int SOCKET_SetSendTimeout(SOCKET_T &sk, uint32_t ms) {
    struct timeval pstTime;
    socklen_t len = sizeof(pstTime);

    pstTime.tv_sec = ms / 1000;
    pstTime.tv_usec = (ms * 1000) % 1000000;

    if (!SOCKET_Valid(sk)) {
        LOG_OSAL_ERROR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (setsockopt(sk.skHandle, SOL_SOCKET, SO_SNDTIMEO, &pstTime, len) < 0) {
        sk.s32Error = __ERROR__;
        LOG_OSAL_ERROR("[%s] Set receiving timeout %d failed, %s\n", __FUNCTION__, ms, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

int SOCKET_SetBlockingMode(SOCKET_T &sk, int32_t mode) {
    const int flags = fcntl(sk.skHandle, F_GETFL, 0);
    if ((flags & O_NONBLOCK) && !mode) {
        LOG_OSAL_WARN("[%s] Socket was already in non-blocking mode\n", __FUNCTION__);
        return RET_OK;
    }
    if (!(flags & O_NONBLOCK) && mode) {
        LOG_OSAL_WARN("[%s] Socket was already in blocking mode\n", __FUNCTION__);
        return RET_OK;
    }
    if (fcntl(sk.skHandle, F_SETFL, mode ? flags ^ O_NONBLOCK : flags | O_NONBLOCK) < 0) {
        sk.s32Error = __ERROR__;
        LOG_OSAL_ERROR("[%s] Set blocking mode failed\n", __FUNCTION__);
        return RET_ERR;
    }
    return RET_OK;
}

int SOCKET_SetOption(SOCKET_T &sk, SOCKET_OPT &opt) {
    if (!SOCKET_Valid(sk)) {
        LOG_OSAL_ERROR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (setsockopt(sk.skHandle, opt.s32Level, opt.s32Option, opt.pBuffer, (socklen_t)opt.uSize) < 0) {
        sk.s32Error = __ERROR__;
        LOG_OSAL_ERROR("[%s] Set option %d failed\n", __FUNCTION__, opt.s32Option);
        return RET_ERR;
    }
    return RET_OK;
}

int SOCKET_GetOption(SOCKET_T &sk, SOCKET_OPT &opt) {
    if (!SOCKET_Valid(sk)) {
        LOG_OSAL_ERROR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (getsockopt(sk.skHandle, opt.s32Level, opt.s32Option, opt.pBuffer, (socklen_t *)&opt.uSize) < 0) {
        sk.s32Error = __ERROR__;
        LOG_OSAL_ERROR("[%s] Get option %d failed\n", __FUNCTION__, opt.s32Option);
        return RET_ERR;
    }
    return RET_OK;
}

SOCKADDR_T SOCKET_GetAddressV4(const char *ip, uint16_t port) {
    SOCKADDR_T addr;
    int ret = 0;
    addr.v4.sin_family = AF_INET;
    addr.v4.sin_port = htons(port);
    if ((ret = inet_pton(AF_INET, ip, &addr.v4.sin_addr)) <= 0) {
        LOG_OSAL_ERROR("[%s] Get v4 from string failed\n", __FUNCTION__);
    }
    return addr;
}

SOCKADDR_T SOCKET_GetAddressV6(const char *ip, uint16_t port) {
    SOCKADDR_T addr;
    int ret = 0;
    addr.v6.sin6_family = AF_INET6;
    addr.v6.sin6_port = htons(port);
    if ((ret = inet_pton(AF_INET6, ip, &addr.v6.sin6_addr)) <= 0) {
        LOG_OSAL_ERROR("[%s] Get v6 from string failed\n", __FUNCTION__);
    }
    return addr;
}

const char *SOCKET_Ipv4ToString(SOCKADDR_T &addr) {
    static char strAddr[64];
    sprintf(strAddr, "%s:%d", inet_ntoa(addr.v4.sin_addr), ntohs(addr.v4.sin_port));
    return strAddr;
}

const char *SOCKET_Ipv6ToString(SOCKADDR_T &addr) {
    static char strAddr[64];
    char inet6[32];
    sprintf(strAddr, "%s:%d", inet_ntop(AF_INET6, &addr.v6.sin6_addr, inet6, INET6_ADDRSTRLEN), ntohs(addr.v6.sin6_port));
    return strAddr;
}

SOCKADDR_T SOCKET_GetSocketName(SOCKET_T &sk) {
    SOCKADDR_T addr;
    socklen_t len = 0;

    if (sk.stAddrInet.s32AddrFamily == SOCKET_ADDR_V4) {
        len = sizeof(sk.stAddrInet.Ip.v4);
    } else {
        len = sizeof(sk.stAddrInet.Ip.v6);
    }
    if (getsockname(sk.skHandle, (struct sockaddr *)&addr, &len) == -1) {
        sk.s32Error = __ERROR__;
        LOG_OSAL_ERROR("Get socket name failed, %s\n", __ERROR_STR__);
    }
    return addr;
}
} // namespace osal
} // namespace gbs
