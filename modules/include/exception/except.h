#ifndef EXCEPT_H
#define EXCEPT_H

#include <exception>
#include <string>
#include <string.h>
#include <stdarg.h>
#include <memory>

#define ipc_throw_exception(...) ipc::core::except::throw_exception(__FILE__, __LINE__, __VA_ARGS__);

namespace ipc::core {

class except_base : std::exception {
protected:
    virtual ~except_base() = default;

public:
    virtual except_base *clone() const noexcept = 0;
};

class except : public except_base {
private:
    std::string m_message = "";

protected:
    explicit except(const char *fnc, int line) noexcept;
    virtual ~except() noexcept;
    except(const except &other) noexcept = default;
    except(except &&other) noexcept = default;
    void raise(const char *format, ...);

public:
    except_base *clone() const noexcept override {
        return new except(*this);
    }

    const char *what() const noexcept override {
        return m_message.c_str();
    }

    template <typename... Args>
    static void throw_exception(const char *function, uint64_t line, Args &&...args) {
        ipc::core::except(function, line).raise(std::forward<Args>(args)...);
    }
};

} // namespace ipc::core

#endif // EXCEPT_H