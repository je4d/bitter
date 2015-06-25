#ifndef BITTER_OFFSET_HPP
#define BITTER_OFFSET_HPP

namespace bitter {

struct offset
{
    constexpr explicit offset(std::ptrdiff_t bits) :
        bits{bits}
    {}
    operator std::ptrdiff_t() { return bits; }
    offset operator -() const { return offset(-bits); }

    template <typename UL>
    std::ptrdiff_t element() const
    {
        int n = 8 * sizeof(UL);
        return bits >= 0 ? (bits / n) : ((bits + 1) / n - 1);
    }

    template <typename UL>
    std::uint8_t bit() const
    {
        int n = 8 * sizeof(UL);
        return bits >= 0 ? (bits % n) : ((bits + 1) % n + n - 1);
    }
    std::ptrdiff_t bits;
};

} // namespace bitter

#endif // BITTER_OFFSET_HPP
