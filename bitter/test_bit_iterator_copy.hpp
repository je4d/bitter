#include <bandit/bandit.h>
using namespace bandit;

#include "bit_iterator.hpp"
#include "test_bit_iterator.hpp"

#include <bitset>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <type_traits>
#include <vector>

using std::size_t;
using std::ptrdiff_t;
using bit_order = bitter::bit_order;
using byte_order = bitter::byte_order;

template <bit_order BO, typename UL, byte_order YO = byte_order::none>
struct testvec_shifted_ {
    using testvec_t = std::array<UL,testvec_bits/(8*sizeof(UL))>;
    static constexpr std::size_t shifts = testvec_bits;
    using shifted_testvecs = std::array<testvec_t,shifts>;
    static const shifted_testvecs data;
    static constexpr uint8_t element_bits = 8*sizeof(UL);
};

template <bit_order BO, typename UL, byte_order YO>
const typename testvec_shifted_<BO,UL,YO>::shifted_testvecs
testvec_shifted_<BO,UL,YO>::data = []{
        using namespace bitter;
        const auto& unshifted = testvec<BO,UL,YO>();
        std::array<bit,testvec_bits> as_bits;
        std::copy(const_bit_iterator<BO, UL, YO>(begin(unshifted), 0),
                  const_bit_iterator<BO, UL, YO>(end(unshifted), 0),
                  begin(as_bits));
        shifted_testvecs ret;
        for (std::size_t i = 0; i < testvec_bits; ++i) {
            testvec_t& tv = ret[i];
            auto it = std::copy(std::next(begin(as_bits), i), end(as_bits),
                                bit_iterator<BO, UL, YO>(tv.data(), 0));
            std::copy(begin(as_bits), std::next(begin(as_bits),i), it);
        }
        return ret;
    }();

/* testvec_shifted
 * testvec_shifted(i) is rotated left by i bits
 */
template <bit_order BO, typename UL, byte_order YO = byte_order::none>
constexpr const std::array<UL,testvec_bits/8/sizeof(UL)>&
testvec_shifted(std::ptrdiff_t shift)
{
    return testvec_shifted_<BO,UL,YO>::data[shift];
}

template <typename F>
void for_each_bit_range(F&& f)
{
    for (size_t b1 = 0; b1 <= testvec_bits; ++b1)
        for (size_t b2 = b1; b2 <= testvec_bits; ++b2)
            std::forward<F>(f)(b1, b2);
}

template <typename F>
void for_each_range_copy(F&& f)
{
    for_each_bit_range([&](std::ptrdiff_t b1, std::ptrdiff_t b2){
        for (std::ptrdiff_t b3 = 0; b3 < testvec_bits-(b2-b1)+1; ++b3)
            std::forward<F>(f)(b1, b2, b3);
    });
}

constexpr static ptrdiff_t range_index(ptrdiff_t b1, ptrdiff_t b2, ptrdiff_t b3)
{
    return b1*(3*testvec_bits*testvec_bits+9*testvec_bits+7-b1*b1)/6
        +((b2-b1)*(2*testvec_bits-b2+b1+3))/2
        +b3;
}

static constexpr std::size_t total_shifted_ranges =
    range_index(testvec_bits + 1, testvec_bits + 1, 0);

template <bit_order BO, typename UL, byte_order YO, bool Fill>
struct testvec_partial_shifted_
{
    using testvec_t = std::array<UL,testvec_bits/(8*sizeof(UL))>;
    using shifted_partial_testvecs = std::array<testvec_t,total_shifted_ranges>;
    static constexpr uint8_t element_bits = 8*sizeof(UL);
    static shifted_partial_testvecs data;
    static std::array<bool,total_shifted_ranges> data_set;
};

template <bit_order BO, typename UL, byte_order YO, bool Fill>
typename testvec_partial_shifted_<BO,UL,YO,Fill>::shifted_partial_testvecs
testvec_partial_shifted_<BO,UL,YO,Fill>::data{};

template <bit_order BO, typename UL, byte_order YO, bool Fill>
std::array<bool, total_shifted_ranges>
testvec_partial_shifted_<BO, UL, YO, Fill>::data_set{};

template <bit_order BO, typename UL, byte_order YO, bool Fill>
const std::array<UL,testvec_bits/8/sizeof(UL)>&
testvec_partial_shifted(ptrdiff_t b1, ptrdiff_t b2, ptrdiff_t b3)
{
    using namespace bitter;
    using storage = testvec_partial_shifted_<BO, UL, YO, Fill>;
    auto idx      = range_index(b1, b2, b3);
    auto &ret = storage::data[idx];
    if (storage::data_set[idx])
        return ret;

    ret = testvec_shifted<BO, UL, YO>((b1 - b3 + testvec_bits) % testvec_bits);
    std::fill(bit_iterator<BO, UL, YO>(ret.data(), 0),
              bit_iterator<BO, UL, YO>(ret.data(), offset(b3)),
              bit(Fill));
    std::fill(bit_iterator<BO, UL, YO>(ret.data(), offset(b3 + (b2-b1))),
              bit_iterator<BO, UL, YO>(ret.data(), offset(testvec_bits)),
              bit(Fill));
    storage::data_set[idx] = true;
    return ret;
}

////////////////////////////////////////////////////////////////////////////////
// copy tests

template <class DataIn, class IterIn, class IterOut>
void copy_fwd_fwd()
{
    using ul_in = typename bit_iterator_traits<IterIn>::underlying_type;
    using ul_out = typename bit_iterator_traits<IterOut>::underlying_type;
    static constexpr bit_order  bo_in =bit_iterator_traits<IterIn>::bit_order;
    static constexpr bit_order  bo_out=bit_iterator_traits<IterOut>::bit_order;
    static constexpr byte_order yo_in =bit_iterator_traits<IterIn>::byte_order;
    static constexpr byte_order yo_out=bit_iterator_traits<IterOut>::byte_order;
    static constexpr uint8_t eb_in = 8 * sizeof(ul_in); // element bits
    static constexpr uint8_t eb_out = 8 * sizeof(ul_out); // element bits
//    static constexpr std::size_t elements_in = testvec_bits/(8*sizeof(ul_in));
    static constexpr std::size_t elements_out = testvec_bits/(8*sizeof(ul_out));

    DataIn* data_in_base = begin(testvec<bo_in,ul_in,yo_in>());
    for_each_range_copy([&](ptrdiff_t b1, ptrdiff_t b2, ptrdiff_t b3){
//        std::cout << "(" << (int)b1 << ", " << (int)b2 << ", " << (int)b3 << ")";
        std::array<ul_out,elements_out> output{};
        std::array<ul_out,elements_out> expected =
            testvec_partial_shifted<bo_out, ul_out, yo_out, false>(b1, b2, b3);
        IterIn it(data_in_base+(b1/eb_in), b1%eb_in);
        IterIn end(data_in_base+(b2/eb_in), b2%eb_in);
        IterOut out(begin(output)+(b3/eb_out), b3%eb_out);
        copy(it, end, out);
        if (output == expected) {
//            std::cout << " OK\n"  << expected << "\n";
        } else {
            std::cout << "\n" << output << "\n" << expected << "\n";
        }
        AssertThat(output, Equals(expected));
    });
}

