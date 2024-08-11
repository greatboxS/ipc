#ifndef ID_PROVIDER_H
#define ID_PROVIDER_H

#include <mutex>

namespace ipc::core {
enum class id_provider_type {
    EventLoop,
    Worker,
    Message,
    Max,
};
template <id_provider_type _Type>
int get_new_id() {
    struct id_wrap_t {
        std::mutex mtx;
        int id;
    };
    static constexpr size_t max = static_cast<size_t>(id_provider_type::Max) + 1U;
    static id_wrap_t m_ids[max] = {
        {{}, 0},
        {{}, 0},
        {{}, 0},
        {{}, 0},
    };

    id_wrap_t &wrap = m_ids[static_cast<size_t>(_Type)];
    std::lock_guard<std::mutex> lock(wrap.mtx);
    wrap.id += 1;
    return wrap.id;
}
} // namespace ipc::core

#endif // ID_PROVIDER_H