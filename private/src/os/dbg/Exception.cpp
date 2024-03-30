#include <errno.h>
#include "Exception.h"
#include "common/Typedef.h"
#include <stdarg.h>
#include <string.h>

namespace except {
Exception::Exception(const char *fnc, int line) noexcept :
    m_msg(NULL), m_prefix(NULL), m_prefixLen(0) {
    m_msg = new char[EXCEPTION_BUFFER_SIZE + 1];
    m_prefix = m_msg;
    memset(m_prefix, 0, EXCEPTION_BUFFER_SIZE);
    snprintf(m_prefix, EXCEPTION_BUFFER_SIZE, "> Throw at [%s, %d]: ", fnc, line);
    m_prefixLen = strnlen(m_prefix, EXCEPTION_BUFFER_SIZE);
}

Exception::Exception(const Exception &copy) noexcept {
    m_msg = new char[EXCEPTION_BUFFER_SIZE + 1];
    memcpy(m_msg, copy.m_msg, EXCEPTION_BUFFER_SIZE);
}

Exception::Exception(Exception &&move) noexcept {
    m_msg = new char[EXCEPTION_BUFFER_SIZE + 1];
    memcpy(m_msg, move.m_msg, EXCEPTION_BUFFER_SIZE);
}

Exception::~Exception() noexcept {
    delete m_msg;
}

void Exception::raise(const char *fnc, int line, const char *format, ...) const {
    va_list args;
    m_prefix = m_msg;
    memset(m_prefix, 0, EXCEPTION_BUFFER_SIZE);
    snprintf(m_prefix, EXCEPTION_BUFFER_SIZE, "%s, %d", fnc, line);
    m_prefixLen = strnlen(m_prefix, EXCEPTION_BUFFER_SIZE);
    va_start(args, format);
    vsnprintf((m_msg + m_prefixLen), (EXCEPTION_BUFFER_SIZE - m_prefixLen), format, args);
    va_end(args);

    throw *this;
}

void Exception::raise(const char *format, ...) const {
    va_list args;
    va_start(args, format);
    vsnprintf((m_msg + m_prefixLen), (EXCEPTION_BUFFER_SIZE - m_prefixLen), format, args);
    va_end(args);

    throw *this;
}

ExceptionHandler *Exception::clone() const noexcept {
    return new Exception(*this);
}

const char *Exception::what() const noexcept {
    return m_msg;
}

} // namespace except