#include <bandit/bandit.h>
using namespace bandit;

#include <cstddef>

#include "offset.hpp"

template <typename UL>
void test_element_and_bit_numbers()
{
    for (int i = -1000; i <= 1000; ++i)
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
        it("generates element and bit offsets for positive numbers", []{
            test_element_and_bit_numbers<char>();
            test_element_and_bit_numbers<unsigned char>();
            test_element_and_bit_numbers<unsigned int>();
            test_element_and_bit_numbers<unsigned long>();
            test_element_and_bit_numbers<unsigned long long>();
        });
    });
});

int main(int argc, char** argv)
{
    return bandit::run(argc, argv);
}
