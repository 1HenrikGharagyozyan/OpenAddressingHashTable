#pragma once

#include <cstddef>
#include <utility>
#include <type_traits>

enum class BucketState
{
    EMPTY,
    OCCUPIED,
    DELETED
};

template<typename Key, typename Value>
class Bucket
{
private:
    using value_type = std::conditional_t<std::is_same_v<Key, Value>, Key, std::pair<const Key, Value>>;
    using mapped_type = Value;

    BucketState _state = BucketState::EMPTY;
    alignas(value_type) unsigned char _storage[sizeof(value_type)];

    value_type* ptr() noexcept
    {
        return reinterpret_cast<value_type*>(&_storage);
    }

    const value_type* ptr() const noexcept
    {
        return reinterpret_cast<const value_type*>(&_storage);
    }

public:
    Bucket() noexcept
        : _state(BucketState::EMPTY)
    {
    }

    ~Bucket()
    {
        destroy_value();
    }

    void set(const value_type& value)
    {
        destroy_value();
        new (&_storage) value_type(value);
        _state = BucketState::OCCUPIED;
    }

    void set(value_type&& value)
    {
        destroy_value();
        new (&_storage) value_type(std::move(value));
        _state = BucketState::OCCUPIED;
    }

    const Key& key() const
    {
        if constexpr (std::is_same_v<Key, Value>)
            return *ptr();
        else
            return ptr()->first;
    }

    void clear()
    {
        destroy_value();
        _state = BucketState::EMPTY;
    }

    mapped_type& get_mapped() noexcept
    {
        if constexpr (std::is_same_v<Key, Value>)
            return *ptr();
        else
            return ptr()->second;
    }

    const mapped_type& get_mapped() const noexcept
    {
        if constexpr (std::is_same_v<Key, Value>)
            return *ptr();
        else
            return ptr()->second;
    }

    template<typename... Args>
    void make_occupied(Args&&... args)
    {
        destroy_value();
        new (&_storage) value_type(std::forward<Args>(args)...);
        _state = BucketState::OCCUPIED;
    }

    void make_empty() noexcept
    {
        destroy_value();
        _state = BucketState::EMPTY;
    }

    void make_deleted() noexcept
    {
        destroy_value();
        _state = BucketState::DELETED;
    }

    [[nodiscard]] bool is_empty() const noexcept { return _state == BucketState::EMPTY; }
    [[nodiscard]] bool is_occupied() const noexcept { return _state == BucketState::OCCUPIED; }
    [[nodiscard]] bool is_deleted() const noexcept { return _state == BucketState::DELETED; }

    [[nodiscard]] BucketState state() const noexcept { return _state; }

    [[nodiscard]] value_type& value() noexcept { return *ptr(); }
    [[nodiscard]] const value_type& value() const noexcept { return *ptr(); }

    [[nodiscard]] value_type& value_ref() noexcept { return *ptr(); }
    [[nodiscard]] const value_type& value_ref() const noexcept { return *ptr(); }

private:
    void destroy_value() noexcept
    {
        if (_state == BucketState::OCCUPIED)
        {
            ptr()->~value_type();
        }
    }
};
