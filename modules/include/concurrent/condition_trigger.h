#ifndef CONDITION_TRIGGER_H
#define CONDITION_TRIGGER_H

#include <memory>

namespace ipc::core {

class condition_trigger {
    condition_trigger(const condition_trigger &) = delete;
    condition_trigger(condition_trigger &&) = delete;
    condition_trigger &operator=(const condition_trigger &) = delete;
    class impl;
    impl *m_impl{nullptr};

public:
    condition_trigger(int timeout = __INT_MAX__);
    ~condition_trigger();
    void wait();
    bool wait_for(int ms);
    bool triggered() const;
    void trigger();
    void reset();
};

using trigger_ptr = std::shared_ptr<condition_trigger>;

static inline trigger_ptr make_trigger(int ms) {
    return std::make_shared<condition_trigger>(ms);
}

} // namespace ipc::core

#endif // CONDITION_TRIGGER_H