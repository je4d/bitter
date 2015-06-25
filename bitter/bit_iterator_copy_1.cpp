#include "test_bit_iterator_copy.hpp"

go_bandit([]{
    describe("copying from a bit_iterator",
             [] { test_copy_fixed_itti<bitter::bit_iterator>(); });
});

int main(int argc, char** argv)
{
    return bandit::run(argc, argv);
}
