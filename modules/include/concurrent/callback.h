#ifndef CALLBACK_H
#define CALLBACK_H

#include <iostream>
#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>

namespace ipc::core {

template <typename... Args>
class callback {
public:
    using call_function_t = std::function<void(Args...)>;

private:
    using weak_state_t = std::weak_ptr<void>;
    using shared_state_t = std::shared_ptr<void>;
    struct entry_t {
        weak_state_t state;
        call_function_t callback;
    };

public:
    class connect {
        friend class callback;

        template <typename T>
        explicit connect(std::shared_ptr<T> state) :
            m_state(state) {}

    public:
        connect(const connect &other) :
            m_state(other.m_state) {}

        connect(connect &&other) noexcept :
            m_state(std::move(other.m_state)) {}

        connect &operator=(const connect &other) {
            if (this != &other) {
                m_state = other.m_state;
            }
            return *this;
        }

        connect &operator=(connect &&other) noexcept {
            if (this != &other) {
                m_state = std::move(other.m_state);
            }
            return *this;
        }

        ~connect() = default;

        bool is_valid() const {
            return (m_state != nullptr);
        }

        void reset() {
            m_state.reset();
        }

    private:
        shared_state_t m_state{nullptr};
    };

    connect register_callback(call_function_t func) {
        auto state = std::make_shared<int>(1);
        m_callbacks.emplace_back(entry_t{state, std::move(func)});
        return connect(state);
    }

    void emit(Args... args) {
        m_callbacks.erase(
            std::remove_if(m_callbacks.begin(), m_callbacks.end(), [&](entry_t &entry) {
                if (entry.state.expired()) {
                    return true;
                }
                entry.callback(std::forward<Args>(args)...);
                return false;
            }),
            m_callbacks.end());
    }

    void operator()(Args... args) {
        emit(std::forward<Args>(args)...);
    }

    auto count() const {
        return m_callbacks.size();
    }

private:
    std::vector<entry_t> m_callbacks;
};

} // namespace ipc::core

#endif // CALLBACK_H
