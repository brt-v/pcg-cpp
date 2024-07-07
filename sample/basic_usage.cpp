#include <iostream>
#include "pcg/pcg_random.hpp"

int main()
{
    pcg32 rng(42);
    std::cout << rng << "\n";
    std::cout << "A random numer: " << rng() << "\n";

    return 0;
}