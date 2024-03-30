#pragma once
#include <exception>
#include <string>
#include "common/Typedef.h"

#define EXCEPTION_BUFFER_SIZE 2048

#define _EXCEPT_THROW(...)     except::Exception(__FUNCTION__, __LINE__).raise(__VA_ARGS__);
#define _EXCEPT_HANDLE(except) fprintf(stderr, "%s", except.what());
#define _EXCEPT_CATCH(...)                \
    catch (except::Exception ex) {        \
        fprintf(stderr, "%s", ex.what()); \
        __VA_ARGS__;                      \
    }                                     \
    catch (std::exception & ex) {         \
        fprintf(stderr, "%s", ex.what()); \
        __VA_ARGS__;                      \
    }

namespace except
{
    class ExceptionHandler : std::exception
    {
    protected:
        virtual void raise(const char *func, int line, const char *format, ...) const = 0;
        virtual void raise(const char *format, ...) const = 0;
        virtual ExceptionHandler *clone() const noexcept = 0;
    };

    class __DLL_DECLSPEC__ Exception : public ExceptionHandler
    {
    private:
        mutable char *m_prefix;
        mutable int m_prefixLen;
        mutable char *m_msg;

    public:
        explicit Exception(const char *fnc, int line) noexcept;
        Exception(const Exception &copy) noexcept;
        Exception(Exception &&move) noexcept;

        ~Exception() noexcept;

        void raise(const char *func, int line, const char *format, ...) const override;
        void raise(const char *format, ...) const override;

        ExceptionHandler *clone() const noexcept override;
        const char *what() const noexcept override;
    };

}; // namespace except