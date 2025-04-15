#include <iostream>
#include "pcg/pcg_random.hpp"

int main()
{
    pcg32 rng(42);
    std::cout << "A random number: " << rng() << "\n";

    return 0;
}