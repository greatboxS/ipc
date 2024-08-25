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
    template <typename Arg>
    class item : public item_p {
        static_assert(std::is_default_constructible<Arg>::value,
                      "Invalid data type! Must not be reference or no default constructor existing");

    public:
        explicit item(const Arg &value) :
            m_value(value) {}

        explicit item(Arg &&value) noexcept(std::is_nothrow_move_constructible<Arg>::value) :
            m_value(std::move(value)) {}

        item(const item &other) = default;

        item(item &&other) noexcept(std::is_nothrow_move_constructible<Arg>::value) = default;

        item &operator=(const item &other) = default;

        item &operator=(item &&other) noexcept(std::is_nothrow_move_assignable<Arg>::value) = default;

        void set(const Arg &value) { m_value = value; }

        Arg get() const { return m_value; }

        Arg &at() { return m_value; }

    private:
        Arg m_value;
    };

    std::unordered_map<Key, item_p *> m_values{};

public:
    virtual ~meta_container() noexcept {
        for (auto &entry : m_values) {
            delete entry.second;
        }
    }

    template <typename _Type>
    meta_container(const Key &key, const _Type &value) :
        m_values{} {
        set<_Type>(key, value);
    }

    meta_container() :
        m_values{} {
    }

    template <typename T>
    meta_container &set(const Key &key, const T &value) {
        auto item = m_values.find(key);
        if (item != m_values.end()) {
            auto holder = dynamic_cast<meta_container::item<T> *>(item->second);
            if (holder) {
                holder->set(value);
            }
        } else {
            m_values[key] = new meta_container::item<T>(value);
        }
        return *this;
    }

    template <typename T>
    std::optional<T> get(const Key &key) const {
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
    T &at(const Key &key) {
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

    auto size() const {
        return m_values.size();
    }
};

using meta_container_i = meta_container<int>;
using meta_container_s = meta_container<std::string>;

} // namespace ipc::core

#endif // __METACONTAINER_H__