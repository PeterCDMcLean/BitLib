template <size_t N = std::dynamic_extent, bool signed = true>
class bit_int : bit_array<bit_value, N> {

    // Use all of bit_array constructors
    using bit_array<bit_value, N>::bit_array;


    bit_int& operator++(const bit_int& other) {}
    bit_int& operator--(const bit_int& other) {}

    bit_int& operator+(const bit_int& other) {}
    bit_int& operator+=(const bit_int& other) {}
    bit_int& operator-(const bit_int& other) {}
    bit_int& operator-=(const bit_int& other) {}
    bit_int& operator*(const bit_int& other) {}
    bit_int& operator*=(const bit_int& other) {}
    bit_int& operator/(const bit_int& other) {}
    bit_int& operator/=(const bit_int& other) {}
    bit_int& operator%(const bit_int& other) {}
    bit_int& operator%=(const bit_int& other) {}

    bool operator>(const bit_int& other) {}
    bool operator>=(const bit_int& other) {}
    bool operator<(const bit_int& other) {}
    bool operator<=(const bit_int& other) {}

    bit_int& operator>>(const size_type shift) {}
    bit_int& operator>>=(const size_type shift) {}
    bit_int& operator<<(const size_type shift) {}
    bit_int& operator<<=(const size_type shift) {}
};
