# PCG Random Number Generation, C++ Edition

[PCG-Random website]: http://www.pcg-random.org

This code provides an implementation of the PCG family of random number
generators, which are fast, statistically excellent, and offer a number of
useful features.

Full details can be found at the [PCG-Random website].  This version
of the code provides many family members -- if you just want one
simple generator, you may prefer the minimal C version of the library.

There are two kinds of generator, normal generators and extended generators.
Extended generators provide *k* dimensional equidistribution and can perform
party tricks, but generally speaking most people only need the normal
generators.

There are two ways to access the generators, using a convenience typedef
or by using the underlying templates directly (similar to C++11's `std::mt19937` typedef vs its `std::mersenne_twister_engine` template).  For most users, the convenience typedef is what you want, and probably you're fine with `pcg32` for 32-bit numbers.  If you want 64-bit numbers, either use `pcg64` (or, if you're on a 32-bit system, making 64 bits from two calls to `pcg32_k2` may be faster).

## Documentation and Examples

Visit [PCG-Random website] for information on how to use this library, or look
at the sample code in the `sample` directory -- hopefully it should be fairly
self explanatory.

## Examples

### How to use this library in a project

Example `main.cpp`:
```cpp
#include "pcg/pcg_random.hpp"

int main()
{
    pcg32 rng(42);
    return rng();
}
```

And `CMakeLists.txt`:
```bash
cmake_minimum_required(VERSION 3.21)

project(UsePCG LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare( pcg-cpp
    GIT_REPOSITORY https://github.com/brt-v/pcg-cpp.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(pcg-cpp)

add_executable(UsePCG main.cpp)
target_link_libraries(UsePCG pcg::pcg)

# Select right project in VS Solution
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT UsePCG)
```
Then run:
```bash
cmake . -B build
cmake --build build/
```

### Running tests

Pull this repository, then:
```bash
cmake . -B build
cmake --build build/ --config Release
ctest -C Release --test-dir build/
```


## Directory Structure

The directories are arranged as follows:

* `include` -- contains `pcg/pcg_random.hpp` and supporting include files
* `test-high` -- test code for the high-level API where the functions have
  shorter, less scary-looking names.
* `sample` -- sample code, some similar to the code in `test-high` but more 
  human readable, some other examples too

# About this fork

The PCG random number generators are great, but the C++ implementation is a bit outdated and unmaintained. This fork is made with the following goals:

- [x] Convert library and tests to CMake, such that it's easy to use in your projects
- [x] Convert the tests to CTest, make them run on Linux
  - [ ] and Windows (some tests fails. Output is identical, but object size are different, I suspect due to EBO)
- Clean up code
  - [x] Make sure it compiles with high error levels
  - [x] Remove unneeded includes
  - [ ] Put everything in namespace `pcg`
  - [x] Apply some modern (C++17) best practices
  - [ ] Split some parts in separate headers to make the base RNG's a lightweight include
- [ ] Add Github actions, make sure tests run on Linux and Windows
- [ ] Check with static analysis 
- [ ] Add clang format

