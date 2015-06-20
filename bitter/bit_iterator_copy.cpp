#include "test_bit_iterator_copy.hpp"

go_bandit([]{
    using namespace bitter;
    it("does a copy", []{
        copy_fwd_fwd<
            const uint64_t,
            const_bit_iterator<bit_order::lsb0,uint64_t,byte_order::msb0>,
            bit_iterator<bit_order::msb0,uint8_t>>();
            });

});

int main(int argc, char** argv)
{
    return bandit::run(argc, argv);
}
