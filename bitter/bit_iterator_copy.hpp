#include <algorithm>

#include "bit_iterator.hpp"

namespace bitter {

namespace detail {

    struct bitswap_data_ { unsigned char data[256]; };
    constexpr bitswap_data_ bitswap_data() {
        return {{
            0x0,  0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
            0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
            0x8,  0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
            0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
            0x4,  0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
            0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
            0xC,  0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
            0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
            0x2,  0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
            0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
            0xA,  0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
            0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
            0x6,  0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
            0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
            0xE,  0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
            0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
            0x1,  0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
            0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
            0x9,  0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
            0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
            0x5,  0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
            0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
            0xD,  0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
            0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
            0x3,  0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
            0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
            0xB,  0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
            0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
            0x7,  0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
            0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
            0xF,  0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
            0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF,
        }};
    }

    template <typename T>
    T bitswap(const T t)
    {
        T ret;
        std::transform(reinterpret_cast<const unsigned char*>(&t),
                       reinterpret_cast<const unsigned char*>(&t + 1),
                       reinterpret_cast<unsigned char*>(&ret),
                       [](unsigned char c) { return bitswap_data().data[c]; });
        return ret;
    }

    template <typename T>
    T byteswap(T t);

    template <>
    inline std::uint16_t byteswap<std::uint16_t>(std::uint16_t v)
    {
        return __builtin_bswap16(v);
    }

    template <>
    inline std::uint32_t byteswap<std::uint32_t>(std::uint32_t v)
    {
        return __builtin_bswap32(v);
    }

    template <>
    inline std::uint64_t byteswap<std::uint64_t>(std::uint64_t v)
    {
        return __builtin_bswap64(v);
    }

    template <typename Derived>
    struct shift_wrappers
    {
        static constexpr uint8_t u8(uint8_t val) { return val; }
        friend constexpr Derived operator<<(Derived v, int s) noexcept
        { return v << u8(s); }
        friend constexpr Derived operator>>(Derived v, int s) noexcept
        { return v >> u8(s); }
    };

    /*
     * Struct that wraps a UL and provides shift operators that shift it as if
     * it's a string of bits, i.e. << removes bits from the start of the
     * message, >> adds 0 bits to the start of the message
     */
    template <bit_order BOI, typename UL, byte_order YOI,
              bit_order BOO,              byte_order YOO,
              bit_order BOS,              byte_order YOS>
    struct copyrep : shift_wrappers<copyrep<BOI, UL, YOI, BOO, YOO, BOS, YOS>>
    {
        using uint8_t = std::uint8_t;
        constexpr copyrep(UL ul) noexcept : ul(in_to_copyrep(ul)) {}
        constexpr operator UL() const noexcept
        { return copyrep_to_out(ul); }

        static constexpr UL in_to_copyrep(UL in) noexcept
        {
            return (YOI == YOS)
                       ? (BOI == BOS) ? in : bitswap(in)
                       : (BOI == BOS) ? byteswap(in) : byteswap(bitswap(in));
        }

        static constexpr UL copyrep_to_out(UL in) noexcept
        {
            return (YOS == YOO)
                       ? (BOS == BOO) ? in : bitswap(in)
                       : (BOS == BOO) ? byteswap(in) : byteswap(bitswap(in));
        }

        static constexpr UL narrow(UL val) { return val; } // narrowing
        friend constexpr copyrep operator<<(copyrep v, uint8_t s) noexcept
        {
            return {direct,
                    narrow(BOS == bit_order::msb0 ? v.ul << s : v.ul >> s)};
        }

        friend constexpr copyrep operator>>(copyrep v, uint8_t s) noexcept
        {
            return {direct,
                    narrow(BOS == bit_order::msb0 ? v.ul >> s : v.ul << s)};
        }

    private:
        struct direct_t{};
        constexpr static direct_t direct{};
        constexpr copyrep(direct_t, UL ul) noexcept : ul(ul) {}

        UL ul;
    };

    template <bit_order BO, byte_order YO>
    struct is_shiftable {
        static constexpr bool value =
            (BO == bit_order::msb0) == (YO == byte_order::msb0);
    };

    template <bit_order BO>
    struct is_shiftable<BO, byte_order::none> {
        static constexpr bool value = true;
    };

    template <bit_order BOI, byte_order YOI, bit_order BOO, byte_order YOO>
    struct shiftable_bit_order {
        static constexpr bit_order value =
            is_shiftable<BOI, YOI>::value ? BOI : is_shiftable<BOO, YOO>::value
                                                      ? BOO
                                                      : BOI;
    };

    template <bit_order BO, typename UL>
    struct shiftable_byte_order;
    template <>
    struct shiftable_byte_order<bit_order::msb0, std::uint8_t>
    { static constexpr byte_order value = byte_order::none; };
    template <>
    struct shiftable_byte_order<bit_order::lsb0, std::uint8_t>
    { static constexpr byte_order value = byte_order::none; };
    template <typename UL>
    struct shiftable_byte_order<bit_order::msb0, UL>
    { static constexpr byte_order value = byte_order::msb0; };
    template <typename UL>
    struct shiftable_byte_order<bit_order::lsb0, UL>
    { static constexpr byte_order value = byte_order::lsb0; };

    template <bit_order BOI, typename UL, byte_order YOI,
              bit_order BOO,              byte_order YOO>
    using copyrep_aligned = copyrep<BOI, UL, YOI, BOO, YOO, BOI, YOI>;

    template <bit_order BOI, typename UL, byte_order YOI,
              bit_order BOO,              byte_order YOO,
              bit_order BOS = shiftable_bit_order<BOI, YOI, BOO, YOO>::value,
              byte_order YOS = shiftable_byte_order<BOS, UL>::value>
    using copyrep_shifting = copyrep<BOI, UL, YOI, BOO, YOO, BOS, YOS>;
}

template <bit_order BOI, typename UL, byte_order YOI,
          bit_order BOO,              byte_order YOO>
bit_iterator<BOO, UL, YOO>
copy_shifting(const_bit_iterator<BOI, UL, YOI> it,
              const_bit_iterator<BOI, UL, YOI> end,
              bit_iterator<BOO, UL, YOO> out)
{
    if (it == end)
        return out;

    using copyrep = detail::copyrep_shifting<BOI, UL, YOI, BOO, YOO>;
    constexpr std::uint8_t element_bits = sizeof(UL) * 8;

    const std::uint8_t lshift =
        (it.bitno - out.bitno + element_bits) % element_bits;
    const std::uint8_t rshift = element_bits - lshift;
    UL buf{};
    UL mask = copyrep(-1) >> out.bitno;
    if (out.bitno < it.bitno) // *out.data needs more bits than *it.data has
        buf = (copyrep{*it.data++} << lshift);
    if (it.data < end.data) {
        if (out.bitno) { // partial first output byte
            buf |= (copyrep{*it.data} >> rshift);
            *out.data = (*out.data & ~mask) | (buf & mask);
            out.data++;
            mask = -1;
            buf = (copyrep{*it.data++} << lshift);
        }
        while (it.data < end.data) {
            buf |= (copyrep{*it.data} >> rshift);
            *out.data++ = buf;
            buf = (copyrep{*it.data++} << lshift);
        }
    }
    if (it.data == end.data and end.bitno) {
        buf |= (copyrep{*it.data} >> rshift);
        if (end.bitno > lshift) {
            // we've got at least 1 bit more than we can write to *out.data
            *out.data = (*out.data & ~mask) | (buf & mask);
            out.data++;
            mask = -1;
            buf = (copyrep{*it.data++} << lshift);
        }
    }
    const auto ret = out + (end - it);
    mask &= copyrep(-1) << ((element_bits - ret.bitno) % element_bits);
    *out.data = (*out.data & ~mask) | (buf & mask);
    return ret;
}

template <bit_order BOI, typename UL, byte_order YOI,
          bit_order BOO, byte_order YOO>
bit_iterator<BOO, UL, YOO> copy(const_bit_iterator<BOI, UL, YOI> it,
                                const_bit_iterator<BOI, UL, YOI> end,
                                bit_iterator<BOO, UL, YOO> out)
{
    if (it.bitno != out.bitno)
        return copy_shifting(it, end, out);
    if (it == end)
        return out;

    using copyrep_mask = detail::copyrep_shifting<BOI, UL, YOI, BOO, YOO>;
    using copyrep      = detail::copyrep_aligned<BOI, UL, YOI, BOO, YOO>;

    constexpr std::uint8_t element_bits = sizeof(UL) * 8;

    UL mask = -1;
    if (it.bitno) {
        mask = copyrep_mask(-1) >> out.bitno;
        if (it.data != end.data) {
            *out.data = (*out.data & ~mask) | (copyrep{*it.data++} & mask);
            ++out.data;
            mask = -1;
        }
    }
    out.data = std::transform(it.data, end.data, out.data,
                              [](UL ul) { return copyrep{ul}; });
    if (end.bitno) {
        mask &= copyrep_mask(-1) << (element_bits - end.bitno);
        *out.data = (*out.data & ~mask) | (copyrep{*end.data} & mask);
    }
    return end - it + out;
}

template <bit_order BOI, typename ULI, byte_order YOI,
          bit_order BOO, typename ULO, byte_order YOO>
bit_iterator<BOO, ULO, YOO> copy(bit_iterator<BOI, ULI, YOI> it,
                                 bit_iterator<BOI, ULI, YOI> end,
                                 bit_iterator<BOO, ULO, YOO> out)
{
    using cit = const_bit_iterator<BOI, ULI, YOI>;
    return copy(cit{it}, cit{end}, out);
}

}
