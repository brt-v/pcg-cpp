add_executable(basic_usage basic_usage.cpp)
target_link_libraries(basic_usage PUBLIC pcg::pcg)

add_executable(codebook codebook.cpp)
target_link_libraries(codebook PUBLIC pcg::pcg)

add_executable(cppref-sample cppref-sample.cpp)
target_link_libraries(cppref-sample PUBLIC pcg::pcg)

add_executable(make-partytrick make-partytrick.cpp)
target_link_libraries(make-partytrick PUBLIC pcg::pcg)

add_executable(pcg-demo pcg-demo.cpp)
target_link_libraries(pcg-demo PUBLIC pcg::pcg)

add_executable(spew spew.cpp)
target_link_libraries(spew PUBLIC pcg::pcg)

add_executable(use-partytrick use-partytrick.cpp)
target_link_libraries(use-partytrick PUBLIC pcg::pcg)
