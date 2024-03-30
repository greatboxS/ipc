#ifndef __IPC_ARGS_H__
#define __IPC_ARGS_H__

#include "ipc_def.h"
#include <iostream>
#include <sstream>
#include <tuple>
#include <typeinfo>
#include <vector>

namespace gbs
{
    namespace ipc
    {
        constexpr static int32_t max_str_len = 1024 * 10;

        struct ArgElement {
            int32_t id;
            int32_t size;
        };

        template <typename T>
        struct Parser {
            static T parseElement(std::stringstream &stream, const std::vector<std::streampos> &pos, std::size_t index) {
                static ArgElement ele;
                T value;
                std::streampos position = pos[index];

                if (index >= pos.size() || stream.eof()) {
                    throw std::out_of_range("Invalid index");
                }

                if (!stream.seekg(position, std::ios_base::beg)) {
                    throw std::runtime_error("Failed to seek to position");
                }

                if (!stream.read(reinterpret_cast<char *>(&ele), sizeof(ArgElement))) {
                    throw std::runtime_error("Failed to read ArgElement");
                }

                if (!stream.read(reinterpret_cast<char *>(&value), sizeof(T))) {
                    throw std::runtime_error("Failed to read value");
                }
                return std::move(value);
            }
        };

        template <>
        inline std::string Parser<std::string>::parseElement(std::stringstream &stream, const std::vector<std::streampos> &pos, std::size_t index) {
            static ArgElement ele;
            std::string str;
            std::streampos position = pos[index];

            if (index >= pos.size() || stream.eof()) {
                throw std::out_of_range("Invalid index");
            }

            if (!stream.seekg(position, std::ios_base::beg)) {
                throw std::runtime_error("Failed to seek to position");
            }

            if (!stream.read(reinterpret_cast<char *>(&ele), sizeof(ArgElement))) {
                throw std::runtime_error("Failed to read ArgElement");
            }

            str.resize(ele.size);
            stream.read(str.data(), ele.size);
            return std::move(str);
        }

        template <>
        inline const char *Parser<const char *>::parseElement(std::stringstream &stream, const std::vector<std::streampos> &pos, std::size_t index) {
            static ArgElement ele;
            char *buf = NULL;
            std::streampos position = pos[index];

            if (index >= pos.size() || stream.eof()) {
                throw std::out_of_range("Invalid index");
            }

            if (!stream.seekg(position, std::ios_base::beg)) {
                throw std::runtime_error("Failed to seek to position");
            }

            if (!stream.read(reinterpret_cast<char *>(&ele), sizeof(ArgElement))) {
                throw std::runtime_error("Failed to read ArgElement");
            }

            buf = new char[ele.size];
            stream.read(buf, ele.size);
            return buf;
        }

        template <typename... Args>
        class IpcArgs
        {
            template <typename... Ts>
            class IpcArgData
            {
            private:
                bool valid;
                std::tuple<Ts...> data;

            public:
                IpcArgData() :
                    valid(false) {}
                ~IpcArgData() {}
                bool has_value() { return valid; }
                bool reset() { return valid = false; }
                void emplace(const std::tuple<Ts...> &args) {
                    data = std::move(args);
                    valid = true;
                }
                const auto get() const { return data; }
            };

            template <std::size_t... Is>
            static auto parseData(std::stringstream &stream, const std::vector<std::streampos> &pos, std::index_sequence<Is...>) {
                return std::move(std::make_tuple(gbs::ipc::Parser<Args>::parseElement(stream, pos, Is)...));
            }

            static auto parseData(std::stringstream &stream, const std::vector<std::streampos> &pos) {
                return std::move(IpcArgs<Args...>::parseData(stream, pos, std::index_sequence_for<Args...>{}));
            }

            static const IpcArgData<Args...> parseResult(std::stringstream &stream) {
                static ArgElement ele;
                IpcArgData<Args...> data;
                std::vector<std::streampos> pos;
                int count = static_cast<int>(sizeof...(Args));

                stream.seekg(0, std::ios_base::beg);
                while (!stream.eof()) {
                    if (--count < 0) break;
                    pos.push_back(stream.tellg());
                    stream.read(reinterpret_cast<char *>(&ele), sizeof(ArgElement));
                    stream.seekg(ele.size, std::ios_base::cur);
                }
                data.emplace(IpcArgs<Args...>::parseData(stream, pos));
                stream.seekg(0, std::ios_base::beg);
                return std::move(data);
            }

        public:
            static const IpcArgData<Args...> parseArgs(const char *buf, std::size_t size) try {
                if (!buf || size == 0 || buf == (const char *)-1) {
                    IPC_THROW("Invalid input data!");
                }

                std::stringstream stream(std::string(buf, size));
                return std::move(parseResult(stream));
            }
            IPC_HANDLE(return {})

            static const IpcArgData<Args...> parseArgs(std::stringstream &stream) try {
                if (stream.str().size() == 0) {
                    IPC_THROW("Invalid input data!");
                }

                return std::move(parseResult(stream));
            }
            IPC_HANDLE(return {});

            static const IpcArgData<Args...> parseArgs(const std::string &str) try {
                if (str.size() == 0) {
                    IPC_THROW("Invalid input data!");
                }

                std::stringstream stream(str);
                return std::move(parseResult(stream));
            }
            IPC_HANDLE(return {});

        public:
            explicit IpcArgs(const char *buf, std::size_t size) :
                m_index(0),
                m_count(utility::TypeArray<Args...>::count),
                m_types(&utility::TypeArray<Args...>::types[0]),
                m_maxIndex(m_count - 1),
                m_data() {
                set(buf, size);
            }

            explicit IpcArgs(const std::string &msg) :
                m_index(0),
                m_count(utility::TypeArray<Args...>::count),
                m_types(&utility::TypeArray<Args...>::types[0]),
                m_maxIndex(m_count - 1),
                m_data() {
                set(msg.data(), msg.size());
            }

            explicit IpcArgs() :
                m_index(0),
                m_count(utility::TypeArray<Args...>::count),
                m_types(&utility::TypeArray<Args...>::types[0]),
                m_maxIndex(m_count - 1),
                m_data() {
                clear();
            }

            ~IpcArgs() {
            }

            template <typename T>
            IpcArgs &append(const T &val) try {
                if (typeid(T).hash_code() != m_types[m_index]->hash_code()) {
                    IPC_THROW("Type is not in order!");
                }

                if (index() == 0) {
                    clear();
                }

                ArgElement ele = {m_index, sizeof(T)};
                m_stream.write(reinterpret_cast<const char *>(&ele), sizeof(ele));
                m_stream.write(reinterpret_cast<const char *>(&val), sizeof(T));

                if (m_index == maxIndex()) {
                    if (this->bin().size() < utility::TypeArray<Args...>::size) {
                        IPC_THROW("Total input argument size is less than total data type size!");
                    }
                    m_data = std::move(IpcArgs<Args...>::parseArgs(m_stream));
                }
                next();
                return *this;
            }
            IPC_HANDLE(return *this);

            template <typename T, typename... Ts>
            IpcArgs &append(const T &arg, Ts... args) {
                append(arg);
                append(args...);
                return *this;
            }

            template <typename T>
            IpcArgs &operator<<(const T &val) { return append(val); }

            IpcArgs &operator<<(const std::string &val) try {
                if (index() == 0) {
                    clear();
                }
                ArgElement ele = {index(), static_cast<int32_t>(val.size())};
                m_stream.write(reinterpret_cast<const char *>(&ele), sizeof(ele));
                m_stream.write(val.c_str(), val.size());

                if (index() == maxIndex()) {
                    if (this->bin().size() < utility::TypeArray<Args...>::size) {
                        IPC_THROW("Total input argument size is less than total data type size!");
                    }
                    m_data = std::move(IpcArgs<Args...>::parseArgs(m_stream));
                    m_stream.seekg(0, std::ios_base::beg);
                }

                next();
                return *this;
            }
            IPC_HANDLE(return *this);

            IpcArgs &operator<<(const char *val) try {
                std::string str(val);
                return (*this << str);
            }
            IPC_HANDLE(return *this);

            template <typename T, typename... Ts>
            IpcArgs &set(const T &arg, Ts... args) {
                clear();
                append(arg, args...);
                return *this;
            }

            /**
             * @fn set()
             * @brief set the data to parse from
             *
             * @param buf   pointer to buffer, each element in buffer seperate by (i32-id, i32-size)
             * @param size  total size of data
             */
            void set(const char *buf, size_t size) try {
                if (!buf || size == 0 || buf == (const char *)-1) {
                    IPC_THROW("Invalid data!\n");
                }
                clear();
                m_stream = std::move(std::stringstream(std::string(buf, size)));
                m_data = std::move(IpcArgs<Args...>::parseArgs(buf, size));
            }
            IPC_HANDLE();

            inline const size_t count() const { return m_count; }
            inline const IpcArgData<Args...> &data() const { return m_data; }
            inline const std::string bin() const { return std::move(m_stream.str()); }
            inline const int32_t index() const { return m_index; }
            void clear() {
                m_index = 0;
                m_data.reset();
                m_stream.str("");
                m_stream.seekg(0, std::ios_base::beg);
            }

        private:
            inline void next() { m_index = ((int)m_index + 1) % m_count; }
            inline int32_t maxIndex() const { return static_cast<int32_t>(m_maxIndex); }

        private:
            std::size_t m_count;
            size_t m_maxIndex;
            int32_t m_index;
            const std::type_info **m_types;
            IpcArgData<Args...> m_data;
            std::stringstream m_stream;
        };
    } // namespace ipc
} // namespace gbs
#endif // __IPC_ARGS_H__