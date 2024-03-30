#include "CSocket.h"
#include "dbg/Debug.h"
#include "osal/Mutex.h"
#include "osal/Socket.h"

namespace gbs
{
    namespace osac
    {
        CSocket::CSocket() :
            m_stSk(), m_s32IsOpen(0), m_s32IsConnected(0), m_s32IsAcceptedSocket(0), m_stRemoteAddr(),
            m_poSocketSync(NULL) {
            m_poSocketSync = new CMutex();
            m_poSocketSync->Create();
        }

        CSocket::CSocket(SOCKET_T &socket) :
            m_stSk(), m_s32IsOpen(0), m_s32IsConnected(0), m_s32IsAcceptedSocket(0), m_stRemoteAddr(),
            m_poSocketSync(NULL) {
            m_stSk = socket;
            m_poSocketSync = new CMutex();
            m_poSocketSync->Create();
            m_s32IsAcceptedSocket = 1;
            m_s32IsConnected = 1;
            m_s32IsOpen = m_stSk.skHandle >= 0 ? 1 : 0;
        }

        CSocket::~CSocket() {
            if (IsOpen())
                Close();
            delete m_poSocketSync;
        }

        /**
         * @fn Open
         * @brief Open a socket
         *
         * @param sockettype	TCP/UDP type
         * @param mode 			Client/Server mode
         * @return int 			0 if success, otherwise -1
         */
        int CSocket::Open(int32_t sockettype, int32_t mode) {
            if (m_s32IsAcceptedSocket) {
                CLOG_ERROR("Can not open accepted socket\n");
                return -1;
            }
            if (IsOpen()) {
                CLOG_WARN("Socket is openning\n");
                return 0;
            }
            m_stSk = osal::SOCKET_Create(sockettype, mode);
            m_s32IsOpen = m_stSk.skHandle >= 0 ? 1 : 0;
            return m_s32IsOpen ? 0 : -1;
        }

        /**
         * @fn Close
         * @brief Close this socket
         *
         * @return int 			0 if success, otherwise -1
         */
        int CSocket::Close() {
            if (IsOpen())
                m_poSocketSync->SafeWrite(m_s32IsOpen, 0);
            m_poSocketSync->Lock();
            int ret = osal::SOCKET_Close(m_stSk);
            m_poSocketSync->UnLock();
            return ret;
        }

        /**
         * @fn IsOpen
         * @brief
         *
         * @return int
         */
        int CSocket::IsOpen() {
            return m_poSocketSync->SafeRead(m_s32IsOpen);
        }

        /**
         * @fn IsConnected
         * @brief
         *
         * @return int
         */
        int CSocket::IsConnected() {
            return m_poSocketSync->SafeRead(m_s32IsConnected);
        }

        /**
         * @fn Connect
         * @brief Connect to remote host, using for connection socket
         *
         * @param remoteip 		Remote host ip
         * @param remoteport 	Remote host port
         * @return int 			0 if success, otherwise -1
         */
        int CSocket::Connect(const char *remoteip, uint16_t remoteport) {
            m_poSocketSync->Lock();
            int ret = osal::SOCKET_Connect(m_stSk, remoteip, remoteport);
            m_s32IsConnected = (ret == 0 ? 1 : 0);
            m_poSocketSync->UnLock();
            return ret;
        }

        /**
         * @fn Disconnect
         * @brief Disconect from host
         *
         * @return int 			0 if success, otherwise -1
         */
        int CSocket::Disconnect() {
            m_poSocketSync->Lock();
            m_s32IsConnected = 0;
            int ret = osal::SOCKET_Disconnect(m_stSk);
            m_poSocketSync->UnLock();
            return ret;
        }

        /**
         * @fn Bind
         * @brief Bind this soket with ip and port
         *
         * @param ip 			Bind ip
         * @param port 			Bind port
         * @return int 			0 if success, otherwise -1
         */
        int CSocket::Bind(const char *ip, uint16_t port) {
            m_stSk.stAddrInet.Ip = osal::SOCKET_GetAddressV4(ip, port);
            return osal::SOCKET_Bind(m_stSk, port);
        }

        /**
         * @fn Listen
         * @brief Listen on this socket, using for TCP server mode
         *
         * @param connection 	Maxinum of incoming connections to be established by this socket
         * @return int 			0 if success, otherwise -1
         */
        int CSocket::Listen(uint32_t connection) { return osal::SOCKET_Listen(m_stSk, connection); }

        /**
         * @fn Accept
         * @brief Get connected socket of this TCP server socket
         *
         * @return CSocket* New remote socket, user must destroys it.
         */
        CSocket *CSocket::Accept() {
            CSocket *poAcceptSk = NULL;
            m_poSocketSync->Lock();
            auto sk = osal::SOCKET_Accept(m_stSk);
            if (sk.skHandle > 0) {
                poAcceptSk = new CSocket(sk);
            }
            m_poSocketSync->UnLock();
            return poAcceptSk;
        }

        /**
         * @fn Send
         * @brief Send to this socket, used for connection or connectionless
         *
         * @param buff 			Pointer to buffer
         * @param size 			Buffer size
         * @return int 			0 if success, otherwise -1
         */
        int CSocket::Send(const char *buff, size_t size) {
            int ret = -1;
            if (!IsConnected()) return ret;
            m_poSocketSync->Lock();
            ret = osal::SOCKET_Send(m_stSk, buff, size);
            m_poSocketSync->UnLock();
            return ret;
        }

        /**
         * @fn Receive
         * @brief Receive data within socket
         *
         * @param buff 			Pointer to buffer
         * @param size 			Buffer size
         * @return int 			0 if success, otherwise -1
         */
        int CSocket::Receive(char *buff, size_t size) {
            int ret = -1;
            if (!IsConnected()) return ret;
            m_poSocketSync->Lock();
            ret = osal::SOCKET_Recv(m_stSk, buff, size);
            m_poSocketSync->UnLock();
            return ret;
        }

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
        int CSocket::SendTo(const char *ip, uint16_t port, const char *buff, size_t size) {
            int ret = 0;
            SOCKADDR_T stAddr = osal::SOCKET_GetAddressV4(ip, port);
            m_poSocketSync->Lock();
            ret = osal::SOCKET_SendTo(m_stSk, stAddr, buff, size);
            m_poSocketSync->UnLock();
            return ret;
        }

        /**
         * @fn ReceiveFrom
         * @brief Receive data from specific host
         *
         * @param buff 			Pointer to buffer
         * @param size 			Buffer size
         * @param addr 			Target host address that this socket is received from
         * @return int 			0 if success, otherwise -1
         */
        int CSocket::ReceiveFrom(char *buff, size_t size, SOCKADDR_T &addr) {
            int ret = 0;
            m_poSocketSync->Lock();
            ret = osal::SOCKET_RecvFrom(m_stSk, addr, buff, size);
            m_poSocketSync->UnLock();
            return ret;
        }

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
        int CSocket::SendMulticast(const char *groupip, uint16_t port, const char *buff, size_t size) {
            int ret = 0;
            m_poSocketSync->Lock();
            ret = osal::SOCKET_SendMulticast(m_stSk, groupip, port, buff, size);
            m_poSocketSync->UnLock();
            return ret;
        }

        /**
         * @fn SetRecvBufferSize
         * @brief Set the socket receive buffer size
         *
         * @param size 			Recv Buffer size
         * @return int 			0 if success, otherwise -1
         */
        int CSocket::SetRecvBufferSize(uint32_t size) {
            int ret = 0;
            m_poSocketSync->Lock();
            ret = osal::SOCKET_SetRecvBuffer(m_stSk, size);
            m_poSocketSync->UnLock();
            return ret;
        }

        /**
         * @fn SetSendBufferSize
         * @brief Set the socket send buffer size
         *
         * @param size 			Send buffer size
         * @return int 			0 if success, otherwise -1
         */
        int CSocket::SetSendBufferSize(uint32_t size) {
            int ret = 0;
            m_poSocketSync->Lock();
            ret = osal::SOCKET_SetSendBuffer(m_stSk, size);
            m_poSocketSync->UnLock();
            return ret;
        }

        /**
         * @fn SetBlockingMode
         * @brief Set the socket block mode
         *
         * @param mode 			Set socket block mode (1 - block(default) and 0 - non-block)
         * @return int 			0 if success, otherwise -1
         */
        int CSocket::SetBlockingMode(int mode) {
            int ret = 0;
            m_poSocketSync->Lock();
            ret = osal::SOCKET_SetBlockingMode(m_stSk, mode);
            m_poSocketSync->UnLock();
            return ret;
        }

        /**
         * @fn SetRecvTimeout
         * @brief Set socket receive timeout in ms
         *
         * @param ms 			Timeout
         * @return int 			0 if success, otherwise -1
         */
        int CSocket::SetRecvTimeout(uint32_t ms) {
            int ret = 0;
            m_poSocketSync->Lock();
            ret = osal::SOCKET_SetRecvTimeout(m_stSk, ms);
            m_poSocketSync->UnLock();
            return ret;
        }

        /**
         * @fn SetSendTimeout
         * @brief Set socket send timeout in ms
         *
         * @param ms 			Timeout
         * @return int 			0 if success, otherwise -1
         */
        int CSocket::SetSendTimeout(uint32_t ms) {
            int ret = 0;
            m_poSocketSync->Lock();
            ret = osal::SOCKET_SetSendTimeout(m_stSk, ms);
            m_poSocketSync->UnLock();
            return ret;
        }

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
        int CSocket::SetOption(int32_t opt, int32_t level, void *optvalue, size_t optsize) {
            int ret = 0;
            SOCKET_OPT stOpt = {level, opt, optvalue, optsize};
            m_poSocketSync->Lock();
            ret = osal::SOCKET_SetOption(m_stSk, stOpt);
            m_poSocketSync->UnLock();
            return ret;
        }

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
        int CSocket::GetOption(int32_t opt, int32_t level, void *optbuff, size_t optsize) {
            int ret = 0;
            SOCKET_OPT stOpt = {level, opt, optbuff, optsize};
            m_poSocketSync->Lock();
            ret = osal::SOCKET_GetOption(m_stSk, stOpt);
            m_poSocketSync->UnLock();
            return ret;
        }

        /**
         * @fn GetAddress
         * @brief Get the Address string
         *
         * @return const char*
         */
        const char *CSocket::GetAddress() {
            if (m_stSk.stAddrInet.s32AddrFamily == SOCKET_ADDR_V4) {
                return osal::SOCKET_Ipv4ToString(m_stSk.stAddrInet.Ip);
            } else {
                return osal::SOCKET_Ipv6ToString(m_stSk.stAddrInet.Ip);
            }
        }

        /**
         * @fn GetSocketAddr
         * @brief Get the Socket address
         *
         * @return SOCKADDR_T
         */
        SOCKADDR_T CSocket::GetSocketAddr() {
            return osal::SOCKET_GetSocketName(m_stSk);
        }

        /**
         * @fn Ipv4ToString
         * @brief
         *
         * @param addr
         * @return const char*
         */
        const char *CSocket::Ipv4ToString(SOCKADDR_T &addr) { return osal::SOCKET_Ipv4ToString(addr); }

        /**
         * @fn Ipv6ToString
         * @brief
         *
         * @param addr
         * @return const char*
         */
        const char *CSocket::Ipv6ToString(SOCKADDR_T &addr) { return osal::SOCKET_Ipv6ToString(addr); }
    } // namespace osac
} // namespace gbs
