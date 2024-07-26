#ifndef MESG_ARGS_H
#define MESG_ARGS_H

#include <iostream>
#include <sstream>
#include <tuple>
#include <typeinfo>
#include <vector>
#include "concurrent/mesg.h"

namespace ipc::core {

constexpr static int32_t max_str_len = 1024 * 10;

template <typename... Args>
struct arg_array {
};

template <typename T, typename... Args>
struct arg_array<T, Args...> {
    static constexpr std::size_t count = sizeof...(Args) + 1;
    static constexpr std::size_t size = sizeof(T) + arg_array<Args...>::size;
    static const char *names[count];
    static const std::type_info *types[count];
};

template <>
struct arg_array<> {
    static constexpr std::size_t count = 0;
    static constexpr std::size_t size = 0;
};

template <typename T, typename... Args>
const char *arg_array<T, Args...>::names[arg_array<T, Args...>::count] = {typeid(T).name(), typeid(Args).name()...};

template <typename T, typename... Args>
const std::type_info *arg_array<T, Args...>::types[arg_array<T, Args...>::count] = {&typeid(T), &typeid(Args)...};

struct arg_element {
    int32_t id;
    int32_t size;
};

template <typename T>
struct parser {
    static T parse_element(std::stringstream &stream, const std::vector<std::streampos> &pos, std::size_t index) {
        static arg_element ele;
        T value;
        std::streampos position = pos[index];

        if (index >= pos.size() || stream.eof()) {
            throw std::out_of_range("Invalid index");
        }

        if (!stream.seekg(position, std::ios_base::beg)) {
            throw std::runtime_error("Failed to seek to position");
        }

        if (!stream.read(reinterpret_cast<char *>(&ele), sizeof(arg_element))) {
            throw std::runtime_error("Failed to read arg_element");
        }

        if (!stream.read(reinterpret_cast<char *>(&value), sizeof(T))) {
            throw std::runtime_error("Failed to read value");
        }
        return std::move(value);
    }
};

template <>
inline std::string parser<std::string>::parse_element(std::stringstream &stream, const std::vector<std::streampos> &pos, std::size_t index) {
    static arg_element ele;
    std::string str;
    std::streampos position = pos[index];

    if (index >= pos.size() || stream.eof()) {
        throw std::out_of_range("Invalid index");
    }

    if (!stream.seekg(position, std::ios_base::beg)) {
        throw std::runtime_error("Failed to seek to position");
    }

    if (!stream.read(reinterpret_cast<char *>(&ele), sizeof(arg_element))) {
        throw std::runtime_error("Failed to read arg_element");
    }

    str.resize(ele.size);
    stream.read(str.data(), ele.size);
    return std::move(str);
}

template <>
inline const char *parser<const char *>::parse_element(std::stringstream &stream, const std::vector<std::streampos> &pos, std::size_t index) {
    static arg_element ele = {0, 0};
    char *buf = nullptr;
    std::streampos position = pos[index];

    if (index >= pos.size() || stream.eof()) {
        throw std::out_of_range("Invalid index");
    }

    if (!stream.seekg(position, std::ios_base::beg)) {
        throw std::runtime_error("Failed to seek to position");
    }

    if (!stream.read(reinterpret_cast<char *>(&ele), sizeof(arg_element))) {
        throw std::runtime_error("Failed to read arg_element");
    }

    buf = new char[ele.size];
    stream.read(buf, ele.size);
    return buf;
}

template <typename... Args>
class message_args {
    template <typename... _Args>
    class arg_data {
    private:
        bool valid;
        std::tuple<_Args...> data;

    public:
        arg_data() :
            valid(false) {}
        ~arg_data() {}
        bool has_value() const { return valid; }
        bool reset() { return valid = false; }
        void emplace(std::tuple<_Args...> &&args) {
            data = std::move(args);
            valid = true;
        }
        const auto get() const { return data; }
    };

    template <std::size_t... _Size>
    static auto parse_data(std::stringstream &stream, const std::vector<std::streampos> &pos, std::index_sequence<_Size...>) {
        return std::move(std::make_tuple(parser<Args>::parse_element(stream, pos, _Size)...));
    }

    static auto parse_data(std::stringstream &stream, const std::vector<std::streampos> &pos) {
        return std::move(message_args<Args...>::parse_data(stream, pos, std::index_sequence_for<Args...>{}));
    }

    static const arg_data<Args...> parse_args_handle(std::stringstream &stream) {
        static arg_element ele;
        arg_data<Args...> data;
        std::vector<std::streampos> pos;
        int count = static_cast<int>(sizeof...(Args));

        stream.seekg(0, std::ios_base::beg);
        while (!stream.eof()) {
            if (--count < 0) break;
            pos.push_back(stream.tellg());
            stream.read(reinterpret_cast<char *>(&ele), sizeof(arg_element));
            stream.seekg(ele.size, std::ios_base::cur);
        }
        data.emplace(message_args<Args...>::parse_data(stream, pos));
        stream.seekg(0, std::ios_base::beg);
        return std::move(data);
    }

public:
    static const arg_data<Args...> parse_args(const char *buf, std::size_t size) try {
        if (!buf || size == 0 || buf == (const char *)-1) {
            throw std::runtime_error("Invalid input data!");
        }

        std::stringstream stream(std::string(buf, size));
        return std::move(parse_args_handle(stream));
    } catch (...) { /*Do nothing */
        return {};
    }

    static const arg_data<Args...> parse_args(std::stringstream &stream) try {
        if (stream.str().size() == 0) {
            throw std::runtime_error("Invalid input data!");
        }

        return std::move(parse_args_handle(stream));
    } catch (...) { /*Do nothing */
        return {};
    }

    static const arg_data<Args...> parse_args(const std::string &str) try {
        if (str.size() == 0) {
            throw std::runtime_error("Invalid input data!");
        }

        std::stringstream stream(str);
        return std::move(parse_args_handle(stream));
    } catch (...) { /*Do nothing */
    }

public:
    explicit message_args(const char *buf, std::size_t size) :
        m_index(0),
        m_count(arg_array<Args...>::count),
        m_types(&arg_array<Args...>::types[0]),
        m_max_index(m_count - 1),
        m_data() {
        set(buf, size);
    }

    explicit message_args(const std::string &msg) :
        m_index(0),
        m_count(arg_array<Args...>::count),
        m_types(&arg_array<Args...>::types[0]),
        m_max_index(m_count - 1),
        m_data() {
        set(msg.data(), msg.size());
    }

    explicit message_args() :
        m_index(0),
        m_count(arg_array<Args...>::count),
        m_types(&arg_array<Args...>::types[0]),
        m_max_index(m_count - 1),
        m_data() {
        clear();
    }

    ~message_args() {
    }

    template <typename T>
    message_args &append(const T &val) try {
        if (typeid(T).hash_code() != m_types[m_index]->hash_code()) {
            throw std::runtime_error("Type is not in order!");
        }

        if (index() == 0) {
            clear();
        }

        arg_element ele = {m_index, sizeof(T)};
        m_stream.write(reinterpret_cast<const char *>(&ele), sizeof(ele));
        m_stream.write(reinterpret_cast<const char *>(&val), sizeof(T));

        if (m_index == max_index()) {
            if (this->bin().size() < arg_array<Args...>::size) {
                throw std::runtime_error("Total input argument size is less than total data type size!");
            }
            m_data = std::move(message_args<Args...>::parse_args(m_stream));
        }
        next();
        return *this;
    } catch (...) { /*Do nothing */
        return *this;
    }

    template <typename T, typename... Ts>
    message_args &append(const T &arg, Ts... args) {
        append(arg);
        append(args...);
        return *this;
    }

    template <typename T>
    message_args &operator<<(const T &val) { return append(val); }

    message_args &operator<<(const std::string &val) try {
        if (index() == 0) {
            clear();
        }
        arg_element ele = {index(), static_cast<int32_t>(val.size())};
        m_stream.write(reinterpret_cast<const char *>(&ele), sizeof(ele));
        m_stream.write(val.c_str(), val.size());

        if (index() == max_index()) {
            if (this->bin().size() < arg_array<Args...>::size) {
                throw std::runtime_error("Total input argument size is less than total data type size!");
            }
            m_data = std::move(message_args<Args...>::parse_args(m_stream));
            m_stream.seekg(0, std::ios_base::beg);
        }

        next();
        return *this;
    } catch (...) { /*Do nothing */
        return *this;
    }

    message_args &operator<<(const char *val) try {
        std::string str(val);
        return (*this << str);
    } catch (...) { /*Do nothing */
        return *this;
    }

    template <typename T, typename... Ts>
    message_args &set(const T &arg, Ts... args) {
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
            throw std::runtime_error("Invalid data!\n");
        }
        clear();
        m_stream = std::move(std::stringstream(std::string(buf, size)));
        m_data = std::move(message_args<Args...>::parse_args(buf, size));
    } catch (...) { /*Do nothing */
    }

    inline const size_t count() const { return m_count; }

    inline const arg_data<Args...> &data() const { return m_data; }

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
    inline int32_t max_index() const { return static_cast<int32_t>(m_max_index); }

private:
    std::size_t m_count = 0;
    size_t m_max_index = 0;
    int32_t m_index = 0;
    const std::type_info **m_types;
    arg_data<Args...> m_data = {};
    std::stringstream m_stream = {};
};

} // namespace ipc::core

#endif // MESG_ARGS_H