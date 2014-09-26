#ifndef BITTER_BYTE_ORDER_HPP
#define BITTER_BYTE_ORDER_HPP

namespace bitter {

enum class byte_order
{
    big_endian,    // most significant byte first
    little_endian, // least significant byte first
    none           // byte order not applicable
};

} // namespace bitter

#endif // BITTER_BYTE_ORDER_HPP
