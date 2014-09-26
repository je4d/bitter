#include <bandit/bandit.h>
using namespace bandit;

#include "bit_iterator.hpp"

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

template <typename OutIt>
std::string copy_and_hexify(const bitvec& source, std::ptrdiff_t n)
{
    std::vector<uint8_t> out((n+7)/8);
    std::copy(begin(source), next(begin(source), n), OutIt(out.data(), 0));
    return hexstring(out.data(), n);
}

template <bitter::bit_order BO>
void core_tests()
{
    static_assert(BO==bitter::bit_order::msb0 or BO==bitter::bit_order::lsb0,
            "unexpected bit order");
    using iter = bitter::bit_iterator<BO>;
    using citer = bitter::const_bit_iterator<BO>;

    it("supports default construction", []{
        iter i1;
        (void) i1;
    });

    it("support comparison of default constructed objects", [] {
        iter i1;
        iter i2;
        AssertThat(i1 == i2, Equals(true));
        AssertThat(i1 != i2, Equals(false));
    });

    it("supports copy-construction", []{
        uint8_t foo[8];
        for (std::size_t c = 0; c < sizeof(foo)/sizeof(uint8_t); ++c)
        {
            for (std::size_t b = 0; b < 8; ++b)
            {
                iter i1(&foo[c], b);
                iter i2(i1);
                AssertThat(i1, Equals(i2));
            }
        }
    });

    it("supports assignment", []{
        uint8_t foo[8];
        iter i1;
        for (std::size_t c = 0; c < sizeof(foo)/sizeof(uint8_t); ++c)
        {
            for (std::size_t b = 0; b < 8; ++b)
            {
                iter i2(&foo[c], b);
                i1 = i2;
                AssertThat(i1, Equals(i2));
            }
        }
    });

    it("supports preincrement", []{
        uint8_t foo[8];
        for (std::size_t c = 0; c < sizeof(foo)/sizeof(uint8_t); ++c)
        {
            for (std::size_t b = 0; b < 8; ++b)
            {
                iter i1(&foo[c], b);
                iter i2(&foo[c+(b+1)/8], (b+1)%8);
                AssertThat(++i1, Equals(i2));
                AssertThat(i1, Equals(i2));
            }
        }
    });

    it("supports postincrement", []{
        uint8_t foo[8];
        for (std::size_t c = 0; c < sizeof(foo)/sizeof(uint8_t); ++c)
        {
            for (std::size_t b = 0; b < 8; ++b)
            {
                iter i1(&foo[c], b);
                iter i2(&foo[c], b);
                iter i3(&foo[c+(b+1)/8], (b+1)%8);
                AssertThat(i1++, Equals(i2));
                AssertThat(i1, Equals(i3));
            }
        }
    });

    it("supports predecrement", []{
        uint8_t foo[8];
        for (std::size_t c = 0; c < sizeof(foo)/sizeof(uint8_t); ++c)
        {
            for (std::size_t b = 0; b < 8; ++b)
            {
                iter i1(&foo[c+(b+1)/8], (b+1)%8);
                iter i2(&foo[c], b);
                AssertThat(--i1, Equals(i2));
                AssertThat(i1, Equals(i2));
            }
        }
    });

    it("supports postdecrement", []{
        uint8_t foo[8];
        for (std::size_t c = 0; c < sizeof(foo)/sizeof(uint8_t); ++c)
        {
            for (std::size_t b = 0; b < 8; ++b)
            {
                iter i1(&foo[c+(b+1)/8], (b+1)%8);
                iter i2(&foo[c+(b+1)/8], (b+1)%8);
                iter i3(&foo[c], b);
                AssertThat(i1--, Equals(i2));
                AssertThat(i1, Equals(i3));
            }
        }
    });

    it("supports equality/inequality comparison", []{
        uint8_t foo[8];
        for (std::size_t c1 = 0; c1 < sizeof(foo)/sizeof(uint8_t); ++c1)
        {
            for (std::size_t b1 = 0; b1 < 8; ++b1)
            {
                iter i1(&foo[c1], b1);
                for (std::size_t c2 = 0; c2 < sizeof(foo)/sizeof(uint8_t); ++c2)
                {
                    for (std::size_t b2 = 0; b2 < 8; ++b2)
                    {
                        iter i2(&foo[c2], b2);
                        AssertThat(i1 == i2, Equals(c1 == c2 and b1 == b2));
                        AssertThat(i1 != i2, Equals(c1 != c2 or b1 != b2));
                    }
                }
            }
        }
    });

    it("satisfies the examples in NIST FIPS 202", []{
        const std::array<uint8_t,1> data = {{0xA3}};
        const bitvec interpreted_as_bits =
            (BO == bitter::bit_order::msb0)
                ? make_bitvec({1,0,1,0, 0,0,1,1})
                : make_bitvec({1,1,0,0, 0,1,0,1});

        bitvec converted_to_bits;
        std::copy(citer(begin(data), 0), citer(end(data), 0),
                  back_inserter(converted_to_bits));
        AssertThat(converted_to_bits, EqualsContainer(interpreted_as_bits));

        unsigned char data_out[] = {0,0};
        std::copy(begin(interpreted_as_bits), end(interpreted_as_bits),
                  iter(data_out,0));
        AssertThat(static_cast<uint8_t>(data_out[0]), Equals(0xa3));
        AssertThat(static_cast<uint8_t>(data_out[1]), Equals(0));
    });

    it("handles partial bytes", [&]{
        std::vector<const char*> results =
            (BO == bitter::bit_order::msb0)
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
            AssertThat(copy_and_hexify<iter>(bits16, i), Equals(results.at(i)));
    });
}

go_bandit([]{
    describe("bit_iterator<bitter::bit_order::msb0>", &core_tests<bitter::bit_order::msb0>);
    describe("bit_iterator<bitter::bit_order::lsb0>", &core_tests<bitter::bit_order::lsb0>);
});

int main(int argc, char** argv)
{
    return bandit::run(argc, argv);
}
