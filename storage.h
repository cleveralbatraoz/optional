#pragma once

#include <type_traits>
#include <utility>

namespace storage_traits {

template <typename T, bool is_trivial>
struct destructible_base
{
    constexpr destructible_base() noexcept
        : dummy(0)
        , contains_value(false)
    {
    }

    constexpr destructible_base(T value)
        : value(std::move(value))
        , contains_value(true)
    {
    }

    ~destructible_base()
    {
        if (contains_value) {
            value.~T();
        }
    }

protected:
    constexpr void clear_value() noexcept
    {
        this->value.~T();
        this->contains_value = false;
    }

    union
    {
        T value;
        char dummy;
    };

    bool contains_value;
};

template <typename T>
struct destructible_base<T, true>
{
    constexpr destructible_base() noexcept
        : dummy(0)
        , contains_value(false)
    {
    }

    constexpr destructible_base(T value)
        : value(std::move(value))
        , contains_value(true)
    {
    }

    ~destructible_base() = default;

protected:
    constexpr void clear_value() noexcept
    {
        this->contains_value = false;
    }

    union
    {
        T value;
        char dummy;
    };

    bool contains_value;
};

template <typename T, bool is_trivial>
struct copy_constructible_base : protected destructible_base<T, std::is_trivially_destructible_v<T>>
{
protected:
    using base = destructible_base<T, std::is_trivially_destructible_v<T>>;

public:
    using base::base;

    constexpr copy_constructible_base(copy_constructible_base const & rhs)
    {
        this->contains_value = rhs.contains_value;
        if (this->contains_value) {
            new (&this->value) T(rhs.value);
        }
    }
};

template <typename T>
struct copy_constructible_base<T, true> : protected destructible_base<T, std::is_trivially_destructible_v<T>>
{
protected:
    using base = destructible_base<T, std::is_trivially_destructible_v<T>>;

public:
    using base::base;

    constexpr copy_constructible_base() = default;
};

template <typename T, bool is_trivial>
struct copy_assignable_base : protected copy_constructible_base<T, std::is_trivially_copy_constructible_v<T>>
{
protected:
    using base = copy_constructible_base<T, std::is_trivially_copy_constructible_v<T>>;

public:
    using base::base;

    copy_assignable_base & operator=(copy_assignable_base const & rhs)
    {
        if (rhs.contains_value) {
            if (!this->contains_value) {
                new (&this->value) T(rhs.value);
            }
            else if (rhs.contains_value) {
                this->value = rhs.value;
            }

            this->contains_value = true;
        }
        else if (this->contains_value) {
            this->clear_value();
        }

        return *this;
    }
};

template <typename T>
struct copy_assignable_base<T, true> : protected copy_constructible_base<T, std::is_trivially_copy_constructible_v<T>>
{
protected:
    using base = copy_constructible_base<T, std::is_trivially_copy_constructible_v<T>>;

public:
    using base::base;

    constexpr copy_assignable_base & operator=(copy_assignable_base const & rhs) = default;
};

template <typename T, bool is_trivial>
struct move_constructible_base : protected copy_assignable_base<T, std::is_trivially_copy_assignable_v<T>>
{
protected:
    using base = copy_assignable_base<T, std::is_trivially_copy_assignable_v<T>>;

public:
    using base::base;

    constexpr move_constructible_base(move_constructible_base const &) = default;

    constexpr move_constructible_base(move_constructible_base && rhs) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        this->contains_value = rhs.contains_value;
        if (this->contains_value) {
            new (&this->value) T(std::move(rhs.value));
        }
    }

    move_constructible_base & operator=(move_constructible_base const &) = default;
};

template <typename T>
struct move_constructible_base<T, true> : protected copy_assignable_base<T, std::is_trivially_copy_assignable_v<T>>
{
protected:
    using base = copy_assignable_base<T, std::is_trivially_copy_assignable_v<T>>;

public:
    using base::base;

    constexpr move_constructible_base(move_constructible_base const &) = default;

    constexpr move_constructible_base(move_constructible_base &&) = default;

    move_constructible_base & operator=(move_constructible_base const &) = default;
};

template <typename T, bool is_trivial>
struct move_assignable_base : protected move_constructible_base<T, std::is_trivially_move_constructible_v<T>>
{
protected:
    using base = move_constructible_base<T, std::is_trivially_move_constructible_v<T>>;

public:
    using base::base;

    constexpr move_assignable_base(move_assignable_base const &) = default;

    constexpr move_assignable_base(move_assignable_base &&) = default;

    move_assignable_base & operator=(move_assignable_base const &) = default;

    move_assignable_base & operator=(move_assignable_base && rhs) noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>)
    {
        if (rhs.contains_value) {
            if (!this->contains_value) {
                new (&this->value) T(std::move(rhs.value));
            }
            else if (rhs.contains_value) {
                this->value = std::move(rhs.value);
            }

            this->contains_value = true;
        }
        else if (this->contains_value) {
            this->clear_value();
        }

        return *this;
    }
};

template <typename T>
struct move_assignable_base<T, true> : protected move_constructible_base<T, std::is_trivially_move_constructible_v<T>>
{
protected:
    using base = move_constructible_base<T, std::is_trivially_move_constructible_v<T>>;
    using base::base;

    constexpr move_assignable_base(move_assignable_base const &) = default;

    constexpr move_assignable_base(move_assignable_base &&) = default;

    move_assignable_base & operator=(move_assignable_base const &) = default;

    move_assignable_base & operator=(move_assignable_base &&) = default;
};

} // namespace storage_traits

template <typename T>
class storage_t final : storage_traits::move_assignable_base<T, std::is_trivially_move_assignable_v<T>>
{
    using base = storage_traits::move_assignable_base<T, std::is_trivially_move_assignable_v<T>>;

public:
    using base::base;

    constexpr T & get() noexcept
    {
        return this->value;
    }

    constexpr T const & get() const noexcept
    {
        return this->value;
    }

    constexpr void reset() noexcept
    {
        if (!this->empty()) {
            this->clear_value();
        }
    }

    constexpr bool empty() const noexcept
    {
        return !this->contains_value;
    }

    template <typename... Args>
    void emplace(Args &&... args)
    {
        if (this->contains_value) {
            this->clear_value();
        }
        new (&this->value) T(std::forward<Args>(args)...);
        this->contains_value = true;
    }
};
