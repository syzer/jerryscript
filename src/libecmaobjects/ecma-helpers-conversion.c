/* Copyright 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmahelpers Helpers for operations with ECMA data types
 * @{
 */

#include "ecma-globals.h"
#include "ecma-helpers.h"
#include "jerry-libc.h"

/*
 * \addtogroup ecmahelpersbigintegers Helpers for operations intermediate 96-bit integers
 * @{
 */

/**
 * Check that parts of 96-bit integer are 32-bit.
 */
#define ECMA_NUMBER_CONVERSION_96BIT_INTEGER_CHECK_PARTS_ARE_32BIT(name) \
{ \
  JERRY_ASSERT (name[0] == (uint32_t) name[0]); \
  JERRY_ASSERT (name[1] == (uint32_t) name[1]); \
  JERRY_ASSERT (name[2] == (uint32_t) name[2]); \
}

/**
 * Declare 96-bit integer.
 */
#define ECMA_NUMBER_CONVERSION_96BIT_INTEGER(name) uint64_t name[3] = { 0, 0, 0 }

/**
 * Initialize 96-bit integer with given 32-bit parts
 */
#define ECMA_NUMBER_CONVERSION_96BIT_INTEGER_INIT(name, high, mid, low) \
{ \
  name[2] = high; \
  name[1] = mid; \
  name[0] = low; \
 \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_CHECK_PARTS_ARE_32BIT (name); \
}

/**
 * Copy specified 96-bit integer
 */
#define ECMA_NUMBER_CONVERSION_96BIT_INTEGER_COPY(name_copy_to, name_copy_from) \
{ \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_CHECK_PARTS_ARE_32BIT (name_copy_to); \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_CHECK_PARTS_ARE_32BIT (name_copy_from); \
  \
  name_copy_to[0] = name_copy_from [0]; \
  name_copy_to[1] = name_copy_from [1]; \
  name_copy_to[2] = name_copy_from [2]; \
}

/**
 * Copy high and middle parts of 96-bit integer to specified uint64_t variable
 */
#define ECMA_NUMBER_CONVERSION_96BIT_INTEGER_ROUND_HIGH_AND_MIDDLE_TO_UINT64(name, uint64_var) \
{ \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_CHECK_PARTS_ARE_32BIT (name); \
  uint64_var = (name[2] << 32u) | (name[1] + ((name[0] >> 31u) != 0 ? 1 : 0)); \
}

/**
 * Copy middle and low parts of 96-bit integer to specified uint64_t variable
 */
#define ECMA_NUMBER_CONVERSION_96BIT_INTEGER_ROUND_MIDDLE_AND_LOW_TO_UINT64(name, uint64_var) \
{ \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_CHECK_PARTS_ARE_32BIT (name); \
  uint64_var = (name[1] << 32u) | (name[0]); \
}

/**
 * Check if specified 96-bit integers are equal
 */
#define ECMA_NUMBER_CONVERSION_96BIT_INTEGER_ARE_EQUAL(name1, name2) \
  ((name1)[0] == (name2[0]) \
   && (name1)[1] == (name2[1]) \
   && (name1)[2] == (name2[2]))

/**
 * Check if bits [lowest_bit, 96) are zero
 */
#define ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_HIGH_BIT_MASK_ZERO(name, lowest_bit) \
  ((lowest_bit) >= 64 ? ((name[2] >> ((lowest_bit) - 64)) == 0) : \
   ((lowest_bit) >= 32 ? (name[2] == 0 && ((name[1] >> ((lowest_bit) - 32)) == 0)) : \
   (name[2] == 0 && name[1] == 0 && ((name[0] >> (lowest_bit)) == 0))))

/**
 * Check if bits [0, highest_bit] are zero
 */
#define ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_LOW_BIT_MASK_ZERO(name, highest_bit) \
  ((highest_bit >= 64) ? (name[1] == 0 && name[0] == 0 && (((uint32_t) name[2] << (95 - (highest_bit))) == 0)) : \
   ((highest_bit >= 32) ? (name[0] == 0 && (((uint32_t) name[1] << (63 - (highest_bit))) == 0)) : \
    (((uint32_t) name[0] << (31 - (highest_bit))) == 0)))

/**
 * Check if 96-bit integer is zero
 */
#define ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_ZERO(name) \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_HIGH_BIT_MASK_ZERO (name, 0)

/**
 * Shift 96-bit integer one bit left
 */
#define ECMA_NUMBER_CONVERSION_96BIT_INTEGER_LEFT_SHIFT(name) \
{ \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_CHECK_PARTS_ARE_32BIT (name); \
  \
  name[2] = (uint32_t) (name[2] << 1u); \
  name[2] |= name[1] >> 31u; \
  name[1] = (uint32_t) (name[1] << 1u); \
  name[1] |= name[0] >> 31u; \
  name[0] = (uint32_t) (name[0] << 1u); \
  \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_CHECK_PARTS_ARE_32BIT (name); \
}

/**
 * Shift 96-bit integer one bit right
 */
#define ECMA_NUMBER_CONVERSION_96BIT_INTEGER_RIGHT_SHIFT(name) \
{ \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_CHECK_PARTS_ARE_32BIT (name); \
  \
  name[0] >>= 1u; \
  name[0] |= (uint32_t) (name[1] << 31u); \
  name[1] >>= 1u; \
  name[1] |= (uint32_t) (name[2] << 31u); \
  name[2] >>= 1u; \
  \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_CHECK_PARTS_ARE_32BIT (name); \
}

/**
 * Increment 96-bit integer
 */
#define ECMA_NUMBER_CONVERSION_96BIT_INTEGER_INC(name) \
{ \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_CHECK_PARTS_ARE_32BIT (name); \
  \
  name [0] += 1ull; \
  name [1] += (name [0] >> 32u); \
  name [0] = (uint32_t) name [0]; \
  name [2] += (name [1] >> 32u); \
  name [1] = (uint32_t) name [1]; \
  \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_CHECK_PARTS_ARE_32BIT (name); \
}

/**
 * Add 96-bit integer
 */
#define ECMA_NUMBER_CONVERSION_96BIT_INTEGER_ADD(name_add_to, name_to_add) \
{ \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_CHECK_PARTS_ARE_32BIT (name_add_to); \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_CHECK_PARTS_ARE_32BIT (name_to_add); \
  \
  name_add_to [0] += name_to_add [0]; \
  name_add_to [1] += name_to_add [1]; \
  name_add_to [2] += name_to_add [2]; \
  name_add_to [1] += (name_add_to [0] >> 32u); \
  name_add_to [0] = (uint32_t) name_add_to [0]; \
  name_add_to [2] += (name_add_to [1] >> 32u); \
  name_add_to [1] = (uint32_t) name_add_to [1]; \
  \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_CHECK_PARTS_ARE_32BIT (name_add_to); \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_CHECK_PARTS_ARE_32BIT (name_to_add); \
}

/**
 * Multiply 96-bit integer by 10
 */
#define ECMA_NUMBER_CONVERSION_96BIT_INTEGER_MUL_10(name) \
{ \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_CHECK_PARTS_ARE_32BIT (name); \
  \
  /* fraction_uint96 = (fraction_uint96 << 3) + (fraction_uint96 << 1) or fraction_uint96 *= 10 */ \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_LEFT_SHIFT (name); \
  \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER (name ## _tmp); \
  \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_COPY (name ## _tmp, name); \
  \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_LEFT_SHIFT (name ## _tmp); \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_LEFT_SHIFT (name ## _tmp); \
  \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_ADD (name, name ## _tmp); \
  \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_CHECK_PARTS_ARE_32BIT (name); \
}

/**
 * Divide 96-bit integer by 10
 */
#define ECMA_NUMBER_CONVERSION_96BIT_INTEGER_DIV_10(name) \
{ \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_CHECK_PARTS_ARE_32BIT (name); \
  \
  /* estimation of reciprocal of 10 */ \
  const uint64_t div10_p_low = 0x9999999aul; \
  const uint64_t div10_p_mid = 0x99999999ul; \
  const uint64_t div10_p_high = 0x19999999ul; \
  \
  uint64_t intermediate [6] = { 0, 0, 0, 0, 0, 0 }; \
  uint64_t tmp; \
  tmp = div10_p_low * name[0]; \
  intermediate [0] += (uint32_t) tmp; \
  intermediate [1] += tmp >> 32u; \
  tmp = div10_p_low  * name[1]; \
  intermediate [1] += (uint32_t) tmp; \
  intermediate [2] += tmp >> 32u; \
  tmp = div10_p_mid  * name[0]; \
  intermediate [1] += (uint32_t) tmp; \
  intermediate [2] += tmp >> 32u; \
  tmp = div10_p_low  * name[2]; \
  intermediate [2] += (uint32_t) tmp; \
  intermediate [3] += tmp >> 32u; \
  tmp = div10_p_mid  * name[1]; \
  intermediate [2] += (uint32_t) tmp; \
  intermediate [3] += tmp >> 32u; \
  tmp = div10_p_high * name[0]; \
  intermediate [2] += (uint32_t) tmp; \
  intermediate [3] += tmp >> 32u; \
  tmp = div10_p_mid  * name[2]; \
  intermediate [3] += (uint32_t) tmp; \
  intermediate [4] += tmp >> 32u; \
  tmp = div10_p_high * name[1]; \
  intermediate [3] += (uint32_t) tmp; \
  intermediate [4] += tmp >> 32u; \
  tmp = div10_p_high * name[2]; \
  intermediate [4] += (uint32_t) tmp; \
  intermediate [5] += tmp >> 32u; \
  \
  intermediate[1] += intermediate[0] >> 32u; \
  intermediate[0] = (uint32_t) intermediate [0]; \
  intermediate[2] += intermediate[1] >> 32u; \
  intermediate[1] = (uint32_t) intermediate [1]; \
  intermediate[3] += intermediate[2] >> 32u; \
  intermediate[2] = (uint32_t) intermediate [2]; \
  intermediate[4] += intermediate[3] >> 32u; \
  intermediate[3] = (uint32_t) intermediate [3]; \
  intermediate[5] += intermediate[4] >> 32u; \
  intermediate[4] = (uint32_t) intermediate [4]; \
  \
  name[0] = intermediate [3]; \
  name[1] = intermediate [4]; \
  name[2] = intermediate [5]; \
  \
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_CHECK_PARTS_ARE_32BIT (name); \
}

/**
 * @}
 */

/**
 * ECMA-defined conversion of string (zero-terminated) to Number.
 *
 * See also:
 *          ECMA-262 v5, 9.3.1
 *
 * Warning:
 *         the conversion routine may be not precise for some cases
 *
 * @return ecma-number
 */
ecma_number_t
ecma_zt_string_to_number (const ecma_char_t *str_p) /**< zero-terminated string */
{
  TODO (Check license issues);

  const ecma_char_t dec_digits_range[10] = { '0', '9' };
  const ecma_char_t hex_lower_digits_range[10] = { 'a', 'f' };
  const ecma_char_t hex_upper_digits_range[10] = { 'A', 'F' };
  const ecma_char_t hex_x_chars[2] = { 'x', 'X' };
  const ecma_char_t white_space[2] = { ' ', '\n' };
  const ecma_char_t e_chars [2] = { 'e', 'E' };
  const ecma_char_t plus_char = '+';
  const ecma_char_t minus_char = '-';
  const ecma_char_t dot_char = '.';

  const ecma_char_t *begin_p = str_p;
  const ecma_char_t *end_p = begin_p;

  while (*end_p != ECMA_CHAR_NULL)
  {
    end_p++;
  }
  end_p--;

  while (begin_p <= end_p
         && (*begin_p == white_space[0]
             || *begin_p == white_space[1]))
  {
    begin_p++;
  }

  while (begin_p <= end_p
         && (*end_p == white_space[0]
             || *end_p == white_space[1]))
  {
    end_p--;
  }

  if (begin_p > end_p)
  {
    return ECMA_NUMBER_ZERO;
  }

  const ssize_t literal_len = end_p - begin_p + 1;

  if (literal_len > 2
      && begin_p[0] == dec_digits_range[0]
      && (begin_p[1] == hex_x_chars[0]
          || begin_p[1] == hex_x_chars[1]))
  {
    /* Hex literal handling */
    begin_p += 2;

    ecma_number_t num = 0;

    for (const ecma_char_t* iter_p = begin_p;
         iter_p <= end_p;
         iter_p++)
    {
      int32_t digit_value;

      if (*iter_p >= dec_digits_range [0]
          && *iter_p <= dec_digits_range [1])
      {
        digit_value = (*iter_p - dec_digits_range[0]);
      }
      else if (*iter_p >= hex_lower_digits_range[0]
               && *iter_p <= hex_lower_digits_range[1])
      {
        digit_value = 10 + (*iter_p - hex_lower_digits_range[0]);
      }
      else if (*iter_p >= hex_upper_digits_range[0]
               && *iter_p <= hex_upper_digits_range[1])
      {
        digit_value = 10 + (*iter_p - hex_upper_digits_range[0]);
      }
      else
      {
        return ecma_number_make_nan ();
      }

      num = num * 16 + (ecma_number_t) digit_value;
    }

    return num;
  }

  bool sign = false; /* positive */

  if (*begin_p == plus_char)
  {
    begin_p++;
  }
  else if (*begin_p == minus_char)
  {
    sign = true; /* negative */

    begin_p++;
  }

  if (begin_p > end_p)
  {
    return ecma_number_make_nan ();
  }

  /* Checking if significant part of parse string is equal to "Infinity" */
  const ecma_char_t *infinity_zt_str_p = ecma_get_magic_string_zt (ECMA_MAGIC_STRING_INFINITY_UL);

  for (const ecma_char_t *iter_p = begin_p, *iter_infinity_p = infinity_zt_str_p;
       ;
       iter_infinity_p++, iter_p++)
  {
    if (*iter_p != *iter_infinity_p)
    {
      break;
    }

    if (iter_p == end_p)
    {
      return ecma_number_make_infinity (sign);
    }
  }

  uint64_t fraction_uint64 = 0;
  uint32_t digits = 0;
  int32_t e = 0;

  /* Parsing digits before dot (or before end of digits part if there is no dot in number) */
  while (begin_p <= end_p)
  {
    int32_t digit_value;

    if (*begin_p >= dec_digits_range [0]
        && *begin_p <= dec_digits_range [1])
    {
      digit_value = (*begin_p - dec_digits_range[0]);
    }
    else
    {
      break;
    }

    if (digits != 0 || digit_value != 0)
    {
      if (digits < ECMA_NUMBER_MAX_DIGITS)
      {
        fraction_uint64 = fraction_uint64 * 10 + (uint32_t) digit_value;
        digits++;
      }
      else if (e <= 100000) /* Some limit to not overflow exponent value
                               (so big exponent anyway will make number
                               rounded to infinity) */
      {
        e++;
      }
    }

    begin_p++;
  }

  if (begin_p <= end_p
      && *begin_p == dot_char)
  {
    begin_p++;

    /* Parsing number's part that is placed after dot */
    while (begin_p <= end_p)
    {
      int32_t digit_value;

      if (*begin_p >= dec_digits_range [0]
          && *begin_p <= dec_digits_range [1])
      {
        digit_value = (*begin_p - dec_digits_range[0]);
      }
      else
      {
        break;
      }

      if (digits < ECMA_NUMBER_MAX_DIGITS)
      {
        if (digits != 0 || digit_value != 0)
        {
          fraction_uint64 = fraction_uint64 * 10 + (uint32_t) digit_value;
          digits++;
        }

        e--;
      }

      begin_p++;
    }
  }

  /* Parsing exponent literal */
  int32_t e_in_lit = 0;
  bool e_in_lit_sign = false;

  if (begin_p <= end_p
      && (*begin_p == e_chars[0]
          || *begin_p == e_chars[1]))
  {
    begin_p++;

    if (*begin_p == plus_char)
    {
      begin_p++;
    }
    else if (*begin_p == minus_char)
    {
      e_in_lit_sign = true;
      begin_p++;
    }

    if (begin_p > end_p)
    {
      return ecma_number_make_nan ();
    }

    while (begin_p <= end_p)
    {
      int32_t digit_value;

      if (*begin_p >= dec_digits_range [0]
          && *begin_p <= dec_digits_range [1])
      {
        digit_value = (*begin_p - dec_digits_range[0]);
      }
      else
      {
        return ecma_number_make_nan ();
      }

      e_in_lit = e_in_lit * 10 + digit_value;

      begin_p++;
    }
  }

  /* Adding value of exponent literal to exponent value */
  if (e_in_lit_sign)
  {
    e -= e_in_lit;
  }
  else
  {
    e += e_in_lit;
  }

  bool e_sign;

  if (e < 0)
  {
    e_sign = true;
    e = -e;
  }
  else
  {
    e_sign = false;
  }

  if (begin_p <= end_p)
  {
    return ecma_number_make_nan ();
  }

  JERRY_ASSERT (begin_p == end_p + 1);

  if (fraction_uint64 == 0)
  {
    return sign ? -ECMA_NUMBER_ZERO : ECMA_NUMBER_ZERO;
  }

  int32_t binary_exponent = 1;

  /*
   * 96-bit mantissa storage
   *
   * Normalized: |4 bits zero|92-bit mantissa with highest bit set to 1 if mantissa is non-zero|
   */
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER (fraction_uint96);
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_INIT (fraction_uint96,
                                             fraction_uint64 >> 32u,
                                             (uint32_t) fraction_uint64,
                                             0ull);

  /* Normalizing mantissa */
  JERRY_ASSERT (ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_HIGH_BIT_MASK_ZERO (fraction_uint96, 92));

  while (ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_HIGH_BIT_MASK_ZERO (fraction_uint96, 91))
  {
    ECMA_NUMBER_CONVERSION_96BIT_INTEGER_LEFT_SHIFT (fraction_uint96);
    binary_exponent--;

    JERRY_ASSERT (!ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_ZERO (fraction_uint96));
  }

  if (!e_sign)
  {
    /* positive or zero decimal exponent */
    JERRY_ASSERT (e >= 0);

    while (e > 0)
    {
      JERRY_ASSERT (ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_HIGH_BIT_MASK_ZERO (fraction_uint96, 92));

      ECMA_NUMBER_CONVERSION_96BIT_INTEGER_MUL_10 (fraction_uint96);

      e--;

      /* Normalizing mantissa */
      while (!ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_HIGH_BIT_MASK_ZERO (fraction_uint96, 92))
      {
        ECMA_NUMBER_CONVERSION_96BIT_INTEGER_RIGHT_SHIFT (fraction_uint96);

        binary_exponent++;
      }
      while (ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_HIGH_BIT_MASK_ZERO (fraction_uint96, 91))
      {
        ECMA_NUMBER_CONVERSION_96BIT_INTEGER_LEFT_SHIFT (fraction_uint96);

        binary_exponent--;

        JERRY_ASSERT (!ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_ZERO (fraction_uint96));
      }
    }
  }
  else
  {
    /* negative decimal exponent */
    JERRY_ASSERT (e != 0);

    while (e > 0)
    {
      /* Denormalizing mantissa, moving highest 1 to 95-bit */
      while (ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_HIGH_BIT_MASK_ZERO (fraction_uint96, 95))
      {
        ECMA_NUMBER_CONVERSION_96BIT_INTEGER_LEFT_SHIFT (fraction_uint96);

        binary_exponent--;

        JERRY_ASSERT (!ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_ZERO (fraction_uint96));
      }

      ECMA_NUMBER_CONVERSION_96BIT_INTEGER_DIV_10 (fraction_uint96);

      e--;
    }

    /* Normalizing mantissa */
    while (!ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_HIGH_BIT_MASK_ZERO (fraction_uint96, 92))
    {
      ECMA_NUMBER_CONVERSION_96BIT_INTEGER_RIGHT_SHIFT (fraction_uint96);

      binary_exponent++;
    }
    while (ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_HIGH_BIT_MASK_ZERO (fraction_uint96, 91))
    {
      ECMA_NUMBER_CONVERSION_96BIT_INTEGER_LEFT_SHIFT (fraction_uint96);

      binary_exponent--;

      JERRY_ASSERT (!ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_ZERO (fraction_uint96));
    }
  }

  JERRY_ASSERT (!ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_ZERO (fraction_uint96));
  JERRY_ASSERT (ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_HIGH_BIT_MASK_ZERO (fraction_uint96, 92));

  /*
   * Preparing mantissa for conversion to 52-bit representation, converting it to:
   *
   * |12 zero bits|84 mantissa bits|
   */
  while (!ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_HIGH_BIT_MASK_ZERO (fraction_uint96, 84 + 1))
  {
    ECMA_NUMBER_CONVERSION_96BIT_INTEGER_RIGHT_SHIFT (fraction_uint96);

    binary_exponent++;
  }
  while (ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_HIGH_BIT_MASK_ZERO (fraction_uint96, 84))
  {
    ECMA_NUMBER_CONVERSION_96BIT_INTEGER_LEFT_SHIFT (fraction_uint96);

    binary_exponent--;

    JERRY_ASSERT (!ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_ZERO (fraction_uint96));
  }

  JERRY_ASSERT (ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_HIGH_BIT_MASK_ZERO (fraction_uint96, 84 + 1));

  ECMA_NUMBER_CONVERSION_96BIT_INTEGER_ROUND_HIGH_AND_MIDDLE_TO_UINT64 (fraction_uint96, fraction_uint64);

  return ecma_number_make_from_sign_mantissa_and_exponent (sign,
                                                           fraction_uint64,
                                                           binary_exponent);
} /* ecma_zt_string_to_number */

/**
 * ECMA-defined conversion of UInt32 to String (zero-terminated).
 *
 * See also:
 *          ECMA-262 v5, 9.8.1
 *
 * @return number of bytes copied to buffer
 */
ssize_t
ecma_uint32_to_string (uint32_t value, /**< value to convert */
                       ecma_char_t *out_buffer_p, /**< buffer for zero-terminated string */
                       ssize_t buffer_size) /**< size of buffer */
{
  const ecma_char_t digits[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

  ecma_char_t *p = (ecma_char_t*) ((uint8_t*) out_buffer_p + buffer_size) - 1;
  *p-- = ECMA_CHAR_NULL;

  size_t bytes_copied = sizeof (ecma_char_t);

  do
  {
    JERRY_ASSERT (p >= out_buffer_p);

    *p-- = digits[value % 10];
    value /= 10;

    bytes_copied += sizeof (ecma_char_t);
  }
  while (value != 0);

  p++;

  JERRY_ASSERT (p >= out_buffer_p);

  if (likely (p != out_buffer_p))
  {
    ssize_t bytes_to_move = ((uint8_t*) out_buffer_p + buffer_size) - (uint8_t*) p;
    __memmove (out_buffer_p, p, (size_t) bytes_to_move);
  }

  return (ssize_t) bytes_copied;
} /* ecma_uint32_to_string */

/**
 * ECMA-defined conversion of UInt32 value to Number value
 *
 * @return number - result of conversion.
 */
ecma_number_t
ecma_uint32_to_number (uint32_t value) /**< unsigned 32-bit integer value */
{
  ecma_number_t num_value = (ecma_number_t) value;

  return num_value;
} /* ecma_uint32_to_number */

/**
 * ECMA-defined conversion of Int32 value to Number value
 *
 * @return number - result of conversion.
 */
ecma_number_t
ecma_int32_to_number (int32_t value) /**< signed 32-bit integer value */
{
  ecma_number_t num_value = (ecma_number_t) value;

  return num_value;
} /* ecma_int32_to_number */

/**
 * ECMA-defined conversion of Number value to Uint32 value
 *
 * See also:
 *          ECMA-262 v5, 9.6
 *
 * @return number - result of conversion.
 */
uint32_t
ecma_number_to_uint32 (ecma_number_t value) /**< unsigned 32-bit integer value */
{
  if (ecma_number_is_nan (value)
      || ecma_number_is_zero (value)
      || ecma_number_is_infinity (value))
  {
    return 0;
  }

  return (uint32_t) value;
} /* ecma_number_to_uint32 */

/**
 * ECMA-defined conversion of Number value to Int32 value
 *
 * See also:
 *          ECMA-262 v5, 9.5
 *
 * @return number - result of conversion.
 */
int32_t
ecma_number_to_int32 (ecma_number_t value) /**< unsigned 32-bit integer value */
{
  if (ecma_number_is_nan (value)
      || ecma_number_is_zero (value)
      || ecma_number_is_infinity (value))
  {
    return 0;
  }

  return (int32_t) (uint32_t) value;
} /* ecma_number_to_int32 */

/**
 * Calculate s, n and k parameters for specified ecma-number according to ECMA-262 v5, 9.8.1, item 5
 */
static void
ecma_number_to_zt_string_calc_number_params (ecma_number_t num, /**< ecma-number */
                                             uint64_t *out_digits_p, /**< out: digits */
                                             int32_t *out_digits_num_p, /**< out: number of digits */
                                             int32_t *out_decimal_exp_p) /**< out: decimal exponent */
{
  ECMA_NUMBER_CONVERSION_96BIT_INTEGER (fraction_uint96);

#if CONFIG_ECMA_NUMBER_TYPE == CONFIG_ECMA_NUMBER_FLOAT32
  uint32_t s[2];
#elif CONFIG_ECMA_NUMBER_TYPE == CONFIG_ECMA_NUMBER_FLOAT64
  uint64_t s[2];
#endif /* CONFIG_ECMA_NUMBER_TYPE == CONFIG_ECMA_NUMBER_FLOAT64 */
  int32_t k[2];
  int32_t n[2];

  for (uint32_t i = 0;
       i <= 1;
       i++)
  {
    uint64_t fraction_uint64;
    int32_t binary_exponent;
    int32_t dot_shift;
    int32_t decimal_exp = 0;

    dot_shift = ecma_number_get_fraction_and_exponent (num, &fraction_uint64, &binary_exponent);

    binary_exponent -= dot_shift;

    JERRY_ASSERT (fraction_uint64 != 0);

    if (i == 0)
    {
      /* Lowest binary fraction that should round to fraction_uint64 */
      ECMA_NUMBER_CONVERSION_96BIT_INTEGER_INIT (fraction_uint96,
                                                 (fraction_uint64 - 1ull) >> 60u,
                                                 ((fraction_uint64 - 1ull) << 4u) >> 32u,
                                                 ((fraction_uint64 - 1ull) << 36u) >> 32u | 0x8u);
    }
    else
    {
      /* Highest binary fraction that should round to fraction_uint64 */
      ECMA_NUMBER_CONVERSION_96BIT_INTEGER_INIT (fraction_uint96,
                                                 (fraction_uint64) >> 60u,
                                                 ((fraction_uint64) << 4u) >> 32u,
                                                 ((fraction_uint64) << 36u) >> 32u | 0x7u);
    }

    binary_exponent -= 4;

    /* Converting binary exponent to decimal exponent */
    if (binary_exponent > 0)
    {
      while (binary_exponent > 0)
      {
        if (!ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_HIGH_BIT_MASK_ZERO (fraction_uint96, 92))
        {
          ECMA_NUMBER_CONVERSION_96BIT_INTEGER_INC (fraction_uint96);
          ECMA_NUMBER_CONVERSION_96BIT_INTEGER_RIGHT_SHIFT (fraction_uint96);
          binary_exponent++;
        }
        else
        {
          ECMA_NUMBER_CONVERSION_96BIT_INTEGER (fraction_uint96_tmp);
          ECMA_NUMBER_CONVERSION_96BIT_INTEGER_COPY (fraction_uint96_tmp, fraction_uint96);
          ECMA_NUMBER_CONVERSION_96BIT_INTEGER_DIV_10 (fraction_uint96_tmp);
          ECMA_NUMBER_CONVERSION_96BIT_INTEGER_MUL_10 (fraction_uint96_tmp);

          if (!ECMA_NUMBER_CONVERSION_96BIT_INTEGER_ARE_EQUAL (fraction_uint96, fraction_uint96_tmp)
              && ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_HIGH_BIT_MASK_ZERO (fraction_uint96, 91))
          {
            ECMA_NUMBER_CONVERSION_96BIT_INTEGER_LEFT_SHIFT (fraction_uint96);
            binary_exponent--;
          }
          else
          {
            ECMA_NUMBER_CONVERSION_96BIT_INTEGER_DIV_10 (fraction_uint96);
            decimal_exp++;
          }
        }
      }
    }
    else if (binary_exponent < 0)
    {
      while (binary_exponent < 0)
      {
        if (ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_LOW_BIT_MASK_ZERO (fraction_uint96, 0)
            || !ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_HIGH_BIT_MASK_ZERO (fraction_uint96, 92))
        {
          ECMA_NUMBER_CONVERSION_96BIT_INTEGER_RIGHT_SHIFT (fraction_uint96);

          binary_exponent++;
        }
        else
        {
          ECMA_NUMBER_CONVERSION_96BIT_INTEGER_MUL_10 (fraction_uint96);

          decimal_exp--;
        }
      }
    }

#if CONFIG_ECMA_NUMBER_TYPE == CONFIG_ECMA_NUMBER_FLOAT64
    uint64_t digits, t;

    /* While fraction doesn't fit to 64-bit integer, divide it by 10
         and simultaneously increment decimal exponent */
    while (!ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_HIGH_BIT_MASK_ZERO (fraction_uint96, 64))
    {
      ECMA_NUMBER_CONVERSION_96BIT_INTEGER_DIV_10 (fraction_uint96);
      decimal_exp++;
    }
#elif CONFIG_ECMA_NUMBER_TYPE == CONFIG_ECMA_NUMBER_FLOAT32
    uint32_t digits, t;

    while (!ECMA_NUMBER_CONVERSION_96BIT_INTEGER_IS_HIGH_BIT_MASK_ZERO (fraction_uint96, 32))
    {
      ECMA_NUMBER_CONVERSION_96BIT_INTEGER_DIV_10 (fraction_uint96);
      decimal_exp++;
    }
#endif /* CONFIG_ECMA_NUMBER_TYPE == CONFIG_ECMA_NUMBER_FLOAT32 */

    uint64_t digits_uint64;
    int32_t digits_num = 0;

    ECMA_NUMBER_CONVERSION_96BIT_INTEGER_ROUND_MIDDLE_AND_LOW_TO_UINT64 (fraction_uint96, digits_uint64);

#if CONFIG_ECMA_NUMBER_TYPE == CONFIG_ECMA_NUMBER_FLOAT64
    digits = digits_uint64;
#elif CONFIG_ECMA_NUMBER_TYPE == CONFIG_ECMA_NUMBER_FLOAT32
    digits = (uint32_t) digits_uint64;

    JERRY_ASSERT (digits == digits_uint64);
#endif /* CONFIG_ECMA_NUMBER_TYPE == CONFIG_ECMA_NUMBER_FLOAT32 */

    /* Calculate number of digits in the number */
    t = digits;
    while (t != 0)
    {
      if (digits_num < ECMA_NUMBER_MAX_DIGITS)
      {
        digits_num++;
      }
      else
      {
        if (t < 10)
        {
          digits += 5;
        }

        digits /= 10;
      }

      t /= 10;

      decimal_exp++;
    }

    /* Saving bound values */
    s[i] = digits;
    k[i] = digits_num;
    n[i] = decimal_exp;
  }

  /* Making bound values' digit sets to be of one length */
  for (uint32_t i = 0; i <= 1; i++)
  {
    while (n[i] - k[i] > n[1 - i] - k[1 - i])
    {
      JERRY_ASSERT (s[i] * 10 > s[i]);

      s[i] *= 10;
      k[i]++;
    }
  }

  JERRY_ASSERT (s[1] > s[0]);

  while (s[0] / 10 != s[1] / 10)
  {
    s[0] /= 10;
    s[1] /= 10;
    k[0]--;
    k[1]--;
  }

  /* Rounding up */
  if (k[0] == k[1])
  {
    *out_digits_p = (s[0] + s[1] + 1) / 2;
  }
  else
  {
    *out_digits_p = s[1];
  }

  *out_digits_num_p = k[1];
  *out_decimal_exp_p = n[1];
} /* ecma_number_to_zt_string_calc_number_params */

/**
 * Convert ecma-number to zero-terminated string
 *
 * See also:
 *          ECMA-262 v5, 9.8.1
 *
 * Warning:
 *         the conversion is not precise for all cases
 *         For example, 12345.123f converts to "12345.12209".
 *
 * @return length of zt-string
 */
ecma_length_t
ecma_number_to_zt_string (ecma_number_t num, /**< ecma-number */
                          ecma_char_t *buffer_p, /**< buffer for zt-string */
                          ssize_t buffer_size) /**< size of buffer */
{
  const ecma_char_t digits[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
  const ecma_char_t e_chars [2] = { 'e', 'E' };
  const ecma_char_t plus_char = '+';
  const ecma_char_t minus_char = '-';
  const ecma_char_t dot_char = '.';

  if (ecma_number_is_nan (num))
  {
    // 1.
    ecma_copy_zt_string_to_buffer (ecma_get_magic_string_zt (ECMA_MAGIC_STRING_NAN),
                                   buffer_p,
                                   buffer_size);
  }
  else
  {
    ecma_char_t *dst_p = buffer_p;

    if (ecma_number_is_zero (num))
    {
      // 2.
      *dst_p++ = digits[0];
      *dst_p++ = ECMA_CHAR_NULL;

      JERRY_ASSERT ((uint8_t*)dst_p - (uint8_t*)buffer_p <= (ssize_t) buffer_size);
    }
    else if (ecma_number_is_negative (num))
    {
      // 3.
      *dst_p++ = minus_char;
      ssize_t new_buffer_size = (buffer_size - ((uint8_t*)dst_p - (uint8_t*)buffer_p));
      ecma_number_to_zt_string (ecma_number_negate (num), dst_p, new_buffer_size);
    }
    else if (ecma_number_is_infinity (num))
    {
      // 4.
      ecma_copy_zt_string_to_buffer (ecma_get_magic_string_zt (ECMA_MAGIC_STRING_INFINITY_UL),
                                     buffer_p,
                                     buffer_size);
    }
    else
    {
      // 5.
      uint32_t num_uint32 = ecma_number_to_uint32 (num);
      if (ecma_uint32_to_number (num_uint32) == num)
      {
        ecma_uint32_to_string (num_uint32, dst_p, buffer_size);
      }
      else
      {
        uint64_t fraction_uint64;
        int32_t binary_exponent;

        ecma_number_get_fraction_and_exponent (num, &fraction_uint64, &binary_exponent);

        /* mantissa */
        uint64_t s_uint64;
        /* decimal exponent */
        int32_t n;
        /* number of digits in k */
        int32_t k;

        ecma_number_to_zt_string_calc_number_params (num,
                                                     &s_uint64,
                                                     &k,
                                                     &n);

#if CONFIG_ECMA_NUMBER_TYPE == CONFIG_ECMA_NUMBER_FLOAT64
        uint64_t s = s_uint64;
#elif CONFIG_ECMA_NUMBER_TYPE == CONFIG_ECMA_NUMBER_FLOAT32
        uint32_t s = (uint32_t) s_uint64;

        JERRY_ASSERT (s == s_uint64);
#endif /* CONFIG_ECMA_NUMBER_TYPE == CONFIG_ECMA_NUMBER_FLOAT32 */

        // 6.
        if (k <= n && n <= 21)
        {
          dst_p += n;
          JERRY_ASSERT ((ssize_t) sizeof (ecma_char_t) * ((dst_p - buffer_p) + 1) <= buffer_size);

          *dst_p = ECMA_CHAR_NULL;

          for (int32_t i = 0; i < n - k; i++)
          {
            *--dst_p = digits [0];
          }

          for (int32_t i = 0; i < k; i++)
          {
            *--dst_p = digits [s % 10];
            s /= 10;
          }
        }
        else if (0 < n && n <= 21)
        {
          // 7.
          dst_p += k + 1;
          JERRY_ASSERT ((ssize_t) sizeof (ecma_char_t) * ((dst_p - buffer_p) + 1) <= buffer_size);

          *dst_p = ECMA_CHAR_NULL;

          for (int32_t i = 0; i < k - n; i++)
          {
            *--dst_p = digits [s % 10];
            s /= 10;
          }

          *--dst_p = dot_char;

          for (int32_t i = 0; i < n; i++)
          {
            *--dst_p = digits [s % 10];
            s /= 10;
          }
        }
        else if (-6 < n && n <= 0)
        {
          // 8.
          dst_p += k - n + 1 + 1;
          JERRY_ASSERT ((ssize_t) sizeof (ecma_char_t) * ((dst_p - buffer_p) + 1) <= buffer_size);

          *dst_p = ECMA_CHAR_NULL;

          for (int32_t i = 0; i < k; i++)
          {
            *--dst_p = digits [s % 10];
            s /= 10;
          }

          for (int32_t i = 0; i < -n; i++)
          {
            *--dst_p = digits [0];
          }

          *--dst_p = dot_char;
          *--dst_p = digits[0];
        }
        else
        {
          if (k == 1)
          {
            // 9.
            JERRY_ASSERT ((ssize_t) sizeof (ecma_char_t) <= buffer_size);

            *dst_p++ = digits [s % 10];
            s /= 10;
          }
          else
          {
            // 10.
            dst_p += k + 1;
            JERRY_ASSERT ((ssize_t) sizeof (ecma_char_t) * (dst_p - buffer_p) <= buffer_size);

            for (int32_t i = 0; i < k - 1; i++)
            {
              *--dst_p = digits [s % 10];
              s /= 10;
            }

            *--dst_p = dot_char;
            *--dst_p = digits[s % 10];
            s /= 10;

            dst_p += k + 1;
          }

          // 9., 10.
          JERRY_ASSERT ((ssize_t) sizeof (ecma_char_t) * (dst_p - buffer_p + 2) <= buffer_size);
          *dst_p++ = e_chars[0];
          *dst_p++ = (n >= 1) ? plus_char : minus_char;
          int32_t t = (n >= 1) ? (n - 1) : -(n - 1);

          if (t == 0)
          {
            JERRY_ASSERT ((ssize_t) sizeof (ecma_char_t) * (dst_p - buffer_p + 1) <= buffer_size);
            *dst_p++ = digits [0];
          }
          else
          {
            int32_t t_mod = 1000000000u;

            while ((t / t_mod) == 0)
            {
              t_mod /= 10;

              JERRY_ASSERT (t != 0);
            }

            while (t_mod != 0)
            {
              JERRY_ASSERT ((ssize_t) sizeof (ecma_char_t) * (dst_p - buffer_p + 1) <= buffer_size);
              *dst_p++ = digits [t / t_mod];

              t -= (t / t_mod) * t_mod;
              t_mod /= 10;
            }
          }

          JERRY_ASSERT ((ssize_t) sizeof (ecma_char_t) * (dst_p - buffer_p + 1) <= buffer_size);
          *dst_p++ = ECMA_CHAR_NULL;
        }

        JERRY_ASSERT (s == 0);
      }
    }
  }

  ecma_length_t length = ecma_zt_string_length (buffer_p);

  return length;
} /* ecma_number_to_zt_string */

/**
 * @}
 * @}
 */
