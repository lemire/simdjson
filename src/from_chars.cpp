#include <limits>
namespace simdjson {
namespace internal {

/**
 * The code in the internal::from_chars function is meant to handle the floating-point number parsing
 * when we have more than 19 digits in the decimal mantissa. This should only be seen
 * in adversarial scenarios: we do not expect production systems to even produce
 * such floating-point numbers.
 *
 * The parser is based on work by Nigel Tao (at https://github.com/google/wuffs/)
 * who credits Ken Thompson for the design (via a reference to the Go source
 * code). See
 * https://github.com/google/wuffs/blob/aa46859ea40c72516deffa1b146121952d6dfd3b/internal/cgen/base/floatconv-submodule-data.c
 * https://github.com/google/wuffs/blob/46cd8105f47ca07ae2ba8e6a7818ef9c0df6c152/internal/cgen/base/floatconv-submodule-code.c
 * It is probably not very fast but it is a fallback that should almost never be
 * called in real life. Google Wuffs is published under APL 2.0.
 **/

namespace {
constexpr uint32_t max_digits = 768;
constexpr int32_t decimal_point_range = 2047;
} // namespace

struct adjusted_mantissa {
  uint64_t mantissa;
  int power2;
  adjusted_mantissa() : mantissa(0), power2(0) {}
};

struct decimal {
  uint32_t num_digits;
  int32_t decimal_point;
  bool negative;
  bool truncated;
  uint8_t digits[max_digits];
};

template <typename T> struct binary_format {
  static constexpr int mantissa_explicit_bits();
  static constexpr int minimum_exponent();
  static constexpr int infinite_power();
  static constexpr int sign_index();
};

template <> constexpr int binary_format<double>::mantissa_explicit_bits() {
  return 52;
}

template <> constexpr int binary_format<double>::minimum_exponent() {
  return -1023;
}
template <> constexpr int binary_format<double>::infinite_power() {
  return 0x7FF;
}

template <> constexpr int binary_format<double>::sign_index() { return 63; }

bool is_integer(char c)  noexcept  { return (c >= '0' && c <= '9'); }

// This should always succeed since it follows a call to parse_number.
decimal parse_decimal(const char *&p) noexcept {
  decimal answer;
  answer.num_digits = 0;
  answer.decimal_point = 0;
  answer.truncated = false;
  answer.negative = (*p == '-');
  if ((*p == '-') || (*p == '+')) {
    ++p;
  }

  while (*p == '0') {
    ++p;
  }
  while (is_integer(*p)) {
    if (answer.num_digits < max_digits) {
      answer.digits[answer.num_digits] = uint8_t(*p - '0');
    }
    answer.num_digits++;
    ++p;
  }
  if (*p == '.') {
    ++p;
    const char *first_after_period = p;
    // if we have not yet encountered a zero, we have to skip it as well
    if (answer.num_digits == 0) {
      // skip zeros
      while (*p == '0') {
        ++p;
      }
    }
    while (is_integer(*p)) {
      if (answer.num_digits < max_digits) {
        answer.digits[answer.num_digits] = uint8_t(*p - '0');
      }
      answer.num_digits++;
      ++p;
    }
    answer.decimal_point = int32_t(first_after_period - p);
  }
  if(answer.num_digits > 0) {
    const char *preverse = p - 1;
    int32_t trailing_zeros = 0;
    while ((*preverse == '0') || (*preverse == '.')) {
      if(*preverse == '0') { trailing_zeros++; };
      --preverse;
    }
    answer.decimal_point += int32_t(answer.num_digits);
    answer.num_digits -= uint32_t(trailing_zeros);
  }
  if(answer.num_digits > max_digits ) {
    answer.num_digits = max_digits;
    answer.truncated = true;
  }
  if (('e' == *p) || ('E' == *p)) {
    ++p;
    bool neg_exp = false;
    if ('-' == *p) {
      neg_exp = true;
      ++p;
    } else if ('+' == *p) {
      ++p;
    }
    int32_t exp_number = 0; // exponential part
    while (is_integer(*p)) {
      uint8_t digit = uint8_t(*p - '0');
      if (exp_number < 0x10000) {
        exp_number = 10 * exp_number + digit;
      }
      ++p;
    }
    answer.decimal_point += (neg_exp ? -exp_number : exp_number);
  }
  return answer;
}

// This should always succeed since it follows a call to parse_number.
// Will not read at or beyond the "end" pointer.
decimal parse_decimal(const char *&p, const char * end) noexcept {
  decimal answer;
  answer.num_digits = 0;
  answer.decimal_point = 0;
  answer.truncated = false;
  if(p == end) { return answer; } // should never happen
  answer.negative = (*p == '-');
  if ((*p == '-') || (*p == '+')) {
    ++p;
  }

  while ((p != end) && (*p == '0')) {
    ++p;
  }
  while ((p != end) && is_integer(*p)) {
    if (answer.num_digits < max_digits) {
      answer.digits[answer.num_digits] = uint8_t(*p - '0');
    }
    answer.num_digits++;
    ++p;
  }
  if ((p != end) && (*p == '.')) {
    ++p;
    if(p == end) { return answer; } // should never happen
    const char *first_after_period = p;
    // if we have not yet encountered a zero, we have to skip it as well
    if (answer.num_digits == 0) {
      // skip zeros
      while (*p == '0') {
        ++p;
      }
    }
    while ((p != end) && is_integer(*p)) {
      if (answer.num_digits < max_digits) {
        answer.digits[answer.num_digits] = uint8_t(*p - '0');
      }
      answer.num_digits++;
      ++p;
    }
    answer.decimal_point = int32_t(first_after_period - p);
  }
  if(answer.num_digits > 0) {
    const char *preverse = p - 1;
    int32_t trailing_zeros = 0;
    while ((*preverse == '0') || (*preverse == '.')) {
      if(*preverse == '0') { trailing_zeros++; };
      --preverse;
    }
    answer.decimal_point += int32_t(answer.num_digits);
    answer.num_digits -= uint32_t(trailing_zeros);
  }
  if(answer.num_digits > max_digits ) {
    answer.num_digits = max_digits;
    answer.truncated = true;
  }
  if ((p != end) && (('e' == *p) || ('E' == *p))) {
    ++p;
    if(p == end) { return answer; } // should never happen
    bool neg_exp = false;
    if ('-' == *p) {
      neg_exp = true;
      ++p;
    } else if ('+' == *p) {
      ++p;
    }
    int32_t exp_number = 0; // exponential part
    while ((p != end) && is_integer(*p)) {
      uint8_t digit = uint8_t(*p - '0');
      if (exp_number < 0x10000) {
        exp_number = 10 * exp_number + digit;
      }
      ++p;
    }
    answer.decimal_point += (neg_exp ? -exp_number : exp_number);
  }
  return answer;
}

namespace {

// remove all final zeroes
inline void trim(decimal &h) {
  while ((h.num_digits > 0) && (h.digits[h.num_digits - 1] == 0)) {
    h.num_digits--;
  }
}

uint32_t number_of_digits_decimal_left_shift(decimal &h, uint32_t shift) {
  shift &= 63;
  const static uint16_t number_of_digits_decimal_left_shift_table[65] = {
      0x0000, 0x0800, 0x0801, 0x0803, 0x1006, 0x1009, 0x100D, 0x1812, 0x1817,
      0x181D, 0x2024, 0x202B, 0x2033, 0x203C, 0x2846, 0x2850, 0x285B, 0x3067,
      0x3073, 0x3080, 0x388E, 0x389C, 0x38AB, 0x38BB, 0x40CC, 0x40DD, 0x40EF,
      0x4902, 0x4915, 0x4929, 0x513E, 0x5153, 0x5169, 0x5180, 0x5998, 0x59B0,
      0x59C9, 0x61E3, 0x61FD, 0x6218, 0x6A34, 0x6A50, 0x6A6D, 0x6A8B, 0x72AA,
      0x72C9, 0x72E9, 0x7B0A, 0x7B2B, 0x7B4D, 0x8370, 0x8393, 0x83B7, 0x83DC,
      0x8C02, 0x8C28, 0x8C4F, 0x9477, 0x949F, 0x94C8, 0x9CF2, 0x051C, 0x051C,
      0x051C, 0x051C,
  };
  uint32_t x_a = number_of_digits_decimal_left_shift_table[shift];
  uint32_t x_b = number_of_digits_decimal_left_shift_table[shift + 1];
  uint32_t num_new_digits = x_a >> 11;
  uint32_t pow5_a = 0x7FF & x_a;
  uint32_t pow5_b = 0x7FF & x_b;
  const static uint8_t
      number_of_digits_decimal_left_shift_table_powers_of_5[0x051C] = {
          5, 2, 5, 1, 2, 5, 6, 2, 5, 3, 1, 2, 5, 1, 5, 6, 2, 5, 7, 8, 1, 2, 5,
          3, 9, 0, 6, 2, 5, 1, 9, 5, 3, 1, 2, 5, 9, 7, 6, 5, 6, 2, 5, 4, 8, 8,
          2, 8, 1, 2, 5, 2, 4, 4, 1, 4, 0, 6, 2, 5, 1, 2, 2, 0, 7, 0, 3, 1, 2,
          5, 6, 1, 0, 3, 5, 1, 5, 6, 2, 5, 3, 0, 5, 1, 7, 5, 7, 8, 1, 2, 5, 1,
          5, 2, 5, 8, 7, 8, 9, 0, 6, 2, 5, 7, 6, 2, 9, 3, 9, 4, 5, 3, 1, 2, 5,
          3, 8, 1, 4, 6, 9, 7, 2, 6, 5, 6, 2, 5, 1, 9, 0, 7, 3, 4, 8, 6, 3, 2,
          8, 1, 2, 5, 9, 5, 3, 6, 7, 4, 3, 1, 6, 4, 0, 6, 2, 5, 4, 7, 6, 8, 3,
          7, 1, 5, 8, 2, 0, 3, 1, 2, 5, 2, 3, 8, 4, 1, 8, 5, 7, 9, 1, 0, 1, 5,
          6, 2, 5, 1, 1, 9, 2, 0, 9, 2, 8, 9, 5, 5, 0, 7, 8, 1, 2, 5, 5, 9, 6,
          0, 4, 6, 4, 4, 7, 7, 5, 3, 9, 0, 6, 2, 5, 2, 9, 8, 0, 2, 3, 2, 2, 3,
          8, 7, 6, 9, 5, 3, 1, 2, 5, 1, 4, 9, 0, 1, 1, 6, 1, 1, 9, 3, 8, 4, 7,
          6, 5, 6, 2, 5, 7, 4, 5, 0, 5, 8, 0, 5, 9, 6, 9, 2, 3, 8, 2, 8, 1, 2,
          5, 3, 7, 2, 5, 2, 9, 0, 2, 9, 8, 4, 6, 1, 9, 1, 4, 0, 6, 2, 5, 1, 8,
          6, 2, 6, 4, 5, 1, 4, 9, 2, 3, 0, 9, 5, 7, 0, 3, 1, 2, 5, 9, 3, 1, 3,
          2, 2, 5, 7, 4, 6, 1, 5, 4, 7, 8, 5, 1, 5, 6, 2, 5, 4, 6, 5, 6, 6, 1,
          2, 8, 7, 3, 0, 7, 7, 3, 9, 2, 5, 7, 8, 1, 2, 5, 2, 3, 2, 8, 3, 0, 6,
          4, 3, 6, 5, 3, 8, 6, 9, 6, 2, 8, 9, 0, 6, 2, 5, 1, 1, 6, 4, 1, 5, 3,
          2, 1, 8, 2, 6, 9, 3, 4, 8, 1, 4, 4, 5, 3, 1, 2, 5, 5, 8, 2, 0, 7, 6,
          6, 0, 9, 1, 3, 4, 6, 7, 4, 0, 7, 2, 2, 6, 5, 6, 2, 5, 2, 9, 1, 0, 3,
          8, 3, 0, 4, 5, 6, 7, 3, 3, 7, 0, 3, 6, 1, 3, 2, 8, 1, 2, 5, 1, 4, 5,
          5, 1, 9, 1, 5, 2, 2, 8, 3, 6, 6, 8, 5, 1, 8, 0, 6, 6, 4, 0, 6, 2, 5,
          7, 2, 7, 5, 9, 5, 7, 6, 1, 4, 1, 8, 3, 4, 2, 5, 9, 0, 3, 3, 2, 0, 3,
          1, 2, 5, 3, 6, 3, 7, 9, 7, 8, 8, 0, 7, 0, 9, 1, 7, 1, 2, 9, 5, 1, 6,
          6, 0, 1, 5, 6, 2, 5, 1, 8, 1, 8, 9, 8, 9, 4, 0, 3, 5, 4, 5, 8, 5, 6,
          4, 7, 5, 8, 3, 0, 0, 7, 8, 1, 2, 5, 9, 0, 9, 4, 9, 4, 7, 0, 1, 7, 7,
          2, 9, 2, 8, 2, 3, 7, 9, 1, 5, 0, 3, 9, 0, 6, 2, 5, 4, 5, 4, 7, 4, 7,
          3, 5, 0, 8, 8, 6, 4, 6, 4, 1, 1, 8, 9, 5, 7, 5, 1, 9, 5, 3, 1, 2, 5,
          2, 2, 7, 3, 7, 3, 6, 7, 5, 4, 4, 3, 2, 3, 2, 0, 5, 9, 4, 7, 8, 7, 5,
          9, 7, 6, 5, 6, 2, 5, 1, 1, 3, 6, 8, 6, 8, 3, 7, 7, 2, 1, 6, 1, 6, 0,
          2, 9, 7, 3, 9, 3, 7, 9, 8, 8, 2, 8, 1, 2, 5, 5, 6, 8, 4, 3, 4, 1, 8,
          8, 6, 0, 8, 0, 8, 0, 1, 4, 8, 6, 9, 6, 8, 9, 9, 4, 1, 4, 0, 6, 2, 5,
          2, 8, 4, 2, 1, 7, 0, 9, 4, 3, 0, 4, 0, 4, 0, 0, 7, 4, 3, 4, 8, 4, 4,
          9, 7, 0, 7, 0, 3, 1, 2, 5, 1, 4, 2, 1, 0, 8, 5, 4, 7, 1, 5, 2, 0, 2,
          0, 0, 3, 7, 1, 7, 4, 2, 2, 4, 8, 5, 3, 5, 1, 5, 6, 2, 5, 7, 1, 0, 5,
          4, 2, 7, 3, 5, 7, 6, 0, 1, 0, 0, 1, 8, 5, 8, 7, 1, 1, 2, 4, 2, 6, 7,
          5, 7, 8, 1, 2, 5, 3, 5, 5, 2, 7, 1, 3, 6, 7, 8, 8, 0, 0, 5, 0, 0, 9,
          2, 9, 3, 5, 5, 6, 2, 1, 3, 3, 7, 8, 9, 0, 6, 2, 5, 1, 7, 7, 6, 3, 5,
          6, 8, 3, 9, 4, 0, 0, 2, 5, 0, 4, 6, 4, 6, 7, 7, 8, 1, 0, 6, 6, 8, 9,
          4, 5, 3, 1, 2, 5, 8, 8, 8, 1, 7, 8, 4, 1, 9, 7, 0, 0, 1, 2, 5, 2, 3,
          2, 3, 3, 8, 9, 0, 5, 3, 3, 4, 4, 7, 2, 6, 5, 6, 2, 5, 4, 4, 4, 0, 8,
          9, 2, 0, 9, 8, 5, 0, 0, 6, 2, 6, 1, 6, 1, 6, 9, 4, 5, 2, 6, 6, 7, 2,
          3, 6, 3, 2, 8, 1, 2, 5, 2, 2, 2, 0, 4, 4, 6, 0, 4, 9, 2, 5, 0, 3, 1,
          3, 0, 8, 0, 8, 4, 7, 2, 6, 3, 3, 3, 6, 1, 8, 1, 6, 4, 0, 6, 2, 5, 1,
          1, 1, 0, 2, 2, 3, 0, 2, 4, 6, 2, 5, 1, 5, 6, 5, 4, 0, 4, 2, 3, 6, 3,
          1, 6, 6, 8, 0, 9, 0, 8, 2, 0, 3, 1, 2, 5, 5, 5, 5, 1, 1, 1, 5, 1, 2,
          3, 1, 2, 5, 7, 8, 2, 7, 0, 2, 1, 1, 8, 1, 5, 8, 3, 4, 0, 4, 5, 4, 1,
          0, 1, 5, 6, 2, 5, 2, 7, 7, 5, 5, 5, 7, 5, 6, 1, 5, 6, 2, 8, 9, 1, 3,
          5, 1, 0, 5, 9, 0, 7, 9, 1, 7, 0, 2, 2, 7, 0, 5, 0, 7, 8, 1, 2, 5, 1,
          3, 8, 7, 7, 7, 8, 7, 8, 0, 7, 8, 1, 4, 4, 5, 6, 7, 5, 5, 2, 9, 5, 3,
          9, 5, 8, 5, 1, 1, 3, 5, 2, 5, 3, 9, 0, 6, 2, 5, 6, 9, 3, 8, 8, 9, 3,
          9, 0, 3, 9, 0, 7, 2, 2, 8, 3, 7, 7, 6, 4, 7, 6, 9, 7, 9, 2, 5, 5, 6,
          7, 6, 2, 6, 9, 5, 3, 1, 2, 5, 3, 4, 6, 9, 4, 4, 6, 9, 5, 1, 9, 5, 3,
          6, 1, 4, 1, 8, 8, 8, 2, 3, 8, 4, 8, 9, 6, 2, 7, 8, 3, 8, 1, 3, 4, 7,
          6, 5, 6, 2, 5, 1, 7, 3, 4, 7, 2, 3, 4, 7, 5, 9, 7, 6, 8, 0, 7, 0, 9,
          4, 4, 1, 1, 9, 2, 4, 4, 8, 1, 3, 9, 1, 9, 0, 6, 7, 3, 8, 2, 8, 1, 2,
          5, 8, 6, 7, 3, 6, 1, 7, 3, 7, 9, 8, 8, 4, 0, 3, 5, 4, 7, 2, 0, 5, 9,
          6, 2, 2, 4, 0, 6, 9, 5, 9, 5, 3, 3, 6, 9, 1, 4, 0, 6, 2, 5,
      };
  const uint8_t *pow5 =
      &number_of_digits_decimal_left_shift_table_powers_of_5[pow5_a];
  uint32_t i = 0;
  uint32_t n = pow5_b - pow5_a;
  for (; i < n; i++) {
    if (i >= h.num_digits) {
      return num_new_digits - 1;
    } else if (h.digits[i] == pow5[i]) {
      continue;
    } else if (h.digits[i] < pow5[i]) {
      return num_new_digits - 1;
    } else {
      return num_new_digits;
    }
  }
  return num_new_digits;
}

} // end of anonymous namespace

uint64_t round(decimal &h) {
  if ((h.num_digits == 0) || (h.decimal_point < 0)) {
    return 0;
  } else if (h.decimal_point > 18) {
    return UINT64_MAX;
  }
  // at this point, we know that h.decimal_point >= 0
  uint32_t dp = uint32_t(h.decimal_point);
  uint64_t n = 0;
  for (uint32_t i = 0; i < dp; i++) {
    n = (10 * n) + ((i < h.num_digits) ? h.digits[i] : 0);
  }
  bool round_up = false;
  if (dp < h.num_digits) {
    round_up = h.digits[dp] >= 5; // normally, we round up
    // but we may need to round to even!
    if ((h.digits[dp] == 5) && (dp + 1 == h.num_digits)) {
      round_up = h.truncated || ((dp > 0) && (1 & h.digits[dp - 1]));
    }
  }
  if (round_up) {
    n++;
  }
  return n;
}

// computes h * 2^-shift
void decimal_left_shift(decimal &h, uint32_t shift) {
  if (h.num_digits == 0) {
    return;
  }
  uint32_t num_new_digits = number_of_digits_decimal_left_shift(h, shift);
  int32_t read_index = int32_t(h.num_digits - 1);
  uint32_t write_index = h.num_digits - 1 + num_new_digits;
  uint64_t n = 0;

  while (read_index >= 0) {
    n += uint64_t(h.digits[read_index]) << shift;
    uint64_t quotient = n / 10;
    uint64_t remainder = n - (10 * quotient);
    if (write_index < max_digits) {
      h.digits[write_index] = uint8_t(remainder);
    } else if (remainder > 0) {
      h.truncated = true;
    }
    n = quotient;
    write_index--;
    read_index--;
  }
  while (n > 0) {
    uint64_t quotient = n / 10;
    uint64_t remainder = n - (10 * quotient);
    if (write_index < max_digits) {
      h.digits[write_index] = uint8_t(remainder);
    } else if (remainder > 0) {
      h.truncated = true;
    }
    n = quotient;
    write_index--;
  }
  h.num_digits += num_new_digits;
  if (h.num_digits > max_digits) {
    h.num_digits = max_digits;
  }
  h.decimal_point += int32_t(num_new_digits);
  trim(h);
}

// computes h * 2^shift
void decimal_right_shift(decimal &h, uint32_t shift) {
  uint32_t read_index = 0;
  uint32_t write_index = 0;

  uint64_t n = 0;

  while ((n >> shift) == 0) {
    if (read_index < h.num_digits) {
      n = (10 * n) + h.digits[read_index++];
    } else if (n == 0) {
      return;
    } else {
      while ((n >> shift) == 0) {
        n = 10 * n;
        read_index++;
      }
      break;
    }
  }
  h.decimal_point -= int32_t(read_index - 1);
  if (h.decimal_point < -decimal_point_range) { // it is zero
    h.num_digits = 0;
    h.decimal_point = 0;
    h.negative = false;
    h.truncated = false;
    return;
  }
  uint64_t mask = (uint64_t(1) << shift) - 1;
  while (read_index < h.num_digits) {
    uint8_t new_digit = uint8_t(n >> shift);
    n = (10 * (n & mask)) + h.digits[read_index++];
    h.digits[write_index++] = new_digit;
  }
  while (n > 0) {
    uint8_t new_digit = uint8_t(n >> shift);
    n = 10 * (n & mask);
    if (write_index < max_digits) {
      h.digits[write_index++] = new_digit;
    } else if (new_digit > 0) {
      h.truncated = true;
    }
  }
  h.num_digits = write_index;
  trim(h);
}

template <typename binary> adjusted_mantissa compute_float(decimal &d) {
  adjusted_mantissa answer;
  if (d.num_digits == 0) {
    // should be zero
    answer.power2 = 0;
    answer.mantissa = 0;
    return answer;
  }
  // At this point, going further, we can assume that d.num_digits > 0.
  // We want to guard against excessive decimal point values because
  // they can result in long running times. Indeed, we do
  // shifts by at most 60 bits. We have that log(10**400)/log(2**60) ~= 22
  // which is fine, but log(10**299995)/log(2**60) ~= 16609 which is not
  // fine (runs for a long time).
  //
  if(d.decimal_point < -324) {
    // We have something smaller than 1e-324 which is always zero
    // in binary64 and binary32.
    // It should be zero.
    answer.power2 = 0;
    answer.mantissa = 0;
    return answer;
  } else if(d.decimal_point >= 310) {
    // We have something at least as large as 0.1e310 which is
    // always infinite.
    answer.power2 = binary::infinite_power();
    answer.mantissa = 0;
    return answer;
  }

  static const uint32_t max_shift = 60;
  static const uint32_t num_powers = 19;
  static const uint8_t powers[19] = {
      0,  3,  6,  9,  13, 16, 19, 23, 26, 29, //
      33, 36, 39, 43, 46, 49, 53, 56, 59,     //
  };
  int32_t exp2 = 0;
  while (d.decimal_point > 0) {
    uint32_t n = uint32_t(d.decimal_point);
    uint32_t shift = (n < num_powers) ? powers[n] : max_shift;
    decimal_right_shift(d, shift);
    if (d.decimal_point < -decimal_point_range) {
      // should be zero
      answer.power2 = 0;
      answer.mantissa = 0;
      return answer;
    }
    exp2 += int32_t(shift);
  }
  // We shift left toward [1/2 ... 1].
  while (d.decimal_point <= 0) {
    uint32_t shift;
    if (d.decimal_point == 0) {
      if (d.digits[0] >= 5) {
        break;
      }
      shift = (d.digits[0] < 2) ? 2 : 1;
    } else {
      uint32_t n = uint32_t(-d.decimal_point);
      shift = (n < num_powers) ? powers[n] : max_shift;
    }
    decimal_left_shift(d, shift);
    if (d.decimal_point > decimal_point_range) {
      // we want to get infinity:
      answer.power2 = 0xFF;
      answer.mantissa = 0;
      return answer;
    }
    exp2 -= int32_t(shift);
  }
  // We are now in the range [1/2 ... 1] but the binary format uses [1 ... 2].
  exp2--;
  constexpr int32_t minimum_exponent = binary::minimum_exponent();
  while ((minimum_exponent + 1) > exp2) {
    uint32_t n = uint32_t((minimum_exponent + 1) - exp2);
    if (n > max_shift) {
      n = max_shift;
    }
    decimal_right_shift(d, n);
    exp2 += int32_t(n);
  }
  if ((exp2 - minimum_exponent) >= binary::infinite_power()) {
    answer.power2 = binary::infinite_power();
    answer.mantissa = 0;
    return answer;
  }

  const int mantissa_size_in_bits = binary::mantissa_explicit_bits() + 1;
  decimal_left_shift(d, mantissa_size_in_bits);

  uint64_t mantissa = round(d);
  // It is possible that we have an overflow, in which case we need
  // to shift back.
  if (mantissa >= (uint64_t(1) << mantissa_size_in_bits)) {
    decimal_right_shift(d, 1);
    exp2 += 1;
    mantissa = round(d);
    if ((exp2 - minimum_exponent) >= binary::infinite_power()) {
      answer.power2 = binary::infinite_power();
      answer.mantissa = 0;
      return answer;
    }
  }
  answer.power2 = exp2 - binary::minimum_exponent();
  if (mantissa < (uint64_t(1) << binary::mantissa_explicit_bits())) {
    answer.power2--;
  }
  answer.mantissa =
      mantissa & ((uint64_t(1) << binary::mantissa_explicit_bits()) - 1);
  return answer;
}

template <typename binary>
adjusted_mantissa parse_long_mantissa(const char *first) {
  decimal d = parse_decimal(first);
  return compute_float<binary>(d);
}

template <typename binary>
adjusted_mantissa parse_long_mantissa(const char *first, const char *end) {
  decimal d = parse_decimal(first, end);
  return compute_float<binary>(d);
}

double from_chars(const char *first) noexcept {
  bool negative = first[0] == '-';
  if (negative) {
    first++;
  }
  adjusted_mantissa am = parse_long_mantissa<binary_format<double>>(first);
  uint64_t word = am.mantissa;
  word |= uint64_t(am.power2)
          << binary_format<double>::mantissa_explicit_bits();
  word = negative ? word | (uint64_t(1) << binary_format<double>::sign_index())
                  : word;
  double value;
  std::memcpy(&value, &word, sizeof(double));
  return value;
}


double from_chars(const char *first, const char *end) noexcept {
  bool negative = first[0] == '-';
  if (negative) {
    first++;
  }
  adjusted_mantissa am = parse_long_mantissa<binary_format<double>>(first, end);
  uint64_t word = am.mantissa;
  word |= uint64_t(am.power2)
          << binary_format<double>::mantissa_explicit_bits();
  word = negative ? word | (uint64_t(1) << binary_format<double>::sign_index())
                  : word;
  double value;
  std::memcpy(&value, &word, sizeof(double));
  return value;
}

} // internal
} // simdjson