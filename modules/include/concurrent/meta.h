#ifndef __METACONTAINER_H__
#define __METACONTAINER_H__

#include <string>
#include <unordered_map>
#include <optional>
#include <type_traits>

namespace ipc::core {
template <typename Key>
class meta_container {
private:
    class item_p {
    public:
        virtual ~item_p() = default;
    };

public:
    template <typename T>
    class item : public item_p {
        static_assert(std::is_default_constructible<T>::value,
                      "Invalid data type! Must not be reference or no default constructor existing");

    public:
        explicit item(const T &value) :
            m_value(value) {}

        explicit item(T &&value) noexcept(std::is_nothrow_move_constructible<T>::value) :
            m_value(std::move(value)) {}

        item(const item &other) = default;

        item(item &&other) noexcept(std::is_nothrow_move_constructible<T>::value) = default;

        item &operator=(const item &other) = default;

        item &operator=(item &&other) noexcept(std::is_nothrow_move_assignable<T>::value) = default;

        void set(const T &value) { m_value = value; }

        T get() const { return m_value; }

        T &at() { return m_value; }

    private:
        T m_value;
    };

public:
    virtual ~meta_container() noexcept {
        for (auto &entry : m_values) {
            delete entry.second;
        }
    }

    template <typename T>
    meta_container &set(const Key key, const T &value) {
        auto item = m_values.find(key);
        if (item != m_values.end()) {
            auto holder = dynamic_cast<item<T> *>(item->second);
            if (holder) {
                holder->set(value);
            }
        } else {
            m_values[key] = new item<T>(value);
        }
        return *this;
    }

    template <typename T>
    std::optional<T> get(const Key key) const {
        auto it = m_values.find(key);
        if (it != m_values.end()) {
            item<T> *holder = dynamic_cast<item<T> *>(it->second);
            if (holder) {
                return std::optional<T>(std::in_place, holder->get());
            }
        }
        return {};
    }

    template <typename T>
    T &at(const Key key) {
        auto it = m_values.find(key);
        if (it != m_values.end()) {
            item<T> *holder = dynamic_cast<item<T> *>(it->second);
            if (holder) {
                return holder->at();
            }
        }
        throw std::invalid_argument("No data found");
    }

    void erase(const Key &key) noexcept {
        auto it = m_values.find(key);
        if (it != m_values.end()) {
            delete it->second;
            m_values.erase(it);
        }
    }
};

using meta_container_i = meta_container<int>;
using meta_container_s = meta_container<std::string>;

} // namespace ipc::core

#endif // __METACONTAINER_H__