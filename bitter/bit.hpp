#ifndef BITTER_BIT_HPP
#define BITTER_BIT_HPP
#include <utility>

namespace bitter {
struct bit {
    constexpr bit() noexcept = default;
    constexpr bit(const bit&) noexcept = default;
    constexpr bit(bit&&) noexcept = default;
    bit& operator=(const bit&) noexcept = default;
    bit& operator=(bit&&) noexcept = default;

    constexpr explicit bit(bool bitval) noexcept : m_bit(bitval) {}
    constexpr explicit operator bool() const noexcept { return m_bit; }

    bit& operator&=(bit o) noexcept { return (m_bit&=o.m_bit), *this; }
    bit& operator&=(bool o) noexcept { return (m_bit&=o), *this; }
    bit& operator|=(bit o) noexcept { return (m_bit|=o.m_bit), *this; }
    bit& operator|=(bool o) noexcept { return (m_bit|=o), *this; }
    bit& operator^=(bit o) noexcept { return (m_bit^=o.m_bit), *this; }
    bit& operator^=(bool o) noexcept { return (m_bit^=o), *this; }

    friend constexpr bool operator==(bit a, bit b) noexcept
    { return a.m_bit == b.m_bit; }
    friend constexpr bool operator==(bool a, bit b) noexcept
    { return bit(a) == b; }
    friend constexpr bool operator==(bit a, bool b) noexcept
    { return a == bit(b); }
    friend constexpr bool operator!=(bit a, bit b) noexcept
    { return !(a==b); }
    friend constexpr bool operator!=(bool a, bit b) noexcept
    { return !(a==b); }
    friend constexpr bool operator!=(bit a, bool b) noexcept
    { return !(a==b); }

    friend constexpr bit operator!(bit a) noexcept
    { return bit(!a.m_bit); }

    friend constexpr bit operator&(bit a, bit b) noexcept
    { return bit(a.m_bit&b.m_bit); }
    friend constexpr bit operator&(bool a, bit b) noexcept
    { return bit(a&b.m_bit); }
    friend constexpr bit operator&(bit a, bool b) noexcept
    { return bit(a.m_bit&b); }

    friend constexpr bit operator|(bit a, bit b) noexcept
    { return bit(a.m_bit|b.m_bit); }
    friend constexpr bit operator|(bool a, bit b) noexcept
    { return bit(a|b.m_bit); }
    friend constexpr bit operator|(bit a, bool b) noexcept
    { return bit(a.m_bit|b); }

    friend constexpr bit operator^(bit a, bit b) noexcept
    { return bit(a.m_bit^b.m_bit); }
    friend constexpr bit operator^(bool a, bit b) noexcept
    { return bit(a^b.m_bit); }
    friend constexpr bit operator^(bit a, bool b) noexcept
    { return bit(a.m_bit^b); }

    friend constexpr bit operator&&(bit a, bit b) noexcept
    { return a&b; }
    friend constexpr bit operator&&(bool a, bit b) noexcept
    { return a&b; }
    friend constexpr bit operator&&(bit a, bool b) noexcept
    { return a&b; }

    friend constexpr bit operator||(bit a, bit b) noexcept
    { return a|b; }
    friend constexpr bit operator||(bool a, bit b) noexcept
    { return a|b; }
    friend constexpr bit operator||(bit a, bool b) noexcept
    { return a|b; }

    friend void swap(bit& a, bit& b) noexcept
    { std::swap(a.m_bit, b.m_bit); }
    friend void swap(bit&& a, bit& b) noexcept
    { swap(a, b); }
    friend void swap(bit& a, bit&& b) noexcept
    { swap(a, b); }

private:
    bool m_bit{false};
};

constexpr bool operator==(bit a, bit b) noexcept;
constexpr bool operator==(bool a, bit b) noexcept;
constexpr bool operator==(bit a, bool b) noexcept;
constexpr bool operator!=(bit a, bit b) noexcept;
constexpr bool operator!=(bool a, bit b) noexcept;
constexpr bool operator!=(bit a, bool b) noexcept;
constexpr bit operator!(bit a) noexcept;
constexpr bit operator&(bit a, bit b) noexcept;
constexpr bit operator&(bool a, bit b) noexcept;
constexpr bit operator&(bit a, bool b) noexcept;
constexpr bit operator|(bit a, bit b) noexcept;
constexpr bit operator|(bool a, bit b) noexcept;
constexpr bit operator|(bit a, bool b) noexcept;
constexpr bit operator^(bit a, bit b) noexcept;
constexpr bit operator^(bool a, bit b) noexcept;
constexpr bit operator^(bit a, bool b) noexcept;
constexpr bit operator&&(bit a, bit b) noexcept;
constexpr bit operator&&(bool a, bit b) noexcept;
constexpr bit operator&&(bit a, bool b) noexcept;
constexpr bit operator||(bit a, bit b) noexcept;
constexpr bit operator||(bool a, bit b) noexcept;
constexpr bit operator||(bit a, bool b) noexcept;
void swap(bit& a, bit& b) noexcept;
void swap(bit&& a, bit& b) noexcept;
void swap(bit& a, bit&& b) noexcept;
} // namespace bitter

#endif // BITTER_BIT_HPP
