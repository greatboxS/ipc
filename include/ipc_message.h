#ifndef __IPC_MESSAGE_H__
#define __IPC_MESSAGE_H__

#include "ipc_args.h"

namespace gbs
{
    namespace ipc
    {
        class IpcProfile;

        class IpcIMessage
        {
        public:
            virtual ~IpcIMessage() = default;
            virtual int32_t transId() const = 0;
            virtual int32_t senderId() const = 0;
            virtual int32_t receiverId() const = 0;
            virtual const std::string &sender() const = 0;
            virtual const std::string &receiver() const = 0;
            virtual const std::string &content() const = 0;
        };

        class IpcMessage : public IpcIMessage
        {
        private:
            int32_t m_id;
            int32_t m_senderId;
            int32_t m_receiverId;
            std::string m_sender;
            std::string m_receiver;
            std::string m_content;

            void setBuffer(const char *data, size_t size) {
                m_content = std::string(data, data + size);
            }

        public:
            explicit IpcMessage(int32_t id, int32_t sender, int32_t recv) :
                m_id(id), m_senderId(sender), m_receiverId(recv), m_content("") {
            }

            explicit IpcMessage(int32_t sener, int32_t recv, const std::string &message) :
                m_id(0), m_senderId(sener), m_receiverId(recv), m_content(message) {
            }

            explicit IpcMessage(const std::string &sender, const std::string &recv, const std::string &message) :
                m_id(0), m_sender(sender), m_receiver(recv), m_content(message) {
            }

            explicit IpcMessage(const std::vector<uint8_t> &message) :
                m_id(0), m_sender(""), m_receiver(""),
                m_content(message.data(), message.data() + message.size()) {
            }

            virtual ~IpcMessage() {
            }

            virtual int32_t transId() const { return m_id; }
            virtual int32_t senderId() const { return m_senderId; }
            virtual int32_t receiverId() const { return m_receiverId; }
            virtual const std::string &sender() const { return m_sender; }
            virtual const std::string &receiver() const { return m_receiver; }
            virtual const std::string &content() const { return m_content; }

            void setContent(const char *buf, size_t size) try {
                if (!buf || size == 0) { IPC_THROW("Invalid data!"); }
                setBuffer(buf, size);
            }
            IPC_HANDLE();

            template <typename... Argv>
            IpcArgs<Argv...> args() {
                return IpcArgs<Argv...>(m_content.data(), m_content.size());
            }

            template <typename... Argv>
            int setArgs(IpcArgs<Argv...> &args) {
                if (args.bin().size() == 0) return -1;
                m_content = args.bin();
                return 0;
            }
        };

    } // namespace ipc
} // namespace gbs

#endif // __IPC_MESSAGE_H__