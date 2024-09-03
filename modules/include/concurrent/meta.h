#ifndef __METACONTAINER_H__
#define __METACONTAINER_H__

#include <string>
#include <unordered_map>
#include <optional>
#include <type_traits>
#include <memory>

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
    public:
        item(const Arg &val) :
            _value(val) {}

        item(Arg &&val) :
            _value(std::move(val)) {}

        void set(const Arg &val) { _value = val; }

        void set(Arg &&val) { _value = std::move(val); }

        Arg &get() { return _value; }

        const Arg &get() const { return _value; }

    private:
        Arg _value;
    };

    class value_proxy {
    public:
        value_proxy(meta_container &container, const Key &key) :
            _container(container), _key(key) {}

        template <typename T>
        value_proxy &operator=(T &&value) {
            _container.set<T>(_key, std::forward<T>(value));
            return *this;
        }

        template <typename T>
        value_proxy &operator=(const T &value) {
            _container.set<T>(_key, value);
            return *this;
        }

        template <typename T>
        operator T &() {
            return _container.get<T>(_key);
        }

    private:
        meta_container &_container;
        Key _key;
    };

    value_proxy operator[](const Key &key) {
        return value_proxy(*this, key);
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
            auto holder = dynamic_cast<meta_container::item<T> *>(item->second.get());
            if (holder != nullptr) {
                holder->set(value);
            }
        } else {
            m_values[key] = std::unique_ptr<meta_container::item_p>(new meta_container::item<T>(value));
        }
        return *this;
    }

    template <typename T>
    T &get(const Key &key) {
        auto it = m_values.find(key);
        if (it == m_values.end()) {
            throw std::runtime_error("Key not found");
        }
        meta_container::item<T> *holder = dynamic_cast<meta_container::item<T> *>(it->second.get());
        if (holder == nullptr) {
            throw std::runtime_error("Type mismatch for the key");
        }
        return holder->get();
    }

    template <typename T>
    std::optional<T> get(const Key &key) const {
        auto it = m_values.find(key);
        if (it != m_values.end()) {
            meta_container::item<T> *holder = dynamic_cast<meta_container::item<T> *>(it->second.get());
            if (holder != nullptr) {
                return std::optional<T>(std::in_place, holder->get());
            }
        }
        return std::nullopt;
    }

    template <typename T>
    const T &data(const Key &key) const {
        auto it = m_values.find(key);
        if (it == m_values.end()) {
            throw std::runtime_error("Key not found");
        }
        meta_container::item<T> *holder = dynamic_cast<meta_container::item<T> *>(it->second.get());
        if (holder == nullptr) {
            throw std::runtime_error("Type mismatch for the key");
        }
        return holder->get();
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

private:
    std::unordered_map<Key, std::unique_ptr<item_p>> m_values;
};

using meta_container_i = meta_container<int>;
using meta_container_s = meta_container<std::string>;

} // namespace ipc::core

#endif // __METACONTAINER_H__