#pragma once
/*
* PCG Random Number Generation for C++
*
* Copyright 2014-2017 Melissa O'Neill <oneill@pcg-random.org>,
*                     and the PCG Project contributors.
*
* SPDX-License-Identifier: (Apache-2.0 OR MIT)
*
* Licensed under the Apache License, Version 2.0 (provided in
* LICENSE-APACHE.txt and at http://www.apache.org/licenses/LICENSE-2.0)
* or under the MIT license (provided in LICENSE-MIT.txt and at
* http://opensource.org/licenses/MIT), at your option. This file may not
* be copied, modified, or distributed except according to those terms.
*
* Distributed on an "AS IS" BASIS, WITHOUT WARRANTY OF ANY KIND, either
* express or implied.  See your chosen license for details.
*
* For additional information about the PCG random number generation scheme,
* visit http://www.pcg-random.org/.
*/

/*
* This file provides support code that is useful for random-number generation
* but not specific to the PCG generation scheme, including:
*      - 128-bit int support for platforms where it isn't available natively
*      - bit twiddling operations
*      - I/O of 128-bit and 8-bit integers
*      - Handling the evilness of SeedSeq
*      - Support for efficiently producing random numbers less than a given
*        bound
*/

/*
 * Abstractions for compiler-specific directives
 */

 #ifdef __GNUC__
 #define PCG_NOINLINE __attribute__((noinline))
#else
 #define PCG_NOINLINE
#endif

#ifdef _MSC_VER
#define PCG_ALWAYS_INLINE __forceinline
#elif __GNUC__
#define PCG_ALWAYS_INLINE __attribute__((always_inline))
#else
#define PCG_ALWAYS_INLINE inline
#endif


/*
* Some members of the PCG library use 128-bit math.  When compiling on 64-bit
* platforms, both GCC and Clang provide 128-bit integer types that are ideal
* for the job.
*
* On 32-bit platforms (or with other compilers), we fall back to a C++
* class that provides 128-bit unsigned integers instead.  It may seem
* like we're reinventing the wheel here, because libraries already exist
* that support large integers, but most existing libraries provide a very
* generic multiprecision code, but here we're operating at a fixed size.
* Also, most other libraries are fairly heavyweight.  So we use a direct
* implementation.  Sadly, it's much slower than hand-coded assembly or
* direct CPU support.
*
*/
#if __SIZEOF_INT128__ && !PCG_FORCE_EMULATED_128BIT_MATH
namespace pcg_extras {
     using  pcg128_t = __uint128_t;
 }
 #define PCG_128BIT_CONSTANT(high,low) \
         ((pcg_extras::pcg128_t(high) << 64) + low)
#elif __has_include(<__msvc_int128.hpp>)
 #include <__msvc_int128.hpp>
 namespace pcg_extras {
     using pcg128_t = std::_Unsigned128;
 }
 #define PCG_128BIT_CONSTANT(high,low) \
         pcg_extras::pcg128_t(low, high)
#else
 #include "pcg_uint128.hpp"
 #define PCG_128BIT_CONSTANT(high,low) \
         pcg_extras::pcg128_t(high,low)
 #define PCG_EMULATED_128BIT_MATH 1
#endif


namespace pcg_extras {

/*
 * We often need to represent a "number of bits".  When used normally, these
 * numbers are never greater than 128, so an unsigned char is plenty.
 * If you're using a nonstandard generator of a larger size, you can set
 * PCG_BITCOUNT_T to have it define it as a larger size.  (Some compilers
 * might produce faster code if you set it to an unsigned int.)
 */

#ifndef PCG_BITCOUNT_T
    using bitcount_t = uint8_t;
#else
    using bitcount_t = PCG_BITCOUNT_T;
#endif

/*
 * Useful bitwise operations.
 */

/*
 * XorShifts are invertable, but they are someting of a pain to invert.
 * This function backs them out.  It's used by the whacky "inside out"
 * generator defined later.
 */

 template <typename itype>
 inline itype unxorshift(itype x, bitcount_t bits, bitcount_t shift)
 {
     if (2*shift >= bits) {
         return x ^ (x >> shift);
     }
     itype lowmask1 = (itype(1U) << (bits - shift*2)) - 1;
     itype highmask1 = ~lowmask1;
     itype top1 = x;
     itype bottom1 = x & lowmask1;
     top1 ^= top1 >> shift;
     top1 &= highmask1;
     x = top1 | bottom1;
     itype lowmask2 = (itype(1U) << (bits - shift)) - 1;
     itype bottom2 = x & lowmask2;
     bottom2 = unxorshift(bottom2, bits - shift, shift);
     bottom2 &= lowmask1;
     return top1 | bottom2;
 }
 
 /*
  * Rotate left and right.
  *
  * In ideal world, compilers would spot idiomatic rotate code and convert it
  * to a rotate instruction.  Of course, opinions vary on what the correct
  * idiom is and how to spot it.  For clang, sometimes it generates better
  * (but still crappy) code if you define PCG_USE_ZEROCHECK_ROTATE_IDIOM.
  */
 
 template <typename itype>
 inline itype rotl(itype value, bitcount_t rot)
 {
     constexpr bitcount_t bits = sizeof(itype) * 8;
     constexpr bitcount_t mask = bits - 1;
 #if PCG_USE_ZEROCHECK_ROTATE_IDIOM
     return rot ? (value << rot) | (value >> (bits - rot)) : value;
 #else
     return (value << rot) | (value >> ((- rot) & mask));
 #endif
 }
 
 template <typename itype>
 inline itype rotr(itype value, bitcount_t rot)
 {
     constexpr bitcount_t bits = sizeof(itype) * 8;
     constexpr bitcount_t mask = bits - 1;
 #if PCG_USE_ZEROCHECK_ROTATE_IDIOM
     return rot ? (value >> rot) | (value << (bits - rot)) : value;
 #else
     return (value >> rot) | (value << ((- rot) & mask));
 #endif
 }
 
 /* Unfortunately, both Clang and GCC sometimes perform poorly when it comes
  * to properly recognizing idiomatic rotate code, so for we also provide
  * assembler directives (enabled with PCG_USE_INLINE_ASM).  Boo, hiss.
  * (I hope that these compilers get better so that this code can die.)
  *
  * These overloads will be preferred over the general template code above.
  */
 #if PCG_USE_INLINE_ASM && __GNUC__ && (__x86_64__  || __i386__)
 
 inline uint8_t rotr(uint8_t value, bitcount_t rot)
 {
     asm ("rorb   %%cl, %0" : "=r" (value) : "0" (value), "c" (rot));
     return value;
 }
 
 inline uint16_t rotr(uint16_t value, bitcount_t rot)
 {
     asm ("rorw   %%cl, %0" : "=r" (value) : "0" (value), "c" (rot));
     return value;
 }
 
 inline uint32_t rotr(uint32_t value, bitcount_t rot)
 {
     asm ("rorl   %%cl, %0" : "=r" (value) : "0" (value), "c" (rot));
     return value;
 }
 
 #if __x86_64__
 inline uint64_t rotr(uint64_t value, bitcount_t rot)
 {
     asm ("rorq   %%cl, %0" : "=r" (value) : "0" (value), "c" (rot));
     return value;
 }
 #endif // __x86_64__
 
 #elif defined(_MSC_VER)
   // Use MSVC++ bit rotation intrinsics
 
 #pragma intrinsic(_rotr, _rotr64, _rotr8, _rotr16)
 
 inline uint8_t rotr(uint8_t value, bitcount_t rot)
 {
     return _rotr8(value, rot);
 }
 
 inline uint16_t rotr(uint16_t value, bitcount_t rot)
 {
     return _rotr16(value, rot);
 }
 
 inline uint32_t rotr(uint32_t value, bitcount_t rot)
 {
     return _rotr(value, rot);
 }
 
 inline uint64_t rotr(uint64_t value, bitcount_t rot)
 {
     return _rotr64(value, rot);
 }
 
 #endif // PCG_USE_INLINE_ASM

}
