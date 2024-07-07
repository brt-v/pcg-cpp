cmake_minimum_required(VERSION 3.21)

project(pcgrandom CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Define library
add_library(pcg INTERFACE 
   include/pcg/pcg_extras.hpp
   include/pcg/pcg_random.hpp
   include/pcg/pcg_uint128.hpp
)
add_library(pcg::pcg ALIAS pcg)
target_include_directories(pcg INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(pcg INTERFACE cxx_std_17)


if(PROJECT_IS_TOP_LEVEL)
   # Compiler warnings
   if (MSVC)
      add_compile_options(/W3 /WX)
   else()
      add_compile_options(-Wall -Wextra -pedantic -Werror)
   endif()

   # Add samples
   add_subdirectory(sample)

   # Add tests
   include(CTest) 
   enable_testing()
   add_subdirectory(test-high)
endif()