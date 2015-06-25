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

template <typename T, typename D>
T make_bit_iterator(D&& d, bitter::offset o)
{
    using fwd_it = typename bit_iterator_traits<T>::forward_iterator;
    constexpr bool rev = bit_iterator_traits<T>::is_reverse;
    return rev ? T{fwd_it{end(d), -o}} : T{fwd_it{begin(d), o}};
}

template <class IterIn, class IterOut>
void test_non_aliasing_copy()
{
    using traits_in = bit_iterator_traits<IterIn>;
    using traits_out= bit_iterator_traits<IterOut>;
    using ul_in = typename traits_in::underlying_type;
    using ul_out= typename traits_out::underlying_type;
    static constexpr bit_order  bo_in =traits_in::bit_order;
    static constexpr bit_order  bo_out=traits_out::bit_order;
    static constexpr byte_order yo_in =traits_in::byte_order;
    static constexpr byte_order yo_out=traits_out::byte_order;
    static constexpr bool rev_in = traits_in::is_reverse;
    static constexpr bool rev_out = traits_out::is_reverse;
    static constexpr bool net_rev = rev_in != rev_out;

    testvec_t<ul_in> data_in = testvec<bo_in, ul_in, yo_in>();
    for_each_range_copy([&](ptrdiff_t b1, ptrdiff_t b2, ptrdiff_t b3) {
        testvec_t<ul_out> output{};
        testvec_t<ul_out> expected =
            testvec_partial_shifted<net_rev, bo_out, ul_out, yo_out, false>(
                rev_in ? testvec_bits - b2 : b1,
                rev_in ? testvec_bits - b1 : b2,
                rev_in ? testvec_bits - (b3 + (b2 - b1)) : b3);
        IterIn it   = make_bit_iterator<IterIn>(data_in, bitter::offset(b1));
        IterIn end  = make_bit_iterator<IterIn>(data_in, bitter::offset(b2));
        IterOut out = make_bit_iterator<IterOut>(output, bitter::offset(b3));
        copy(it, end, out);
        if (output != expected) std::cout << "\n(" << (int)b1 << ", " << (int)b2 << ", " << (int)b3
                      << ")\n" << output << "\n" << expected << "\n";
        AssertThat(output, Equals(expected));
    });
}

////////////////////////////////////////////////////////////////////////////////
// Test dispatch/fan-out
//
// Aim is to test all reasonable combinations of instantiations of bit iterators
//
// Function naming:
//
// test_copy_fixed(_(it|itt|bo)[io]?)*
//
// The optional suffices tell which parts of which iterator type have been
// determined already.
//
// Prefixes:
//
// "it"  iterator type
// "itt" iterator template type
// "bo"  bit order
//
// Suffices:
// "i"   input
// "o"   output
//
// "test_copy_fixed_iti_itto":
//     - input iterator type is known
//     - output iterator template type is known

struct test_copy_iterator_types
{
    template <bit_order BO, typename UL, byte_order YO>
    using reverse_bit_iterator =
        std::reverse_iterator<bitter::bit_iterator<BO, UL, YO>>;
    template <bit_order BO, typename UL, byte_order YO>
    using const_reverse_bit_iterator =
        std::reverse_iterator<bitter::const_bit_iterator<BO, UL, YO>>;
};

template <template <bit_order, typename, byte_order> class InIter>
struct test_copy_iterator_types_fixed_itt
{
    template <typename UL, byte_order YO>
    using msb0_iterator = InIter<bit_order::msb0, UL, YO>;
    template <typename UL, byte_order YO>
    using lsb0_iterator = InIter<bit_order::lsb0, UL, YO>;
};

template <typename InIter, typename OutIter>
void test_copy_fixed_iti_ito()
{
    it("copies correctly", &test_non_aliasing_copy<InIter, OutIter>);
}

template <typename InIter, template <typename UL, byte_order YO> class OutIter>
void test_copy_fixed_iti_itto_boo()
{
    describe("to underlying-type=uint8_t", [] {
        test_copy_fixed_iti_ito<InIter, OutIter<uint8_t, byte_order::none>>();
    });
    describe("to underlying-type=uint16_t,byte_order=msb0", [] {
        test_copy_fixed_iti_ito<InIter, OutIter<uint16_t, byte_order::msb0>>();
    });
    describe("to underlying-type=uint16_t,byte_order=lsb0", [] {
        test_copy_fixed_iti_ito<InIter, OutIter<uint16_t, byte_order::lsb0>>();
    });
    describe("to underlying-type=uint32_t,byte_order=msb0", [] {
        test_copy_fixed_iti_ito<InIter, OutIter<uint32_t, byte_order::msb0>>();
    });
    describe("to underlying-type=uint32_t,byte_order=lsb0", [] {
        test_copy_fixed_iti_ito<InIter, OutIter<uint32_t, byte_order::lsb0>>();
    });
    describe("to underlying-type=uint64_t,byte_order=msb0", [] {
        test_copy_fixed_iti_ito<InIter, OutIter<uint64_t, byte_order::msb0>>();
    });
    describe("to underlying-type=uint64_t,byte_order=lsb0", [] {
        test_copy_fixed_iti_ito<InIter, OutIter<uint64_t, byte_order::lsb0>>();
    });
}

template <template <typename UL, byte_order YO> class InIter,
          template <typename UL, byte_order YO> class OutIter>
void test_copy_fixed_itt_bo()
{
    describe("from underlying-type=uint8_t", [] {
        test_copy_fixed_iti_itto_boo<InIter<uint8_t, byte_order::none>,
                                     OutIter>();
    });
    describe("from underlying-type=uint16_t,byte_order=msb0", [] {
        test_copy_fixed_iti_itto_boo<InIter<uint16_t, byte_order::msb0>,
                                     OutIter>();
    });
    describe("from underlying-type=uint16_t,byte_order=lsb0", [] {
        test_copy_fixed_iti_itto_boo<InIter<uint16_t, byte_order::lsb0>,
                                     OutIter>();
    });
    describe("from underlying-type=uint32_t,byte_order=msb0", [] {
        test_copy_fixed_iti_itto_boo<InIter<uint32_t, byte_order::msb0>,
                                     OutIter>();
    });
    describe("from underlying-type=uint32_t,byte_order=lsb0", [] {
        test_copy_fixed_iti_itto_boo<InIter<uint32_t, byte_order::lsb0>,
                                     OutIter>();
    });
    describe("from underlying-type=uint64_t,byte_order=msb0", [] {
        test_copy_fixed_iti_itto_boo<InIter<uint64_t, byte_order::msb0>,
                                     OutIter>();
    });
    describe("from underlying-type=uint64_t,byte_order=lsb0", [] {
        test_copy_fixed_iti_itto_boo<InIter<uint64_t, byte_order::lsb0>,
                                     OutIter>();
    });
}

template <template <typename UL, byte_order YO> class InIter,
          template <bit_order BO, typename UL, byte_order YO> class OutIter>
void test_copy_fixed_itt_boi()
{
    using types = test_copy_iterator_types_fixed_itt<OutIter>;
    describe("bit_order=msb0", [] {
        test_copy_fixed_itt_bo<InIter, types::template msb0_iterator>();
    });
    describe("bit_order=lsb0", [] {
        test_copy_fixed_itt_bo<InIter, types::template lsb0_iterator>();
    });
}

template <template <typename UL, byte_order YO> class InIter>
void test_copy_fixed_itti_boi()
{
    using types = test_copy_iterator_types;
    describe("to a bit_iterator", [] {
        test_copy_fixed_itt_boi<InIter, bitter::bit_iterator>();
    });
    describe("to a std::reverse_iterator<bit_iterator>", [] {
        test_copy_fixed_itt_boi<InIter, types::reverse_bit_iterator>();
    });
}

template <template <bit_order BO, typename UL, byte_order YO> class InIter>
void test_copy_fixed_itti()
{
    using types = test_copy_iterator_types_fixed_itt<InIter>;
    describe("from bit_order=msb0",
             [] { test_copy_fixed_itti_boi<types::template msb0_iterator>(); });
    describe("from bit_order=lsb0",
             [] { test_copy_fixed_itti_boi<types::template lsb0_iterator>(); });
}

