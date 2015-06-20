#include "bit_iterator.hpp"

#include <bitset>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <type_traits>

using std::size_t;
using std::ptrdiff_t;
using bit_order = bitter::bit_order;
using byte_order = bitter::byte_order;

namespace bitter {
    std::ostream&operator<<(std::ostream& o,const bit& b)
    {
        return o << (b ? "1_b" : "0_b");
    }
    template <typename UL>
    std::ostream&operator<<(std::ostream& o,const bitref<UL>& b)
    {
        return o << (bit(b) ? "1_b" : "0_b");
    }
    template <bit_order BO, typename UL, byte_order YO>
    std::ostream& operator<<(std::ostream& o,
                             const const_bit_iterator<BO,UL,YO>& it)
    {
        void* addr = it.data;
        return o << "[" << addr << ", " << static_cast<int>(it.bitno) << "]";
    }
    template <bit_order BO, typename UL, byte_order YO>
    std::ostream& operator<<(std::ostream& o, const bit_iterator<BO,UL,YO>& it)
    {
        void* addr = it.data;
        return o << "[" << addr << ", " << static_cast<int>(it.bitno) << "]";
    }
}

namespace std
{
    template <typename T, std::size_t N>
    typename std::enable_if<std::is_integral<T>::value,
    std::ostream&>::type operator<<(std::ostream& o, const std::array<T,N>& a)
    {
        o << "[";
        bool first = true;
        for (auto& x : a) {
            o << (first ? "" : ", ") << std::bitset<sizeof(T)*8>(x);
            first = false;
        }
        return o<<"]";
    }
}

static constexpr std::ptrdiff_t testvec_bits = 32*4;

template <bit_order BO, typename UL, byte_order YO = byte_order::none>
struct testvec_;

template<> struct testvec_<bit_order::lsb0,uint8_t> {
    static constexpr std::array<uint8_t,testvec_bits/8> data {{
        0xa2, 0x2b, 0x34, 0x6c, 0xc9, 0xf8, 0x1f, 0xce,
        0x4c, 0xd2, 0x4a, 0x55, 0x55, 0xb5, 0x5a, 0x6b }}; };
constexpr std::array<uint8_t,testvec_bits/8>
     testvec_<bit_order::lsb0,uint8_t>::data;
template<> struct testvec_<bit_order::lsb0,uint16_t,byte_order::lsb0> {////
    static constexpr std::array<uint16_t,testvec_bits/16> data {{
        0x2ba2, 0x6c34, 0xf8c9, 0xce1f, 0xd24c, 0x554a, 0xb555, 0x6b5a }}; };
constexpr std::array<uint16_t,testvec_bits/16>
     testvec_<bit_order::lsb0,uint16_t,byte_order::lsb0>::data;
template<> struct testvec_<bit_order::lsb0,uint32_t,byte_order::lsb0> {
    static constexpr std::array<uint32_t,testvec_bits/32> data {{
        0x6c342ba2, 0xce1ff8c9, 0x554ad24c, 0x6b5ab555 }}; };
constexpr std::array<uint32_t,testvec_bits/32>
     testvec_<bit_order::lsb0,uint32_t,byte_order::lsb0>::data;
template<> struct testvec_<bit_order::lsb0,uint64_t,byte_order::lsb0> {
    static constexpr std::array<uint64_t,testvec_bits/64> data {{
        0xce1ff8c96c342ba2, 0x6b5ab555554ad24c }}; };
constexpr std::array<uint64_t,testvec_bits/64>
     testvec_<bit_order::lsb0,uint64_t,byte_order::lsb0>::data;
template<> struct testvec_<bit_order::lsb0,uint16_t,byte_order::msb0> {
    static constexpr std::array<uint16_t,testvec_bits/16> data {{
        0xa22b, 0x346c, 0xc9f8, 0x1fce, 0x4cd2, 0x4a55, 0x55b5, 0x5a6b }}; };
constexpr std::array<uint16_t,testvec_bits/16>
     testvec_<bit_order::lsb0,uint16_t,byte_order::msb0>::data;
template<> struct testvec_<bit_order::lsb0,uint32_t,byte_order::msb0> {
    static constexpr std::array<uint32_t,testvec_bits/32> data {{
        0xa22b346c, 0xc9f81fce, 0x4cd24a55, 0x55b55a6b }}; };
constexpr std::array<uint32_t,testvec_bits/32>
     testvec_<bit_order::lsb0,uint32_t,byte_order::msb0>::data;
template<> struct testvec_<bit_order::lsb0,uint64_t,byte_order::msb0> {
    static constexpr std::array<uint64_t,testvec_bits/64> data {{
        0xa22b346cc9f81fce, 0x4cd24a5555b55a6b }}; };
constexpr std::array<uint64_t,testvec_bits/64>
     testvec_<bit_order::lsb0,uint64_t,byte_order::msb0>::data;
template<> struct testvec_<bit_order::msb0,uint8_t> {
    static constexpr std::array<uint8_t,testvec_bits/8> data {{
        0x45, 0xd4, 0x2c, 0x36, 0x93, 0x1f, 0xf8, 0x73,
        0x32, 0x4b, 0x52, 0xaa, 0xaa, 0xad, 0x5a, 0xd6 }}; };
constexpr std::array<uint8_t,testvec_bits/8>
     testvec_<bit_order::msb0,uint8_t>::data;
template<> struct testvec_<bit_order::msb0,uint16_t,byte_order::lsb0> {
    static constexpr std::array<uint16_t,testvec_bits/16> data {{
        0xd445, 0x362c, 0x1f93, 0x73f8, 0x4b32, 0xaa52, 0xadaa, 0xd65a }}; };
constexpr std::array<uint16_t,testvec_bits/16>
     testvec_<bit_order::msb0,uint16_t,byte_order::lsb0>::data;
template<> struct testvec_<bit_order::msb0,uint32_t,byte_order::lsb0> {
    static constexpr std::array<uint32_t,testvec_bits/32> data {{
        0x362cd445, 0x73f81f93, 0xaa524b32, 0xd65aadaa }}; };
constexpr std::array<uint32_t,testvec_bits/32>
     testvec_<bit_order::msb0,uint32_t,byte_order::lsb0>::data;
template<> struct testvec_<bit_order::msb0,uint64_t,byte_order::lsb0> {
    static constexpr std::array<uint64_t,testvec_bits/64> data {{
        0x73f81f93362cd445, 0xd65aadaaaa524b32 }}; };
constexpr std::array<uint64_t,testvec_bits/64>
     testvec_<bit_order::msb0,uint64_t,byte_order::lsb0>::data;
template<> struct testvec_<bit_order::msb0,uint16_t,byte_order::msb0> {
    static constexpr std::array<uint16_t,testvec_bits/16> data {{
        0x45d4, 0x2c36, 0x931f, 0xf873, 0x324b, 0x52aa, 0xaaad, 0x5ad6 }}; };
constexpr std::array<uint16_t,testvec_bits/16>
     testvec_<bit_order::msb0,uint16_t,byte_order::msb0>::data;
template<> struct testvec_<bit_order::msb0,uint32_t,byte_order::msb0> {
    static constexpr std::array<uint32_t,testvec_bits/32> data {{
        0x45d42c36, 0x931ff873, 0x324b52aa, 0xaaad5ad6 }}; };
constexpr std::array<uint32_t,testvec_bits/32>
     testvec_<bit_order::msb0,uint32_t,byte_order::msb0>::data;
template<> struct testvec_<bit_order::msb0,uint64_t,byte_order::msb0> {
    static constexpr std::array<uint64_t,testvec_bits/64> data {{
        0x45d42c36931ff873, 0x324b52aaaaad5ad6 }}; };
constexpr std::array<uint64_t,testvec_bits/64>
     testvec_<bit_order::msb0,uint64_t,byte_order::msb0>::data;

template <bit_order BO, typename UL, byte_order YO = byte_order::none>
constexpr const std::array<UL,testvec_bits/8/sizeof(UL)>& testvec()
{
    return testvec_<BO,UL,YO>::data;
}

template <typename T>
struct bit_iterator_traits;

template <bitter::bit_order BO, typename UL, bitter::byte_order YO>
struct bit_iterator_traits<bitter::bit_iterator<BO,UL,YO>>
{
    static constexpr bitter::bit_order bit_order = BO;
    using underlying_type = UL;
    static constexpr bitter::byte_order byte_order = YO;
};

template <bitter::bit_order BO, typename UL, bitter::byte_order YO>
struct bit_iterator_traits<bitter::const_bit_iterator<BO,UL,YO>>
{
    static constexpr bitter::bit_order bit_order = BO;
    using underlying_type = UL;
    static constexpr bitter::byte_order byte_order = YO;
};


