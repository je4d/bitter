#include <bandit/bandit.h>
using namespace bandit;

#include "bit_iterator.hpp"

#include <cstdint>
#include <cmath>

using std::size_t;
using std::ptrdiff_t;

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

std::array<uint8_t,8> exparr_msb0 =
{ /* 00010110 */ 0x16,
  /* 11101011 */ 0xeb,
  /* 00110011 */ 0x33,
  /* 00011000 */ 0x18,
  /* 01110000 */ 0x70,
  /* 11110000 */ 0xf0,
  /* 01111100 */ 0x7c,
  /* 00001111 */ 0x0f };
std::array<uint8_t,8> exparr_lsb0 =
{ /* 01101000 */ 0x68,
  /* 11010111 */ 0xd7,
  /* 11001100 */ 0xcc,
  /* 00011000 */ 0x18,
  /* 00001110 */ 0x0e,
  /* 00001111 */ 0x0f,
  /* 00111110 */ 0x3e,
  /* 11110000 */ 0xf0 };

static constexpr std::size_t exparr_bits = sizeof(exparr_lsb0) * 8;

template <bitter::bit_order BO>
std::array<uint8_t,8> exparr()
{
    static constexpr bool is_msb0 = BO == bitter::bit_order::msb0;
    return is_msb0 ? exparr_msb0 : exparr_lsb0;
}

template <bitter::bit_order BO, typename T>
void for_each_bit(T&& t)
{
    std::array<uint8_t,8> data = exparr<BO>();
    for (std::size_t b = 0; b < 8*data.size(); ++b)
        t(data.data(), b);
}

bitter::bit expval(std::size_t pos)
{
    int a = pos+1;
    double b = std::log(a)*10.0;
    int c = ((b - std::fmod(b,1.0))+0.5);
    int d = c%2;
    return bitter::bit(d==1);
}

template <bitter::bit_order BO, typename T>
void for_each_bit_pair(T&& t)
{
    std::array<uint8_t,8> data = exparr<BO>();
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
            if (b+1 < exparr_bits) AssertThat(*i1, Equals(expval(b+1)));
        });
    });

    it("supports postincrement", []{
        for_each_bit<bit_order>([](Data* arr, std::size_t b){
            Iter i1(&arr[b/8], b%8);
            const Iter i2(&arr[b/8], b%8);
            const Iter i3(&arr[(b+1)/8], (b+1)%8);
            AssertThat(i1++, Equals(i2));
            AssertThat(i1, Equals(i3));
            if (b+1 < exparr_bits) AssertThat(*i1, Equals(expval(b+1)));
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

template <typename T>
std::array<uint8_t,8> for_each_bit2(T&& t)
{
    std::array<uint8_t,8> ret{};
    for (std::size_t b = 0; b < 8*ret.size(); ++b)
        t(ret.data(), b);
    return ret;
}

template <typename OutIt>
std::string copy_and_hexify(const bitvec& source, ptrdiff_t n)
{
    std::vector<uint8_t> out((n+7)/8);
    std::copy(begin(source), next(begin(source), n), OutIt(out.data(), 0));
    return hexstring(out.data(), n);
}

template <class Data, class Iter>
void mutating_tests()
{
    static constexpr bitter::bit_order bit_order
        = bit_iterator_traits<Iter>::bit_order;

    it("supports dereference-and-assign", []{
        AssertThat(for_each_bit2([&](Data* arr, std::size_t b){
            Iter i1(&arr[b/8], b%8);
            *i1 = expval(b);
        }), Equals(exparr<bit_order>()));
    });

    it("supports dereference-postincrement-and-assign", []{
        std::array<uint8_t,8> arr{};
        Iter it(arr.data(),0);
        for (std::size_t b = 0; b < exparr_bits; ++b) {
            *it++ = expval(b);
        }
        AssertThat(arr, Equals(exparr<bit_order>()));
    });

    it("satisfies the examples in NIST FIPS 202, bits -> bytes", []{
        unsigned char data_out[] = {0,0};
        const bitvec interpreted_as_bits =
            (bit_order == bitter::bit_order::msb0)
                ? make_bitvec({1,0,1,0, 0,0,1,1})
                : make_bitvec({1,1,0,0, 0,1,0,1});
        std::copy(begin(interpreted_as_bits), end(interpreted_as_bits),
                  Iter(data_out,0));
        AssertThat(static_cast<uint8_t>(data_out[0]), Equals(0xa3));
        AssertThat(static_cast<uint8_t>(data_out[1]), Equals(0));
    });

    it("handles partial bytes", [&]{
        std::vector<const char*> results =
            (bit_order == bitter::bit_order::msb0)
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
            mutating_tests<uint8_t,bit_iterator<msb0>>();
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
            mutating_tests<uint8_t,bit_iterator<lsb0>>();
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
