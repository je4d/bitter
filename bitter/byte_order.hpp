#ifndef BITTER_BYTE_ORDER_HPP
#define BITTER_BYTE_ORDER_HPP

namespace bitter {

enum class byte_order
{
    none, // byte order not applicable
    msb0, // most significant byte is byte 0
    lsb0, // least significant byte is byte 0
};

} // namespace bitter

#endif // BITTER_BYTE_ORDER_HPP
