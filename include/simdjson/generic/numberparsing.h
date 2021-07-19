#include "simdjson/internal/numberparsing_tables.h"
#include <limits>

namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace {
/// @private
namespace numberparsing {



#ifdef JSON_TEST_NUMBERS
#define INVALID_NUMBER(SRC) (found_invalid_number((SRC)), NUMBER_ERROR)
#define WRITE_INTEGER(VALUE, SRC, WRITER) (found_integer((VALUE), (SRC)), (WRITER).append_s64((VALUE)))
#define WRITE_UNSIGNED(VALUE, SRC, WRITER) (found_unsigned_integer((VALUE), (SRC)), (WRITER).append_u64((VALUE)))
#define WRITE_DOUBLE(VALUE, SRC, WRITER) (found_float((VALUE), (SRC)), (WRITER).append_double((VALUE)))
#else
#define INVALID_NUMBER(SRC) (NUMBER_ERROR)
#define WRITE_INTEGER(VALUE, SRC, WRITER) (WRITER).append_s64((VALUE))
#define WRITE_UNSIGNED(VALUE, SRC, WRITER) (WRITER).append_u64((VALUE))
#define WRITE_DOUBLE(VALUE, SRC, WRITER) (WRITER).append_double((VALUE))
#endif

namespace {
// Convert a mantissa, an exponent and a sign bit into an ieee64 double.
// The real_exponent needs to be in [0, 2046] (technically real_exponent = 2047 would be acceptable).
// The mantissa should be in [0,1<<53). The bit at index (1ULL << 52) while be zeroed.
simdjson_really_inline double to_double(uint64_t mantissa, uint64_t real_exponent, bool negative) {
    double d;
    mantissa &= ~(1ULL << 52);
    mantissa |= real_exponent << 52;
    mantissa |= ((static_cast<uint64_t>(negative)) << 63);
    std::memcpy(&d, &mantissa, sizeof(d));
    return d;
}
}
// Attempts to compute i * 10^(power) exactly; and if "negative" is
// true, negate the result.
// This function will only work in some cases, when it does not work, success is
// set to false. This should work *most of the time* (like 99% of the time).
// We assume that power is in the [smallest_power,
// largest_power] interval: the caller is responsible for this check.
simdjson_really_inline bool compute_float_64(int64_t power, uint64_t i, bool negative, double &d) {
  // we start with a fast path
  // It was described in
  // Clinger WD. How to read floating point numbers accurately.
  // ACM SIGPLAN Notices. 1990
#ifndef FLT_EVAL_METHOD
#error "FLT_EVAL_METHOD should be defined, please include cfloat."
#endif
#if (FLT_EVAL_METHOD != 1) && (FLT_EVAL_METHOD != 0)
  // We cannot be certain that x/y is rounded to nearest.
  if (0 <= power && power <= 22 && i <= 9007199254740991) {
#else
  if (-22 <= power && power <= 22 && i <= 9007199254740991) {
#endif
    // convert the integer into a double. This is lossless since
    // 0 <= i <= 2^53 - 1.
    d = double(i);
    //
    // The general idea is as follows.
    // If 0 <= s < 2^53 and if 10^0 <= p <= 10^22 then
    // 1) Both s and p can be represented exactly as 64-bit floating-point
    // values
    // (binary64).
    // 2) Because s and p can be represented exactly as floating-point values,
    // then s * p
    // and s / p will produce correctly rounded values.
    //
    if (power < 0) {
      d = d / simdjson::internal::power_of_ten[-power];
    } else {
      d = d * simdjson::internal::power_of_ten[power];
    }
    if (negative) {
      d = -d;
    }
    return true;
  }
  // When 22 < power && power <  22 + 16, we could
  // hope for another, secondary fast path.  It was
  // described by David M. Gay in  "Correctly rounded
  // binary-decimal and decimal-binary conversions." (1990)
  // If you need to compute i * 10^(22 + x) for x < 16,
  // first compute i * 10^x, if you know that result is exact
  // (e.g., when i * 10^x < 2^53),
  // then you can still proceed and do (i * 10^x) * 10^22.
  // Is this worth your time?
  // You need  22 < power *and* power <  22 + 16 *and* (i * 10^(x-22) < 2^53)
  // for this second fast path to work.
  // If you you have 22 < power *and* power <  22 + 16, and then you
  // optimistically compute "i * 10^(x-22)", there is still a chance that you
  // have wasted your time if i * 10^(x-22) >= 2^53. It makes the use cases of
  // this optimization maybe less common than we would like. Source:
  // http://www.exploringbinary.com/fast-path-decimal-to-floating-point-conversion/
  // also used in RapidJSON: https://rapidjson.org/strtod_8h_source.html

  // The fast path has now failed, so we are failing back on the slower path.

  // In the slow path, we need to adjust i so that it is > 1<<63 which is always
  // possible, except if i == 0, so we handle i == 0 separately.
  if(i == 0) {
    d = 0.0;
    return true;
  }


  // The exponent is 1024 + 63 + power
  //     + floor(log(5**power)/log(2)).
  // The 1024 comes from the ieee64 standard.
  // The 63 comes from the fact that we use a 64-bit word.
  //
  // Computing floor(log(5**power)/log(2)) could be
  // slow. Instead we use a fast function.
  //
  // For power in (-400,350), we have that
  // (((152170 + 65536) * power ) >> 16);
  // is equal to
  //  floor(log(5**power)/log(2)) + power when power >= 0
  // and it is equal to
  //  ceil(log(5**-power)/log(2)) + power when power < 0
  //
  // The 65536 is (1<<16) and corresponds to
  // (65536 * power) >> 16 ---> power
  //
  // ((152170 * power ) >> 16) is equal to
  // floor(log(5**power)/log(2))
  //
  // Note that this is not magic: 152170/(1<<16) is
  // approximatively equal to log(5)/log(2).
  // The 1<<16 value is a power of two; we could use a
  // larger power of 2 if we wanted to.
  //
  int64_t exponent = (((152170 + 65536) * power) >> 16) + 1024 + 63;


  // We want the most significant bit of i to be 1. Shift if needed.
  int lz = leading_zeroes(i);
  i <<= lz;


  // We are going to need to do some 64-bit arithmetic to get a precise product.
  // We use a table lookup approach.
  // It is safe because
  // power >= smallest_power
  // and power <= largest_power
  // We recover the mantissa of the power, it has a leading 1. It is always
  // rounded down.
  //
  // We want the most significant 64 bits of the product. We know
  // this will be non-zero because the most significant bit of i is
  // 1.
  const uint32_t index = 2 * uint32_t(power - simdjson::internal::smallest_power);
  // Optimization: It may be that materializing the index as a variable might confuse some compilers and prevent effective complex-addressing loads. (Done for code clarity.)
  //
  // The full_multiplication function computes the 128-bit product of two 64-bit words
  // with a returned value of type value128 with a "low component" corresponding to the
  // 64-bit least significant bits of the product and with a "high component" corresponding
  // to the 64-bit most significant bits of the product.
  simdjson::internal::value128 firstproduct = jsoncharutils::full_multiplication(i, simdjson::internal::power_of_five_128[index]);
  // Both i and power_of_five_128[index] have their most significant bit set to 1 which
  // implies that the either the most or the second most significant bit of the product
  // is 1. We pack values in this manner for efficiency reasons: it maximizes the use
  // we make of the product. It also makes it easy to reason about the product: there
  // is 0 or 1 leading zero in the product.

  // Unless the least significant 9 bits of the high (64-bit) part of the full
  // product are all 1s, then we know that the most significant 55 bits are
  // exact and no further work is needed. Having 55 bits is necessary because
  // we need 53 bits for the mantissa but we have to have one rounding bit and
  // we can waste a bit if the most significant bit of the product is zero.
  if((firstproduct.high & 0x1FF) == 0x1FF) {
    // We want to compute i * 5^q, but only care about the top 55 bits at most.
    // Consider the scenario where q>=0. Then 5^q may not fit in 64-bits. Doing
    // the full computation is wasteful. So we do what is called a "truncated
    // multiplication".
    // We take the most significant 64-bits, and we put them in
    // power_of_five_128[index]. Usually, that's good enough to approximate i * 5^q
    // to the desired approximation using one multiplication. Sometimes it does not suffice.
    // Then we store the next most significant 64 bits in power_of_five_128[index + 1], and
    // then we get a better approximation to i * 5^q. In very rare cases, even that
    // will not suffice, though it is seemingly very hard to find such a scenario.
    //
    // That's for when q>=0. The logic for q<0 is somewhat similar but it is somewhat
    // more complicated.
    //
    // There is an extra layer of complexity in that we need more than 55 bits of
    // accuracy in the round-to-even scenario.
    //
    // The full_multiplication function computes the 128-bit product of two 64-bit words
    // with a returned value of type value128 with a "low component" corresponding to the
    // 64-bit least significant bits of the product and with a "high component" corresponding
    // to the 64-bit most significant bits of the product.
    simdjson::internal::value128 secondproduct = jsoncharutils::full_multiplication(i, simdjson::internal::power_of_five_128[index + 1]);
    firstproduct.low += secondproduct.high;
    if(secondproduct.high > firstproduct.low) { firstproduct.high++; }
    // At this point, we might need to add at most one to firstproduct, but this
    // can only change the value of firstproduct.high if firstproduct.low is maximal.
    if(simdjson_unlikely(firstproduct.low  == 0xFFFFFFFFFFFFFFFF)) {
      // This is very unlikely, but if so, we need to do much more work!
      return false;
    }
  }
  uint64_t lower = firstproduct.low;
  uint64_t upper = firstproduct.high;
  // The final mantissa should be 53 bits with a leading 1.
  // We shift it so that it occupies 54 bits with a leading 1.
  ///////
  uint64_t upperbit = upper >> 63;
  uint64_t mantissa = upper >> (upperbit + 9);
  lz += int(1 ^ upperbit);

  // Here we have mantissa < (1<<54).
  int64_t real_exponent = exponent - lz;
  if (simdjson_unlikely(real_exponent <= 0)) { // we have a subnormal?
    // Here have that real_exponent <= 0 so -real_exponent >= 0
    if(-real_exponent + 1 >= 64) { // if we have more than 64 bits below the minimum exponent, you have a zero for sure.
      d = 0.0;
      return true;
    }
    // next line is safe because -real_exponent + 1 < 0
    mantissa >>= -real_exponent + 1;
    // Thankfully, we can't have both "round-to-even" and subnormals because
    // "round-to-even" only occurs for powers close to 0.
    mantissa += (mantissa & 1); // round up
    mantissa >>= 1;
    // There is a weird scenario where we don't have a subnormal but just.
    // Suppose we start with 2.2250738585072013e-308, we end up
    // with 0x3fffffffffffff x 2^-1023-53 which is technically subnormal
    // whereas 0x40000000000000 x 2^-1023-53  is normal. Now, we need to round
    // up 0x3fffffffffffff x 2^-1023-53  and once we do, we are no longer
    // subnormal, but we can only know this after rounding.
    // So we only declare a subnormal if we are smaller than the threshold.
    real_exponent = (mantissa < (uint64_t(1) << 52)) ? 0 : 1;
    d = to_double(mantissa, real_exponent, negative);
    return true;
  }
  // We have to round to even. The "to even" part
  // is only a problem when we are right in between two floats
  // which we guard against.
  // If we have lots of trailing zeros, we may fall right between two
  // floating-point values.
  //
  // The round-to-even cases take the form of a number 2m+1 which is in (2^53,2^54]
  // times a power of two. That is, it is right between a number with binary significand
  // m and another number with binary significand m+1; and it must be the case
  // that it cannot be represented by a float itself.
  //
  // We must have that w * 10 ^q == (2m+1) * 2^p for some power of two 2^p.
  // Recall that 10^q = 5^q * 2^q.
  // When q >= 0, we must have that (2m+1) is divible by 5^q, so 5^q <= 2^54. We have that
  //  5^23 <=  2^54 and it is the last power of five to qualify, so q <= 23.
  // When q<0, we have  w  >=  (2m+1) x 5^{-q}.  We must have that w<2^{64} so
  // (2m+1) x 5^{-q} < 2^{64}. We have that 2m+1>2^{53}. Hence, we must have
  // 2^{53} x 5^{-q} < 2^{64}.
  // Hence we have 5^{-q} < 2^{11}$ or q>= -4.
  //
  // We require lower <= 1 and not lower == 0 because we could not prove that
  // that lower == 0 is implied; but we could prove that lower <= 1 is a necessary and sufficient test.
  if (simdjson_unlikely((lower <= 1) && (power >= -4) && (power <= 23) && ((mantissa & 3) == 1))) {
    if((mantissa  << (upperbit + 64 - 53 - 2)) ==  upper) {
      mantissa &= ~1;             // flip it so that we do not round up
    }
  }

  mantissa += mantissa & 1;
  mantissa >>= 1;

  // Here we have mantissa < (1<<53), unless there was an overflow
  if (mantissa >= (1ULL << 53)) {
    //////////
    // This will happen when parsing values such as 7.2057594037927933e+16
    ////////
    mantissa = (1ULL << 52);
    real_exponent++;
  }
  mantissa &= ~(1ULL << 52);
  // we have to check that real_exponent is in range, otherwise we bail out
  if (simdjson_unlikely(real_exponent > 2046)) {
    // We have an infinite value!!! We could actually throw an error here if we could.
    return false;
  }
  d = to_double(mantissa, real_exponent, negative);
  return true;
}

// We call a fallback floating-point parser that might be slow. Note
// it will accept JSON numbers, but the JSON spec. is more restrictive so
// before you call parse_float_fallback, you need to have validated the input
// string with the JSON grammar.
// It will return an error (false) if the parsed number is infinite.
// The string parsing itself always succeeds. We know that there is at least
// one digit.
static bool parse_float_fallback(const uint8_t *ptr, double *outDouble) {
  *outDouble = simdjson::internal::from_chars(reinterpret_cast<const char *>(ptr));
  // We do not accept infinite values.

  // Detecting finite values in a portable manner is ridiculously hard, ideally
  // we would want to do:
  // return !std::isfinite(*outDouble);
  // but that mysteriously fails under legacy/old libc++ libraries, see
  // https://github.com/simdjson/simdjson/issues/1286
  //
  // Therefore, fall back to this solution (the extra parens are there
  // to handle that max may be a macro on windows).
  return !(*outDouble > (std::numeric_limits<double>::max)() || *outDouble < std::numeric_limits<double>::lowest());
}
static bool parse_float_fallback(const uint8_t *ptr, const uint8_t *end_ptr, double *outDouble) {
  *outDouble = simdjson::internal::from_chars(reinterpret_cast<const char *>(ptr), reinterpret_cast<const char *>(end_ptr));
  // We do not accept infinite values.

  // Detecting finite values in a portable manner is ridiculously hard, ideally
  // we would want to do:
  // return !std::isfinite(*outDouble);
  // but that mysteriously fails under legacy/old libc++ libraries, see
  // https://github.com/simdjson/simdjson/issues/1286
  //
  // Therefore, fall back to this solution (the extra parens are there
  // to handle that max may be a macro on windows).
  return !(*outDouble > (std::numeric_limits<double>::max)() || *outDouble < std::numeric_limits<double>::lowest());
}

// check quickly whether the next 8 chars are made of digits
// at a glance, it looks better than Mula's
// http://0x80.pl/articles/swar-digits-validate.html
simdjson_really_inline bool is_made_of_eight_digits_fast(const uint8_t *chars) {
  uint64_t val;
  // this can read up to 7 bytes beyond the buffer size, but we require
  // SIMDJSON_PADDING of padding
  static_assert(7 <= SIMDJSON_PADDING, "SIMDJSON_PADDING must be bigger than 7");
  std::memcpy(&val, chars, 8);
  // a branchy method might be faster:
  // return (( val & 0xF0F0F0F0F0F0F0F0 ) == 0x3030303030303030)
  //  && (( (val + 0x0606060606060606) & 0xF0F0F0F0F0F0F0F0 ) ==
  //  0x3030303030303030);
  return (((val & 0xF0F0F0F0F0F0F0F0) |
           (((val + 0x0606060606060606) & 0xF0F0F0F0F0F0F0F0) >> 4)) ==
          0x3333333333333333);
}

template<typename W>
error_code slow_float_parsing(simdjson_unused const uint8_t * src, W writer) {
  double d;
  if (parse_float_fallback(src, &d)) {
    writer.append_double(d);
    return SUCCESS;
  }
  return INVALID_NUMBER(src);
}

template<typename I>
NO_SANITIZE_UNDEFINED // We deliberately allow overflow here and check later
simdjson_really_inline bool parse_digit(const uint8_t c, I &i) {
  const uint8_t digit = static_cast<uint8_t>(c - '0');
  if (digit > 9) {
    return false;
  }
  // PERF NOTE: multiplication by 10 is cheaper than arbitrary integer multiplication
  i = 10 * i + digit; // might overflow, we will handle the overflow later
  return true;
}

simdjson_really_inline error_code parse_decimal(simdjson_unused const uint8_t *const src, const uint8_t *&p, uint64_t &i, int64_t &exponent) {
  // we continue with the fiction that we have an integer. If the
  // floating point number is representable as x * 10^z for some integer
  // z that fits in 53 bits, then we will be able to convert back the
  // the integer into a float in a lossless manner.
  const uint8_t *const first_after_period = p;

#ifdef SWAR_NUMBER_PARSING
  // this helps if we have lots of decimals!
  // this turns out to be frequent enough.
  if (is_made_of_eight_digits_fast(p)) {
    i = i * 100000000 + parse_eight_digits_unrolled(p);
    p += 8;
  }
#endif
  // Unrolling the first digit makes a small difference on some implementations (e.g. westmere)
  if (parse_digit(*p, i)) { ++p; }
  while (parse_digit(*p, i)) { p++; }
  exponent = first_after_period - p;
  // Decimal without digits (123.) is illegal
  if (exponent == 0) {
    return INVALID_NUMBER(src);
  }
  return SUCCESS;
}

simdjson_really_inline error_code parse_exponent(simdjson_unused const uint8_t *const src, const uint8_t *&p, int64_t &exponent) {
  // Exp Sign: -123.456e[-]78
  bool neg_exp = ('-' == *p);
  if (neg_exp || '+' == *p) { p++; } // Skip + as well

  // Exponent: -123.456e-[78]
  auto start_exp = p;
  int64_t exp_number = 0;
  while (parse_digit(*p, exp_number)) { ++p; }
  // It is possible for parse_digit to overflow.
  // In particular, it could overflow to INT64_MIN, and we cannot do - INT64_MIN.
  // Thus we *must* check for possible overflow before we negate exp_number.

  // Performance notes: it may seem like combining the two "simdjson_unlikely checks" below into
  // a single simdjson_unlikely path would be faster. The reasoning is sound, but the compiler may
  // not oblige and may, in fact, generate two distinct paths in any case. It might be
  // possible to do uint64_t(p - start_exp - 1) >= 18 but it could end up trading off
  // instructions for a simdjson_likely branch, an unconclusive gain.

  // If there were no digits, it's an error.
  if (simdjson_unlikely(p == start_exp)) {
    return INVALID_NUMBER(src);
  }
  // We have a valid positive exponent in exp_number at this point, except that
  // it may have overflowed.

  // If there were more than 18 digits, we may have overflowed the integer. We have to do
  // something!!!!
  if (simdjson_unlikely(p > start_exp+18)) {
    // Skip leading zeroes: 1e000000000000000000001 is technically valid and doesn't overflow
    while (*start_exp == '0') { start_exp++; }
    // 19 digits could overflow int64_t and is kind of absurd anyway. We don't
    // support exponents smaller than -999,999,999,999,999,999 and bigger
    // than 999,999,999,999,999,999.
    // We can truncate.
    // Note that 999999999999999999 is assuredly too large. The maximal ieee64 value before
    // infinity is ~1.8e308. The smallest subnormal is ~5e-324. So, actually, we could
    // truncate at 324.
    // Note that there is no reason to fail per se at this point in time.
    // E.g., 0e999999999999999999999 is a fine number.
    if (p > start_exp+18) { exp_number = 999999999999999999; }
  }
  // At this point, we know that exp_number is a sane, positive, signed integer.
  // It is <= 999,999,999,999,999,999. As long as 'exponent' is in
  // [-8223372036854775808, 8223372036854775808], we won't overflow. Because 'exponent'
  // is bounded in magnitude by the size of the JSON input, we are fine in this universe.
  // To sum it up: the next line should never overflow.
  exponent += (neg_exp ? -exp_number : exp_number);
  return SUCCESS;
}

simdjson_really_inline size_t significant_digits(const uint8_t * start_digits, size_t digit_count) {
  // It is possible that the integer had an overflow.
  // We have to handle the case where we have 0.0000somenumber.
  const uint8_t *start = start_digits;
  while ((*start == '0') || (*start == '.')) {
    start++;
  }
  // we over-decrement by one when there is a '.'
  return digit_count - size_t(start - start_digits);
}

template<typename W>
simdjson_really_inline error_code write_float(const uint8_t *const src, bool negative, uint64_t i, const uint8_t * start_digits, size_t digit_count, int64_t exponent, W &writer) {
  // If we frequently had to deal with long strings of digits,
  // we could extend our code by using a 128-bit integer instead
  // of a 64-bit integer. However, this is uncommon in practice.
  //
  // 9999999999999999999 < 2**64 so we can accommodate 19 digits.
  // If we have a decimal separator, then digit_count - 1 is the number of digits, but we
  // may not have a decimal separator!
  if (simdjson_unlikely(digit_count > 19 && significant_digits(start_digits, digit_count) > 19)) {
    // Ok, chances are good that we had an overflow!
    // this is almost never going to get called!!!
    // we start anew, going slowly!!!
    // This will happen in the following examples:
    // 10000000000000000000000000000000000000000000e+308
    // 3.1415926535897932384626433832795028841971693993751
    //
    // NOTE: This makes a *copy* of the writer and passes it to slow_float_parsing. This happens
    // because slow_float_parsing is a non-inlined function. If we passed our writer reference to
    // it, it would force it to be stored in memory, preventing the compiler from picking it apart
    // and putting into registers. i.e. if we pass it as reference, it gets slow.
    // This is what forces the skip_double, as well.
    error_code error = slow_float_parsing(src, writer);
    writer.skip_double();
    return error;
  }
  // NOTE: it's weird that the simdjson_unlikely() only wraps half the if, but it seems to get slower any other
  // way we've tried: https://github.com/simdjson/simdjson/pull/990#discussion_r448497331
  // To future reader: we'd love if someone found a better way, or at least could explain this result!
  if (simdjson_unlikely(exponent < simdjson::internal::smallest_power) || (exponent > simdjson::internal::largest_power)) {
    //
    // Important: smallest_power is such that it leads to a zero value.
    // Observe that 18446744073709551615e-343 == 0, i.e. (2**64 - 1) e -343 is zero
    // so something x 10^-343 goes to zero, but not so with  something x 10^-342.
    static_assert(simdjson::internal::smallest_power <= -342, "smallest_power is not small enough");
    //
    if((exponent < simdjson::internal::smallest_power) || (i == 0)) {
      WRITE_DOUBLE(0, src, writer);
      return SUCCESS;
    } else { // (exponent > largest_power) and (i != 0)
      // We have, for sure, an infinite value and simdjson refuses to parse infinite values.
      return INVALID_NUMBER(src);
    }
  }
  double d;
  if (!compute_float_64(exponent, i, negative, d)) {
    // we are almost never going to get here.
    if (!parse_float_fallback(src, &d)) { return INVALID_NUMBER(src); }
  }
  WRITE_DOUBLE(d, src, writer);
  return SUCCESS;
}

// for performance analysis, it is sometimes  useful to skip parsing
#ifdef SIMDJSON_SKIPNUMBERPARSING

template<typename W>
simdjson_really_inline error_code parse_number(const uint8_t *const, W &writer) {
  writer.append_s64(0);        // always write zero
  return SUCCESS;              // always succeeds
}

simdjson_unused simdjson_really_inline simdjson_result<uint64_t> parse_unsigned(const uint8_t * const src) noexcept { return 0; }
simdjson_unused simdjson_really_inline simdjson_result<int64_t> parse_integer(const uint8_t * const src) noexcept { return 0; }
simdjson_unused simdjson_really_inline simdjson_result<double> parse_double(const uint8_t * const src) noexcept { return 0; }

#else

// parse the number at src
// define JSON_TEST_NUMBERS for unit testing
//
// It is assumed that the number is followed by a structural ({,},],[) character
// or a white space character. If that is not the case (e.g., when the JSON
// document is made of a single number), then it is necessary to copy the
// content and append a space before calling this function.
//
// Our objective is accurate parsing (ULP of 0) at high speed.
template<typename W>
simdjson_really_inline error_code parse_number(const uint8_t *const src, W &writer) {

  //
  // Check for minus sign
  //
  bool negative = (*src == '-');
  const uint8_t *p = src + negative;

  //
  // Parse the integer part.
  //
  // PERF NOTE: we don't use is_made_of_eight_digits_fast because large integers like 123456789 are rare
  const uint8_t *const start_digits = p;
  uint64_t i = 0;
  while (parse_digit(*p, i)) { p++; }

  // If there were no digits, or if the integer starts with 0 and has more than one digit, it's an error.
  // Optimization note: size_t is expected to be unsigned.
  size_t digit_count = size_t(p - start_digits);
  if (digit_count == 0 || ('0' == *start_digits && digit_count > 1)) { return INVALID_NUMBER(src); }

  //
  // Handle floats if there is a . or e (or both)
  //
  int64_t exponent = 0;
  bool is_float = false;
  if ('.' == *p) {
    is_float = true;
    ++p;
    SIMDJSON_TRY( parse_decimal(src, p, i, exponent) );
    digit_count = int(p - start_digits); // used later to guard against overflows
  }
  if (('e' == *p) || ('E' == *p)) {
    is_float = true;
    ++p;
    SIMDJSON_TRY( parse_exponent(src, p, exponent) );
  }
  if (is_float) {
    const bool dirty_end = jsoncharutils::is_not_structural_or_whitespace(*p);
    SIMDJSON_TRY( write_float(src, negative, i, start_digits, digit_count, exponent, writer) );
    if (dirty_end) { return INVALID_NUMBER(src); }
    return SUCCESS;
  }

  // The longest negative 64-bit number is 19 digits.
  // The longest positive 64-bit number is 20 digits.
  // We do it this way so we don't trigger this branch unless we must.
  size_t longest_digit_count = negative ? 19 : 20;
  if (digit_count > longest_digit_count) { return INVALID_NUMBER(src); }
  if (digit_count == longest_digit_count) {
    if (negative) {
      // Anything negative above INT64_MAX+1 is invalid
      if (i > uint64_t(INT64_MAX)+1) { return INVALID_NUMBER(src);  }
      WRITE_INTEGER(~i+1, src, writer);
      if (jsoncharutils::is_not_structural_or_whitespace(*p)) { return INVALID_NUMBER(src); }
      return SUCCESS;
    // Positive overflow check:
    // - A 20 digit number starting with 2-9 is overflow, because 18,446,744,073,709,551,615 is the
    //   biggest uint64_t.
    // - A 20 digit number starting with 1 is overflow if it is less than INT64_MAX.
    //   If we got here, it's a 20 digit number starting with the digit "1".
    // - If a 20 digit number starting with 1 overflowed (i*10+digit), the result will be smaller
    //   than 1,553,255,926,290,448,384.
    // - That is smaller than the smallest possible 20-digit number the user could write:
    //   10,000,000,000,000,000,000.
    // - Therefore, if the number is positive and lower than that, it's overflow.
    // - The value we are looking at is less than or equal to 9,223,372,036,854,775,808 (INT64_MAX).
    //
    }  else if (src[0] != uint8_t('1') || i <= uint64_t(INT64_MAX)) { return INVALID_NUMBER(src); }
  }

  // Write unsigned if it doesn't fit in a signed integer.
  if (i > uint64_t(INT64_MAX)) {
    WRITE_UNSIGNED(i, src, writer);
  } else {
    WRITE_INTEGER(negative ? (~i+1) : i, src, writer);
  }
  if (jsoncharutils::is_not_structural_or_whitespace(*p)) { return INVALID_NUMBER(src); }
  return SUCCESS;
}

// Inlineable functions
namespace {

// This table can be used to characterize the final character of an integer
// string. For JSON structural character and allowable white space characters,
// we return SUCCESS. For 'e', '.' and 'E', we return INCORRECT_TYPE. Otherwise
// we return NUMBER_ERROR.
// Optimization note: we could easily reduce the size of the table by half (to 128)
// at the cost of an extra branch.
// Optimization note: we want the values to use at most 8 bits (not, e.g., 32 bits):
static_assert(error_code(uint8_t(NUMBER_ERROR))== NUMBER_ERROR, "bad NUMBER_ERROR cast");
static_assert(error_code(uint8_t(SUCCESS))== SUCCESS, "bad NUMBER_ERROR cast");
static_assert(error_code(uint8_t(INCORRECT_TYPE))== INCORRECT_TYPE, "bad NUMBER_ERROR cast");

const uint8_t integer_string_finisher[256] = {
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, SUCCESS,
    SUCCESS,      NUMBER_ERROR,   NUMBER_ERROR, SUCCESS,      NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   SUCCESS,      NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, SUCCESS,
    NUMBER_ERROR, INCORRECT_TYPE, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, SUCCESS,      NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, INCORRECT_TYPE,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, SUCCESS,        NUMBER_ERROR, SUCCESS,      NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, INCORRECT_TYPE, NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, SUCCESS,      NUMBER_ERROR,
    SUCCESS,      NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR, NUMBER_ERROR,   NUMBER_ERROR, NUMBER_ERROR, NUMBER_ERROR,
    NUMBER_ERROR};

// Parse any number from 0 to 18,446,744,073,709,551,615
simdjson_unused simdjson_really_inline simdjson_result<uint64_t> parse_unsigned(const uint8_t * const src) noexcept {
  const uint8_t *p = src;
  //
  // Parse the integer part.
  //
  // PERF NOTE: we don't use is_made_of_eight_digits_fast because large integers like 123456789 are rare
  const uint8_t *const start_digits = p;
  uint64_t i = 0;
  while (parse_digit(*p, i)) { p++; }

  // If there were no digits, or if the integer starts with 0 and has more than one digit, it's an error.
  // Optimization note: size_t is expected to be unsigned.
  size_t digit_count = size_t(p - start_digits);
  // The longest positive 64-bit number is 20 digits.
  // We do it this way so we don't trigger this branch unless we must.
  // Optimization note: the compiler can probably merge
  // ((digit_count == 0) || (digit_count > 20))
  // into a single  branch since digit_count is unsigned.
  if ((digit_count == 0) || (digit_count > 20)) { return INCORRECT_TYPE; }
  // Here digit_count > 0.
  if (('0' == *start_digits) && (digit_count > 1)) { return NUMBER_ERROR; }
  // We can do the following...
  // if (!jsoncharutils::is_structural_or_whitespace(*p)) {
  //  return (*p == '.' || *p == 'e' || *p == 'E') ? INCORRECT_TYPE : NUMBER_ERROR;
  // }
  // as a single table lookup:
  if (integer_string_finisher[*p] != SUCCESS) { return error_code(integer_string_finisher[*p]); }

  if (digit_count == 20) {
    // Positive overflow check:
    // - A 20 digit number starting with 2-9 is overflow, because 18,446,744,073,709,551,615 is the
    //   biggest uint64_t.
    // - A 20 digit number starting with 1 is overflow if it is less than INT64_MAX.
    //   If we got here, it's a 20 digit number starting with the digit "1".
    // - If a 20 digit number starting with 1 overflowed (i*10+digit), the result will be smaller
    //   than 1,553,255,926,290,448,384.
    // - That is smaller than the smallest possible 20-digit number the user could write:
    //   10,000,000,000,000,000,000.
    // - Therefore, if the number is positive and lower than that, it's overflow.
    // - The value we are looking at is less than or equal to 9,223,372,036,854,775,808 (INT64_MAX).
    //
    if (src[0] != uint8_t('1') || i <= uint64_t(INT64_MAX)) { return INCORRECT_TYPE; }
  }

  return i;
}


// Parse any number from 0 to 18,446,744,073,709,551,615
// Never read at src_end or beyond
simdjson_unused simdjson_really_inline simdjson_result<uint64_t> parse_unsigned(const uint8_t * const src, const uint8_t * const src_end) noexcept {
  const uint8_t *p = src;
  //
  // Parse the integer part.
  //
  // PERF NOTE: we don't use is_made_of_eight_digits_fast because large integers like 123456789 are rare
  const uint8_t *const start_digits = p;
  uint64_t i = 0;
  while ((p != src_end) && parse_digit(*p, i)) { p++; }

  // If there were no digits, or if the integer starts with 0 and has more than one digit, it's an error.
  // Optimization note: size_t is expected to be unsigned.
  size_t digit_count = size_t(p - start_digits);
  // The longest positive 64-bit number is 20 digits.
  // We do it this way so we don't trigger this branch unless we must.
  // Optimization note: the compiler can probably merge
  // ((digit_count == 0) || (digit_count > 20))
  // into a single  branch since digit_count is unsigned.
  if ((digit_count == 0) || (digit_count > 20)) { return INCORRECT_TYPE; }
  // Here digit_count > 0.
  if (('0' == *start_digits) && (digit_count > 1)) { return NUMBER_ERROR; }
  // We can do the following...
  // if (!jsoncharutils::is_structural_or_whitespace(*p)) {
  //  return (*p == '.' || *p == 'e' || *p == 'E') ? INCORRECT_TYPE : NUMBER_ERROR;
  // }
  // as a single table lookup:
  if ((p != src_end) && integer_string_finisher[*p] != SUCCESS) { return error_code(integer_string_finisher[*p]); }

  if (digit_count == 20) {
    // Positive overflow check:
    // - A 20 digit number starting with 2-9 is overflow, because 18,446,744,073,709,551,615 is the
    //   biggest uint64_t.
    // - A 20 digit number starting with 1 is overflow if it is less than INT64_MAX.
    //   If we got here, it's a 20 digit number starting with the digit "1".
    // - If a 20 digit number starting with 1 overflowed (i*10+digit), the result will be smaller
    //   than 1,553,255,926,290,448,384.
    // - That is smaller than the smallest possible 20-digit number the user could write:
    //   10,000,000,000,000,000,000.
    // - Therefore, if the number is positive and lower than that, it's overflow.
    // - The value we are looking at is less than or equal to 9,223,372,036,854,775,808 (INT64_MAX).
    //
    if (src[0] != uint8_t('1') || i <= uint64_t(INT64_MAX)) { return INCORRECT_TYPE; }
  }

  return i;
}

// Parse any number from  -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807
simdjson_unused simdjson_really_inline simdjson_result<int64_t> parse_integer(const uint8_t *src) noexcept {
  //
  // Check for minus sign
  //
  bool negative = (*src == '-');
  const uint8_t *p = src + negative;

  //
  // Parse the integer part.
  //
  // PERF NOTE: we don't use is_made_of_eight_digits_fast because large integers like 123456789 are rare
  const uint8_t *const start_digits = p;
  uint64_t i = 0;
  while (parse_digit(*p, i)) { p++; }

  // If there were no digits, or if the integer starts with 0 and has more than one digit, it's an error.
  // Optimization note: size_t is expected to be unsigned.
  size_t digit_count = size_t(p - start_digits);
  // We go from
  // -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807
  // so we can never represent numbers that have more than 19 digits.
  size_t longest_digit_count = 19;
  // Optimization note: the compiler can probably merge
  // ((digit_count == 0) || (digit_count > longest_digit_count))
  // into a single  branch since digit_count is unsigned.
  if ((digit_count == 0) || (digit_count > longest_digit_count)) { return INCORRECT_TYPE; }
  // Here digit_count > 0.
  if (('0' == *start_digits) && (digit_count > 1)) { return NUMBER_ERROR; }
  // We can do the following...
  // if (!jsoncharutils::is_structural_or_whitespace(*p)) {
  //  return (*p == '.' || *p == 'e' || *p == 'E') ? INCORRECT_TYPE : NUMBER_ERROR;
  // }
  // as a single table lookup:
  if(integer_string_finisher[*p] != SUCCESS) { return error_code(integer_string_finisher[*p]); }
  // Negative numbers have can go down to - INT64_MAX - 1 whereas positive numbers are limited to INT64_MAX.
  // Performance note: This check is only needed when digit_count == longest_digit_count but it is
  // so cheap that we might as well always make it.
  if(i > uint64_t(INT64_MAX) + uint64_t(negative)) { return INCORRECT_TYPE; }
  return negative ? (~i+1) : i;
}

// Parse any number from  -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807
// Never read at src_end or beyond
simdjson_unused simdjson_really_inline simdjson_result<int64_t> parse_integer(const uint8_t * const src, const uint8_t * const src_end) noexcept {
  //
  // Check for minus sign
  //
  if(src == src_end) { return NUMBER_ERROR; }
  bool negative = (*src == '-');
  const uint8_t *p = src + negative;

  //
  // Parse the integer part.
  //
  // PERF NOTE: we don't use is_made_of_eight_digits_fast because large integers like 123456789 are rare
  const uint8_t *const start_digits = p;
  uint64_t i = 0;
  while ((p != src_end) && parse_digit(*p, i)) { p++; }

  // If there were no digits, or if the integer starts with 0 and has more than one digit, it's an error.
  // Optimization note: size_t is expected to be unsigned.
  size_t digit_count = size_t(p - start_digits);
  // We go from
  // -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807
  // so we can never represent numbers that have more than 19 digits.
  size_t longest_digit_count = 19;
  // Optimization note: the compiler can probably merge
  // ((digit_count == 0) || (digit_count > longest_digit_count))
  // into a single  branch since digit_count is unsigned.
  if ((digit_count == 0) || (digit_count > longest_digit_count)) { return INCORRECT_TYPE; }
  // Here digit_count > 0.
  if (('0' == *start_digits) && (digit_count > 1)) { return NUMBER_ERROR; }
  // We can do the following...
  // if (!jsoncharutils::is_structural_or_whitespace(*p)) {
  //  return (*p == '.' || *p == 'e' || *p == 'E') ? INCORRECT_TYPE : NUMBER_ERROR;
  // }
  // as a single table lookup:
  if((p != src_end) && integer_string_finisher[*p] != SUCCESS) { return error_code(integer_string_finisher[*p]); }
  // Negative numbers have can go down to - INT64_MAX - 1 whereas positive numbers are limited to INT64_MAX.
  // Performance note: This check is only needed when digit_count == longest_digit_count but it is
  // so cheap that we might as well always make it.
  if(i > uint64_t(INT64_MAX) + uint64_t(negative)) { return INCORRECT_TYPE; }
  return negative ? (~i+1) : i;
}

simdjson_unused simdjson_really_inline simdjson_result<double> parse_double(const uint8_t * src) noexcept {
  //
  // Check for minus sign
  //
  bool negative = (*src == '-');
  src += negative;

  //
  // Parse the integer part.
  //
  uint64_t i = 0;
  const uint8_t *p = src;
  p += parse_digit(*p, i);
  bool leading_zero = (i == 0);
  while (parse_digit(*p, i)) { p++; }
  // no integer digits, or 0123 (zero must be solo)
  if ( p == src ) { return INCORRECT_TYPE; }
  if ( (leading_zero && p != src+1)) { return NUMBER_ERROR; }

  //
  // Parse the decimal part.
  //
  int64_t exponent = 0;
  bool overflow;
  if (simdjson_likely(*p == '.')) {
    p++;
    const uint8_t *start_decimal_digits = p;
    if (!parse_digit(*p, i)) { return NUMBER_ERROR; } // no decimal digits
    p++;
    while (parse_digit(*p, i)) { p++; }
    exponent = -(p - start_decimal_digits);

    // Overflow check. More than 19 digits (minus the decimal) may be overflow.
    overflow = p-src-1 > 19;
    if (simdjson_unlikely(overflow && leading_zero)) {
      // Skip leading 0.00000 and see if it still overflows
      const uint8_t *start_digits = src + 2;
      while (*start_digits == '0') { start_digits++; }
      overflow = start_digits-src > 19;
    }
  } else {
    overflow = p-src > 19;
  }

  //
  // Parse the exponent
  //
  if (*p == 'e' || *p == 'E') {
    p++;
    bool exp_neg = *p == '-';
    p += exp_neg || *p == '+';

    uint64_t exp = 0;
    const uint8_t *start_exp_digits = p;
    while (parse_digit(*p, exp)) { p++; }
    // no exp digits, or 20+ exp digits
    if (p-start_exp_digits == 0 || p-start_exp_digits > 19) { return NUMBER_ERROR; }

    exponent += exp_neg ? 0-exp : exp;
  }

  if (jsoncharutils::is_not_structural_or_whitespace(*p)) { return NUMBER_ERROR; }

  overflow = overflow || exponent < simdjson::internal::smallest_power || exponent > simdjson::internal::largest_power;

  //
  // Assemble (or slow-parse) the float
  //
  double d;
  if (simdjson_likely(!overflow)) {
    if (compute_float_64(exponent, i, negative, d)) { return d; }
  }
  if (!parse_float_fallback(src-negative, &d)) {
    return NUMBER_ERROR;
  }
  return d;
}


// Never read at src_end or beyond
simdjson_unused simdjson_really_inline simdjson_result<double> parse_double(const uint8_t * src, const uint8_t * const src_end) noexcept {
  if(src == src_end) { return NUMBER_ERROR; }
  //
  // Check for minus sign
  //
  bool negative = (*src == '-');
  src += negative;

  //
  // Parse the integer part.
  //
  uint64_t i = 0;
  const uint8_t *p = src;
  if(p == src_end) { return NUMBER_ERROR; }
  p += parse_digit(*p, i);
  bool leading_zero = (i == 0);
  while ((p != src_end) && parse_digit(*p, i)) { p++; }
  // no integer digits, or 0123 (zero must be solo)
  if ( p == src ) { return INCORRECT_TYPE; }
  if ( (leading_zero && p != src+1)) { return NUMBER_ERROR; }

  //
  // Parse the decimal part.
  //
  int64_t exponent = 0;
  bool overflow;
  if (simdjson_likely((p != src_end) && (*p == '.'))) {
    p++;
    const uint8_t *start_decimal_digits = p;
    if ((p == src_end) || !parse_digit(*p, i)) { return NUMBER_ERROR; } // no decimal digits
    p++;
    while ((p != src_end) && parse_digit(*p, i)) { p++; }
    exponent = -(p - start_decimal_digits);

    // Overflow check. More than 19 digits (minus the decimal) may be overflow.
    overflow = p-src-1 > 19;
    if (simdjson_unlikely(overflow && leading_zero)) {
      // Skip leading 0.00000 and see if it still overflows
      const uint8_t *start_digits = src + 2;
      while (*start_digits == '0') { start_digits++; }
      overflow = start_digits-src > 19;
    }
  } else {
    overflow = p-src > 19;
  }

  //
  // Parse the exponent
  //
  if ((p != src_end) && (*p == 'e' || *p == 'E')) {
    p++;
    if(p == src_end) { return NUMBER_ERROR; }
    bool exp_neg = *p == '-';
    p += exp_neg || *p == '+';

    uint64_t exp = 0;
    const uint8_t *start_exp_digits = p;
    while ((p != src_end) && parse_digit(*p, exp)) { p++; }
    // no exp digits, or 20+ exp digits
    if (p-start_exp_digits == 0 || p-start_exp_digits > 19) { return NUMBER_ERROR; }

    exponent += exp_neg ? 0-exp : exp;
  }

  if ((p != src_end) && jsoncharutils::is_not_structural_or_whitespace(*p)) { return NUMBER_ERROR; }

  overflow = overflow || exponent < simdjson::internal::smallest_power || exponent > simdjson::internal::largest_power;

  //
  // Assemble (or slow-parse) the float
  //
  double d;
  if (simdjson_likely(!overflow)) {
    if (compute_float_64(exponent, i, negative, d)) { return d; }
  }
  if (!parse_float_fallback(src-negative, src_end, &d)) {
    return NUMBER_ERROR;
  }
  return d;
}

} //namespace {}
#endif // SIMDJSON_SKIPNUMBERPARSING

} // namespace numberparsing
} // unnamed namespace
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson
