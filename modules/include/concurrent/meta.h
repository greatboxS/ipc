/**
 * @file metacontainer.h
 * @brief Defines the `meta_container` class template for storing and managing key-value pairs.
 */

#ifndef METACONTAINER_H
#define METACONTAINER_H

#include <string>
#include <unordered_map>
#include <optional>
#include <type_traits>
#include <memory>
#include <stdint.h>
#include "except.h"

namespace ipc::core {

/**
 * @brief A generic container for storing key-value pairs with runtime type safety.
 *
 * This class template allows for storing values of different types using a key, ensuring type safety
 * and the ability to retrieve values by their original type.
 *
 * @tparam Key Type used as the key in the container (e.g., int, std::string).
 */
template <typename Key>
class meta_container {
private:
    /**
     * @brief Abstract base class for holding values of different types.
     */
    class item_p {
    public:
        virtual ~item_p() = default;
    };

public:
    /**
     * @brief Template class for holding values of a specific type.
     *
     * @tparam Arg Type of the value to hold.
     */
    template <typename Arg>
    class item : public item_p {
    public:
        /**
         * @brief Constructs an item holding a value by reference.
         *
         * @param val The value to hold.
         */
        item(const Arg &val) :
            _value(val) {}

        /**
         * @brief Constructs an item holding a value by moving it.
         *
         * @param val The value to hold.
         */
        item(Arg &&val) :
            _value(std::move(val)) {}

        /**
         * @brief Sets a new value.
         *
         * @param val The new value.
         */
        void set(const Arg &val) { _value = val; }

        /**
         * @brief Sets a new value by moving it.
         *
         * @param val The new value.
         */
        void set(Arg &&val) { _value = std::move(val); }

        /**
         * @brief Retrieves the stored value.
         *
         * @return Reference to the stored value.
         */
        Arg &get() { return _value; }

        /**
         * @brief Retrieves the stored value (const version).
         *
         * @return Const reference to the stored value.
         */
        const Arg &get() const { return _value; }

    private:
        Arg _value; ///< The value stored in the container.
    };

    /**
     * @brief Proxy class for assigning and retrieving values from the container using a key.
     */
    class value_proxy {
    public:
        /**
         * @brief Constructs a proxy for a specific key in the container.
         *
         * @param container Reference to the meta_container.
         * @param key The key associated with the value.
         */
        value_proxy(meta_container &container, const Key &key) :
            _container(container), _key(key) {}

        /**
         * @brief Assigns a value to the key.
         *
         * @tparam T The type of the value to assign.
         * @param value The value to assign.
         * @return Reference to this proxy.
         */
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

        explicit operator int() {
            return _container.get<int>(_key);
        }

        explicit operator unsigned int() {
            return _container.get<unsigned int>(_key);
        }

        explicit operator int64_t() {
            return _container.get<int64_t>(_key);
        }

        explicit operator uint64_t() {
            return _container.get<uint64_t>(_key);
        }

        explicit operator double() {
            return _container.get<double>(_key);
        }

        explicit operator float() {
            return _container.get<float>(_key);
        }

        explicit operator const std::string &() const {
            return _container.data<std::string>(_key);
        }

        explicit operator const char *() const {
            return _container.get<std::string>(_key).c_str();
        }

    private:
        meta_container &_container; ///< Reference to the container.
        Key _key;                   ///< The key associated with the value.
    };

    /**
     * @brief Constructor to initialize the container with a key-value pair.
     *
     * @tparam _Type The type of the value.
     * @param key The key.
     * @param value The value to store.
     */
    template <typename _Type>
    meta_container(const Key &key, const _Type &value) :
        m_values{} {
        set<_Type>(key, value);
    }

    /**
     * @brief Default constructor.
     */
    meta_container() :
        m_values{} {}

    /**
     * @brief Overload of the subscript operator to provide a proxy for accessing and modifying values.
     *
     * @param key The key.
     * @return A value_proxy for the given key.
     */
    value_proxy operator[](const Key &key) {
        return value_proxy(*this, key);
    }

    /**
     * @brief Sets a value for a key.
     *
     * If a value already exists for the key, it is replaced. If the type does not match, an exception is thrown.
     *
     * @tparam T The type of the value.
     * @param key The key.
     * @param value The value to store.
     * @return Reference to the container.
     */
    template <typename T>
    meta_container &set(const Key &key, const T &value) {
        auto item = m_values.find(key);
        if (item != m_values.end()) {
            auto holder = dynamic_cast<meta_container::item<T> *>(item->second.get());
            if (holder == nullptr) {
                ipc_throw_exception("Type mismatch for the key");
            }
            holder->set(value);
        } else {
            m_values[key] = std::unique_ptr<meta_container::item_p>(new meta_container::item<T>(value));
        }
        return *this;
    }

    /**
     * @brief Retrieves the value for a key.
     *
     * Throws an exception if the key is not found or the type does not match.
     *
     * @tparam T The type of the value.
     * @param key The key.
     * @return Reference to the value.
     */
    template <typename T>
    T &get(const Key &key) {
        auto it = m_values.find(key);
        if (it == m_values.end()) {
            ipc_throw_exception("Key not found");
        }
        meta_container::item<T> *holder = dynamic_cast<meta_container::item<T> *>(it->second.get());
        if (holder == nullptr) {
            ipc_throw_exception("Type mismatch for the key");
        }
        return holder->get();
    }

    /**
     * @brief Retrieves the value associated with the given key as an optional.
     *
     * This method attempts to retrieve the value associated with the specified key. If the key is found and
     * the value matches the requested type, the method returns an `std::optional` containing the value. If the
     * key is not found or the type does not match, it returns `std::nullopt`.
     *
     * @tparam T The type of the value to retrieve.
     * @param key The key for which to retrieve the value.
     * @return An `std::optional<T>` containing the value if the key is found and the type matches; otherwise, `std::nullopt`.
     */
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

    /**
     * @brief Retrieves the value associated with the given key as a constant reference.
     *
     * This method retrieves the value associated with the specified key and returns it as a constant reference.
     * Throws an exception if the key is not found or if the type of the value does not match the requested type.
     *
     * @tparam T The type of the value to retrieve.
     * @param key The key for which to retrieve the value.
     * @return A constant reference to the value associated with the key.
     * @throw std::runtime_error if the key is not found or the type does not match.
     */
    template <typename T>
    const T &data(const Key &key) const {
        auto it = m_values.find(key);
        if (it == m_values.end()) {
            ipc_throw_exception("Key not found");
        }
        meta_container::item<T> *holder = dynamic_cast<meta_container::item<T> *>(it->second.get());
        if (holder == nullptr) {
            ipc_throw_exception("Type mismatch for the key");
        }
        return holder->get();
    }

    /**
     * @brief Removes the key-value pair associated with the given key.
     *
     * This method removes the key-value pair associated with the specified key from the container. If the key is
     * not found, the container remains unchanged.
     *
     * @param key The key of the pair to remove.
     */
    void erase(const Key &key) noexcept {
        auto it = m_values.find(key);
        if (it != m_values.end()) {
            m_values.erase(it);
        }
    }

    /**
     * @brief Returns the number of key-value pairs in the container.
     *
     * This method returns the number of key-value pairs currently stored in the container.
     *
     * @return The number of key-value pairs in the container.
     */
    auto size() const {
        return m_values.size();
    }

private:
    std::unordered_map<Key, std::unique_ptr<item_p>> m_values; ///< Internal map for storing key-value pairs.
};

/// Specialization of meta_container for integer keys.
using meta_container_i = meta_container<int>;

/// Specialization of meta_container for string keys.
using meta_container_s = meta_container<std::string>;

} // namespace ipc::core

#endif // METACONTAINER_H
