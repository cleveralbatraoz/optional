#pragma once

#include <utility>
#include <variant>

struct nullopt_t
{
};

inline constexpr nullopt_t nullopt;

struct in_place_t
{
};

inline constexpr in_place_t in_place;

template <typename T>
struct optional
{
    constexpr optional() noexcept = default;

    constexpr optional(nullopt_t) noexcept
    {
    }

    constexpr optional(T value)
        : stg(std::move(value))
    {
    }

    constexpr optional(optional const & rhs) = default;

    constexpr optional(optional && rhs) = default;

    optional & operator=(optional const & rhs) = default;

    optional & operator=(optional && rhs) = default;

    optional & operator=(nullopt_t null) noexcept
    {
        this->stg = null;
        return *this;
    }

    template <typename... Args>
    explicit constexpr optional(in_place_t, Args &&... args)
        : stg(std::move(T(std::forward<Args>(args)...)))
    {
    }

    constexpr T & operator*() noexcept
    {
        return std::get<T>(this->stg);
    }

    constexpr T const & operator*() const noexcept
    {
        return std::get<T>(this->stg);
    }

    constexpr T * operator->() noexcept
    {
        return &std::get<T>(this->stg);
    }

    constexpr T const * operator->() const noexcept
    {
        return &std::get<T>(this->stg);
    }

    constexpr explicit operator bool() const noexcept
    {
        return (std::get_if<T>(&stg) != nullptr);
    }

    template <typename... Args>
    void emplace(Args &&... args)
    {
        try {
            stg = std::move(T(std::forward<Args>(args)...));
        }
        catch (...) {
            stg = nullopt;
            throw;
        }
    }

    constexpr void reset()
    {
        this->stg = nullopt;
    }

private:
    std::variant<T, nullopt_t> stg = nullopt;
};

template <typename T>
constexpr bool operator==(optional<T> const & a, optional<T> const & b)
{
    return ((!static_cast<bool>(a) && !static_cast<bool>(b)) ||
            (static_cast<bool>(a) && static_cast<bool>(b) && (*a == *b)));
}

template <typename T>
constexpr bool operator!=(optional<T> const & a, optional<T> const & b)
{
    return !(a == b);
}

template <typename T>
constexpr bool operator<(optional<T> const & a, optional<T> const & b)
{
    return ((static_cast<bool>(a) < static_cast<bool>(b)) ||
            (static_cast<bool>(a) && static_cast<bool>(b) && (*a < *b)));
}

template <typename T>
constexpr bool operator<=(optional<T> const & a, optional<T> const & b)
{
    return (a < b || a == b);
}

template <typename T>
constexpr bool operator>(optional<T> const & a, optional<T> const & b)
{
    return b < a;
}

template <typename T>
constexpr bool operator>=(optional<T> const & a, optional<T> const & b)
{
    return !(a < b);
}
