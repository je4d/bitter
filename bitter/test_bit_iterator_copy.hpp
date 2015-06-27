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

/// testvec_shifted
//  testvec_shifted(i) is testvec rotated left by i bits, then maybe reversed
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

////////////////////////////////////////////////////////////////////////////////
// Test dispatch/fan-out notes
//
// Aim is to test all reasonable combinations of instantiations of bit iterators
//
// Function naming:
//
// test[[_non]_aliasing]_copy_fixed(_(it|itt|bo)[io]?)*
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
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Type template helpers

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

template <template <typename, byte_order> class Iter>
struct test_copy_iterator_types_fixed_itt_bo
{
    using uint8_iterator = Iter<uint8_t, byte_order::none>;
    template <byte_order YO>
    using uint16_iterator = Iter<uint16_t, YO>;
    template <byte_order YO>
    using uint32_iterator = Iter<uint32_t, YO>;
    template <byte_order YO>
    using uint64_iterator = Iter<uint64_t, YO>;
};

////////////////////////////////////////////////////////////////////////////////
// Non-aliasing-specific fan-out

template <class IterIn, class IterOut, bool Fill>
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
    static const testvec_t<ul_out> Zeros{};
    static const testvec_t<ul_out> Ones = [] {
        testvec_t<ul_out> ret;
        std::fill(begin(ret), end(ret), -1);
        return ret;
    }();

    testvec_t<ul_in> data_in = testvec<bo_in, ul_in, yo_in>();
    for_each_range_copy([&](ptrdiff_t b1, ptrdiff_t b2, ptrdiff_t b3) {
        testvec_t<ul_out> output = Fill ? Ones : Zeros;
        testvec_t<ul_out> expected =
            testvec_partial_shifted<net_rev, bo_out, ul_out, yo_out, Fill>(
                rev_in ? testvec_bits - b2 : b1,
                rev_in ? testvec_bits - b1 : b2,
                rev_in ? testvec_bits - (b3 + (b2 - b1)) : b3);
        IterIn it   = make_bit_iterator<IterIn>(data_in, bitter::offset(b1));
        IterIn end  = make_bit_iterator<IterIn>(data_in, bitter::offset(b2));
        IterOut out = make_bit_iterator<IterOut>(output, bitter::offset(b3));
        copy(it, end, out);
        if (output != expected)
            std::cout << "\n(" << (int)b1 << ", " << (int)b2 << ", " << (int)b3
                      << ")\n" << output << "\n" << expected << "\n";
        AssertThat(output, Equals(expected));
    });
}

template <typename InIter, typename OutIter>
void test_non_aliasing_copy_fixed_it()
{
    it("correctly performs non-aliasing copies (fill=0)",
       &test_non_aliasing_copy<InIter, OutIter, false>);
    it("correctly performs non-aliasing copies (fill=1)",
       &test_non_aliasing_copy<InIter, OutIter, true>);
}

template <typename InIter, template <typename UL, byte_order YO> class OutIter>
void test_non_aliasing_copy_fixed_iti_itto_boo()
{
    describe(
        "to underlying-type=uint8_t",
        &test_non_aliasing_copy_fixed_it<InIter,
                                         OutIter<uint8_t, byte_order::none>>);
    describe(
        "to underlying-type=uint16_t,byte_order=msb0",
        &test_non_aliasing_copy_fixed_it<InIter,
                                         OutIter<uint16_t, byte_order::msb0>>);
    describe(
        "to underlying-type=uint16_t,byte_order=lsb0",
        &test_non_aliasing_copy_fixed_it<InIter,
                                         OutIter<uint16_t, byte_order::lsb0>>);
    describe(
        "to underlying-type=uint32_t,byte_order=msb0",
        &test_non_aliasing_copy_fixed_it<InIter,
                                         OutIter<uint32_t, byte_order::msb0>>);
    describe(
        "to underlying-type=uint32_t,byte_order=lsb0",
        &test_non_aliasing_copy_fixed_it<InIter,
                                         OutIter<uint32_t, byte_order::lsb0>>);
    describe(
        "to underlying-type=uint64_t,byte_order=msb0",
        &test_non_aliasing_copy_fixed_it<InIter,
                                         OutIter<uint64_t, byte_order::msb0>>);
    describe(
        "to underlying-type=uint64_t,byte_order=lsb0",
        &test_non_aliasing_copy_fixed_it<InIter,
                                         OutIter<uint64_t, byte_order::lsb0>>);
}

template <template <typename UL, byte_order YO> class InIter,
          template <typename UL, byte_order YO> class OutIter>
void test_non_aliasing_copy_fixed_itt_bo()
{
    describe("from underlying-type=uint8_t",
             &test_non_aliasing_copy_fixed_iti_itto_boo<
                 InIter<uint8_t, byte_order::none>, OutIter>);
    describe("from underlying-type=uint16_t,byte_order=msb0",
             &test_non_aliasing_copy_fixed_iti_itto_boo<
                 InIter<uint16_t, byte_order::msb0>, OutIter>);
    describe("from underlying-type=uint16_t,byte_order=lsb0",
             &test_non_aliasing_copy_fixed_iti_itto_boo<
                 InIter<uint16_t, byte_order::lsb0>, OutIter>);
    describe("from underlying-type=uint32_t,byte_order=msb0",
             &test_non_aliasing_copy_fixed_iti_itto_boo<
                 InIter<uint32_t, byte_order::msb0>, OutIter>);
    describe("from underlying-type=uint32_t,byte_order=lsb0",
             &test_non_aliasing_copy_fixed_iti_itto_boo<
                 InIter<uint32_t, byte_order::lsb0>, OutIter>);
    describe("from underlying-type=uint64_t,byte_order=msb0",
             &test_non_aliasing_copy_fixed_iti_itto_boo<
                 InIter<uint64_t, byte_order::msb0>, OutIter>);
    describe("from underlying-type=uint64_t,byte_order=lsb0",
             &test_non_aliasing_copy_fixed_iti_itto_boo<
                 InIter<uint64_t, byte_order::lsb0>, OutIter>);
}

////////////////////////////////////////////////////////////////////////////////
// Aliasing-specific fan-out

template <class IterIn, class IterOut>
void test_aliasing_copy()
{
    using traits_in  = bit_iterator_traits<IterIn>;
    using traits_out = bit_iterator_traits<IterOut>;
    static_assert(std::is_same<typename traits_in::underlying_type,
                               typename traits_out::underlying_type>::value,
                  "test_aliasing_copy cannot be used with iterators that have "
                  "mismatching underlying types");
    static constexpr bit_order bo_in   = traits_in::bit_order;
    static constexpr bit_order bo_out  = traits_out::bit_order;
    using ul_in                        = typename traits_in::underlying_type;
    using ul_out                       = typename traits_out::underlying_type;
    static constexpr byte_order yo_in  = traits_in::byte_order;
    static constexpr byte_order yo_out = traits_out::byte_order;
    static constexpr bool rev_in = traits_in::is_reverse;
    static constexpr bool rev_out = traits_out::is_reverse;
    testvec_t<ul_out> exp_base = testvec<bo_in, ul_in, yo_in>();
    for_each_range_copy([&](ptrdiff_t b1, ptrdiff_t b2, ptrdiff_t b3) {
        std::array<bool, testvec_bits> validate_copy{{}};
        for (auto br = b1, bw = b3; br < b2; ++br,++bw)
        {
            validate_copy[bitter::detail::bitidx<bo_out, ul_out, yo_out>(
                bitter::offset(rev_out ? testvec_bits - bw - 1 : bw))] = true;
            if (validate_copy[bitter::detail::bitidx<bo_in, ul_in, yo_in>(
                    bitter::offset(rev_in ? testvec_bits - br - 1 : br))])
                return;
        }

        testvec_t<ul_out> expected = testvec<bo_in, ul_in, yo_in>();

        IterIn it_x  = make_bit_iterator<IterIn>(exp_base, bitter::offset(b1));
        IterIn end_x = make_bit_iterator<IterIn>(exp_base, bitter::offset(b2));
        IterOut exp  = make_bit_iterator<IterOut>(expected, bitter::offset(b3));
        std::copy(it_x, end_x, exp);

        testvec_t<ul_out> data_io = testvec<bo_in, ul_in, yo_in>();

        IterIn it   = make_bit_iterator<IterIn>(data_io, bitter::offset(b1));
        IterIn end  = make_bit_iterator<IterIn>(data_io, bitter::offset(b2));
        IterOut out = make_bit_iterator<IterOut>(data_io, bitter::offset(b3));
        copy(it, end, out);
        if (data_io != expected)
            std::cout << "\n(" << (int)b1 << ", " << (int)b2 << ", " << (int)b3
                      << ")\n" << data_io << "\n" << expected << "\n";
        AssertThat(data_io, Equals(expected));
    });
}

template <typename InIter, typename OutIter>
void test_aliasing_copy_fixed_it()
{
    it("correctly performs aliasing copies",
       &test_aliasing_copy<InIter, OutIter>);
}

template <template <byte_order YO> class InIter,
          template <byte_order YO> class OutIter>
void test_aliasing_copy_fixed_itt_bo_ul()
{
    describe("from byte_order=msb0 to byte_order=msb0",
             &test_aliasing_copy_fixed_it<InIter<byte_order::msb0>,
                                          OutIter<byte_order::msb0>>);
    describe("from byte_order=msb0 to byte_order=lsb0",
             &test_aliasing_copy_fixed_it<InIter<byte_order::msb0>,
                                          OutIter<byte_order::lsb0>>);
    describe("from byte_order=lsb0 to byte_order=msb0",
             &test_aliasing_copy_fixed_it<InIter<byte_order::lsb0>,
                                          OutIter<byte_order::msb0>>);
    describe("from byte_order=lsb0 to byte_order=lsb0",
             &test_aliasing_copy_fixed_it<InIter<byte_order::lsb0>,
                                          OutIter<byte_order::lsb0>>);
}

template <template <typename UL, byte_order YO> class InIter,
          template <typename UL, byte_order YO> class OutIter>
void test_aliasing_copy_fixed_itt_bo()
{
    using in_types  = test_copy_iterator_types_fixed_itt_bo<InIter>;
    using out_types = test_copy_iterator_types_fixed_itt_bo<OutIter>;
    describe("to/from a uint8_t buffer",
             &test_aliasing_copy_fixed_it<typename in_types::uint8_iterator,
                                          typename out_types::uint8_iterator>);
    describe("to/from a uint16_t buffer",
             &test_aliasing_copy_fixed_itt_bo_ul<
                 in_types::template uint16_iterator,
                 out_types::template uint16_iterator>);
    describe("to/from a uint32_t buffer",
             &test_aliasing_copy_fixed_itt_bo_ul<
                 in_types::template uint32_iterator,
                 out_types::template uint32_iterator>);
    describe("to/from a uint64_t buffer",
             &test_aliasing_copy_fixed_itt_bo_ul<
                 in_types::template uint64_iterator,
                 out_types::template uint64_iterator>);
}

////////////////////////////////////////////////////////////////////////////////
// Shared fan-out

template <template <typename UL, byte_order YO> class InIter,
          template <bit_order BO, typename UL, byte_order YO> class OutIter>
void test_copy_fixed_itt_boi()
{
    using types = test_copy_iterator_types_fixed_itt<OutIter>;
    describe("bit_order=msb0", [] {
        test_non_aliasing_copy_fixed_itt_bo<InIter,
                                            types::template msb0_iterator>();
        test_aliasing_copy_fixed_itt_bo<InIter,
                                        types::template msb0_iterator>();
    });
    describe("bit_order=lsb0", [] {
        test_non_aliasing_copy_fixed_itt_bo<InIter,
                                            types::template lsb0_iterator>();
        test_aliasing_copy_fixed_itt_bo<InIter,
                                        types::template lsb0_iterator>();
    });
}

template <template <typename UL, byte_order YO> class InIter>
void test_copy_fixed_itti_boi()
{
    using types = test_copy_iterator_types;
    describe("to a bit_iterator",
             &test_copy_fixed_itt_boi<InIter, bitter::bit_iterator>);
    describe("to a std::reverse_iterator<bit_iterator>",
             &test_copy_fixed_itt_boi<InIter, types::reverse_bit_iterator>);
}

template <template <bit_order BO, typename UL, byte_order YO> class InIter>
void test_copy_fixed_itti()
{
    using types = test_copy_iterator_types_fixed_itt<InIter>;
    describe("from bit_order=msb0",
             &test_copy_fixed_itti_boi<types::template msb0_iterator>);
    describe("from bit_order=lsb0",
             &test_copy_fixed_itti_boi<types::template lsb0_iterator>);
}

