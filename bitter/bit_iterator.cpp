#include <bandit/bandit.h>
using namespace bandit;

#include "bit_iterator.hpp"

#include <cstdint>
#include <cmath>

using std::size_t;
using std::ptrdiff_t;
using bit_order = bitter::bit_order;
using byte_order = bitter::byte_order;

const unsigned char* operator "" _uc(const char* str, std::size_t len)
{ return reinterpret_cast<const unsigned char*>(str); }

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

template <typename T>
struct ArrayDelete {
    void operator()(T*t) { delete[] t; }
};

std::string hexstring(uint8_t *data, std::size_t n)
{
    const auto sz = (n+7)/8;
    const auto retlen = 2*sz;
    std::vector<char> ret(retlen+1);
    char* out = ret.data();
    std::for_each(data, data+sz, [&](uint8_t c){
        std::sprintf(out, "%02X", static_cast<uint8_t>(c));
        out += 2;
    });
    *prev(end(ret)) = 0;
    return std::string(ret.data(), retlen);
}

static constexpr std::size_t testvec_bits = 32*8;

template <bit_order BO, typename UL, byte_order EI = byte_order::none>
struct testvec_;

template<> struct testvec_<bit_order::lsb0,uint8_t> {
    static constexpr std::array<uint8_t,testvec_bits/8> data {{
        0xa2, 0x2b, 0x34, 0x6c, 0xc9, 0xf8, 0x1f, 0xce,
        0x4c, 0xd2, 0x4a, 0x55, 0x55, 0xb5, 0x5a, 0x6b,
        0x69, 0x4b, 0x92, 0x6d, 0xb2, 0x99, 0xcc, 0xcc,
        0xcc, 0xcc, 0x8c, 0x19, 0x73, 0xce, 0x18, 0xe7 }}; };
constexpr std::array<uint8_t,testvec_bits/8>
     testvec_<bit_order::lsb0,uint8_t>::data;
template<> struct testvec_<bit_order::lsb0,uint16_t,byte_order::little_endian> {
    static constexpr std::array<uint16_t,testvec_bits/16> data {{
        0x2ba2, 0x6c34, 0xf8c9, 0xce1f, 0xd24c, 0x554a, 0xb555, 0x6b5a,
        0x4b69, 0x6d92, 0x99b2, 0xcccc, 0xcccc, 0x198c, 0xce73, 0xe718 }}; };
constexpr std::array<uint16_t,testvec_bits/16>
     testvec_<bit_order::lsb0,uint16_t,byte_order::little_endian>::data;
template<> struct testvec_<bit_order::lsb0,uint32_t,byte_order::little_endian> {
    static constexpr std::array<uint32_t,testvec_bits/32> data {{
        0x6c342ba2, 0xce1ff8c9, 0x554ad24c, 0x6b5ab555,
        0x6d924b69, 0xcccc99b2, 0x198ccccc, 0xe718ce73 }}; };
constexpr std::array<uint32_t,testvec_bits/32>
     testvec_<bit_order::lsb0,uint32_t,byte_order::little_endian>::data;
template<> struct testvec_<bit_order::lsb0,uint64_t,byte_order::little_endian> {
    static constexpr std::array<uint64_t,testvec_bits/64> data {{
        0xce1ff8c96c342ba2, 0x6b5ab555554ad24c,
        0xcccc99b26d924b69, 0xe718ce73198ccccc }}; };
constexpr std::array<uint64_t,testvec_bits/64>
     testvec_<bit_order::lsb0,uint64_t,byte_order::little_endian>::data;
template<> struct testvec_<bit_order::lsb0,uint32_t,byte_order::pdp_endian> {
    static constexpr std::array<uint32_t,testvec_bits/32> data {{
        0x2ba26c34, 0xf8c9ce1f, 0xd24c554a, 0xb5556b5a,
        0x4b696d92, 0x99b2cccc, 0xcccc198c, 0xce73e718 }}; };
constexpr std::array<uint32_t,testvec_bits/32>
     testvec_<bit_order::lsb0,uint32_t,byte_order::pdp_endian>::data;
template<> struct testvec_<bit_order::lsb0,uint16_t,byte_order::big_endian> {
    static constexpr std::array<uint16_t,testvec_bits/16> data {{
        0xa22b, 0x346c, 0xc9f8, 0x1fce, 0x4cd2, 0x4a55, 0x55b5, 0x5a6b,
        0x694b, 0x926d, 0xb299, 0xcccc, 0xcccc, 0x8c19, 0x73ce, 0x18e7 }}; };
constexpr std::array<uint16_t,testvec_bits/16>
     testvec_<bit_order::lsb0,uint16_t,byte_order::big_endian>::data;
template<> struct testvec_<bit_order::lsb0,uint32_t,byte_order::big_endian> {
    static constexpr std::array<uint32_t,testvec_bits/32> data {{
        0xa22b346c, 0xc9f81fce, 0x4cd24a55, 0x55b55a6b,
        0x694b926d, 0xb299cccc, 0xcccc8c19, 0x73ce18e7 }}; };
constexpr std::array<uint32_t,testvec_bits/32>
     testvec_<bit_order::lsb0,uint32_t,byte_order::big_endian>::data;
template<> struct testvec_<bit_order::lsb0,uint64_t,byte_order::big_endian> {
    static constexpr std::array<uint64_t,testvec_bits/64> data {{
        0xa22b346cc9f81fce, 0x4cd24a5555b55a6b,
        0x694b926db299cccc, 0xcccc8c1973ce18e7 }}; };
constexpr std::array<uint64_t,testvec_bits/64>
     testvec_<bit_order::lsb0,uint64_t,byte_order::big_endian>::data;
template<> struct testvec_<bit_order::msb0,uint8_t> {
    static constexpr std::array<uint8_t,testvec_bits/8> data {{
        0x45, 0xd4, 0x2c, 0x36, 0x93, 0x1f, 0xf8, 0x73,
        0x32, 0x4b, 0x52, 0xaa, 0xaa, 0xad, 0x5a, 0xd6,
        0x96, 0xd2, 0x49, 0xb6, 0x4d, 0x99, 0x33, 0x33,
        0x33, 0x33, 0x31, 0x98, 0xce, 0x73, 0x18, 0xe7 }}; };
constexpr std::array<uint8_t,testvec_bits/8>
     testvec_<bit_order::msb0,uint8_t>::data;
template<> struct testvec_<bit_order::msb0,uint16_t,byte_order::little_endian> {
    static constexpr std::array<uint16_t,testvec_bits/16> data {{
        0xd445, 0x362c, 0x1f93, 0x73f8, 0x4b32, 0xaa52, 0xadaa, 0xd65a,
        0xd296, 0xb649, 0x994d, 0x3333, 0x3333, 0x9831, 0x73ce, 0xe718 }}; };
constexpr std::array<uint16_t,testvec_bits/16>
     testvec_<bit_order::msb0,uint16_t,byte_order::little_endian>::data;
template<> struct testvec_<bit_order::msb0,uint32_t,byte_order::little_endian> {
    static constexpr std::array<uint32_t,testvec_bits/32> data {{
        0x362cd445, 0x73f81f93, 0xaa524b32, 0xd65aadaa,
        0xb649d296, 0x3333994d, 0x98313333, 0xe71873ce }}; };
constexpr std::array<uint32_t,testvec_bits/32>
     testvec_<bit_order::msb0,uint32_t,byte_order::little_endian>::data;
template<> struct testvec_<bit_order::msb0,uint64_t,byte_order::little_endian> {
    static constexpr std::array<uint64_t,testvec_bits/64> data {{
        0x73f81f93362cd445, 0xd65aadaaaa524b32,
        0x3333994db649d296, 0xe71873ce98313333 }}; };
constexpr std::array<uint64_t,testvec_bits/64>
     testvec_<bit_order::msb0,uint64_t,byte_order::little_endian>::data;
template<> struct testvec_<bit_order::msb0,uint32_t,byte_order::pdp_endian> {
    static constexpr std::array<uint32_t,testvec_bits/32> data {{
        0xd445362c, 0x1f9373f8, 0x4b32aa52, 0xadaad65a,
        0xd296b649, 0x994d3333, 0x33339831, 0x73cee718 }}; };
constexpr std::array<uint32_t,testvec_bits/32>
     testvec_<bit_order::msb0,uint32_t,byte_order::pdp_endian>::data;
template<> struct testvec_<bit_order::msb0,uint16_t,byte_order::big_endian> {
    static constexpr std::array<uint16_t,testvec_bits/16> data {{
        0x45d4, 0x2c36, 0x931f, 0xf873, 0x324b, 0x52aa, 0xaaad, 0x5ad6,
        0x96d2, 0x49b6, 0x4d99, 0x3333, 0x3333, 0x3198, 0xce73, 0x18e7 }}; };
constexpr std::array<uint16_t,testvec_bits/16>
     testvec_<bit_order::msb0,uint16_t,byte_order::big_endian>::data;
template<> struct testvec_<bit_order::msb0,uint32_t,byte_order::big_endian> {
    static constexpr std::array<uint32_t,testvec_bits/32> data {{
        0x45d42c36, 0x931ff873, 0x324b52aa, 0xaaad5ad6,
        0x96d249b6, 0x4d993333, 0x33333198, 0xce7318e7 }}; };
constexpr std::array<uint32_t,testvec_bits/32>
     testvec_<bit_order::msb0,uint32_t,byte_order::big_endian>::data;
template<> struct testvec_<bit_order::msb0,uint64_t,byte_order::big_endian> {
    static constexpr std::array<uint64_t,testvec_bits/64> data {{
        0x45d42c36931ff873, 0x324b52aaaaad5ad6,
        0x96d249b64d993333, 0x33333198ce7318e7 }}; };
constexpr std::array<uint64_t,testvec_bits/64>
     testvec_<bit_order::msb0,uint64_t,byte_order::big_endian>::data;


template <bit_order BO, typename UL, byte_order EI = byte_order::none>
constexpr const std::array<UL,testvec_bits/8/sizeof(UL)>& testvec()
{
    return testvec_<BO,UL,EI>::data;
}

template <bit_order BO, /*typename T, byte_order EI, */typename T>
void for_each_bit(T&& t)
{
    std::array<uint8_t,testvec_bits/8> data = testvec<BO,uint8_t>();
    for (std::size_t b = 0; b < 8*data.size(); ++b)
        t(data.data(), b);
}

bitter::bit expval(std::size_t pos)
{
    double b = std::log(pos+1)*97.0;
    int c = ((b - std::fmod(b,1.0))+0.5);
    int d = c%2;
    return bitter::bit(d==1);
}

template <bitter::bit_order BO, typename T>
void for_each_bit_pair(T&& t)
{
    std::array<uint8_t,testvec_bits/8> data = testvec<BO,uint8_t>();
    static constexpr std::size_t bits = 8*data.size();
    for (ptrdiff_t b1 = 0; b1 < bits; ++b1)
        for (ptrdiff_t b2 = 0; b2 < bits; ++b2)
            t(data.data(), b1, b2);
}

template <typename T>
struct bit_iterator_traits;

template <bitter::bit_order BO, typename UL, bitter::byte_order EI>
struct bit_iterator_traits<bitter::bit_iterator<BO,UL,EI>>
{
    static constexpr bitter::bit_order bit_order = BO;
    using underlying_type = UL;
    static constexpr bitter::byte_order byte_order = EI;
};

template <bitter::bit_order BO, typename UL, bitter::byte_order EI>
struct bit_iterator_traits<bitter::const_bit_iterator<BO,UL,EI>>
{
    static constexpr bitter::bit_order bit_order = BO;
    using underlying_type = UL;
    static constexpr bitter::byte_order byte_order = EI;
};

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
    static constexpr bitter::bit_order bit_order
        = bit_iterator_traits<Iter>::bit_order;

    /* Construction and assignment *////////////////////////////////////////////

    it("supports construction from byte and bitno", []{
        for_each_bit<bit_order>([](Data* arr, std::size_t b){
            Iter i(&arr[b/8], b%8);
        });
    });

    it("supports copy-construction", []{
        for_each_bit<bit_order>([](Data* arr, std::size_t b){
            const Iter i1(&arr[b/8], b%8);
            Iter i2(i1);
            AssertThat(i1, Equals(i2));
            AssertThat(*i2, Equals(expval(b)));
        });
    });

    it("supports assignment", []{
        Iter i1;
        for_each_bit<bit_order>([&](Data* arr, std::size_t b){
            Iter i2(&arr[b/8], b%8);
            i1 = i2;
            AssertThat(i1, Equals(i2));
            AssertThat(*i1, Equals(expval(b)));
        });
    });

    it("is swappable", []{
        for_each_bit_pair<bit_order>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/8], b1%8);
            const Iter i1s(&arr[b1/8], b1%8);
            Iter i2(&arr[b2/8], b2%8);
            const Iter i2s(&arr[b2/8], b2%8);
            swap(i1, i2);
            AssertThat(i1, Equals(i2s));
            AssertThat(i2, Equals(i1s));
            AssertThat(*i1, Equals(expval(b2)));
            AssertThat(*i2, Equals(expval(b1)));
        });
    });

    it("is swappable when LHS is moved", []{
        for_each_bit_pair<bit_order>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/8], b1%8);
            const Iter i1s(&arr[b1/8], b1%8);
            Iter i2(&arr[b2/8], b2%8);
            const Iter i2s(&arr[b2/8], b2%8);
            swap(std::move(i1), i2);
            AssertThat(i1, Equals(i2s));
            AssertThat(i2, Equals(i1s));
            AssertThat(*i1, Equals(expval(b2)));
            AssertThat(*i2, Equals(expval(b1)));
        });
    });

    it("is swappable when RHS is moved", []{
        for_each_bit_pair<bit_order>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/8], b1%8);
            const Iter i1s(&arr[b1/8], b1%8);
            Iter i2(&arr[b2/8], b2%8);
            const Iter i2s(&arr[b2/8], b2%8);
            swap(i1, std::move(i2));
            AssertThat(i1, Equals(i2s));
            AssertThat(i2, Equals(i1s));
            AssertThat(*i1, Equals(expval(b2)));
            AssertThat(*i2, Equals(expval(b1)));
        });
    });

    /* Dereferencing *//////////////////////////////////////////////////////////

    it("is dereferencable with ->", []{
        for_each_bit<bit_order>([](Data* arr, std::size_t b){
            Iter i(&arr[b/8], b%8);
            AssertThat(i->operator bool(), Equals(expval(b)));
        });
    });

    it("is dereferencable with *", []{
        for_each_bit<bit_order>([](Data* arr, std::size_t b){
            Iter i(&arr[b/8], b%8);
            AssertThat(*i, Equals(expval(b)));
        });
    });

    it("is dereferencable with []", []{
        for_each_bit_pair<bit_order>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/8], b1%8);
            const Iter i1s(&arr[b1/8], b1%8);
            std::ptrdiff_t delta = b2 - b1;
            AssertThat(i1[delta], Equals(expval(b2)));
            AssertThat(i1, Equals(i1s));
        });
    });

    /* Navigation mutators *////////////////////////////////////////////////////

    it("supports preincrement", []{
        for_each_bit<bit_order>([](Data* arr, std::size_t b){
            Iter i1(&arr[b/8], b%8);
            const Iter i2(&arr[(b+1)/8], (b+1)%8);
            AssertThat(++i1, Equals(i2));
            AssertThat(i1, Equals(i2));
            if (b+1 < testvec_bits) AssertThat(*i1, Equals(expval(b+1)));
        });
    });

    it("supports postincrement", []{
        for_each_bit<bit_order>([](Data* arr, std::size_t b){
            Iter i1(&arr[b/8], b%8);
            const Iter i2(&arr[b/8], b%8);
            const Iter i3(&arr[(b+1)/8], (b+1)%8);
            AssertThat(i1++, Equals(i2));
            AssertThat(i1, Equals(i3));
            if (b+1 < testvec_bits) AssertThat(*i1, Equals(expval(b+1)));
        });
    });

    it("supports predecrement", []{
        for_each_bit<bit_order>([](Data* arr, std::size_t b){
            Iter i1(&arr[(b+1)/8], (b+1)%8);
            Iter i2(&arr[b/8], b%8);
            AssertThat(--i1, Equals(i2));
            AssertThat(i1, Equals(i2));
            AssertThat(*i1, Equals(expval(b)));
        });
    });

    it("supports postdecrement", []{
        for_each_bit<bit_order>([](Data* arr, std::size_t b){
            Iter i1(&arr[(b+1)/8], (b+1)%8);
            Iter i2(&arr[(b+1)/8], (b+1)%8);
            Iter i3(&arr[b/8], b%8);
            AssertThat(i1--, Equals(i2));
            AssertThat(i1, Equals(i3));
            AssertThat(*i1, Equals(expval(b)));
        });
    });

    it("supports addition-assignment", []{
        for_each_bit_pair<bit_order>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            ptrdiff_t delta = b2 - b1;
            Iter i1(&arr[b1/8], b1%8);
            Iter i2(&arr[b2/8], b2%8);
            AssertThat(i1+=delta, Equals(i2));
            AssertThat(i1, Equals(i2));
            AssertThat(*i1, Equals(expval(b2)));
        });
    });

    it("supports subtraction-assignment", []{
        for_each_bit_pair<bit_order>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            std::ptrdiff_t delta = b1 - b2;
            Iter i1(&arr[b1/8], b1%8);
            Iter i2(&arr[b2/8], b2%8);
            AssertThat(i1-=delta, Equals(i2));
            AssertThat(i1, Equals(i2));
            AssertThat(*i1, Equals(expval(b2)));
        });
    });

    /* Navigation free function opearators *////////////////////////////////////

    it("supports subtracting one iterator from another", []{
        for_each_bit_pair<bit_order>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/8], b1%8);
            Iter i2(&arr[b2/8], b2%8);
            AssertThat(i2 - i1, Equals(b2 - b1));
        });
    });

    it("supports addition of iterator with ptrdiff_t", []{
        for_each_bit_pair<bit_order>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/8], b1%8);
            const Iter i1s(&arr[b1/8], b1%8);
            Iter i2(&arr[b2/8], b2%8);
            const Iter i2s(&arr[b2/8], b2%8);
            std::ptrdiff_t delta = b2 - b1;
            AssertThat(i1 + delta, Equals(i2));
            AssertThat(*(i1 + delta), Equals(expval(b2)));
            AssertThat(i1, Equals(i1s));
            AssertThat(i2, Equals(i2s));
        });
    });

    it("supports addition of ptrdiff_t with iterator", []{
        for_each_bit_pair<bit_order>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/8], b1%8);
            const Iter i1s(&arr[b1/8], b1%8);
            Iter i2(&arr[b2/8], b2%8);
            const Iter i2s(&arr[b2/8], b2%8);
            std::ptrdiff_t delta = b2 - b1;
            AssertThat(delta + i1, Equals(i2));
            AssertThat(*(delta + i1), Equals(expval(b2)));
            AssertThat(i1, Equals(i1s));
            AssertThat(i1, Equals(i1s));
            AssertThat(i2, Equals(i2s));
        });
    });

    it("supports subtraction of ptrdiff_t from iterator", []{
        for_each_bit_pair<bit_order>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/8], b1%8);
            const Iter i1s(&arr[b1/8], b1%8);
            Iter i2(&arr[b2/8], b2%8);
            const Iter i2s(&arr[b2/8], b2%8);
            std::ptrdiff_t delta = b1 - b2;
            AssertThat(i1 - delta, Equals(i2));
            AssertThat(*(i1 - delta), Equals(expval(b2)));
            AssertThat(i1, Equals(i1s));
            AssertThat(i2, Equals(i2s));
        });
    });

    /* Comparison free function opearators *////////////////////////////////////

    it("supports equality comparison", []{
        for_each_bit_pair<bit_order>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/8], b1%8);
            Iter i2(&arr[b2/8], b2%8);
            AssertThat(i1 == i2, Equals(b1 == b2));
        });
    });

    it("supports inequality comparison", []{
        for_each_bit_pair<bit_order>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/8], b1%8);
            Iter i2(&arr[b2/8], b2%8);
            AssertThat(i1 != i2, Equals(b1 != b2));
        });
    });

    /* Relational free function opearators *////////////////////////////////////

    it("supports less-than comparison", []{
        for_each_bit_pair<bit_order>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/8], b1%8);
            Iter i2(&arr[b2/8], b2%8);
            AssertThat(i1 < i2, Equals(b1 < b2));
        });
    });

    it("supports less-than-or-equal comparison", []{
        for_each_bit_pair<bit_order>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/8], b1%8);
            Iter i2(&arr[b2/8], b2%8);
            AssertThat(i1 <= i2, Equals(b1 <= b2));
        });
    });

    it("supports greater-than comparison", []{
        for_each_bit_pair<bit_order>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/8], b1%8);
            Iter i2(&arr[b2/8], b2%8);
            AssertThat(i1 > i2, Equals(b1 > b2));
        });
    });

    it("supports greater-than-or-equal comparison", []{
        for_each_bit_pair<bit_order>([](Data* arr, ptrdiff_t b1, ptrdiff_t b2){
            Iter i1(&arr[b1/8], b1%8);
            Iter i2(&arr[b2/8], b2%8);
            AssertThat(i1 >= i2, Equals(b1 >= b2));
        });
    });

    /* Misc *///////////////////////////////////////////////////////////////////

    it("satisfies the examples in NIST FIPS 202, bytes -> bits", []{
        std::array<Data,1> data = {{0xA3}};
        const bitvec interpreted_as_bits =
            (bit_order == bitter::bit_order::msb0)
                ? make_bitvec({1,0,1,0, 0,0,1,1})
                : make_bitvec({1,1,0,0, 0,1,0,1});

        bitvec converted_to_bits;
        std::copy(Iter(begin(data), 0), Iter(end(data), 0),
                  back_inserter(converted_to_bits));
        AssertThat(converted_to_bits, EqualsContainer(interpreted_as_bits));
    });
}

template <typename UL, typename F>
std::array<uint8_t,testvec_bits/8> for_each_bit2(F&& f)
{
    std::array<UL,testvec_bits/(8*sizeof(UL))> ret{};
    for (std::size_t b = 0; b < 8*ret.size(); ++b)
        std::forward<F>(f)(ret.data(), b);
    return ret;
}

template <typename OutIt>
std::string copy_and_hexify(const bitvec& source, ptrdiff_t n)
{
    std::vector<uint8_t> out((n+7)/8);
    std::copy(begin(source), next(begin(source), n), OutIt(out.data(), 0));
    return hexstring(out.data(), n);
}

template <class Iter>
void mutating_tests()
{
    static constexpr bitter::bit_order bo
        = bit_iterator_traits<Iter>::bit_order;
    using ul = typename bit_iterator_traits<Iter>::underlying_type;
    static constexpr bitter::byte_order ei
        = bit_iterator_traits<Iter>::byte_order;

    static constexpr std::size_t testvec_elements = testvec_bits/(8*sizeof(ul));

    it("supports dereference-and-assign", []{
        AssertThat((for_each_bit2<ul>([&](ul* arr, std::size_t b){
            Iter i1(&arr[b/8], b%8);
            *i1 = expval(b);
        })), Equals(testvec<bo,ul,ei>()));
    });

    it("supports dereference-postincrement-and-assign", []{
        std::array<ul,testvec_elements> arr{};
        Iter it(arr.data(),0);
        for (std::size_t b = 0; b < testvec_bits; ++b)
            *it++ = expval(b);
        AssertThat(arr, Equals(testvec<bo,ul,ei>()));
    });

    if (sizeof(ul) == 8) {
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
    }

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
            AssertThat(copy_and_hexify<Iter>(bits16, i), Equals(results.at(i)));
    });
}

go_bandit([]{
    using namespace bitter;
    static constexpr bit_order msb0 = bit_order::msb0;
    static constexpr bit_order lsb0 = bit_order::lsb0;
    describe("const_bit_iterator<bit_order::msb0>", []{
        core_tests<const_bit_iterator<msb0>>();
        describe("over const uint8_t",
            &observing_tests<const uint8_t,const_bit_iterator<msb0>>);
        describe("over uint8_t",
            &observing_tests<uint8_t,const_bit_iterator<msb0>>);
    });
    describe("bit_iterator<bit_order::msb0>", []{
        core_tests<bit_iterator<msb0>>();
        describe("over uint8_t", []{
            observing_tests<uint8_t,bit_iterator<msb0>>();
            mutating_tests<bit_iterator<msb0>>();
        });
    });
    describe("const_bit_iterator<bit_order::lsb0>", []{
        core_tests<const_bit_iterator<lsb0>>();
        describe("over const uint8_t",
            &observing_tests<const uint8_t,const_bit_iterator<lsb0>>);
        describe("over uint8_t",
            &observing_tests<uint8_t,const_bit_iterator<lsb0>>);
    });
    describe("bit_iterator<bit_order::lsb0>", []{
        core_tests<bit_iterator<lsb0>>();
        describe("over uint8_t", []{
            observing_tests<uint8_t,bit_iterator<lsb0>>();
            mutating_tests<bit_iterator<lsb0>>();
        });
    });

    describe("byteidx", []{
        using namespace bitter::detail;
        using namespace bitter;
        constexpr auto big_endian = byte_order::big_endian;
        constexpr auto little_endian = byte_order::little_endian;
        constexpr auto pdp_endian = byte_order::pdp_endian;
        it("supports big endian", [&]{
            AssertThat((byteidx<char,big_endian>(0)),Equals(0));
            AssertThat((byteidx<uint8_t,big_endian>(0)),Equals(0));
            AssertThat((byteidx<uint16_t,big_endian>(0)),Equals(1));
            AssertThat((byteidx<uint16_t,big_endian>(1)),Equals(0));
            AssertThat((byteidx<uint32_t,big_endian>(0)),Equals(3));
            AssertThat((byteidx<uint32_t,big_endian>(1)),Equals(2));
            AssertThat((byteidx<uint32_t,big_endian>(2)),Equals(1));
            AssertThat((byteidx<uint32_t,big_endian>(3)),Equals(0));
            AssertThat((byteidx<uint64_t,big_endian>(0)),Equals(7));
            AssertThat((byteidx<uint64_t,big_endian>(1)),Equals(6));
            AssertThat((byteidx<uint64_t,big_endian>(2)),Equals(5));
            AssertThat((byteidx<uint64_t,big_endian>(3)),Equals(4));
            AssertThat((byteidx<uint64_t,big_endian>(4)),Equals(3));
            AssertThat((byteidx<uint64_t,big_endian>(5)),Equals(2));
            AssertThat((byteidx<uint64_t,big_endian>(6)),Equals(1));
            AssertThat((byteidx<uint64_t,big_endian>(7)),Equals(0));
        });

        it("supports big endian", [&]{
            AssertThat((byteidx<char,little_endian>(0)),Equals(0));
            AssertThat((byteidx<uint8_t,little_endian>(0)),Equals(0));
            AssertThat((byteidx<uint16_t,little_endian>(0)),Equals(0));
            AssertThat((byteidx<uint16_t,little_endian>(1)),Equals(1));
            AssertThat((byteidx<uint32_t,little_endian>(0)),Equals(0));
            AssertThat((byteidx<uint32_t,little_endian>(1)),Equals(1));
            AssertThat((byteidx<uint32_t,little_endian>(2)),Equals(2));
            AssertThat((byteidx<uint32_t,little_endian>(3)),Equals(3));
            AssertThat((byteidx<uint64_t,little_endian>(0)),Equals(0));
            AssertThat((byteidx<uint64_t,little_endian>(1)),Equals(1));
            AssertThat((byteidx<uint64_t,little_endian>(2)),Equals(2));
            AssertThat((byteidx<uint64_t,little_endian>(3)),Equals(3));
            AssertThat((byteidx<uint64_t,little_endian>(4)),Equals(4));
            AssertThat((byteidx<uint64_t,little_endian>(5)),Equals(5));
            AssertThat((byteidx<uint64_t,little_endian>(6)),Equals(6));
            AssertThat((byteidx<uint64_t,little_endian>(7)),Equals(7));
        });

        it("supports pdp endian", [&]{
            AssertThat((byteidx<uint32_t,pdp_endian>(0)),Equals(2));
            AssertThat((byteidx<uint32_t,pdp_endian>(1)),Equals(3));
            AssertThat((byteidx<uint32_t,pdp_endian>(2)),Equals(0));
            AssertThat((byteidx<uint32_t,pdp_endian>(3)),Equals(1));
        });
    });

    describe("bitidx, lsb0", []{
        using namespace bitter::detail;
        using namespace bitter;
        constexpr auto msb0 = bit_order::msb0;
        constexpr auto lsb0 = bit_order::lsb0;
        constexpr auto big_endian = byte_order::big_endian;
//        constexpr auto little_endian = byte_order::little_endian;
        constexpr auto pdp_endian = byte_order::pdp_endian;
        it("supports one-byte types/msb0", [&]{
            AssertThat((bitidx<msb0,char,big_endian>(0)),Equals(7));
            AssertThat((bitidx<msb0,char,big_endian>(1)),Equals(6));
            AssertThat((bitidx<msb0,char,big_endian>(2)),Equals(5));
            AssertThat((bitidx<msb0,char,big_endian>(3)),Equals(4));
            AssertThat((bitidx<msb0,char,big_endian>(4)),Equals(3));
            AssertThat((bitidx<msb0,char,big_endian>(5)),Equals(2));
            AssertThat((bitidx<msb0,char,big_endian>(6)),Equals(1));
            AssertThat((bitidx<msb0,char,big_endian>(7)),Equals(0));

            AssertThat((bitidx<msb0,uint8_t,big_endian>(0)),Equals(7));
            AssertThat((bitidx<msb0,uint8_t,big_endian>(1)),Equals(6));
            AssertThat((bitidx<msb0,uint8_t,big_endian>(2)),Equals(5));
            AssertThat((bitidx<msb0,uint8_t,big_endian>(3)),Equals(4));
            AssertThat((bitidx<msb0,uint8_t,big_endian>(4)),Equals(3));
            AssertThat((bitidx<msb0,uint8_t,big_endian>(5)),Equals(2));
            AssertThat((bitidx<msb0,uint8_t,big_endian>(6)),Equals(1));
            AssertThat((bitidx<msb0,uint8_t,big_endian>(7)),Equals(0));
        });

        it("supports one-byte types/lsb0", [&]{
            AssertThat((bitidx<lsb0,char,big_endian>(0)),Equals(0));
            AssertThat((bitidx<lsb0,char,big_endian>(1)),Equals(1));
            AssertThat((bitidx<lsb0,char,big_endian>(2)),Equals(2));
            AssertThat((bitidx<lsb0,char,big_endian>(3)),Equals(3));
            AssertThat((bitidx<lsb0,char,big_endian>(4)),Equals(4));
            AssertThat((bitidx<lsb0,char,big_endian>(5)),Equals(5));
            AssertThat((bitidx<lsb0,char,big_endian>(6)),Equals(6));
            AssertThat((bitidx<lsb0,char,big_endian>(7)),Equals(7));

            AssertThat((bitidx<lsb0,uint8_t,big_endian>(0)),Equals(0));
            AssertThat((bitidx<lsb0,uint8_t,big_endian>(1)),Equals(1));
            AssertThat((bitidx<lsb0,uint8_t,big_endian>(2)),Equals(2));
            AssertThat((bitidx<lsb0,uint8_t,big_endian>(3)),Equals(3));
            AssertThat((bitidx<lsb0,uint8_t,big_endian>(4)),Equals(4));
            AssertThat((bitidx<lsb0,uint8_t,big_endian>(5)),Equals(5));
            AssertThat((bitidx<lsb0,uint8_t,big_endian>(6)),Equals(6));
            AssertThat((bitidx<lsb0,uint8_t,big_endian>(7)),Equals(7));
        });

        it("supports big endian/msb0", [&]{
            AssertThat((bitidx<msb0,uint16_t,big_endian>( 0)),Equals(15));
            AssertThat((bitidx<msb0,uint16_t,big_endian>( 1)),Equals(14));
            AssertThat((bitidx<msb0,uint16_t,big_endian>( 2)),Equals(13));
            AssertThat((bitidx<msb0,uint16_t,big_endian>( 3)),Equals(12));
            AssertThat((bitidx<msb0,uint16_t,big_endian>( 4)),Equals(11));
            AssertThat((bitidx<msb0,uint16_t,big_endian>( 5)),Equals(10));
            AssertThat((bitidx<msb0,uint16_t,big_endian>( 6)),Equals( 9));
            AssertThat((bitidx<msb0,uint16_t,big_endian>( 7)),Equals( 8));
            AssertThat((bitidx<msb0,uint16_t,big_endian>( 8)),Equals( 7));
            AssertThat((bitidx<msb0,uint16_t,big_endian>( 9)),Equals( 6));
            AssertThat((bitidx<msb0,uint16_t,big_endian>(10)),Equals( 5));
            AssertThat((bitidx<msb0,uint16_t,big_endian>(11)),Equals( 4));
            AssertThat((bitidx<msb0,uint16_t,big_endian>(12)),Equals( 3));
            AssertThat((bitidx<msb0,uint16_t,big_endian>(13)),Equals( 2));
            AssertThat((bitidx<msb0,uint16_t,big_endian>(14)),Equals( 1));
            AssertThat((bitidx<msb0,uint16_t,big_endian>(15)),Equals( 0));

            AssertThat((bitidx<msb0,uint32_t,big_endian>( 8)),Equals(23));
            AssertThat((bitidx<msb0,uint32_t,big_endian>( 9)),Equals(22));
            AssertThat((bitidx<msb0,uint32_t,big_endian>(10)),Equals(21));
            AssertThat((bitidx<msb0,uint32_t,big_endian>(11)),Equals(20));
            AssertThat((bitidx<msb0,uint32_t,big_endian>(12)),Equals(19));
            AssertThat((bitidx<msb0,uint32_t,big_endian>(13)),Equals(18));
            AssertThat((bitidx<msb0,uint32_t,big_endian>(14)),Equals(17));
            AssertThat((bitidx<msb0,uint32_t,big_endian>(15)),Equals(16));

            AssertThat((bitidx<msb0,uint64_t,big_endian>(40)),Equals(23));
            AssertThat((bitidx<msb0,uint64_t,big_endian>(41)),Equals(22));
            AssertThat((bitidx<msb0,uint64_t,big_endian>(42)),Equals(21));
            AssertThat((bitidx<msb0,uint64_t,big_endian>(43)),Equals(20));
            AssertThat((bitidx<msb0,uint64_t,big_endian>(44)),Equals(19));
            AssertThat((bitidx<msb0,uint64_t,big_endian>(45)),Equals(18));
            AssertThat((bitidx<msb0,uint64_t,big_endian>(46)),Equals(17));
            AssertThat((bitidx<msb0,uint64_t,big_endian>(47)),Equals(16));
        });

        it("supports big endian/lsb0", [&]{
            AssertThat((bitidx<lsb0,uint16_t,big_endian>( 0)),Equals( 8));
            AssertThat((bitidx<lsb0,uint16_t,big_endian>( 1)),Equals( 9));
            AssertThat((bitidx<lsb0,uint16_t,big_endian>( 2)),Equals(10));
            AssertThat((bitidx<lsb0,uint16_t,big_endian>( 3)),Equals(11));
            AssertThat((bitidx<lsb0,uint16_t,big_endian>( 4)),Equals(12));
            AssertThat((bitidx<lsb0,uint16_t,big_endian>( 5)),Equals(13));
            AssertThat((bitidx<lsb0,uint16_t,big_endian>( 6)),Equals(14));
            AssertThat((bitidx<lsb0,uint16_t,big_endian>( 7)),Equals(15));
            AssertThat((bitidx<lsb0,uint16_t,big_endian>( 8)),Equals(0));
            AssertThat((bitidx<lsb0,uint16_t,big_endian>( 9)),Equals(1));
            AssertThat((bitidx<lsb0,uint16_t,big_endian>(10)),Equals(2));
            AssertThat((bitidx<lsb0,uint16_t,big_endian>(11)),Equals(3));
            AssertThat((bitidx<lsb0,uint16_t,big_endian>(12)),Equals(4));
            AssertThat((bitidx<lsb0,uint16_t,big_endian>(13)),Equals(5));
            AssertThat((bitidx<lsb0,uint16_t,big_endian>(14)),Equals(6));
            AssertThat((bitidx<lsb0,uint16_t,big_endian>(15)),Equals(7));

            AssertThat((bitidx<lsb0,uint32_t,big_endian>( 8)),Equals(16));
            AssertThat((bitidx<lsb0,uint32_t,big_endian>( 9)),Equals(17));
            AssertThat((bitidx<lsb0,uint32_t,big_endian>(10)),Equals(18));
            AssertThat((bitidx<lsb0,uint32_t,big_endian>(11)),Equals(19));
            AssertThat((bitidx<lsb0,uint32_t,big_endian>(12)),Equals(20));
            AssertThat((bitidx<lsb0,uint32_t,big_endian>(13)),Equals(21));
            AssertThat((bitidx<lsb0,uint32_t,big_endian>(14)),Equals(22));
            AssertThat((bitidx<lsb0,uint32_t,big_endian>(15)),Equals(23));

            AssertThat((bitidx<lsb0,uint64_t,big_endian>(40)),Equals(16));
            AssertThat((bitidx<lsb0,uint64_t,big_endian>(41)),Equals(17));
            AssertThat((bitidx<lsb0,uint64_t,big_endian>(42)),Equals(18));
            AssertThat((bitidx<lsb0,uint64_t,big_endian>(43)),Equals(19));
            AssertThat((bitidx<lsb0,uint64_t,big_endian>(44)),Equals(20));
            AssertThat((bitidx<lsb0,uint64_t,big_endian>(45)),Equals(21));
            AssertThat((bitidx<lsb0,uint64_t,big_endian>(46)),Equals(22));
            AssertThat((bitidx<lsb0,uint64_t,big_endian>(47)),Equals(23));
        });

        it("supports pdp endian", [&]{
            AssertThat((byteidx<uint32_t,pdp_endian>(0)),Equals(2));
            AssertThat((byteidx<uint32_t,pdp_endian>(1)),Equals(3));
            AssertThat((byteidx<uint32_t,pdp_endian>(2)),Equals(0));
            AssertThat((byteidx<uint32_t,pdp_endian>(3)),Equals(1));
        });
    });
});

int main(int argc, char** argv)
{
    return bandit::run(argc, argv);
}
