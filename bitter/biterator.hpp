#ifndef BITTER_BITERATOR_HPP
#define BITTER_BITERATOR_HPP

#include "bit.hpp"
#include "bit_order.hpp"

#include <cstddef>
#include <utility>

namespace bitter {

namespace detail {

template <bit_order BO>
constexpr std::uint8_t bitidx(std::uint8_t bitno)
{
    return (BO == bit_order::lsb0) ? bitno : (7-bitno);
}

struct bitref
{
    bitref() noexcept = default;
    bitref(char* c, std::uint8_t bitidx) noexcept : byte{c}, bitidx{bitidx} {}

    bitref& operator=(bit b) noexcept
    {
        bool bb(b);
        *byte = (*byte & ~(1<<bitidx)) | (bb<<bitidx);
        return *this;
    }

    operator bit() const noexcept
    {
        return bit(*byte & (1<<bitidx));
    }
private:
    char* byte{nullptr};
    std::uint8_t bitidx{0}; /// bitidx = n to represent the nth least
                            /// significant bit.
                            /// To represent the first bit in the part of a
                            /// message held by this byte, an LSB0 iterator will
                            /// set bitidx to 0, and an MSB0 iterator will set
                            /// bitidx to 7.
};

struct const_bitptr
{
    const bit* operator->() const { return &b; }
private:
    bit b;
};

struct bitptr
{
    bit* operator->() { return &b; }
    const bit* operator->() const { return &b; }
private:
    bit b;
};

struct biterator_base : std::iterator<std::random_access_iterator_tag,bit>
{
    constexpr biterator_base() noexcept = default;
    constexpr biterator_base(char* byte, std::uint8_t bitno) noexcept :
        byte{byte},
        bitno{bitno}
    {}

    void bump_up()
    {
        if (++bitno == 8)
            bitno = (++byte,0);
    }

    void bump_down()
    {
        if (!--bitno)
            bitno = (--byte,7);
    }

    void adjust(std::ptrdiff_t delta)
    {
        auto div = std::div(delta+bitno, ptrdiff_t(8));
        if (div.rem < 0) {
            div.quot -= 1;
            div.rem += 8;
        }
        byte += div.quot;
        bitno = div.rem;
    }

    friend bool operator==(biterator_base a, biterator_base b) noexcept
    { return a.byte == b.byte and a.bitno == b.bitno; }
    friend bool operator!=(biterator_base a, biterator_base b) noexcept
    { return !(a==b); }
    friend bool operator<(biterator_base a, biterator_base b) noexcept
    { return (a.byte == b.byte) ? (a.bitno < b.bitno) : (a.byte < b.byte); }
    friend bool operator<=(biterator_base a, biterator_base b) noexcept
    { return !(b < a); }
    friend bool operator>(biterator_base a, biterator_base b) noexcept
    { return b < a; }
    friend bool operator>=(biterator_base a, biterator_base b) noexcept
    { return !(a < b); }

    friend void swap(biterator_base& a, biterator_base& b) noexcept
    { std::swap(a.byte, b.byte); std::swap(a.bitno, b.bitno); }
    friend void swap(biterator_base& a, biterator_base*& b) noexcept
    { swap(a, b); }
    friend void swap(biterator_base&& a, biterator_base& b) noexcept
    { swap(a, b); }

    char* byte{0};
    std::uint8_t bitno{0};
};

template <typename T>
struct biterator_impl : biterator_base
{
    constexpr biterator_impl() noexcept = default;
    constexpr biterator_impl(char* byte, std::uint8_t bitno) noexcept :
        biterator_base{byte, bitno}
    {}

    T& operator++() noexcept
    { bump_up(); return static_cast<T&>(*this); }
    T operator++(int) noexcept
    { T ret(*this); ++*this; return ret; }
    T& operator+=(std::ptrdiff_t n) noexcept
    { adjust(n); return static_cast<T&>(*this); }

    T& operator--() noexcept
    { bump_down(); return static_cast<T&>(*this); }
    T operator--(int) noexcept
    { T ret(*this); --*this; return ret; }
    T& operator-=(std::ptrdiff_t n) noexcept
    { adjust(-n); return static_cast<T&>(*this); }

    friend std::ptrdiff_t operator-(T a, T b) noexcept
    { return (a.byte - b.byte)*8
            +static_cast<int8_t>(a.bitno)
            -static_cast<int8_t>(b.bitno); }

    friend T operator+(T i, std::ptrdiff_t n) noexcept
    { i += n; return i; }
    friend T operator+(std::ptrdiff_t n, T i) noexcept
    { i += n; return i; }
    friend T operator-(T i, std::ptrdiff_t n) noexcept
    { i -= n; return i; }
    friend T operator-(std::ptrdiff_t n, T i) noexcept
    { i -= n; return i; }
};

} // namespace detail

template <bit_order BO>
struct biterator : detail::biterator_impl<biterator<BO>>
{
    using base      = detail::biterator_impl<biterator<BO>>;
    using reference = detail::bitref;
    using pointer   = detail::bitptr;
    using iterator  = biterator;

    constexpr biterator() noexcept
    {}

    biterator(char* byte, std::uint8_t bitno) :
        base{byte, bitno}
    {}

    reference operator*() const noexcept
    { return reference(this->byte, detail::bitidx<BO>(this->bitno)); }

    reference operator[](std::size_t n) const noexcept
    { return *(*this+n); }
};

template <bit_order BO>
struct const_biterator : detail::biterator_impl<const_biterator<BO>>
{
    using base            = detail::biterator_impl<const_biterator<BO>>;
    using reference       = bit;
    using const_reference = bit;
    using pointer         = detail::const_bitptr;
    using const_pointer   = detail::const_bitptr;
    using iterator        = const_biterator;
    using const_iterator  = const_biterator;

    constexpr const_biterator() noexcept = default;

    const_biterator(const char* byte, std::uint8_t bitno) noexcept :
        base{const_cast<char*>(byte), bitno}
    {}

    const_reference operator*() const noexcept
    { return detail::bitref(this->byte, detail::bitidx<BO>(this->bitno)); }

    const_reference operator[](std::size_t n) const noexcept
    { return *(*this+n); }
};

} // namespace bitter

#endif // BITTER_BITERATOR_HPP
