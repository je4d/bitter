#include <bandit/bandit.h>
using namespace bandit;

#include "biterator.hpp"

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
std::string hexstring(char (&data)[N])
{
    char ret[N*2];
    char* out = ret;
    std::for_each(data, data+N, [&](char c){
        std::sprintf(out, "%02X", static_cast<uint8_t>(c));
        out += 2;
    });
    return std::string(ret);
}

template <typename T>
struct ArrayDelete {
    void operator()(T*t) { delete[] t; }
};

std::string hexstring(char *data, std::size_t n)
{
    const auto sz = (n+7)/8;
    const auto retlen = 2*sz;
    std::vector<char> ret(retlen+1);
    char* out = ret.data();
    std::for_each(data, data+sz, [&](char c){
        std::sprintf(out, "%02X", static_cast<uint8_t>(c));
        out += 2;
    });
    *prev(end(ret)) = 0;
    return std::string(ret.data(), retlen);
}

template <typename OutIt>
std::string copy_and_hexify(const bitvec& source, std::ptrdiff_t n)
{
    std::vector<char> out((n+7)/8);
    std::copy(begin(source), next(begin(source), n), OutIt(out.data(), 0));
    return hexstring(out.data(), n);
}

go_bandit([]{
    using lsb0iter = bitter::biterator<bitter::bit_order::lsb0>;
    using msb0iter = bitter::biterator<bitter::bit_order::msb0>;
    using lsb0citer = bitter::const_biterator<bitter::bit_order::lsb0>;
    using msb0citer = bitter::const_biterator<bitter::bit_order::msb0>;
    const bitvec bits16(16, bitter::bit(1));

    it("compares equal to self when default constructed", []{
        lsb0iter l1;
        lsb0iter l2;
        AssertThat(l1, Equals(l2));
        msb0iter m1;
        msb0iter m2;
        AssertThat(m1, Equals(m2));
    });

    it("satisfies the examples in NIST FIPS 202", []{
        const char* data = "\xA3";
        const bitvec interpreted_as_msb0 = make_bitvec({1,0,1,0, 0,0,1,1});
        const bitvec interpreted_as_lsb0 = make_bitvec({1,1,0,0, 0,1,0,1});

        bitvec converted_to_msb0;
        std::copy(msb0citer(data, 0), msb0citer(data+1, 0),
                  back_inserter(converted_to_msb0));
        AssertThat(converted_to_msb0, EqualsContainer(interpreted_as_msb0));

        bitvec converted_to_lsb0;
        std::copy(lsb0citer(data, 0), lsb0citer(data+1, 0),
                  back_inserter(converted_to_lsb0));
        AssertThat(converted_to_lsb0, EqualsContainer(interpreted_as_lsb0));

        char data_m[] = {0,0};
        std::copy(begin(interpreted_as_msb0), end(interpreted_as_msb0),
                  msb0iter(data_m,0));
        AssertThat(static_cast<uint8_t>(data_m[0]), Equals(0xa3));
        AssertThat(static_cast<uint8_t>(data_m[1]), Equals(0));

        char data_l[] = {0,0};
        std::copy(begin(interpreted_as_lsb0), end(interpreted_as_lsb0),
                  lsb0iter(data_l,0));
        AssertThat(static_cast<uint8_t>(data_l[0]), Equals(0xa3));
        AssertThat(static_cast<uint8_t>(data_l[1]), Equals(0));
    });

    it("handles partial bytes with msb0", [&]{
        AssertThat(copy_and_hexify<msb0iter>(bits16, 0),  Equals(""));
        AssertThat(copy_and_hexify<msb0iter>(bits16, 1),  Equals("80"));
        AssertThat(copy_and_hexify<msb0iter>(bits16, 2),  Equals("C0"));
        AssertThat(copy_and_hexify<msb0iter>(bits16, 3),  Equals("E0"));
        AssertThat(copy_and_hexify<msb0iter>(bits16, 4),  Equals("F0"));
        AssertThat(copy_and_hexify<msb0iter>(bits16, 5),  Equals("F8"));
        AssertThat(copy_and_hexify<msb0iter>(bits16, 6),  Equals("FC"));
        AssertThat(copy_and_hexify<msb0iter>(bits16, 7),  Equals("FE"));
        AssertThat(copy_and_hexify<msb0iter>(bits16, 8),  Equals("FF"));
        AssertThat(copy_and_hexify<msb0iter>(bits16, 9),  Equals("FF80"));
        AssertThat(copy_and_hexify<msb0iter>(bits16, 10), Equals("FFC0"));
        AssertThat(copy_and_hexify<msb0iter>(bits16, 11), Equals("FFE0"));
        AssertThat(copy_and_hexify<msb0iter>(bits16, 12), Equals("FFF0"));
        AssertThat(copy_and_hexify<msb0iter>(bits16, 13), Equals("FFF8"));
        AssertThat(copy_and_hexify<msb0iter>(bits16, 14), Equals("FFFC"));
        AssertThat(copy_and_hexify<msb0iter>(bits16, 15), Equals("FFFE"));
        AssertThat(copy_and_hexify<msb0iter>(bits16, 16), Equals("FFFF"));
    });

    it("handles partial bytes with lsb0", [&]{
        AssertThat(copy_and_hexify<lsb0iter>(bits16, 0),  Equals(""));
        AssertThat(copy_and_hexify<lsb0iter>(bits16, 1),  Equals("01"));
        AssertThat(copy_and_hexify<lsb0iter>(bits16, 2),  Equals("03"));
        AssertThat(copy_and_hexify<lsb0iter>(bits16, 3),  Equals("07"));
        AssertThat(copy_and_hexify<lsb0iter>(bits16, 4),  Equals("0F"));
        AssertThat(copy_and_hexify<lsb0iter>(bits16, 5),  Equals("1F"));
        AssertThat(copy_and_hexify<lsb0iter>(bits16, 6),  Equals("3F"));
        AssertThat(copy_and_hexify<lsb0iter>(bits16, 7),  Equals("7F"));
        AssertThat(copy_and_hexify<lsb0iter>(bits16, 8),  Equals("FF"));
        AssertThat(copy_and_hexify<lsb0iter>(bits16, 9),  Equals("FF01"));
        AssertThat(copy_and_hexify<lsb0iter>(bits16, 10), Equals("FF03"));
        AssertThat(copy_and_hexify<lsb0iter>(bits16, 11), Equals("FF07"));
        AssertThat(copy_and_hexify<lsb0iter>(bits16, 12), Equals("FF0F"));
        AssertThat(copy_and_hexify<lsb0iter>(bits16, 13), Equals("FF1F"));
        AssertThat(copy_and_hexify<lsb0iter>(bits16, 14), Equals("FF3F"));
        AssertThat(copy_and_hexify<lsb0iter>(bits16, 15), Equals("FF7F"));
        AssertThat(copy_and_hexify<lsb0iter>(bits16, 16), Equals("FFFF"));
    });

});

int main(int argc, char** argv)
{
    return bandit::run(argc, argv);
}
