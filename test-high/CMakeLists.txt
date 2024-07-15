
function(create_pcgtest TEST_NAME)
    set(RUN_ON_WINDOWS ON )
    if (ARGC EQUAL 2)
        set(RUN_ON_WINDOWS ${ARGV1}) # Optional 2nd argument to disable test on Windows
    endif()

    # create test
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
    target_link_libraries(${TEST_NAME} PRIVATE pcg::pcg)

    # For Visual Sutio
    set_target_properties(${TEST_NAME} PROPERTIES FOLDER "test-high")

    # Runs a test and writes output to a check-*.res file
    add_test(
        NAME ${TEST_NAME}_run
        COMMAND $<TARGET_FILE:${TEST_NAME}>
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/actual
    )
    
    if (UNIX OR (WIN32 AND RUN_ON_WINDOWS))
        # Compare .res file to expected output
        add_test(
            NAME ${TEST_NAME}_check
            COMMAND ${CMAKE_COMMAND} -E compare_files --ignore-eol ${TEST_NAME}.out ${CMAKE_CURRENT_SOURCE_DIR}/expected/${TEST_NAME}.out
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/actual
        )
    endif()
endfunction()

add_custom_target(create-output-directory ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory actual)

create_pcgtest(check-pcg128_once_insecure)
create_pcgtest(check-pcg128_oneseq_once_insecure)
create_pcgtest(check-pcg16_once_insecure)
create_pcgtest(check-pcg16_oneseq_once_insecure)
create_pcgtest(check-pcg32)
create_pcgtest(check-pcg32_c1024)
create_pcgtest(check-pcg32_c1024_fast)
create_pcgtest(check-pcg32_c64)
create_pcgtest(check-pcg32_c64_fast)
create_pcgtest(check-pcg32_c64_oneseq)
create_pcgtest(check-pcg32_fast)
create_pcgtest(check-pcg32_k1024)
create_pcgtest(check-pcg32_k1024_fast)
create_pcgtest(check-pcg32_k16384)
create_pcgtest(check-pcg32_k16384_fast)
create_pcgtest(check-pcg32_k2)
create_pcgtest(check-pcg32_k2_fast)
create_pcgtest(check-pcg32_k64)
create_pcgtest(check-pcg32_k64_fast)
create_pcgtest(check-pcg32_k64_oneseq)
create_pcgtest(check-pcg32_once_insecure)
create_pcgtest(check-pcg32_oneseq)
create_pcgtest(check-pcg32_oneseq_once_insecure)
create_pcgtest(check-pcg64)
create_pcgtest(check-pcg64_c1024 OFF)
create_pcgtest(check-pcg64_c1024_fast OFF)
create_pcgtest(check-pcg64_c32 OFF)
create_pcgtest(check-pcg64_c32_fast OFF)
create_pcgtest(check-pcg64_c32_oneseq OFF)
create_pcgtest(check-pcg64_fast)
create_pcgtest(check-pcg64_k1024 OFF)
create_pcgtest(check-pcg64_k1024_fast OFF)
create_pcgtest(check-pcg64_k32 OFF)
create_pcgtest(check-pcg64_k32_fast OFF)
create_pcgtest(check-pcg64_k32_oneseq OFF)
create_pcgtest(check-pcg64_once_insecure)
create_pcgtest(check-pcg64_oneseq)
create_pcgtest(check-pcg64_oneseq_once_insecure)
create_pcgtest(check-pcg8_once_insecure)
create_pcgtest(check-pcg8_oneseq_once_insecure)
