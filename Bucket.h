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

template<typename Key, typename T>
class Bucket
{
private:
    using value_type = std::pair<const Key, T>;
    using mapped_type = T;

    BucketState _state = BucketState::EMPTY;
    alignas(value_type) unsigned char _storage[sizeof(value_type)];

    value_type* ptr() noexcept
    {
        return std::launder (reinterpret_cast<value_type*>(&_storage));
    }

    const value_type* ptr() const noexcept
    {
        return std::launder(reinterpret_cast<const value_type*>(&_storage));
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

    Bucket(const Bucket&) = delete;
    Bucket& operator=(const Bucket&) = delete;

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

    template<typename... Args>
    void make_occupied(Args&&... args)
    {
        destroy_value();
        new (&_storage) value_type(std::forward<Args>(args)...);
        _state = BucketState::OCCUPIED;
    }

    void make_occupied(value_type&& value)
    {
        destroy_value();
        new (&_storage) value_type(std::move(value));
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

    void clear() noexcept
    {
        make_empty();
    }

    [[nodiscard]] bool is_empty() const noexcept { return _state == BucketState::EMPTY; }
    [[nodiscard]] bool is_occupied() const noexcept { return _state == BucketState::OCCUPIED; }
    [[nodiscard]] bool is_deleted() const noexcept { return _state == BucketState::DELETED; }

    [[nodiscard]] BucketState state() const noexcept { return _state; }

    const Key& key() const noexcept { return ptr()->first; }

    mapped_type& get_mapped() noexcept { return ptr()->second; }
    const mapped_type& get_mapped() const noexcept { return ptr()->second; }

    [[nodiscard]] value_type& value() noexcept { return *ptr(); }
    [[nodiscard]] const value_type& value() const noexcept { return *ptr(); }

    [[nodiscard]] value_type& value_ref() noexcept { return *ptr(); }
    [[nodiscard]] const value_type& value_ref() const noexcept { return *ptr(); }

private:
    void destroy_value() noexcept
    {
        if (_state == BucketState::OCCUPIED)
            ptr()->~value_type();
    }
};
