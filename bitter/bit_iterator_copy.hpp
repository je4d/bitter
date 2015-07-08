#include <algorithm>

#include "bit_iterator.hpp"

namespace bitter {

namespace detail {
    template <typename T>
    T bswap(T t);

    template <>
    inline std::uint16_t bswap<std::uint16_t>(std::uint16_t v)
    {
        return __builtin_bswap16(v);
    }

    template <>
    inline std::uint32_t bswap<std::uint32_t>(std::uint32_t v)
    {
        return __builtin_bswap32(v);
    }

    template <>
    inline std::uint64_t bswap<std::uint64_t>(std::uint64_t v)
    {
        return __builtin_bswap64(v);
    }

    /*
     * Struct that wraps a UL and provides shift operators that shift it as if it's
     * a string of bits, i.e. << removes bits from the start of the message, >>
     * adds 0 bits to the start of the message
     */
    template <bit_order BO, typename UL, byte_order YO>
    struct shiftable
    {
        using uint8_t = std::uint8_t;
        constexpr shiftable(UL ul) noexcept : ul(to_shiftable(ul)) {}
        constexpr operator UL() const noexcept
        {
            return from_shiftable(ul);
        }

        static constexpr UL to_shiftable(UL in) noexcept
        {
            return (sizeof(UL) == 1 or
                    (BO == bit_order::msb0) == (YO == byte_order::msb0))
                       ? in
                       : bswap(in);
        }
        static constexpr UL from_shiftable(UL in) noexcept
        {
            return to_shiftable(in);
        }
        static constexpr UL narrow(UL val) { return val; } // narrowing
        static constexpr uint8_t u8(uint8_t val) { return val; }

        friend constexpr shiftable operator<<(shiftable v, uint8_t s) noexcept
        {
            return {direct,
                    narrow(BO == bit_order::msb0 ? v.ul << s : v.ul >> s)};
        }

        friend constexpr shiftable operator<<(shiftable v, int s) noexcept
        {
            return v << u8(s);
        }

        friend constexpr shiftable operator>>(shiftable v, uint8_t s) noexcept
        {
            return {direct,
                    narrow(BO == bit_order::msb0 ? v.ul >> s : v.ul << s)};
        }

        friend constexpr shiftable operator>>(shiftable v, int s) noexcept
        {
            return v >> u8(s);
        }

    private:
        struct direct_t{};
        constexpr static direct_t direct{};
        constexpr shiftable(direct_t, UL ul) noexcept : ul(ul) {}

        UL ul;
    };
}

template <bitter::bit_order BO, typename UL, bitter::byte_order YO>
bit_iterator<BO, UL, YO>
copy_shifting(bitter::const_bit_iterator<BO, UL, YO> it,
              bitter::const_bit_iterator<BO, UL, YO> end,
              bitter::bit_iterator<BO, UL, YO> out)
{
    if (it == end)
        return out;

    using shiftrep = detail::shiftable<BO,UL,YO>;
    constexpr std::uint8_t element_bits = sizeof(UL) * 8;

    const std::uint8_t lshift =
        (it.bitno - out.bitno + element_bits) % element_bits;
    const std::uint8_t rshift = element_bits - lshift;
    UL buf{};
    UL mask = shiftrep(-1) >> out.bitno;
    if (out.bitno < it.bitno) // *out.data needs more bits than *it.data has
        buf = (shiftrep{*it.data++} << lshift);
    if (it.data < end.data) {
        if (out.bitno) { // partial first output byte
            buf |= (shiftrep{*it.data} >> rshift);
            *out.data = (*out.data & ~mask) | (buf & mask);
            out.data++;
            mask = shiftrep(-1);
            buf = (shiftrep{*it.data++} << lshift);
        }
        while (it.data < end.data) {
            buf |= (shiftrep{*it.data} >> rshift);
            *out.data++ = buf;
            buf = (shiftrep{*it.data++} << lshift);
        }
    }
    if (it.data == end.data and end.bitno) {
        buf |= (shiftrep{*it.data} >> rshift);
        if (end.bitno > lshift) {
            // we've got at least 1 bit more than we can write to *out.data
            *out.data = (*out.data & ~mask) | (buf & mask);
            out.data++;
            mask = shiftrep(-1);
            buf = (shiftrep{*it.data++} << lshift);
        }
    }
    const auto ret = out + (end - it);
    mask &= shiftrep(-1) << ((element_bits - ret.bitno) % element_bits);
    *out.data = (*out.data & ~mask) | (buf & mask);
    return ret;
}

template <bitter::bit_order BO, typename UL, bitter::byte_order YO>
bit_iterator<BO, UL, YO> copy(bitter::const_bit_iterator<BO, UL, YO> it,
                              bitter::const_bit_iterator<BO, UL, YO> end,
                              bitter::bit_iterator<BO, UL, YO> out)
{
    if (it.bitno != out.bitno)
        return copy_shifting(it, end, out);
    if (it == end)
        return out;

    using shiftrep = detail::shiftable<BO,UL,YO>;
    constexpr std::uint8_t element_bits = sizeof(UL) * 8;

    UL mask = shiftrep(-1);
    if (it.bitno) {
        mask = shiftrep(-1) >> out.bitno;
        if (it.data != end.data) {
            *out.data = (*out.data & ~mask) | (*it.data++ & mask);
            ++out.data;
            mask = shiftrep(-1);
        }
    }
    out.data = std::copy(it.data, end.data, out.data);
    if (end.bitno) {
        mask &= shiftrep(-1) << (element_bits - end.bitno);
        *out.data = (*out.data & ~mask) | (*end.data & mask);
    }
    return end - it + out;
}

template <bitter::bit_order BO, typename UL, bitter::byte_order YO>
bit_iterator<BO, UL, YO> copy(bitter::bit_iterator<BO, UL, YO> it,
                              bitter::bit_iterator<BO, UL, YO> end,
                              bitter::bit_iterator<BO, UL, YO> out)
{
    using cit = bitter::const_bit_iterator<BO, UL, YO>;
    return copy(cit{it}, cit{end}, out);
}

}
