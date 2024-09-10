#ifndef CONDITION_TRIGGER_H
#define CONDITION_TRIGGER_H

namespace ipc::core {

class condition_trigger {
    condition_trigger(const condition_trigger &) = delete;
    condition_trigger(condition_trigger &&) = delete;
    condition_trigger &operator=(const condition_trigger &) = delete;
    class impl;
    impl *m_impl{nullptr};

public:
    condition_trigger();
    ~condition_trigger();
    void wait();
    bool wait_for(int ms);
    bool triggered() const;
    void trigger();
    void reset();
};
} // namespace ipc::core

#endif // CONDITION_TRIGGER_H