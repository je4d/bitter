#include "test_bit_iterator_copy.hpp"

go_bandit([] {
    using types = test_copy_iterator_types;
    describe("copying from a std::reverse_iterator<const_bit_iterator>",
             &test_copy_fixed_itti<types::const_reverse_bit_iterator>);
});

int main(int argc, char** argv)
{
    return bandit::run(argc, argv);
}
