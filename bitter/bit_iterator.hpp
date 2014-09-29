#ifndef BITTER_BIT_ITERATOR_HPP
#define BITTER_BIT_ITERATOR_HPP

#include "bit.hpp"
#include "bit_order.hpp"
#include "byte_order.hpp"

#include <cstddef>
#include <utility>

namespace bitter {

namespace detail {

using std::uint8_t;

// byteidx, bitidx
//
// if an integer 't' is interpreted as a string of bits, what is the
// significance of the nth bit of the bitstring in t?
//
// e.g. in a uint64_t on a big endian architecture with msb0 bit ordering,
// where is bit #5?
// The bit #5 is in the first byte stored, which is the most significant byte
// in big-endian numbers. So it's in the most significant byte, i.e. its
// significance is between 2^56 and 2^63.
// The most significant bit within that is bit 0, so bit #5 is the 6th most
// significant bit. the most significant end of our range is 2^63, so counting
// back we get a significance of 2^58, which gives us a 'bit index' of 58.
//

template <byte_order EI>
constexpr std::uint8_t byteidx(std::uint8_t byteno, std::uint8_t bytes)
{
    return (EI == byte_order::lsb0 or EI == byte_order::none)
        ? byteno
        :  (EI == byte_order::msb0)
            ? (bytes - 1) - byteno
            :  (EI == byte_order::pdp_endian)
                ? (bytes == 4)
                    ? (byteno < (bytes/2))
                        ? byteidx<byte_order::lsb0>(byteno, bytes/2) + bytes/2
                        : byteidx<byte_order::lsb0>(byteno-bytes/2, bytes/2)
                    : byteidx<byte_order::lsb0>(byteno, bytes)
                :  throw "byteidx needs a valid byte_order";
}

template <typename UL, byte_order EI>
constexpr std::uint8_t byteidx(std::uint8_t byteno)
{
    return byteidx<EI>(byteno, sizeof(UL));
}

template <bit_order BO, typename UL, byte_order EI>
constexpr std::uint8_t bitidx(std::uint8_t bitno)
{
    return 8*byteidx<UL,EI>(bitno/8)
        + ((BO == bit_order::lsb0) ? (bitno%8) : (7-(bitno%8)));
}

struct const_bitptr
{
    explicit const_bitptr(bit b) : b{b} {}
    const bit* operator->() const { return &b; }
private:
    bit b;
};

struct bitptr
{
    explicit bitptr(bit b) : b{b} {}
    bit* operator->() { return &b; }
    const bit* operator->() const { return &b; }
private:
    bit b;
};

struct bit_iterator_base : std::iterator<std::random_access_iterator_tag,bit>
{
    constexpr bit_iterator_base() noexcept = default;
    constexpr bit_iterator_base(uint8_t* byte, uint8_t bitno) noexcept :
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
        if (bitno-- == 0)
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

    friend bool operator==(bit_iterator_base a, bit_iterator_base b) noexcept
    { return a.byte == b.byte and a.bitno == b.bitno; }
    friend bool operator!=(bit_iterator_base a, bit_iterator_base b) noexcept
    { return !(a==b); }
    friend bool operator<(bit_iterator_base a, bit_iterator_base b) noexcept
    { return (a.byte == b.byte) ? (a.bitno < b.bitno) : (a.byte < b.byte); }
    friend bool operator<=(bit_iterator_base a, bit_iterator_base b) noexcept
    { return !(b < a); }
    friend bool operator>(bit_iterator_base a, bit_iterator_base b) noexcept
    { return b < a; }
    friend bool operator>=(bit_iterator_base a, bit_iterator_base b) noexcept
    { return !(a < b); }

    friend void swap(bit_iterator_base& a, bit_iterator_base& b) noexcept
    { std::swap(a.byte, b.byte); std::swap(a.bitno, b.bitno); }
    friend void swap(bit_iterator_base& a, bit_iterator_base&& b) noexcept
    { swap(a, b); }
    friend void swap(bit_iterator_base&& a, bit_iterator_base& b) noexcept
    { swap(a, b); }

    uint8_t* byte{0};
    uint8_t bitno{0};
};

template <typename T>
struct bit_iterator_impl : bit_iterator_base
{
    constexpr bit_iterator_impl() noexcept = default;
    constexpr bit_iterator_impl(uint8_t* byte, uint8_t bitno) noexcept :
        bit_iterator_base{byte, bitno}
    {}

    T& operator++() noexcept
    { bump_up(); return static_cast<T&>(*this); }
    T operator++(int) noexcept
    { T ret(static_cast<T&>(*this)); ++*this; return ret; }
    T& operator+=(std::ptrdiff_t n) noexcept
    { adjust(n); return static_cast<T&>(*this); }

    T& operator--() noexcept
    { bump_down(); return static_cast<T&>(*this); }
    T operator--(int) noexcept
    { T ret(static_cast<T&>(*this)); --*this; return ret; }
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
};

} // namespace detail

/* note: bitref cannot be in the detail namespace, or functions taking a bit
 * will not be found via ADL on a bitref argument */
struct bitref
{
    bitref() noexcept = default;
    bitref(uint8_t* c, uint8_t bitidx) noexcept : byte{c}, bitidx{bitidx} {}

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
    uint8_t* byte{nullptr};
    uint8_t bitidx{0}; /// bitidx = n to represent the nth least
                       /// significant bit.
                       /// To represent the first bit in the part of a
                       /// message held by this byte, an LSB0 bit_iterator will
                       /// set bitidx to 0, and an MSB0 bit_iterator will set
                       /// bitidx to 7.
};

template <bit_order BO,typename UL=std::uint8_t,byte_order EI=byte_order::none>
struct bit_iterator : detail::bit_iterator_impl<bit_iterator<BO,UL,EI>>
{
    using base      = detail::bit_iterator_impl<bit_iterator<BO,UL,EI>>;
    using reference = bitref;
    using pointer   = detail::bitptr;
    using iterator  = bit_iterator;

    constexpr bit_iterator() noexcept
    {}

    bit_iterator(std::uint8_t* byte, std::uint8_t bitno) :
        base{byte, bitno}
    {}

    constexpr pointer operator->() const noexcept
    { return pointer(**this); }

    constexpr reference operator*() const noexcept
    { return reference(this->byte, detail::bitidx<BO,UL,EI>(this->bitno)); }

    constexpr reference operator[](std::size_t n) const noexcept
    { return *(*this+n); }
};

template <bit_order BO,typename UL=std::uint8_t,byte_order EI=byte_order::none>
struct const_bit_iterator : detail::bit_iterator_impl<
                                const_bit_iterator<BO,UL,EI>>
{
    using base            = detail::bit_iterator_impl<
                                const_bit_iterator<BO,UL,EI>>;
    using reference       = bit;
    using const_reference = bit;
    using pointer         = detail::const_bitptr;
    using const_pointer   = detail::const_bitptr;
    using iterator        = const_bit_iterator;
    using const_iterator  = const_bit_iterator;

    constexpr const_bit_iterator() noexcept = default;

    const_bit_iterator(const std::uint8_t* byte, std::uint8_t bitno) noexcept :
        base{const_cast<std::uint8_t*>(byte), bitno}
    {}

    constexpr const_pointer operator->() const noexcept
    { return const_pointer(**this); }

    constexpr const_reference operator*() const noexcept
    { return bitref(this->byte, detail::bitidx<BO,UL,EI>(this->bitno)); }

    const_reference operator[](std::size_t n) const noexcept
    { return *(*this+n); }
};

} // namespace bitter

#endif // BITTER_BIT_ITERATOR_HPP
