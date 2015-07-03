#include <bandit/bandit.h>
using namespace bandit;

#include "test_bit_iterator_copy_quick.hpp"

constexpr std::array<for_each_quicktest_range::test_range, 1000>
    for_each_quicktest_range::ranges;

void test_non_aliasing_copy_quick_srcs();
void test_non_aliasing_copy_quick_const_srcs();
void test_non_aliasing_copy_quick_dests();
void test_non_aliasing_copy_quick_const_dests();

go_bandit([] {
    describe("copying from a bitter::bit_iterator", [] {
        test_non_aliasing_copy_quick_srcs();
        test_non_aliasing_copy_quick_dests();
    });
    describe("copying from a bitter::const_bit_iterator", [] {
        test_non_aliasing_copy_quick_const_srcs();
        test_non_aliasing_copy_quick_const_dests();
    });
});

int main(int argc, char** argv)
{
    return bandit::run(argc, argv);
}
