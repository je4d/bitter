#ifndef BITTER_BIT_HPP
#define BITTER_BIT_HPP
#include <utility>

namespace bitter {
struct bit {
    bit() = default;
    bit(const bit&) = default;
    bit(bit&&) = default;
    bit& operator=(const bit&) = default;
    bit& operator=(bit&&) = default;

    explicit bit(bool bitval) : m_bit(bitval) {}
    explicit operator bool() const { return m_bit; }

    bit& operator&=(bit o) { return (m_bit&=o.m_bit), *this; }
    bit& operator|=(bit o) { return (m_bit|=o.m_bit), *this; }
    bit& operator^=(bit o) { return (m_bit^=o.m_bit), *this; }

    friend bool operator==(bit a, bit b)
    { return a.m_bit == b.m_bit; }
    friend bool operator!=(bit a, bit b)
    { return !(a==b); }

    friend bit operator!(bit a)
    { return bit(!a.m_bit); }
    friend bit operator&(bit a, bit b)
    { return bit(a.m_bit&b.m_bit); }
    friend bit operator|(bit a, bit b)
    { return bit(a.m_bit|b.m_bit); }
    friend bit operator^(bit a, bit b)
    { return bit(a.m_bit^b.m_bit); }

    friend bit operator&&(bit a, bit b)
    { return bit(a.m_bit&&b.m_bit); }
    friend bit operator||(bit a, bit b)
    { return bit(a.m_bit||b.m_bit); }

    friend void swap(bit& a, bit& b)
    { std::swap(a.m_bit, b.m_bit); }
private:
    bool m_bit{false};
};
} // namespace bitter

#endif // BITTER_BIT_HPP
