#ifndef BITTER_BYTE_ORDER_HPP
#define BITTER_BYTE_ORDER_HPP

namespace bitter {

enum class byte_order
{
    none,          // byte order not applicable
    big_endian,    // most significant byte first
    little_endian, // least significant byte first
    pdp_endian,    // only applies up to 32b
    msb0 =  big_endian,
    lsb0 =  little_endian,
};

} // namespace bitter

#endif // BITTER_BYTE_ORDER_HPP
