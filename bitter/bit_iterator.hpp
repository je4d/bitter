#ifndef BITTER_BIT_ITERATOR_HPP
#define BITTER_BIT_ITERATOR_HPP

#include "bit.hpp"
#include "bit_order.hpp"
#include "byte_order.hpp"
#include "offset.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>

namespace bitter {

namespace detail {

using std::uint8_t;

// byteidx, bitidx
//
// if an integer 't' is interpreted as a string of bits, what is the
// significance of the nth bit of the bitstring in t?
//
// e.g. in a uint64_t with msb0 byte order and msb0 bit ordering where is
// bit #5?
// The bit #5 is in byte zero, which is the most significant byte (i.e. msb0).
// So bit #5's significance in the uint64_t is in [2^56 and 2^64).
// The most significant bit within that is bit 0, so bit #5 is the 6th most
// significant bit. the most significant end of our range is 2^63, so counting
// back we get a significance of 2^58, which gives us a 'bit index' of 58.
//
template <byte_order YO>
constexpr uint8_t byteidx(uint8_t byteno, uint8_t bytes)
{
    return (YO == byte_order::none)
        ? 0
        :  (YO == byte_order::msb0)
            ? (bytes - 1) - byteno
            : (YO == byte_order::lsb0)
                ? byteno
                :  throw "byteidx needs a valid byte_order";
}

template <typename UL, byte_order YO>
constexpr uint8_t byteidx(uint8_t byteno)
{
    return byteidx<YO>(byteno, sizeof(UL));
}

template <bit_order BO, typename UL, byte_order YO>
constexpr uint8_t bitidx(uint8_t bitno)
{
    return 8*byteidx<UL,YO>(bitno/8)
        + ((BO == bit_order::lsb0) ? (bitno%8) : (7-(bitno%8)));
}

template <bit_order BO, typename UL, byte_order YO>
constexpr std::ptrdiff_t bitidx(offset bitno)
{
    return bitno - bitno.bit<UL>() + bitidx<BO, UL, YO>(bitno.bit<UL>());
}

struct const_bitptr
{
    explicit constexpr const_bitptr(bit b) : b{b} {}
    constexpr const bit* operator->() const { return &b; }
private:
    bit b;
};

struct bitptr
{
    explicit constexpr bitptr(bit b) : b{b} {}
    bit* operator->() { return &b; }
    constexpr const bit* operator->() const { return &b; }
private:
    bit b;
};

template <typename UL>
struct bit_iterator_base : std::iterator<std::random_access_iterator_tag,bit>
{
    constexpr bit_iterator_base() noexcept = default;
    constexpr bit_iterator_base(UL* data, uint8_t bitno) noexcept :
        data{data},
        bitno{bitno}
    {}

    void bump_up()
    {
        if (++bitno == sizeof(UL)*8)
            bitno = (++data,0);
    }

    void bump_down()
    {
        if (bitno-- == 0)
            bitno = (--data,sizeof(UL)*8-1);
    }

    void adjust(std::ptrdiff_t delta)
    {
        auto div = std::div(delta+bitno, ptrdiff_t(sizeof(UL)*8));
        if (div.rem < 0) {
            div.quot -= 1;
            div.rem += sizeof(UL)*8;
        }
        data += div.quot;
        bitno = div.rem;
    }

    friend bool operator==(bit_iterator_base a, bit_iterator_base b) noexcept
    { return a.data == b.data and a.bitno == b.bitno; }
    friend bool operator!=(bit_iterator_base a, bit_iterator_base b) noexcept
    { return !(a==b); }
    friend bool operator<(bit_iterator_base a, bit_iterator_base b) noexcept
    { return (a.data == b.data) ? (a.bitno < b.bitno) : (a.data < b.data); }
    friend bool operator<=(bit_iterator_base a, bit_iterator_base b) noexcept
    { return !(b < a); }
    friend bool operator>(bit_iterator_base a, bit_iterator_base b) noexcept
    { return b < a; }
    friend bool operator>=(bit_iterator_base a, bit_iterator_base b) noexcept
    { return !(a < b); }

    friend void swap(bit_iterator_base& a, bit_iterator_base& b) noexcept
    { std::swap(a.data, b.data); std::swap(a.bitno, b.bitno); }
    friend void swap(bit_iterator_base& a, bit_iterator_base&& b) noexcept
    { swap(a, b); }
    friend void swap(bit_iterator_base&& a, bit_iterator_base& b) noexcept
    { swap(a, b); }

    UL* data{0};
    uint8_t bitno{0};
};

template <typename T, typename UL>
struct bit_iterator_impl : bit_iterator_base<UL>
{
    constexpr bit_iterator_impl() noexcept = default;
    constexpr bit_iterator_impl(UL* data, uint8_t bitno) noexcept :
        bit_iterator_base<UL>{data, bitno}
    {}

    T& operator++() noexcept
    { this->bump_up(); return static_cast<T&>(*this); }
    T operator++(int) noexcept
    { T ret(static_cast<T&>(*this)); ++*this; return ret; }
    T& operator+=(std::ptrdiff_t n) noexcept
    { this->adjust(n); return static_cast<T&>(*this); }

    T& operator--() noexcept
    { this->bump_down(); return static_cast<T&>(*this); }
    T operator--(int) noexcept
    { T ret(static_cast<T&>(*this)); --*this; return ret; }
    T& operator-=(std::ptrdiff_t n) noexcept
    { this->adjust(-n); return static_cast<T&>(*this); }

    friend std::ptrdiff_t operator-(T a, T b) noexcept
    { return (a.data - b.data)*sizeof(UL)*8
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
template <typename UL>
struct bitref
{
    static constexpr UL one = 1;

    constexpr bitref() noexcept = default;
    constexpr bitref(UL* ul, std::uint8_t bitidx) noexcept :
        data{ul},
        bitidx{bitidx}
    {}

    bitref& operator=(bit b) noexcept
    {
        bool bb(b);
        *data = (*data & ~(one<<bitidx)) | (static_cast<UL>(bb)<<bitidx);
        return *this;
    }

    // In an expression such as *bit_iter1 = *bit_iter2, both the RHS and LHS
    // are bitrefs. The default assignment operator won't do what we want here.
    bitref& operator=(const bitref& br)
    {
        return (*this) = static_cast<bit>(br);
    }

    constexpr operator bit() const noexcept
    { return bit(*data & (one<<bitidx)); }
private:
    UL* data{nullptr};
    /** bitidx = n to represent the nth least
     *  significant bit.
     *  To represent the first bit in the part of a
     *  message held by a data, an LSB0 bit_iterator will
     *  set bitidx to 0, and an MSB0 bit_iterator will set
     *  bitidx to 7.
     */
    std::uint8_t bitidx{0};
};

template <bit_order BO,typename UL=std::uint8_t,byte_order YO=byte_order::none>
struct bit_iterator : detail::bit_iterator_impl<bit_iterator<BO,UL,YO>,UL>
{
    using base      = detail::bit_iterator_impl<bit_iterator<BO,UL,YO>,UL>;
    using reference = bitref<UL>;
    using pointer   = detail::bitptr;
    using iterator  = bit_iterator;

    constexpr bit_iterator() noexcept
    {}

    bit_iterator(UL* data, std::uint8_t bitno) :
        base{data, bitno}
    {}

    bit_iterator(UL* data, offset off) :
        bit_iterator(data + off.element<UL>(), off.bit<UL>())
    {}

    constexpr pointer operator->() const noexcept
    { return pointer(**this); }

    constexpr reference operator*() const noexcept
    { return reference(this->data, detail::bitidx<BO,UL,YO>(this->bitno)); }

    constexpr reference operator[](std::size_t n) const noexcept
    { return *(*this+n); }
};

template <bit_order BO,typename UL=std::uint8_t,byte_order YO=byte_order::none>
struct const_bit_iterator : detail::bit_iterator_impl<
                                const_bit_iterator<BO,UL,YO>,UL>
{
    using base = detail::bit_iterator_impl< const_bit_iterator<BO,UL,YO>,UL>;
    using reference       = const bit;
    using const_reference = const bit;
    using pointer         = detail::const_bitptr;
    using const_pointer   = detail::const_bitptr;
    using iterator        = const_bit_iterator;
    using const_iterator  = const_bit_iterator;

    constexpr const_bit_iterator() noexcept = default;
    constexpr const_bit_iterator(bit_iterator<BO, UL, YO> o) noexcept
        : const_bit_iterator(o.data, o.bitno)
    {}

    const_bit_iterator(const UL* data, std::uint8_t bitno) noexcept :
        base{const_cast<UL*>(data), bitno}
    {}

    const_bit_iterator(const UL* data, offset off) :
        const_bit_iterator(data + off.element<UL>(), off.bit<UL>())
    {}

    constexpr const_pointer operator->() const noexcept
    { return const_pointer(**this); }

    constexpr const_reference operator*() const noexcept
    { return bitref<UL>(this->data, detail::bitidx<BO,UL,YO>(this->bitno)); }

    constexpr const_reference operator[](std::size_t n) const noexcept
    { return *(*this+n); }
};

} // namespace bitter

#endif // BITTER_BIT_ITERATOR_HPP
