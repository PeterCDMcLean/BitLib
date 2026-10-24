#include <cstdint>
#include <iostream>

#include "bitlib/bitlib.hpp"

// Compile-time half-precision float inheriting from bit_array<16>
// Layout (little bit endian): bit[15]=sign, bits[10,15)=exponent, bits[0,10)=mantissa
struct fp_half : bit::bit_array<16> {
  using bit::bit_array<16>::bit_array;

  // Sign: bit 15 (MSB) - uses back() with compile-time alignment
  constexpr auto sign() { return this->back(); }

  // Exponent: bits [10, 15) - 5 bits, compile-time slice
  constexpr auto exponent() { return this->template slice<10, 15>(); }

  // Mantissa: bits [0, 10) - 10 bits, compile-time slice
  constexpr auto mantissa() { return this->template slice<0, 10>(); }
};

int main() {
  // Construct from raw 16-bit value: sign=0, exp=0b10101 (21), mantissa=0b0101010101 (341)
  fp_half fp(0b0'10101'0101010101);

  // Modify fields through compile-time slices
  fp.sign() = bit::bit1;      // Flip sign bit
  fp.exponent() = 0x5'1E_b;   // Set exponent to 30 (5-bit slice)
  fp.mantissa() = 0xA'2AA_b;  // Set mantissa to 682 (10-bit slice)

  // Read back through compile-time slices
  std::cout << "sign: " << static_cast<bool>(fp.sign()) << "\n";          // 1
  std::cout << "exp:  " << static_cast<uint16_t>(fp.exponent()) << "\n";  // 30
  std::cout << "man:  " << static_cast<uint16_t>(fp.mantissa()) << "\n";  // 682

  return 0;
}
