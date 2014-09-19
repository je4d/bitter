#include <bandit/bandit.h>
using namespace bandit;

#include "bit.hpp"

go_bandit([]{
    describe("bit:", []{
        const bitter::bit b_true(true);
        const bitter::bit b_false(false);

        it("is false when default constructed", []{
            AssertThat(bool(bitter::bit{}), Equals(false));
            bitter::bit b;
            AssertThat(static_cast<bool>(b), Equals(false));
        });

        it("is constexpr default constructible", []{
            constexpr bitter::bit b = bitter::bit();;
            (void) b;
        });

        it("is constructible from bool", []{
            constexpr bitter::bit b_false(false);
            AssertThat(static_cast<bool>(b_false), Equals(false));
            constexpr bitter::bit b_true(true);
            AssertThat(static_cast<bool>(b_true), Equals(true));
        });

        it("is equality comparable", [&]{
            AssertThat(b_true, Equals(bitter::bit(true)));
            AssertThat(b_false, Equals(bitter::bit(false)));
        });

        it("is equality comparable to bool", [&]{
            AssertThat(b_false, Equals(false));
            AssertThat(false, Equals(b_false));
            AssertThat(b_true, Equals(true));
            AssertThat(true, Equals(b_true));
        });

        it("is inequality comparable", [&]{
            AssertThat(b_true != bitter::bit(false), Equals(true));
            AssertThat(b_false != bitter::bit(true), Equals(true));
        });

        it("is inequality comparable to bool", [&]{
            AssertThat(b_false != true, Equals(true));
            AssertThat(false != b_true, Equals(true));
            AssertThat(b_true != false, Equals(true));
            AssertThat(true != b_false, Equals(true));
        });

        it("is bitand-assignable", [&]{
            bitter::bit b1{b_true};
            b1 &= b_true;
            AssertThat(b1, Equals(b_true));

            bitter::bit b2{b_true};
            b2 &= b_false;
            AssertThat(b2, Equals(b_false));

            bitter::bit b3{b_false};
            b3 &= b_true;
            AssertThat(b3, Equals(b_false));

            bitter::bit b4{b_false};
            b4 &= b_false;
            AssertThat(b4, Equals(b_false));
        });

        it("is bitand-assignable from bool", [&]{
            bitter::bit b1{b_true};
            b1 &= true;
            AssertThat(b1, Equals(b_true));

            bitter::bit b2{b_true};
            b2 &= false;
            AssertThat(b2, Equals(b_false));

            bitter::bit b3{b_false};
            b3 &= true;
            AssertThat(b3, Equals(b_false));

            bitter::bit b4{b_false};
            b4 &= false;
            AssertThat(b4, Equals(b_false));
        });

        it("is bitor-assignable", [&]{
            bitter::bit b1{b_true};
            b1 |= b_true;
            AssertThat(b1, Equals(b_true));

            bitter::bit b2{b_true};
            b2 |= b_false;
            AssertThat(b2, Equals(b_true));

            bitter::bit b3{b_false};
            b3 |= b_true;
            AssertThat(b3, Equals(b_true));

            bitter::bit b4{b_false};
            b4 |= b_false;
            AssertThat(b4, Equals(b_false));
        });

        it("is bitor-assignable from bool", [&]{
            bitter::bit b1{b_true};
            b1 |= true;
            AssertThat(b1, Equals(b_true));

            bitter::bit b2{b_true};
            b2 |= false;
            AssertThat(b2, Equals(b_true));

            bitter::bit b3{b_false};
            b3 |= true;
            AssertThat(b3, Equals(b_true));

            bitter::bit b4{b_false};
            b4 |= false;
            AssertThat(b4, Equals(b_false));
        });

        it("is bitxor-assignable", [&]{
            bitter::bit b1{b_true};
            b1 ^= b_true;
            AssertThat(b1, Equals(b_false));

            bitter::bit b2{b_true};
            b2 ^= b_false;
            AssertThat(b2, Equals(b_true));

            bitter::bit b3{b_false};
            b3 ^= b_true;
            AssertThat(b3, Equals(b_true));

            bitter::bit b4{b_false};
            b4 ^= b_false;
            AssertThat(b4, Equals(b_false));
        });

        it("is bitxor-assignable from bool", [&]{
            bitter::bit b1{b_true};
            b1 ^= true;
            AssertThat(b1, Equals(b_false));

            bitter::bit b2{b_true};
            b2 ^= false;
            AssertThat(b2, Equals(b_true));

            bitter::bit b3{b_false};
            b3 ^= true;
            AssertThat(b3, Equals(b_true));

            bitter::bit b4{b_false};
            b4 ^= false;
            AssertThat(b4, Equals(b_false));
        });

        it("is invertible", [&]{
            AssertThat(!b_true, Equals(b_false));
            AssertThat(!b_false, Equals(b_true));
        });

        it("is bitandable", [&]{
            AssertThat(b_true & b_true, Equals(b_true));
            AssertThat(b_true & b_false, Equals(b_false));
            AssertThat(b_false & b_true, Equals(b_false));
            AssertThat(b_false & b_false, Equals(b_false));
        });

        it("is bitandable with bool", [&]{
            AssertThat(true & b_true, Equals(b_true));
            AssertThat(true & b_false, Equals(b_false));
            AssertThat(false & b_true, Equals(b_false));
            AssertThat(false & b_false, Equals(b_false));
            AssertThat(b_true & true, Equals(b_true));
            AssertThat(b_true & false, Equals(b_false));
            AssertThat(b_false & true, Equals(b_false));
            AssertThat(b_false & false, Equals(b_false));
        });

        it("is bitorable", [&]{
            AssertThat(b_true | b_true, Equals(b_true));
            AssertThat(b_true | b_false, Equals(b_true));
            AssertThat(b_false | b_true, Equals(b_true));
            AssertThat(b_false | b_false, Equals(b_false));
        });

        it("is bitorable with bool", [&]{
            AssertThat(true | b_true, Equals(b_true));
            AssertThat(true | b_false, Equals(b_true));
            AssertThat(false | b_true, Equals(b_true));
            AssertThat(false | b_false, Equals(b_false));
            AssertThat(b_true | true, Equals(b_true));
            AssertThat(b_true | false, Equals(b_true));
            AssertThat(b_false | true, Equals(b_true));
            AssertThat(b_false | false, Equals(b_false));
        });

        it("is bitxorable", [&]{
            AssertThat(b_true ^ b_true, Equals(b_false));
            AssertThat(b_true ^ b_false, Equals(b_true));
            AssertThat(b_false ^ b_true, Equals(b_true));
            AssertThat(b_false ^ b_false, Equals(b_false));
        });

        it("is bitxorable with bool", [&]{
            AssertThat(true ^ b_true, Equals(b_false));
            AssertThat(true ^ b_false, Equals(b_true));
            AssertThat(false ^ b_true, Equals(b_true));
            AssertThat(false ^ b_false, Equals(b_false));
            AssertThat(b_true ^ true, Equals(b_false));
            AssertThat(b_true ^ false, Equals(b_true));
            AssertThat(b_false ^ true, Equals(b_true));
            AssertThat(b_false ^ false, Equals(b_false));
        });

        it("is andable", [&]{
            AssertThat(b_true && b_true, Equals(b_true));
            AssertThat(b_true && b_false, Equals(b_false));
            AssertThat(b_false && b_true, Equals(b_false));
            AssertThat(b_false && b_false, Equals(b_false));
        });

        it("is andable with bool", [&]{
            AssertThat(true && b_true, Equals(b_true));
            AssertThat(true && b_false, Equals(b_false));
            AssertThat(false && b_true, Equals(b_false));
            AssertThat(false && b_false, Equals(b_false));
            AssertThat(b_true && true, Equals(b_true));
            AssertThat(b_true && false, Equals(b_false));
            AssertThat(b_false && true, Equals(b_false));
            AssertThat(b_false && false, Equals(b_false));
        });

        it("is orable", [&]{
            AssertThat(b_true || b_true, Equals(b_true));
            AssertThat(b_true || b_false, Equals(b_true));
            AssertThat(b_false || b_true, Equals(b_true));
            AssertThat(b_false || b_false, Equals(b_false));
        });

        it("is orable with bool", [&]{
            AssertThat(true || b_true, Equals(b_true));
            AssertThat(true || b_false, Equals(b_true));
            AssertThat(false || b_true, Equals(b_true));
            AssertThat(false || b_false, Equals(b_false));
            AssertThat(b_true || true, Equals(b_true));
            AssertThat(b_true || false, Equals(b_true));
            AssertThat(b_false || true, Equals(b_true));
            AssertThat(b_false || false, Equals(b_false));
        });

        it("is swappable", [&]{
            bitter::bit bt1{b_true};
            bitter::bit bf1{b_false};
            swap(bt1, bf1);
            AssertThat(bt1, Equals(b_false));
            AssertThat(bf1, Equals(b_true));

            bitter::bit bt2{b_true};
            bitter::bit bf2{b_false};
            swap(std::move(bt2), bf2);
            AssertThat(bt2, Equals(b_false));
            AssertThat(bf2, Equals(b_true));

            bitter::bit bt3{b_true};
            bitter::bit bf3{b_false};
            swap(bt3, std::move(bf3));
            AssertThat(bt3, Equals(b_false));
            AssertThat(bf3, Equals(b_true));
        });
    });
});

int main(int argc, char** argv)
{
    return bandit::run(argc, argv);
}
