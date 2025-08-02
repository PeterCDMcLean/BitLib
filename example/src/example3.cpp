#include "bitlib/bitlib.hpp"

struct fp_dynamic : bit::bit_array<> {
  const size_t mantissa_bits;
  fp_dynamic() = delete;
  fp_dynamic(
    size_t exponent_bits,
    size_t mantissa_bits) :
      bit::bit_array<>(1+exponent_bits+mantissa_bits),
      mantissa_bits(mantissa_bits) {}
  bit::bit_reference<> sign() {
    return (*this).back();
  }
  auto exponent() {
    return (*this)(mantissa_bits, this->size()-1);
  }
  auto mantissa() {
    return (*this)(0, mantissa_bits);
  }
};

#include <iostream>

int main(int argn, char* argv[]) {
  fp_dynamic fp(6u, 7u);
  fp.sign() = bit::bit1;
  fp.exponent() = 6'5_b;
  fp.mantissa() = 0x7'3F_b;
  std::cout << "fp: " << fp << "\n";
}
