#include "exception/except.h"

namespace ipc::core {

static constexpr size_t EXCEPTION_BUFFER_SIZE = (1024U * 5U);

except::except(const char *fnc, int line) noexcept :
    m_message("") {

    char buffer[EXCEPTION_BUFFER_SIZE + 1];
    if (fnc != nullptr) {
        snprintf(buffer, EXCEPTION_BUFFER_SIZE, "\033[1;31mipc::core::thrown at \033[4m%s:%d\033[0m\n", fnc, line);
    }
    m_message = std::string(buffer);
}

except::~except() noexcept {
}

void except::raise(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[EXCEPTION_BUFFER_SIZE + 1];
    vsnprintf(buffer, EXCEPTION_BUFFER_SIZE, format, args);
    va_end(args);
    m_message.append("\033[1;36m").append(std::string(buffer)).append("\033[0m");
    throw *this;
}
} // namespace ipc::core
