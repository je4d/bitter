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


template <typename UL>
using testvec_t = std::array<UL,testvec_bits/(8*sizeof(UL))>;

template <bit_order BO, typename UL, byte_order YO = byte_order::none>
const testvec_t<UL> &testvec_rev()
{
    using namespace bitter;
    static const testvec_t<UL> reversed = [] {
        const auto &fwd = testvec<BO, UL, YO>();
        testvec_t<UL> ret;
        std::array<bit, testvec_bits> as_bits;
        std::copy(const_bit_iterator<BO, UL, YO>(begin(fwd), 0),
                  const_bit_iterator<BO, UL, YO>(end(fwd), 0), begin(as_bits));
        std::copy(as_bits.rbegin(), as_bits.rend(),
                  bit_iterator<BO, UL, YO>(begin(ret), 0));
        return ret;
    }();
    return reversed;
}

/* testvec_shifted
 * testvec_shifted(i) is testvec rotated left by i bits, then maybe reversed
 */
template <bool R, bit_order BO, typename UL, byte_order YO = byte_order::none>
const testvec_t<UL> &testvec_shifted(std::ptrdiff_t shift)
{
    static constexpr std::size_t shifts = testvec_bits;
    using shifted_testvecs = std::array<testvec_t<UL>,shifts>;
    static const shifted_testvecs data = []{
        using namespace bitter;
        const auto &unshifted =
            R ? testvec_rev<BO, UL, YO>() : testvec<BO, UL, YO>();
        std::array<bit,testvec_bits> as_bits;
        std::copy(const_bit_iterator<BO, UL, YO>(begin(unshifted), 0),
                  const_bit_iterator<BO, UL, YO>(end(unshifted), 0),
                  begin(as_bits));
        shifted_testvecs ret;
        for (std::size_t i = 0; i < testvec_bits; ++i) {
            testvec_t<UL>& tv = ret[i];
            auto j = R ? (testvec_bits-i) : i;
            auto it = std::copy(std::next(begin(as_bits), j), end(as_bits),
                                bit_iterator<BO, UL, YO>(tv.data(), 0));
            std::copy(begin(as_bits), std::next(begin(as_bits),j), it);
        }
        return ret;
    }();
    return data[shift];
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

template <bool R, bit_order BO, typename UL, byte_order YO, bool Fill>
testvec_t<UL> testvec_partial_shifted(ptrdiff_t b1, ptrdiff_t b2, ptrdiff_t b3)
{
    using namespace bitter;
    testvec_t<UL> ret;

    ret =
        testvec_shifted<R, BO, UL, YO>((b1 - b3 + testvec_bits) % testvec_bits);
    auto b4 = b3 + (b2 - b1);
    std::fill(bit_iterator<BO, UL, YO>(ret.data(), 0),
              bit_iterator<BO, UL, YO>(ret.data(),
                                       offset(R ? testvec_bits - b4 : b3)),
              bit(Fill));
    std::fill(bit_iterator<BO, UL, YO>(ret.data(),
                                       offset(R ? testvec_bits - b3 : b4)),
              bit_iterator<BO, UL, YO>(ret.data(), offset(testvec_bits)),
              bit(Fill));
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
    static constexpr std::size_t elements_in = testvec_bits/(8*sizeof(ul_in));
    static constexpr std::size_t elements_out = testvec_bits/(8*sizeof(ul_out));

    std::array<ul_in,elements_in> data_in = testvec<bo_in,ul_in,yo_in>();
    DataIn* data_in_base = begin(data_in);
    for_each_range_copy([&](ptrdiff_t b1, ptrdiff_t b2, ptrdiff_t b3){
//        std::cout << "(" << (int)b1 << ", " << (int)b2 << ", " << (int)b3 << ")";
        std::array<ul_out,elements_out> output{};
        std::array<ul_out, elements_out> expected =
            testvec_partial_shifted<false, bo_out, ul_out, yo_out, false>(
                b1, b2, b3);
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

