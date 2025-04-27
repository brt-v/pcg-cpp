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

#include <cstdint>
#include <type_traits>
#include <iostream>
#include "pcg_random.hpp"

#ifdef __GNUC__
    #include <cxxabi.h>
#endif


namespace pcg_extras {

/*
 * C++ requires us to be able to serialize RNG state by printing or reading
 * it from a stream.  Because we use 128-bit ints, we also need to be able
 * ot print them, so here is code to do so.
 *
 * This code provides enough functionality to print 128-bit ints in decimal
 * and zero-padded in hex.  It's not a full-featured implementation.
 */

template <typename CharT, typename Traits>
std::basic_ostream<CharT,Traits>&
operator<<(std::basic_ostream<CharT,Traits>& out, pcg128_t value)
{
    auto desired_base = out.flags() & out.basefield;
    bool want_hex = desired_base == out.hex;

    if (want_hex) {
        uint64_t highpart = uint64_t(value >> 64);
        uint64_t lowpart  = uint64_t(value);
        auto desired_width = out.width();
        if (desired_width > 16) {
            out.width(desired_width - 16);
        }
        if (highpart != 0 || desired_width > 16)
            out << highpart;
        CharT oldfill = '\0';
        if (highpart != 0) {
            out.width(16);
            oldfill = out.fill('0');
        }
        auto oldflags = out.setf(decltype(desired_base){}, out.showbase);
        out << lowpart;
        out.setf(oldflags);
        if (highpart != 0) {
            out.fill(oldfill);
        }
        return out;
    }
    constexpr size_t MAX_CHARS_128BIT = 40;

    char buffer[MAX_CHARS_128BIT];
    char* pos = buffer+sizeof(buffer);
    *(--pos) = '\0';
    constexpr auto BASE = pcg128_t(10ULL);
    do {
        auto div = value / BASE;
        auto mod = uint32_t(value - (div * BASE));
        *(--pos) = '0' + char(mod);
        value = div;
    } while(value != pcg128_t(0ULL));
    return out << pos;
}

template <typename CharT, typename Traits>
std::basic_istream<CharT,Traits>&
operator>>(std::basic_istream<CharT,Traits>& in, pcg128_t& value)
{
    typename std::basic_istream<CharT,Traits>::sentry s(in);

    if (!s)
         return in;

    constexpr auto BASE = pcg128_t(10ULL);
    pcg128_t current(0ULL);
    bool did_nothing = true;
    bool overflow = false;
    for(;;) {
        CharT wide_ch = in.get();
        if (!in.good()) {
            in.clear(std::ios::eofbit);
            break;
        }
        auto ch = in.narrow(wide_ch, '\0');
        if (ch < '0' || ch > '9') {
            in.unget();
            break;
        }
        did_nothing = false;
        pcg128_t digit(uint32_t(ch - '0'));
        pcg128_t timesbase = current*BASE;
        overflow = overflow || timesbase < current;
        current = timesbase + digit;
        overflow = overflow || current < digit;
    }

    if (did_nothing || overflow) {
        in.setstate(std::ios::failbit);
        if (overflow)
            current = ~pcg128_t(0ULL);
    }

    value = current;

    return in;
}

/*
 * Likewise, if people use tiny rngs, we'll be serializing uint8_t.
 * If we just used the provided IO operators, they'd read/write chars,
 * not ints, so we need to define our own.  We *can* redefine this operator
 * here because we're in our own namespace.
 */

template <typename CharT, typename Traits>
std::basic_ostream<CharT,Traits>&
operator<<(std::basic_ostream<CharT,Traits>&out, uint8_t value)
{
    return out << uint32_t(value);
}

template <typename CharT, typename Traits>
std::basic_istream<CharT,Traits>&
operator>>(std::basic_istream<CharT,Traits>& in, uint8_t& target)
{
    uint32_t value = 0xdecea5edU;
    in >> value;
    if (!in && value == 0xdecea5edU)
        return in;
    if (value > uint8_t(~0)) {
        in.setstate(std::ios::failbit);
        value = ~0U;
    }
    target = uint8_t(value);
    return in;
}

/* Unfortunately, the above functions don't get found in preference to the
 * built in ones, so we create some more specific overloads that will.
 * Ugh.
 */

inline std::ostream& operator<<(std::ostream& out, uint8_t value)
{
    return pcg_extras::operator<< <char>(out, value);
}

inline std::istream& operator>>(std::istream& in, uint8_t& value)
{
    return pcg_extras::operator>> <char>(in, value);
}


template <typename Iter, typename RandType>
void shuffle(Iter from, Iter to, RandType&& rng)
{
    using delta_t  = typename std::iterator_traits<Iter>::difference_type;
    using result_t = typename std::remove_reference<RandType>::type::result_type;
    auto count = to - from;
    while (count > 1) {
        delta_t chosen = delta_t(bounded_rand(rng, result_t(count)));
        --count;
        --to;
        using std::swap;
        swap(*(from + chosen), *to);
    }
}

/*
 * Although std::seed_seq is useful, it isn't everything.  Often we want to
 * initialize a random-number generator some other way, such as from a random
 * device.
 *
 * Technically, it does not meet the requirements of a SeedSequence because
 * it lacks some of the rarely-used member functions (some of which would
 * be impossible to provide).  However the C++ standard is quite specific
 * that actual engines only called the generate method, so it ought not to be
 * a problem in practice.
 */

template <typename RngType>
class seed_seq_from {
private:
    RngType rng_;

    using result_type = uint_least32_t;

public:
    template<typename... Args>
    seed_seq_from(Args&&... args) :
        rng_(std::forward<Args>(args)...)
    {
        // Nothing (else) to do...
    }

    template<typename Iter>
    void generate(Iter start, Iter finish)
    {
        for (auto i = start; i != finish; ++i)
            *i = result_type(rng_());
    }

    [[nodiscard]] constexpr size_t size() const
    {
        return (sizeof(typename RngType::result_type) > sizeof(result_type)
                && RngType::max() > ~size_t(0UL))
             ? ~size_t(0UL)
             : size_t(RngType::max());
    }
};

/*
 * Sometimes you might want a distinct seed based on when the program
 * was compiled.  That way, a particular instance of the program will
 * behave the same way, but when recompiled it'll produce a different
 * value.
 */

template <typename IntType>
struct static_arbitrary_seed {
private:
    static constexpr IntType fnv(IntType hash, const char* pos) {
        return *pos == '\0'
             ? hash
             : fnv((hash * IntType(16777619U)) ^ *pos, (pos+1));
    }

public:
    static constexpr IntType value = fnv(IntType(2166136261U ^ sizeof(IntType)),
                        __DATE__ __TIME__ __FILE__);
};


} // namespace pcg_extras
