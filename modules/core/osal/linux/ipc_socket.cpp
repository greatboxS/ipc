#include "osal/ipc_socket.h"
#include <arpa/inet.h>

namespace ipc::core {
static inline int socket_is_valid(SOCKET_T &sk) {
    return (sk.skHandle == -1 ? 0 : 1);
}

SOCKET_T socket_create(int32_t sockettype, int blockmode, int addrfamily) {
    SOCKET_T stSocket;
    int domain = AF_UNIX;
    int type = SOCK_SEQPACKET;
    uint32_t addr_size = 0;
    int family = addrfamily;

    if (addrfamily == SOCKET_ADDR_V4) {
        domain = AF_INET;
        addr_size = sizeof(SOCKADDR_V4);
    } else if (addrfamily == SOCKET_ADDR_V6) {
        domain = AF_INET6;
        addr_size = sizeof(SOCKADDR_V6);
    } else {
        // Do nothing
    }

    memset(&stSocket, 0, sizeof(stSocket));
    stSocket.skHandle = -1;

    if (sockettype == eSOCKET_TCP) {
        type = SOCK_STREAM;
    } else if (sockettype = eSOCKET_UDP) {
        type = SOCK_DGRAM;
    } else if (sockettype = eSOCKET_HOST) {
        domain = AF_UNIX;
        type = SOCK_SEQPACKET;
        family = AF_UNIX;
        addr_size = sizeof(SOCKADDR_H);
    } else {
        // Do nothing
    }

    if ((stSocket.skHandle = socket(domain, type, 0)) == INVALID_SOCKET) {
        stSocket.s32Error = __ERROR__;
        OSAL_ERR("[%s] Create socket failed, %d\n", __FUNCTION__, stSocket.s32Error);
        return stSocket;
    }

    stSocket.s32SocketType = sockettype;
    stSocket.s32BlockMode = blockmode;
    stSocket.stAddrInet.s32AddrFamily = family;
    stSocket.stAddrInet.u32Size = addr_size;
    return stSocket;
}

int socket_close(SOCKET_T &sk) {
    if (!socket_is_valid(sk)) {
        OSAL_ERR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (close(sk.skHandle) < 0) {
        sk.s32Error = __ERROR__;
        OSAL_ERR("[%s] Close socket failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

int socket_connect(SOCKET_T &sk, const char *remoteip, uint16_t remoteport) {
    void *socketaddr = NULL;
    SOCKADDR_V4 v4_addr;
    SOCKADDR_V6 v6_addr;
    SOCKADDR_H un_addr;
    socklen_t size = 0;

    if (!remoteip) {
        OSAL_ERR("[%s] Remote ip is null\n", __FUNCTION__);
        return RET_ERR;
    }

    if (!socket_is_valid(sk)) {
        OSAL_ERR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }

    if (sk.s32SocketType != eSOCKET_TCP) {
        OSAL_INFO("[%s] Can not connect a not streaming socket\n", __FUNCTION__);
        return RET_ERR;
    }

    if (sk.stAddrInet.s32AddrFamily == SOCKET_ADDR_V4) {
        size = sizeof(SOCKADDR_V4);
        v4_addr.sin_family = AF_INET;
        v4_addr.sin_port = htons(remoteport);
        if (inet_pton(AF_INET, remoteip, &v4_addr.sin_addr) <= 0) {
            sk.s32Error = __ERROR__;
            OSAL_ERR("[%s] Can not convert remote ip to Ipv4 address\n", __FUNCTION__);
            return RET_ERR;
        }

        socketaddr = &v4_addr;
    } else if (sk.stAddrInet.s32AddrFamily == SOCKET_ADDR_V6) {
        size = sizeof(SOCKADDR_V6);
        v6_addr.sin6_family = AF_INET6;
        v6_addr.sin6_port = htons(remoteport);
        if (inet_pton(AF_INET6, remoteip, &v6_addr.sin6_addr) <= 0) {
            sk.s32Error = __ERROR__;
            OSAL_ERR("[%s] Can not convert remote ip to Ipv6 address\n", __FUNCTION__);
            return RET_ERR;
        }

        socketaddr = &v6_addr;
    } else {
        size = sizeof(SOCKADDR_H);
        memset(&un_addr, 0, sizeof(SOCKADDR_H));
        un_addr.sun_family = AF_UNIX;
        strncpy(un_addr.sun_path, remoteip, sizeof(un_addr.sun_path) - 1);

        socketaddr = &un_addr;
    }

    if (connect(sk.skHandle, (sockaddr *)socketaddr, size) < 0) {
        sk.s32Error = __ERROR__;
        OSAL_ERR("[%s] Socket connect error, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

int socket_disconnect(SOCKET_T &sk) {
    if (shutdown(sk.skHandle, SHUT_RDWR) < 0) {
        sk.s32Error = __ERROR__;
        OSAL_ERR("[%s] Shutdown failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

int socket_listen(SOCKET_T &sk, uint32_t connection) {
    if (!socket_is_valid(sk)) {
        OSAL_ERR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (listen(sk.skHandle, connection) < 0) {
        sk.s32Error = __ERROR__;
        OSAL_ERR("[%s] Listen failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

int socket_bind(SOCKET_T &sk, uint16_t port, const char *path) {
    void *socketaddr = NULL;
    SOCKADDR_V4 v4_addr;
    SOCKADDR_V6 v6_addr;
    SOCKADDR_H un_addr;
    socklen_t size = 0;

    memset(&v4_addr, 0, sizeof(v4_addr));
    memset(&v6_addr, 0, sizeof(v6_addr));

    if (!socket_is_valid(sk)) {
        OSAL_ERR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }

    if (sk.stAddrInet.s32AddrFamily == SOCKET_ADDR_V4) {
        size = sizeof(SOCKADDR_V4);
        v4_addr.sin_family = AF_INET;
        v4_addr.sin_port = htons(port);
        v4_addr.sin_addr.s_addr = sk.stAddrInet.Ip.v4.sin_addr.s_addr;
        socketaddr = &v4_addr;
    } else if (sk.stAddrInet.s32AddrFamily == SOCKET_ADDR_V6) {
        size = sizeof(SOCKADDR_V6);
        v6_addr.sin6_family = AF_INET6;
        v6_addr.sin6_port = htons(port);
        v6_addr.sin6_addr = sk.stAddrInet.Ip.v6.sin6_addr;
        socketaddr = &v6_addr;
    } else {
        if (path != nullptr) {
            memset(&un_addr, 0, sizeof(un_addr));
            un_addr.sun_family = AF_UNIX;
            strncpy(un_addr.sun_path, path, sizeof(un_addr.sun_path) - 1);
            unlink(path);
            socketaddr = &un_addr;
        } else {
            OSAL_ERR("Invalid socket path\n");
            return RET_ERR;
        }
    }

#ifndef DISABLE_REUSE_BINDING_SOCKET
    int reuse = 1;
    SOCKET_OPT stOpt = {SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)};

    /**
     * After the SO_REUSEADDR option is active, the following situation is supported:
     * A server can bind() the same port multiple times as long as every invocation uses a different
     * local IP address and the wildcard address INADDR_ANY is used only one time per port.
     */
    socket_set_option(sk, stOpt);
#endif
    if (bind(sk.skHandle, (sockaddr *)socketaddr, size) < 0) {
        sk.s32Error = __ERROR__;
        OSAL_ERR("[%s] Bind failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }

    return RET_OK;
}

/**
 * @fn socket_accept
 * @brief
 *
 * @param sk
 * @return SOCKET_T
 */
SOCKET_T socket_accept(SOCKET_T &sk) {
    SOCKET_T stSocket;

    memset(&stSocket, 0, sizeof(stSocket));
    stSocket.skHandle = -1;

    /* accept socket ip version will be inherited from host socket */
    stSocket.stAddrInet.s32AddrFamily = sk.stAddrInet.s32AddrFamily;
    if (stSocket.stAddrInet.s32AddrFamily == SOCKET_ADDR_V4) {
        stSocket.stAddrInet.u32Size = sizeof(SOCKADDR_V4);
    } else if (stSocket.stAddrInet.s32AddrFamily == SOCKET_ADDR_V6) {
        stSocket.stAddrInet.u32Size = sizeof(SOCKADDR_V6);
    } else {
        stSocket.stAddrInet.u32Size = sizeof(SOCKADDR_H);
    }

    if (!socket_is_valid(sk)) {
        OSAL_ERR("[%s] Invalid socket\n", __FUNCTION__);
        return stSocket;
    }

    if ((stSocket.skHandle = accept(sk.skHandle, (sockaddr *)&stSocket.stAddrInet.Ip, (socklen_t *)&stSocket.stAddrInet.u32Size)) < 0) {
        stSocket.s32Error = __ERROR__;
        OSAL_ERR("[%s] Accept failed, %s\n", __FUNCTION__, __ERROR_STR__);
    }
    return stSocket;
}

int socket_send(SOCKET_T &sk, const char *buff, size_t size) {
    int bytes = 0;

    if (!socket_is_valid(sk)) {
        OSAL_ERR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (!buff || size == 0) {
        OSAL_ERR("[%s] Invalid buffer\n", __FUNCTION__);
        return RET_ERR;
    }

    if ((bytes = send(sk.skHandle, buff, size, 0)) < 0) {
        sk.s32Error = __ERROR__;
        OSAL_ERR("[%s] Send failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return bytes;
}

int socket_recv(SOCKET_T &sk, char *buff, size_t size) {
    int bytes = 0;

    if (!socket_is_valid(sk)) {
        OSAL_ERR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (!buff || size == 0) {
        OSAL_ERR("[%s] Invalid buffer\n", __FUNCTION__);
        return RET_ERR;
    }

    if ((bytes = recv(sk.skHandle, buff, size, 0)) < 0) {
        sk.s32Error = __ERROR__;
        // OSAL_ERR("[%s] Receive failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return bytes;
}

int socket_send_to(SOCKET_T &sk, SOCKADDR_T &sendaddr, const char *buff, size_t size) {
    int bytes = 0;
    socklen_t addrSize = 0;

    if (!socket_is_valid(sk)) {
        OSAL_ERR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (!buff || size == 0) {
        OSAL_ERR("[%s] Invalid buffer\n", __FUNCTION__);
        return RET_ERR;
    }

    if (sk.stAddrInet.s32AddrFamily == SOCKET_ADDR_V4) {
        addrSize = sizeof(SOCKADDR_V4);
    } else if (sk.stAddrInet.s32AddrFamily == SOCKET_ADDR_V6) {
        addrSize = sizeof(SOCKADDR_V6);
    } else {
        addrSize = sizeof(SOCKADDR_H);
    }

    if ((bytes = sendto(sk.skHandle, (void *)buff, size, 0, (sockaddr *)&sendaddr, addrSize)) < 0) {
        sk.s32Error = __ERROR__;
        OSAL_ERR("[%s] Send to failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return bytes;
}

int socket_recv_from(SOCKET_T &sk, SOCKADDR_T &recvaddr, char *buff, size_t size) {
    int bytes = 0;
    socklen_t addrSize = 0;

    if (!socket_is_valid(sk)) {
        OSAL_ERR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (!buff || size == 0) {
        OSAL_ERR("[%s] Invalid buffer\n", __FUNCTION__);
        return RET_ERR;
    }

    if (sk.stAddrInet.s32AddrFamily == SOCKET_ADDR_V4) {
        addrSize = sizeof(SOCKADDR_V4);
    } else if (sk.stAddrInet.s32AddrFamily == SOCKET_ADDR_V6) {
        addrSize = sizeof(SOCKADDR_V6);
    } else {
        addrSize = sizeof(SOCKADDR_H);
    }

    if ((bytes = recvfrom(sk.skHandle, (void *)buff, size, 0, (sockaddr *)&recvaddr, &addrSize)) < 0) {
        sk.s32Error = __ERROR__;
        // OSAL_ERR("[%s] Receive failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return bytes;
}

int socket_send_multicast(SOCKET_T &sk, const char *groupip, uint16_t port, const char *buff, size_t size) {
    int bytes = 0;
    socklen_t addrSize = 0;
    struct sockaddr_in stGroupSocket;
    char loopch = 0;

    if (!socket_is_valid(sk)) {
        OSAL_ERR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (!buff || size == 0) {
        OSAL_ERR("[%s] Invalid buffer\n", __FUNCTION__);
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
        OSAL_ERR("[%s] setting IP_MULTICAST_LOOP: failed, %s", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    /*
     * Set local interface for outbound multicast datagrams.
     * The IP address specified must be associated with a local,
     * multicast-capable interface.
     */
    if (setsockopt(sk.skHandle, IPPROTO_IP, IP_MULTICAST_IF, &sk.stAddrInet.Ip.v4, sizeof(sk.stAddrInet.Ip.v4)) < 0) {
        sk.s32Error = __ERROR__;
        OSAL_ERR("[%s] Setting local interface for multicast failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }

    /*
     * Send a message to the multicast group specified by the
     * stGroupSocket sockaddr structure.
     */
    if (sendto(sk.skHandle, buff, size, 0, (struct sockaddr *)&stGroupSocket, sizeof(stGroupSocket)) < 0) {
        sk.s32Error = __ERROR__;
        OSAL_ERR("[%s] Sending datagram message failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

int socket_set_recv_buff(SOCKET_T &sk, uint32_t size) {
    int32_t s32Size = static_cast<int32_t>(size);
    socklen_t len = sizeof(int32_t);

    if (!socket_is_valid(sk)) {
        OSAL_ERR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (setsockopt(sk.skHandle, SOL_SOCKET, SO_RCVBUF, &s32Size, len) < 0) {
        sk.s32Error = __ERROR__;
        OSAL_ERR("[%s] Set buffer failed\n", __FUNCTION__);
        return RET_ERR;
    }
    return RET_OK;
}

int socket_set_send_buff(SOCKET_T &sk, uint32_t size) {
    int32_t s32Size = static_cast<int32_t>(size);
    socklen_t len = sizeof(int32_t);

    if (!socket_is_valid(sk)) {
        OSAL_ERR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (setsockopt(sk.skHandle, SOL_SOCKET, SO_SNDBUF, &s32Size, len) < 0) {
        sk.s32Error = __ERROR__;
        OSAL_ERR("[%s] Set buffer failed\n", __FUNCTION__);
        return RET_ERR;
    }
    return RET_OK;
}

int socket_set_recv_timeout(SOCKET_T &sk, uint32_t ms) {
    struct timeval pstTime;
    socklen_t len = sizeof(pstTime);

    pstTime.tv_sec = ms / 1000;
    pstTime.tv_usec = (ms * 1000) % 1000000;

    if (!socket_is_valid(sk)) {
        OSAL_ERR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (setsockopt(sk.skHandle, SOL_SOCKET, SO_RCVTIMEO, &pstTime, len) < 0) {
        sk.s32Error = __ERROR__;
        OSAL_ERR("[%s] Set receiving timeout %d failed, %s\n", __FUNCTION__, ms, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

int socket_set_send_timeout(SOCKET_T &sk, uint32_t ms) {
    struct timeval pstTime;
    socklen_t len = sizeof(pstTime);

    pstTime.tv_sec = ms / 1000;
    pstTime.tv_usec = (ms * 1000) % 1000000;

    if (!socket_is_valid(sk)) {
        OSAL_ERR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (setsockopt(sk.skHandle, SOL_SOCKET, SO_SNDTIMEO, &pstTime, len) < 0) {
        sk.s32Error = __ERROR__;
        OSAL_ERR("[%s] Set receiving timeout %d failed, %s\n", __FUNCTION__, ms, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

int socket_set_blocking_mode(SOCKET_T &sk, int32_t mode) {
    const int flags = fcntl(sk.skHandle, F_GETFL, 0);
    if ((flags & O_NONBLOCK) && !mode) {
        OSAL_INFO("[%s] Socket was already in non-blocking mode\n", __FUNCTION__);
        return RET_OK;
    }
    if (!(flags & O_NONBLOCK) && mode) {
        OSAL_INFO("[%s] Socket was already in blocking mode\n", __FUNCTION__);
        return RET_OK;
    }
    if (fcntl(sk.skHandle, F_SETFL, mode ? flags ^ O_NONBLOCK : flags | O_NONBLOCK) < 0) {
        sk.s32Error = __ERROR__;
        OSAL_ERR("[%s] Set blocking mode failed\n", __FUNCTION__);
        return RET_ERR;
    }
    return RET_OK;
}

int socket_set_option(SOCKET_T &sk, SOCKET_OPT &opt) {
    if (!socket_is_valid(sk)) {
        OSAL_ERR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (setsockopt(sk.skHandle, opt.s32Level, opt.s32Option, opt.pBuffer, (socklen_t)opt.uSize) < 0) {
        sk.s32Error = __ERROR__;
        OSAL_ERR("[%s] Set option %d failed\n", __FUNCTION__, opt.s32Option);
        return RET_ERR;
    }
    return RET_OK;
}

int socket_get_option(SOCKET_T &sk, SOCKET_OPT &opt) {
    if (!socket_is_valid(sk)) {
        OSAL_ERR("[%s] Invalid socket\n", __FUNCTION__);
        return RET_ERR;
    }
    if (getsockopt(sk.skHandle, opt.s32Level, opt.s32Option, opt.pBuffer, (socklen_t *)&opt.uSize) < 0) {
        sk.s32Error = __ERROR__;
        OSAL_ERR("[%s] Get option %d failed\n", __FUNCTION__, opt.s32Option);
        return RET_ERR;
    }
    return RET_OK;
}

SOCKADDR_T socket_get_addr_v4(const char *ip, uint16_t port) {
    SOCKADDR_T addr;
    int ret = 0;
    addr.v4.sin_family = AF_INET;
    addr.v4.sin_port = htons(port);
    if ((ret = inet_pton(AF_INET, ip, &addr.v4.sin_addr)) <= 0) {
        OSAL_ERR("[%s] Get v4 from string failed\n", __FUNCTION__);
    }
    return addr;
}

SOCKADDR_T socket_get_addr_v6(const char *ip, uint16_t port) {
    SOCKADDR_T addr;
    int ret = 0;
    addr.v6.sin6_family = AF_INET6;
    addr.v6.sin6_port = htons(port);
    if ((ret = inet_pton(AF_INET6, ip, &addr.v6.sin6_addr)) <= 0) {
        OSAL_ERR("[%s] Get v6 from string failed\n", __FUNCTION__);
    }
    return addr;
}

const char *socket_ip_v4_to_string(SOCKADDR_T &addr) {
    static char strAddr[64];
    sprintf(strAddr, "%s:%d", inet_ntoa(addr.v4.sin_addr), ntohs(addr.v4.sin_port));
    return strAddr;
}

const char *socket_ip_v6_to_string(SOCKADDR_T &addr) {
    static char strAddr[64];
    char inet6[32];
    sprintf(strAddr, "%s:%d", inet_ntop(AF_INET6, &addr.v6.sin6_addr, inet6, INET6_ADDRSTRLEN), ntohs(addr.v6.sin6_port));
    return strAddr;
}

SOCKADDR_T socket_get_name(SOCKET_T &sk) {
    SOCKADDR_T addr;
    socklen_t len = 0;

    if (sk.stAddrInet.s32AddrFamily == SOCKET_ADDR_V4) {
        len = sizeof(sk.stAddrInet.Ip.v4);
    } else if (sk.stAddrInet.s32AddrFamily == SOCKET_ADDR_V6) {
        len = sizeof(sk.stAddrInet.Ip.v6);
    } else {
        len = sizeof(sk.stAddrInet.Ip.un);
    }
    if (getsockname(sk.skHandle, (struct sockaddr *)&addr, &len) < 0) {
        sk.s32Error = __ERROR__;
        OSAL_ERR("Get socket name failed, %s\n", __ERROR_STR__);
    }
    return addr;
}
} // namespace ipc::core