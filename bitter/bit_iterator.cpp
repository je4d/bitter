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

using bitvec = std::vector<bitter::bit>;
bitvec make_bitvec(std::initializer_list<int> il)
{
    bitvec ret;
    ret.reserve(il.size());
    std::transform(begin(il), end(il), back_inserter(ret),
        [](int i)->bitter::bit{
            if (i<0 or i>1)
                throw std::domain_error("make_bitvec: bits can only be 0 or 1");
            return bitter::bit(i);
        });
    return ret;
}

template <std::size_t N>
std::string hexstring(uint8_t (&data)[N])
{
    char ret[N*2];
    char* out = ret;
    std::for_each(data, data+N, [&](uint8_t c){
        std::sprintf(out, "%02X", static_cast<uint8_t>(c));
        out += 2;
    });
    return std::string(ret);
}

template <typename UL>
std::string hexstring(const UL *data, std::size_t n)
{
    const auto retlen = 2*n;
    std::vector<char> ret(retlen+1);
    char* out = ret.data();
    std::for_each(data, data+n, [&](UL e){
        std::sprintf(out, "%0*llX",
            static_cast<int>(2*sizeof(UL)), static_cast<long long unsigned>(e));
        out += 2;
    });
    *prev(end(ret)) = 0;
    return std::string(ret.data(), retlen);
}

bitter::bit expval(std::size_t pos)
{
    double b = std::log(pos+1)*97.0;
    int c = ((b - std::fmod(b,1.0))+0.5);
    int d = c%2;
    return bitter::bit(d==1);
}

const std::array<bitter::bit,testvec_bits> expvals =
    []()->std::array<bitter::bit,testvec_bits>{
        std::array<bitter::bit,testvec_bits> ret;
        for (std::size_t i = 0; i < testvec_bits; ++i)
            ret[i] = expval(i);
        return ret;
    }();


template <bit_order BO, typename UL, byte_order YO, typename F>
void for_each_bit(F&& f)
{
    testvec_t<UL> data = testvec<BO,UL,YO>();
    for (std::size_t b = 0; b < testvec_bits; ++b)
        std::forward<F>(f)(data.data(), b);
}

template <bit_order BO, typename UL, byte_order YO, typename F>
std::array<bitter::bit,testvec_bits> for_each_bit_check(F&& f)
{
    testvec_t<UL> data = testvec<BO,UL,YO>();
    std::array<bitter::bit,testvec_bits> out;
    for (std::size_t b = 0; b < testvec_bits; ++b)
        out[b] = std::forward<F>(f)(data.data(), b);
    return out;
}

template <bitter::bit_order BO, typename UL, byte_order YO, typename F>
void for_each_bit_pair(F&& f)
{
    testvec_t<UL> data = testvec<BO,UL,YO>();
    for (ptrdiff_t b1 = 0; b1 < testvec_bits; ++b1)
        for (ptrdiff_t b2 = 0; b2 < testvec_bits; ++b2)
            std::forward<F>(f)(data.data(), b1, b2);
}

template <class Iter>
void core_tests()
{
    static constexpr bitter::bit_order BO
        = bit_iterator_traits<Iter>::bit_order;

    static_assert(BO==bitter::bit_order::msb0 or BO==bitter::bit_order::lsb0,
            "unexpected bit order");

    it("supports default construction", []{
        Iter i1;
        (void) i1;
    });

    it("support comparison of default constructed objects", [] {
        Iter i1;
        Iter i2;
        AssertThat(i1 == i2, Equals(true));
        AssertThat(i1 != i2, Equals(false));
    });
}

template <class Data, class Iter>
void observing_tests()
{
    static constexpr bit_order bo  = bit_iterator_traits<Iter>::bit_order;
    using ul = typename              bit_iterator_traits<Iter>::underlying_type;
    static constexpr byte_order yo = bit_iterator_traits<Iter>::byte_order;
    static constexpr uint8_t eb = 8 * sizeof(ul); // element bits

    /* Construction and assignment *////////////////////////////////////////////

    it("supports construction from byte and bitno", []{
        for_each_bit<bo,ul,yo>([](Data* arr, std::size_t b){
            Iter i(&arr[b/eb], b%eb);
        });
    });

    it("supports construction from byte and offset", []{
        for_each_bit<bo,ul,yo>([](Data* arr, std::size_t b){
            Iter i_off(arr, bitter::offset(b));
            Iter i_direct(&arr[b/eb], b%eb);
            AssertThat(i_off, Equals(i_direct));
        });
    });

    it("supports copy-construction", []{
        for_each_bit<bo,ul,yo>([](Data* arr, std::size_t b){
            const Iter i1(&arr[b/eb], b%eb);
            Iter i2(i1);
            AssertThat(i1, Equals(i2));
            AssertThat(*i2, Equals(*i2));
        });
    });

    it("supports assignment", []{
        Iter i1;
        for_each_bit<bo,ul,yo>([&](Data* arr, std::size_t b){
            Iter i2(&arr[b/eb], b%eb);
            i1 = i2;
            AssertThat(i1, Equals(i2));
            AssertThat(*i1, Equals(*i2));
        });
    });

    it("is swappable", []{
        for_each_bit_pair<bo,ul,yo>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/eb], b1%eb);
            const Iter i1s(&arr[b1/eb], b1%eb);
            Iter i2(&arr[b2/eb], b2%eb);
            const Iter i2s(&arr[b2/eb], b2%eb);
            swap(i1, i2);
            AssertThat(i1, Equals(i2s));
            AssertThat(i2, Equals(i1s));
            AssertThat(*i1, Equals(*i2s));
            AssertThat(*i2, Equals(*i1s));
        });
    });

    it("is swappable when LHS is moved", []{
        for_each_bit_pair<bo,ul,yo>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/eb], b1%eb);
            const Iter i1s(&arr[b1/eb], b1%eb);
            Iter i2(&arr[b2/eb], b2%eb);
            const Iter i2s(&arr[b2/eb], b2%eb);
            swap(std::move(i1), i2);
            AssertThat(i2, Equals(i1s));
            AssertThat(*i2, Equals(*i1s));
        });
    });

    it("is swappable when RHS is moved", []{
        for_each_bit_pair<bo,ul,yo>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/eb], b1%eb);
            const Iter i1s(&arr[b1/eb], b1%eb);
            Iter i2(&arr[b2/eb], b2%eb);
            const Iter i2s(&arr[b2/eb], b2%eb);
            swap(i1, std::move(i2));
            AssertThat(i1, Equals(i2s));
            AssertThat(*i1, Equals(*i2s));
        });
    });

    /* Dereferencing *//////////////////////////////////////////////////////////

    it("is dereferencable with ->", []{
        AssertThat((for_each_bit_check<bo,ul,yo>([](Data* arr, std::size_t b)->bitter::bit {
            Iter i(&arr[b/eb], b%eb);
            return bitter::bit(i->operator bool());
        })), Equals(expvals));
    });

    it("is dereferencable with *", []{
        for_each_bit<bo,ul,yo>([](Data* arr, std::size_t b){
            Iter i(&arr[b/eb], b%eb);
            AssertThat(*i, Equals(expval(b)));
        });
    });

    it("is dereferencable with []", []{
        for_each_bit_pair<bo,ul,yo>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/eb], b1%eb);
            const Iter i1s(&arr[b1/eb], b1%eb);
            std::ptrdiff_t delta = b2 - b1;
            AssertThat(i1[delta], Equals(expval(b2)));
            AssertThat(i1, Equals(i1s));
        });
    });

    /* Navigation mutators *////////////////////////////////////////////////////

    it("supports preincrement", []{
        for_each_bit<bo,ul,yo>([](Data* arr, std::size_t b){
            Iter i1(&arr[b/eb], b%eb);
            const Iter i2(&arr[(b+1)/eb], (b+1)%eb);
            AssertThat(++i1, Equals(i2));
            AssertThat(i1, Equals(i2));
            if (b+1 < testvec_bits) AssertThat(*i1, Equals(expval(b+1)));
        });
    });

    it("supports postincrement", []{
        for_each_bit<bo,ul,yo>([](Data* arr, std::size_t b){
            Iter i1(&arr[b/eb], b%eb);
            const Iter i2(&arr[b/eb], b%eb);
            const Iter i3(&arr[(b+1)/eb], (b+1)%eb);
            AssertThat(i1++, Equals(i2));
            AssertThat(i1, Equals(i3));
            if (b+1 < testvec_bits) AssertThat(*i1, Equals(expval(b+1)));
        });
    });

    it("supports predecrement", []{
        for_each_bit<bo,ul,yo>([](Data* arr, std::size_t b){
            Iter i1(&arr[(b+1)/eb], (b+1)%eb);
            Iter i2(&arr[b/eb], b%eb);
            AssertThat(--i1, Equals(i2));
            AssertThat(i1, Equals(i2));
            AssertThat(*i1, Equals(expval(b)));
        });
    });

    it("supports postdecrement", []{
        for_each_bit<bo,ul,yo>([](Data* arr, std::size_t b){
            Iter i1(&arr[(b+1)/eb], (b+1)%eb);
            Iter i2(&arr[(b+1)/eb], (b+1)%eb);
            Iter i3(&arr[b/eb], b%eb);
            AssertThat(i1--, Equals(i2));
            AssertThat(i1, Equals(i3));
            AssertThat(*i1, Equals(expval(b)));
        });
    });

    it("supports addition-assignment", []{
        for_each_bit_pair<bo,ul,yo>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            ptrdiff_t delta = b2 - b1;
            Iter i1(&arr[b1/eb], b1%eb);
            Iter i2(&arr[b2/eb], b2%eb);
            AssertThat(i1+=delta, Equals(i2));
            AssertThat(i1, Equals(i2));
            AssertThat(*i1, Equals(expval(b2)));
        });
    });

    it("supports subtraction-assignment", []{
        for_each_bit_pair<bo,ul,yo>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            std::ptrdiff_t delta = b1 - b2;
            Iter i1(&arr[b1/eb], b1%eb);
            Iter i2(&arr[b2/eb], b2%eb);
            AssertThat(i1-=delta, Equals(i2));
            AssertThat(i1, Equals(i2));
            AssertThat(*i1, Equals(expval(b2)));
        });
    });

    /* Navigation free function opearators *////////////////////////////////////

    it("supports subtracting one iterator from another", []{
        for_each_bit_pair<bo,ul,yo>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/eb], b1%eb);
            Iter i2(&arr[b2/eb], b2%eb);
            AssertThat(i2 - i1, Equals(b2 - b1));
        });
    });

    it("supports addition of iterator with ptrdiff_t", []{
        for_each_bit_pair<bo,ul,yo>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/eb], b1%eb);
            const Iter i1s(&arr[b1/eb], b1%eb);
            Iter i2(&arr[b2/eb], b2%eb);
            const Iter i2s(&arr[b2/eb], b2%eb);
            std::ptrdiff_t delta = b2 - b1;
            AssertThat(i1 + delta, Equals(i2));
            AssertThat(*(i1 + delta), Equals(expval(b2)));
            AssertThat(i1, Equals(i1s));
            AssertThat(i2, Equals(i2s));
        });
    });

    it("supports addition of ptrdiff_t with iterator", []{
        for_each_bit_pair<bo,ul,yo>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/eb], b1%eb);
            const Iter i1s(&arr[b1/eb], b1%eb);
            Iter i2(&arr[b2/eb], b2%eb);
            const Iter i2s(&arr[b2/eb], b2%eb);
            std::ptrdiff_t delta = b2 - b1;
            AssertThat(delta + i1, Equals(i2));
            AssertThat(*(delta + i1), Equals(expval(b2)));
            AssertThat(i1, Equals(i1s));
            AssertThat(i1, Equals(i1s));
            AssertThat(i2, Equals(i2s));
        });
    });

    it("supports subtraction of ptrdiff_t from iterator", []{
        for_each_bit_pair<bo,ul,yo>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/eb], b1%eb);
            const Iter i1s(&arr[b1/eb], b1%eb);
            Iter i2(&arr[b2/eb], b2%eb);
            const Iter i2s(&arr[b2/eb], b2%eb);
            std::ptrdiff_t delta = b1 - b2;
            AssertThat(i1 - delta, Equals(i2));
            AssertThat(*(i1 - delta), Equals(expval(b2)));
            AssertThat(i1, Equals(i1s));
            AssertThat(i2, Equals(i2s));
        });
    });

    /* Comparison free function opearators *////////////////////////////////////

    it("supports equality comparison", []{
        for_each_bit_pair<bo,ul,yo>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/eb], b1%eb);
            Iter i2(&arr[b2/eb], b2%eb);
            AssertThat(i1 == i2, Equals(b1 == b2));
        });
    });

    it("supports inequality comparison", []{
        for_each_bit_pair<bo,ul,yo>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/eb], b1%eb);
            Iter i2(&arr[b2/eb], b2%eb);
            AssertThat(i1 != i2, Equals(b1 != b2));
        });
    });

    /* Relational free function opearators *////////////////////////////////////

    it("supports less-than comparison", []{
        for_each_bit_pair<bo,ul,yo>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/eb], b1%eb);
            Iter i2(&arr[b2/eb], b2%eb);
            AssertThat(i1 < i2, Equals(b1 < b2));
        });
    });

    it("supports less-than-or-equal comparison", []{
        for_each_bit_pair<bo,ul,yo>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/eb], b1%eb);
            Iter i2(&arr[b2/eb], b2%eb);
            AssertThat(i1 <= i2, Equals(b1 <= b2));
        });
    });

    it("supports greater-than comparison", []{
        for_each_bit_pair<bo,ul,yo>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/eb], b1%eb);
            Iter i2(&arr[b2/eb], b2%eb);
            AssertThat(i1 > i2, Equals(b1 > b2));
        });
    });

    it("supports greater-than-or-equal comparison", []{
        for_each_bit_pair<bo,ul,yo>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/eb], b1%eb);
            Iter i2(&arr[b2/eb], b2%eb);
            AssertThat(i1 >= i2, Equals(b1 >= b2));
        });
    });

    /* Misc *///////////////////////////////////////////////////////////////////

    if (sizeof(ul) == 1) {
        it("satisfies the examples in NIST FIPS 202, bytes -> bits", []{
            std::array<Data,1> data = {{0xA3}};
            const bitvec interpreted_as_bits =
                (bo == bit_order::msb0)
                    ? make_bitvec({1,0,1,0, 0,0,1,1})
                    : make_bitvec({1,1,0,0, 0,1,0,1});

            bitvec converted_to_bits; std::copy(Iter(begin(data), 0), Iter(end(data), 0), back_inserter(converted_to_bits));
            AssertThat(converted_to_bits, EqualsContainer(interpreted_as_bits));
        });
    }
}

template <typename UL, typename F>
testvec_t<UL> for_each_bit2(F&& f)
{
    testvec_t<UL> ret{};
    for (std::size_t b = 0; b < 8*sizeof(UL)*ret.size(); ++b)
        std::forward<F>(f)(ret.data(), b);
    return ret;
}

template <typename OutIt>
std::string copy_and_hexify(const bitvec& source, ptrdiff_t n)
{
    using ul = typename bit_iterator_traits<OutIt>::underlying_type;
    auto elements = (n+8*sizeof(ul)-1)/(8*sizeof(ul));
    std::vector<ul> out(elements);
    std::copy(begin(source), next(begin(source), n), OutIt(out.data(), 0));
    return hexstring(out.data(), elements);
}

template <class Iter>
void mutating_tests()
{
    static constexpr bit_order bo =  bit_iterator_traits<Iter>::bit_order;
    using ul = typename              bit_iterator_traits<Iter>::underlying_type;
    static constexpr byte_order yo = bit_iterator_traits<Iter>::byte_order;
    static constexpr uint8_t eb = 8 * sizeof(ul); // element bits
    static constexpr std::size_t testvec_elements = testvec_bits/(8*sizeof(ul));

    it("supports dereference-and-assign", []{
        AssertThat((for_each_bit2<ul>([&](ul* arr, std::size_t b){
            Iter i1(&arr[b/eb], b%eb);
            *i1 = expval(b);
        })), Equals(testvec<bo,ul,yo>()));
    });

    it("supports dereference-postincrement-and-assign", []{
        std::array<ul,testvec_elements> arr{};
        Iter it(arr.data(),0);
        for (std::size_t b = 0; b < testvec_bits; ++b)
            *it++ = expval(b);
        AssertThat(arr, Equals(testvec<bo,ul,yo>()));
    });

    if (sizeof(ul) == 1) {
        it("satisfies the examples in NIST FIPS 202, bits -> bytes", []{
            ul data_out[] = {0,0};
            const bitvec interpreted_as_bits =
                (bo == bit_order::msb0)
                    ? make_bitvec({1,0,1,0, 0,0,1,1})
                    : make_bitvec({1,1,0,0, 0,1,0,1});
            std::copy(begin(interpreted_as_bits), end(interpreted_as_bits),
                      Iter(data_out,0));
            AssertThat(static_cast<uint8_t>(data_out[0]), Equals(0xa3));
            AssertThat(static_cast<uint8_t>(data_out[1]), Equals(0));
        });

    it("handles partial bytes", [&]{
            std::vector<const char*> results =
                (bo == bit_order::msb0)
                    ? std::vector<const char*>{"",
                                               "80",   "C0",   "E0",   "F0",
                                               "F8",   "FC",   "FE",   "FF",
                                               "FF80", "FFC0", "FFE0", "FFF0",
                                               "FFF8", "FFFC", "FFFE", "FFFF"}
                    : std::vector<const char*>{"",
                                               "01",   "03",   "07",   "0F",
                                               "1F",   "3F",   "7F",   "FF",
                                               "FF01", "FF03", "FF07", "FF0F",
                                               "FF1F", "FF3F", "FF7F", "FFFF"};
            const bitvec bits16(16, bitter::bit(1));
            for (int i = 0; i <= 16; ++i)
                AssertThat(copy_and_hexify<Iter>(bits16, i),
                    Equals(results.at(i)));
        });
    }
}

namespace sfinae {
template <typename T, typename U>
auto deref_assign(T &&t,
                  U &&u) -> decltype(*std::forward<T>(t) = std::forward<U>(u),
                                     std::declval<std::true_type>())
{
    return {};
}
std::false_type deref_assign(...) { return {}; }
}

template <class Iter>
void const_tests()
{
    it("cannot be dereferenced and assigned to", [] {
        Iter it;
        AssertThat(std::remove_reference<decltype(
                       sfinae::deref_assign(it, *it))>::type::value,
                   Equals(false));
    });
}

template <bit_order BO, typename UL, byte_order YO = byte_order::none>
void single_iterator_config_tests()
{
    using namespace bitter;
    describe("const_bit_iterator<bit_order,underlier,byte_order>", []{
        core_tests<const_bit_iterator<BO,UL,YO>>();
        const_tests<const_bit_iterator<BO,UL,YO>>();
        describe("over const array",
            &observing_tests<const UL,const_bit_iterator<BO,UL,YO>>);
        describe("over mutable array",
            &observing_tests<UL,const_bit_iterator<BO,UL,YO>>);
    });
    describe("bit_iterator<bitter::bit_order BO>", []{
        core_tests<bit_iterator<BO,UL,YO>>();
        observing_tests<UL,bit_iterator<BO,UL,YO>>();
        mutating_tests<bit_iterator<BO,UL,YO>>();
    });
}

template <bitter::bit_order BO>
void single_iterator_one_bitorder_tests()
{
    describe("over uint8_t", single_iterator_config_tests<BO,uint8_t>);
    describe("over byte_order::msb0 uint16_t",
        single_iterator_config_tests<BO,uint16_t,byte_order::msb0>);
    describe("over byte_order::msb0 uint32_t",
        single_iterator_config_tests<BO,uint32_t,byte_order::msb0>);
    describe("over byte_order::msb0 uint64_t",
        single_iterator_config_tests<BO,uint64_t,byte_order::msb0>);
    describe("over byte_order::lsb0 uint16_t",
        single_iterator_config_tests<BO,uint16_t,byte_order::lsb0>);
    describe("over byte_order::lsb0 uint32_t",
        single_iterator_config_tests<BO,uint32_t,byte_order::lsb0>);
    describe("over byte_order::lsb0 uint64_t",
        single_iterator_config_tests<BO,uint64_t,byte_order::lsb0>);
}

////////////////////////////////////////////////////////////////////////////////
// main

go_bandit([]{
    using namespace bitter;
    static constexpr bit_order msb0 = bit_order::msb0;
    describe("msb0 iterators", single_iterator_one_bitorder_tests<msb0>);
    static constexpr bit_order lsb0 = bit_order::lsb0;
    describe("lsb0 iterators", single_iterator_one_bitorder_tests<lsb0>);
    describe("byteidx", []{
        using namespace bitter::detail;
        using namespace bitter;
        it("supports most-significant-byte-is-0", [&]{
            AssertThat((byteidx<char,byte_order::msb0>(0)),Equals(0));
            AssertThat((byteidx<uint8_t,byte_order::msb0>(0)),Equals(0));
            AssertThat((byteidx<uint16_t,byte_order::msb0>(0)),Equals(1));
            AssertThat((byteidx<uint16_t,byte_order::msb0>(1)),Equals(0));
            AssertThat((byteidx<uint32_t,byte_order::msb0>(0)),Equals(3));
            AssertThat((byteidx<uint32_t,byte_order::msb0>(1)),Equals(2));
            AssertThat((byteidx<uint32_t,byte_order::msb0>(2)),Equals(1));
            AssertThat((byteidx<uint32_t,byte_order::msb0>(3)),Equals(0));
            AssertThat((byteidx<uint64_t,byte_order::msb0>(0)),Equals(7));
            AssertThat((byteidx<uint64_t,byte_order::msb0>(1)),Equals(6));
            AssertThat((byteidx<uint64_t,byte_order::msb0>(2)),Equals(5));
            AssertThat((byteidx<uint64_t,byte_order::msb0>(3)),Equals(4));
            AssertThat((byteidx<uint64_t,byte_order::msb0>(4)),Equals(3));
            AssertThat((byteidx<uint64_t,byte_order::msb0>(5)),Equals(2));
            AssertThat((byteidx<uint64_t,byte_order::msb0>(6)),Equals(1));
            AssertThat((byteidx<uint64_t,byte_order::msb0>(7)),Equals(0));
        });

        it("supports least-significant-byte-is-0", [&]{
            AssertThat((byteidx<char,byte_order::lsb0>(0)),Equals(0));
            AssertThat((byteidx<uint8_t,byte_order::lsb0>(0)),Equals(0));
            AssertThat((byteidx<uint16_t,byte_order::lsb0>(0)),Equals(0));
            AssertThat((byteidx<uint16_t,byte_order::lsb0>(1)),Equals(1));
            AssertThat((byteidx<uint32_t,byte_order::lsb0>(0)),Equals(0));
            AssertThat((byteidx<uint32_t,byte_order::lsb0>(1)),Equals(1));
            AssertThat((byteidx<uint32_t,byte_order::lsb0>(2)),Equals(2));
            AssertThat((byteidx<uint32_t,byte_order::lsb0>(3)),Equals(3));
            AssertThat((byteidx<uint64_t,byte_order::lsb0>(0)),Equals(0));
            AssertThat((byteidx<uint64_t,byte_order::lsb0>(1)),Equals(1));
            AssertThat((byteidx<uint64_t,byte_order::lsb0>(2)),Equals(2));
            AssertThat((byteidx<uint64_t,byte_order::lsb0>(3)),Equals(3));
            AssertThat((byteidx<uint64_t,byte_order::lsb0>(4)),Equals(4));
            AssertThat((byteidx<uint64_t,byte_order::lsb0>(5)),Equals(5));
            AssertThat((byteidx<uint64_t,byte_order::lsb0>(6)),Equals(6));
            AssertThat((byteidx<uint64_t,byte_order::lsb0>(7)),Equals(7));
        });
    });

    describe("bitidx, lsb0", []{
        using namespace bitter::detail;
        using namespace bitter;
        constexpr auto msb0 = bit_order::msb0;
        constexpr auto lsb0 = bit_order::lsb0;
        it("supports one-byte types/msb0", [&]{
            AssertThat((bitidx<msb0,char,byte_order::msb0>(0)),Equals(7));
            AssertThat((bitidx<msb0,char,byte_order::msb0>(1)),Equals(6));
            AssertThat((bitidx<msb0,char,byte_order::msb0>(2)),Equals(5));
            AssertThat((bitidx<msb0,char,byte_order::msb0>(3)),Equals(4));
            AssertThat((bitidx<msb0,char,byte_order::msb0>(4)),Equals(3));
            AssertThat((bitidx<msb0,char,byte_order::msb0>(5)),Equals(2));
            AssertThat((bitidx<msb0,char,byte_order::msb0>(6)),Equals(1));
            AssertThat((bitidx<msb0,char,byte_order::msb0>(7)),Equals(0));

            AssertThat((bitidx<msb0,uint8_t,byte_order::msb0>(0)),Equals(7));
            AssertThat((bitidx<msb0,uint8_t,byte_order::msb0>(1)),Equals(6));
            AssertThat((bitidx<msb0,uint8_t,byte_order::msb0>(2)),Equals(5));
            AssertThat((bitidx<msb0,uint8_t,byte_order::msb0>(3)),Equals(4));
            AssertThat((bitidx<msb0,uint8_t,byte_order::msb0>(4)),Equals(3));
            AssertThat((bitidx<msb0,uint8_t,byte_order::msb0>(5)),Equals(2));
            AssertThat((bitidx<msb0,uint8_t,byte_order::msb0>(6)),Equals(1));
            AssertThat((bitidx<msb0,uint8_t,byte_order::msb0>(7)),Equals(0));
        });

        it("supports one-byte types/lsb0", [&]{
            AssertThat((bitidx<lsb0,char,byte_order::msb0>(0)),Equals(0));
            AssertThat((bitidx<lsb0,char,byte_order::msb0>(1)),Equals(1));
            AssertThat((bitidx<lsb0,char,byte_order::msb0>(2)),Equals(2));
            AssertThat((bitidx<lsb0,char,byte_order::msb0>(3)),Equals(3));
            AssertThat((bitidx<lsb0,char,byte_order::msb0>(4)),Equals(4));
            AssertThat((bitidx<lsb0,char,byte_order::msb0>(5)),Equals(5));
            AssertThat((bitidx<lsb0,char,byte_order::msb0>(6)),Equals(6));
            AssertThat((bitidx<lsb0,char,byte_order::msb0>(7)),Equals(7));

            AssertThat((bitidx<lsb0,uint8_t,byte_order::msb0>(0)),Equals(0));
            AssertThat((bitidx<lsb0,uint8_t,byte_order::msb0>(1)),Equals(1));
            AssertThat((bitidx<lsb0,uint8_t,byte_order::msb0>(2)),Equals(2));
            AssertThat((bitidx<lsb0,uint8_t,byte_order::msb0>(3)),Equals(3));
            AssertThat((bitidx<lsb0,uint8_t,byte_order::msb0>(4)),Equals(4));
            AssertThat((bitidx<lsb0,uint8_t,byte_order::msb0>(5)),Equals(5));
            AssertThat((bitidx<lsb0,uint8_t,byte_order::msb0>(6)),Equals(6));
            AssertThat((bitidx<lsb0,uint8_t,byte_order::msb0>(7)),Equals(7));
        });

        it("supports big endian/msb0", [&]{
            AssertThat((bitidx<msb0,uint16_t,byte_order::msb0>( 0)),Equals(15));
            AssertThat((bitidx<msb0,uint16_t,byte_order::msb0>( 1)),Equals(14));
            AssertThat((bitidx<msb0,uint16_t,byte_order::msb0>( 2)),Equals(13));
            AssertThat((bitidx<msb0,uint16_t,byte_order::msb0>( 3)),Equals(12));
            AssertThat((bitidx<msb0,uint16_t,byte_order::msb0>( 4)),Equals(11));
            AssertThat((bitidx<msb0,uint16_t,byte_order::msb0>( 5)),Equals(10));
            AssertThat((bitidx<msb0,uint16_t,byte_order::msb0>( 6)),Equals( 9));
            AssertThat((bitidx<msb0,uint16_t,byte_order::msb0>( 7)),Equals( 8));
            AssertThat((bitidx<msb0,uint16_t,byte_order::msb0>( 8)),Equals( 7));
            AssertThat((bitidx<msb0,uint16_t,byte_order::msb0>( 9)),Equals( 6));
            AssertThat((bitidx<msb0,uint16_t,byte_order::msb0>(10)),Equals( 5));
            AssertThat((bitidx<msb0,uint16_t,byte_order::msb0>(11)),Equals( 4));
            AssertThat((bitidx<msb0,uint16_t,byte_order::msb0>(12)),Equals( 3));
            AssertThat((bitidx<msb0,uint16_t,byte_order::msb0>(13)),Equals( 2));
            AssertThat((bitidx<msb0,uint16_t,byte_order::msb0>(14)),Equals( 1));
            AssertThat((bitidx<msb0,uint16_t,byte_order::msb0>(15)),Equals( 0));

            AssertThat((bitidx<msb0,uint32_t,byte_order::msb0>( 8)),Equals(23));
            AssertThat((bitidx<msb0,uint32_t,byte_order::msb0>( 9)),Equals(22));
            AssertThat((bitidx<msb0,uint32_t,byte_order::msb0>(10)),Equals(21));
            AssertThat((bitidx<msb0,uint32_t,byte_order::msb0>(11)),Equals(20));
            AssertThat((bitidx<msb0,uint32_t,byte_order::msb0>(12)),Equals(19));
            AssertThat((bitidx<msb0,uint32_t,byte_order::msb0>(13)),Equals(18));
            AssertThat((bitidx<msb0,uint32_t,byte_order::msb0>(14)),Equals(17));
            AssertThat((bitidx<msb0,uint32_t,byte_order::msb0>(15)),Equals(16));

            AssertThat((bitidx<msb0,uint64_t,byte_order::msb0>(40)),Equals(23));
            AssertThat((bitidx<msb0,uint64_t,byte_order::msb0>(41)),Equals(22));
            AssertThat((bitidx<msb0,uint64_t,byte_order::msb0>(42)),Equals(21));
            AssertThat((bitidx<msb0,uint64_t,byte_order::msb0>(43)),Equals(20));
            AssertThat((bitidx<msb0,uint64_t,byte_order::msb0>(44)),Equals(19));
            AssertThat((bitidx<msb0,uint64_t,byte_order::msb0>(45)),Equals(18));
            AssertThat((bitidx<msb0,uint64_t,byte_order::msb0>(46)),Equals(17));
            AssertThat((bitidx<msb0,uint64_t,byte_order::msb0>(47)),Equals(16));
        });

        it("supports big endian/lsb0", [&]{
            AssertThat((bitidx<lsb0,uint16_t,byte_order::msb0>( 0)),Equals( 8));
            AssertThat((bitidx<lsb0,uint16_t,byte_order::msb0>( 1)),Equals( 9));
            AssertThat((bitidx<lsb0,uint16_t,byte_order::msb0>( 2)),Equals(10));
            AssertThat((bitidx<lsb0,uint16_t,byte_order::msb0>( 3)),Equals(11));
            AssertThat((bitidx<lsb0,uint16_t,byte_order::msb0>( 4)),Equals(12));
            AssertThat((bitidx<lsb0,uint16_t,byte_order::msb0>( 5)),Equals(13));
            AssertThat((bitidx<lsb0,uint16_t,byte_order::msb0>( 6)),Equals(14));
            AssertThat((bitidx<lsb0,uint16_t,byte_order::msb0>( 7)),Equals(15));
            AssertThat((bitidx<lsb0,uint16_t,byte_order::msb0>( 8)),Equals(0));
            AssertThat((bitidx<lsb0,uint16_t,byte_order::msb0>( 9)),Equals(1));
            AssertThat((bitidx<lsb0,uint16_t,byte_order::msb0>(10)),Equals(2));
            AssertThat((bitidx<lsb0,uint16_t,byte_order::msb0>(11)),Equals(3));
            AssertThat((bitidx<lsb0,uint16_t,byte_order::msb0>(12)),Equals(4));
            AssertThat((bitidx<lsb0,uint16_t,byte_order::msb0>(13)),Equals(5));
            AssertThat((bitidx<lsb0,uint16_t,byte_order::msb0>(14)),Equals(6));
            AssertThat((bitidx<lsb0,uint16_t,byte_order::msb0>(15)),Equals(7));

            AssertThat((bitidx<lsb0,uint32_t,byte_order::msb0>( 8)),Equals(16));
            AssertThat((bitidx<lsb0,uint32_t,byte_order::msb0>( 9)),Equals(17));
            AssertThat((bitidx<lsb0,uint32_t,byte_order::msb0>(10)),Equals(18));
            AssertThat((bitidx<lsb0,uint32_t,byte_order::msb0>(11)),Equals(19));
            AssertThat((bitidx<lsb0,uint32_t,byte_order::msb0>(12)),Equals(20));
            AssertThat((bitidx<lsb0,uint32_t,byte_order::msb0>(13)),Equals(21));
            AssertThat((bitidx<lsb0,uint32_t,byte_order::msb0>(14)),Equals(22));
            AssertThat((bitidx<lsb0,uint32_t,byte_order::msb0>(15)),Equals(23));

            AssertThat((bitidx<lsb0,uint64_t,byte_order::msb0>(40)),Equals(16));
            AssertThat((bitidx<lsb0,uint64_t,byte_order::msb0>(41)),Equals(17));
            AssertThat((bitidx<lsb0,uint64_t,byte_order::msb0>(42)),Equals(18));
            AssertThat((bitidx<lsb0,uint64_t,byte_order::msb0>(43)),Equals(19));
            AssertThat((bitidx<lsb0,uint64_t,byte_order::msb0>(44)),Equals(20));
            AssertThat((bitidx<lsb0,uint64_t,byte_order::msb0>(45)),Equals(21));
            AssertThat((bitidx<lsb0,uint64_t,byte_order::msb0>(46)),Equals(22));
            AssertThat((bitidx<lsb0,uint64_t,byte_order::msb0>(47)),Equals(23));
        });
    });
});

int main(int argc, char** argv)
{
    return bandit::run(argc, argv);
}
