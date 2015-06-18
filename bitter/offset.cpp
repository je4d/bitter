#include <bandit/bandit.h>
using namespace bandit;

#include <cstddef>

#include "offset.hpp"

template <typename UL>
void test_element_and_bit_numbers(int from, int to)
{
    for (int i = from; i <= to; ++i)
    {
        bitter::offset o(i);
        auto e = o.element<UL>();
        auto b = o.bit<UL>();
        int element_size = sizeof(UL);
        AssertThat(i, Equals(e*element_size*8 + b));
        AssertThat(b, Is().Not().LessThan(0));
        AssertThat(b, IsLessThan(8*element_size));
    }
}

go_bandit([]{
    describe("bitter::offset: ", []{
        it("converts back to a ptrdiff_t", []{
            bitter::offset o{123};
            std::ptrdiff_t i = o;
            AssertThat(i, Equals(123));
        });
        it("converts back to an int", []{
            bitter::offset o{123};
            int i = o;
            AssertThat(i, Equals(123));
        });
        it("holds large numbers", []{
            bitter::offset o{1234567890};
            std::ptrdiff_t i = o;
            AssertThat(i, Equals(1234567890));

            bitter::offset o_neg{-1234567890};
            std::ptrdiff_t i_neg = o_neg;
            AssertThat(i_neg, Equals(-1234567890));
        });
        it("generates correct element and bit numbers for positive offsets", []{
            test_element_and_bit_numbers<char>(1,1000);
            test_element_and_bit_numbers<unsigned char>(1,1000);
            test_element_and_bit_numbers<unsigned int>(1,1000);
            test_element_and_bit_numbers<unsigned long>(1,1000);
            test_element_and_bit_numbers<unsigned long long>(1,1000);
        });
        it("generates correct element and bit numbers for negative offsets", []{
            test_element_and_bit_numbers<char>(-1000,-1);
            test_element_and_bit_numbers<unsigned char>(-1000,-1);
            test_element_and_bit_numbers<unsigned int>(-1000,-1);
            test_element_and_bit_numbers<unsigned long>(-1000,-1);
            test_element_and_bit_numbers<unsigned long long>(-1000,-1);
        });
        it("generates correct element and bit numbers for zero offset", []{
            test_element_and_bit_numbers<char>(0,0);
            test_element_and_bit_numbers<unsigned char>(0,0);
            test_element_and_bit_numbers<unsigned int>(0,0);
            test_element_and_bit_numbers<unsigned long>(0,0);
            test_element_and_bit_numbers<unsigned long long>(0,0);
        });
    });
});

int main(int argc, char** argv)
{
    return bandit::run(argc, argv);
}
