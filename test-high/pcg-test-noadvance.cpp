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
 * This file is based on the demo program for the C generation schemes.
 * It shows some basic generation tasks.
 */

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <climits>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <climits>
#include <random>       // for random_device

#include "pcg/pcg_random.hpp"

// This code can be compiled with the preprocessor symbol RNG set to the
// PCG generator you'd like to test.

#ifndef RNG
    #define RNG pcg32
    #define TWO_ARG_INIT 1
#endif

#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x)      STRINGIFY_IMPL(x)

using namespace std;
using pcg_extras::operator<<;

#if !PCG_EMULATED_128BIT_MATH || !AWKWARD_128BIT_CODE

int main(int argc, char** argv)
{
    // Write output to file
    std::ofstream file("check-" STRINGIFY(RNG) ".out", std::ios::binary);
    if (!file) {
        return -1;
    }

    // Read command-line options
    int rounds = 5;
    bool nondeterministic_seed = false;

    ++argv;
    --argc;
    if (argc > 0 && strcmp(argv[0], "-r") == 0) {
        nondeterministic_seed = true;
        ++argv;
        --argc;
    }
    if (argc > 0) {
        rounds = atoi(argv[0]);
    }

    /* Many of the generators can be initialized with two arguments; the second
     * one specifies the stream.
     */

#if TWO_ARG_INIT
    RNG rng(42u, 54u);
#else
    RNG rng(42u);
#endif

    if (nondeterministic_seed) {
        // Seed with external entropy from std::random_device (a handy
        // utility provided by pcg_extras).
        rng.seed(pcg_extras::seed_seq_from<std::random_device>());
    }

    constexpr auto bits = sizeof(RNG::result_type) * CHAR_BIT;
    constexpr int how_many_nums = bits <= 8  ? 14
                                : bits <= 16 ? 10
                                :              6;
    constexpr int wrap_nums_at  = bits >  64 ? 2
                                : bits >  32 ? 3
                                :              how_many_nums;

    file << STRINGIFY(RNG) << ":\n"
    //   << "      -  aka:         " << pcg_extras::printable_typename<RNG>()
    // ^-- we skip this line because the output is long, scary, ugly, and
    //     and varies depending on the platform
         << "      -  result:      " <<  bits << "-bit unsigned int\n"
         << "      -  period:      2^" << RNG::period_pow2();
    if (RNG::streams_pow2() > 0) {
         file << "   (* 2^" << RNG::streams_pow2() << " streams)";
    }
    file << "\n      -  size:        " << sizeof(RNG) << " bytes\n\n";

    for (int round = 1; round <= rounds; ++round) {
        file << "Round " << round << ":\n";

        /* Make some N-bit numbers */
        file << setw(4) << setfill(' ') << bits << "bit:";
        for (int i = 0; i < how_many_nums; ++i) {
            if (i > 0 && i % wrap_nums_at == 0)
                file << "\n\t";
            file << " 0x" << hex << setfill('0') 
                 << setw(sizeof(RNG::result_type)*2) << rng();
        }
        file << dec << endl;

        /* Toss some coins */
        file << "  Coins: ";
        for (int i = 0; i < 65; ++i) {
            file << (rng(2) ? "H" : "T");
        }
        file << endl;

        /* Roll some dice */
        file << "  Rolls:";
        for (int i = 0; i < 33; ++i) {
            file << " " << (uint32_t(rng(6)) + 1);
        }

        /* Deal some cards using pcg_extras::shuffle, which follows
         * the algorithm for shuffling that most programmers would expect.
         * (It's unspecified how std::shuffle works.)
         */      
        enum { SUITS = 4, NUMBERS = 13, CARDS = 52 };
        char cards[CARDS];
        std::iota(begin(cards), end(cards), 0);
        pcg_extras::shuffle(begin(cards), end(cards), rng);

        /* Output the shuffled deck */
        file << "\n  Cards:";
        static const signed char number[] = {'A', '2', '3', '4', '5', '6', '7',
                                             '8', '9', 'T', 'J', 'Q', 'K'};
        static const signed char suit[] =   {'h', 'c', 'd', 's'};
        int i = 0;
        for (auto card : cards) {
            ++i;
            file << " " << number[card / SUITS] << suit[card % SUITS];
            if (i % 22 == 0) {
                file << "\n\t";
            }
        }
        
        file << "\n" << endl;
    }

    return 0;
}

#else //  i.e. PCG_EMULATED_128BIT_MATH && AWKWARD_128BIT_CODE

int main()
{
    // It's not that it *can't* be done, it just requires either C++14-style
    // constexpr or some things not to be declared const.
    cout << "Sorry, " STRINGIFY(RNG) " not supported with emulated 128-bit math"
         << endl;
}

#endif
