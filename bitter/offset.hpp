#ifndef BITTER_OFFSET_HPP
#define BITTER_OFFSET_HPP

namespace bitter {

namespace detail {
    template <typename UL>
    constexpr std::uint8_t sz() { return 8*sizeof(UL); }
}

struct offset
{

    constexpr explicit offset(std::ptrdiff_t bits) :
        bits{bits}
    {}
    operator std::ptrdiff_t() { return bits; }
    offset operator -() const { return offset(-bits); }

    template <typename UL>
    constexpr std::ptrdiff_t element() const
    {
        return bits >= 0 ? (bits / detail::sz<UL>())
                         : ((bits + 1) / detail::sz<UL>() - 1);
    }

    template <typename UL>
    constexpr std::uint8_t bit() const
    {
        return bits >= 0
                   ? (bits % detail::sz<UL>())
                   : ((bits + 1) % detail::sz<UL>() + detail::sz<UL>() - 1);
    }
    std::ptrdiff_t bits;
};

} // namespace bitter

#endif // BITTER_OFFSET_HPP
