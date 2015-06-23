#include "test_bit_iterator_copy.hpp"

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
    using DataIn = typename bit_iterator_traits<InIter>::const_underlying_type;
    it("copies correctly", &copy_fwd_fwd<DataIn, InIter, OutIter>);
}

template <typename InIter, template <typename UL, byte_order YO> class OutIter>
void test_copy_fixed_iti_itto_boo()
{
    describe("underlying-type=uint8_t", [] {
        test_copy_fixed_iti_ito<InIter, OutIter<uint8_t, byte_order::none>>();
    });
    describe("underlying-type=uint16_t byte_order=msb0", [] {
        test_copy_fixed_iti_ito<InIter, OutIter<uint16_t, byte_order::msb0>>();
    });
    describe("underlying-type=uint16_t byte_order=lsb0", [] {
        test_copy_fixed_iti_ito<InIter, OutIter<uint16_t, byte_order::lsb0>>();
    });
    describe("underlying-type=uint32_t byte_order=msb0", [] {
        test_copy_fixed_iti_ito<InIter, OutIter<uint32_t, byte_order::msb0>>();
    });
    describe("underlying-type=uint32_t byte_order=lsb0", [] {
        test_copy_fixed_iti_ito<InIter, OutIter<uint32_t, byte_order::lsb0>>();
    });
    describe("underlying-type=uint64_t byte_order=msb0", [] {
        test_copy_fixed_iti_ito<InIter, OutIter<uint64_t, byte_order::msb0>>();
    });
    describe("underlying-type=uint64_t byte_order=lsb0", [] {
        test_copy_fixed_iti_ito<InIter, OutIter<uint64_t, byte_order::lsb0>>();
    });
}

template <typename InIter,
          template <bit_order BO, typename UL, byte_order YO> class OutIter>
void test_copy_fixed_iti_itto()
{
    using types = test_copy_iterator_types_fixed_itt<OutIter>;
    describe("bit_order=msb0", [] {
        test_copy_fixed_iti_itto_boo<InIter, types::template msb0_iterator>();
    });
    describe("bit_order=lsb0", [] {
        test_copy_fixed_iti_itto_boo<InIter, types::template lsb0_iterator>();
    });
}

template <typename InIter>
void test_copy_fixed_iti()
{
//    using types = test_copy_iterator_types;
    describe("copying to a bit_iterator", [] {
        test_copy_fixed_iti_itto<InIter, bitter::bit_iterator>(); });
//    describe("copying to a std::reverse_iterator<bit_iterator>", [] {
//        test_copy_fixed_iti_itto<InIter, types::reverse_bit_iterator>(); });
}

template <template <typename UL, byte_order YO> class InIter>
void test_copy_fixed_itti_boi()
{
    describe("underlying-type=uint8_t",
             [] { test_copy_fixed_iti<InIter<uint8_t, byte_order::none>>(); });
    describe("underlying-type=uint16_t byte_order=msb0",
             [] { test_copy_fixed_iti<InIter<uint16_t, byte_order::msb0>>(); });
    describe("underlying-type=uint16_t byte_order=lsb0",
             [] { test_copy_fixed_iti<InIter<uint16_t, byte_order::lsb0>>(); });
    describe("underlying-type=uint32_t byte_order=msb0",
             [] { test_copy_fixed_iti<InIter<uint32_t, byte_order::msb0>>(); });
    describe("underlying-type=uint32_t byte_order=lsb0",
             [] { test_copy_fixed_iti<InIter<uint32_t, byte_order::lsb0>>(); });
    describe("underlying-type=uint64_t byte_order=msb0",
             [] { test_copy_fixed_iti<InIter<uint64_t, byte_order::msb0>>(); });
    describe("underlying-type=uint64_t byte_order=lsb0",
             [] { test_copy_fixed_iti<InIter<uint64_t, byte_order::lsb0>>(); });
}

template <template <bit_order BO, typename UL, byte_order YO> class InIter>
void test_copy_fixed_itti()
{
    using types = test_copy_iterator_types_fixed_itt<InIter>;
    describe("bit_order=msb0",
             [] { test_copy_fixed_itti_boi<types::template msb0_iterator>(); });
    describe("bit_order=lsb0",
             [] { test_copy_fixed_itti_boi<types::template lsb0_iterator>(); });
}

void test_copy_all()
{
//    using types = test_copy_iterator_types;
    describe("copying from a bit_iterator",
             [] { test_copy_fixed_itti<bitter::bit_iterator>(); });
//    describe("copying from a std::reverse_iterator<bit_iterator>",
//             [] { test_copy_fixed_itti<types::reverse_bit_iterator>(); });
    describe("copying from a const_bit_iterator",
             [] { test_copy_fixed_itti<bitter::const_bit_iterator>(); });
//    describe("copying from a std::reverse_iterator<const_bit_iterator>",
//             [] { test_copy_fixed_itti<types::const_reverse_bit_iterator>(); });
}

go_bandit([]{
    using namespace bitter;
    test_copy_all();
});

int main(int argc, char** argv)
{
    return bandit::run(argc, argv);
}
