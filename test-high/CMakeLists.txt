
function(create_pcgtest TEST_NAME)
    # create test
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
    target_link_libraries(${TEST_NAME} PRIVATE pcg::pcg)

    # define tests
    # add_test(
    #     NAME ${TEST_NAME}
    #     COMMAND $<TARGET_FILE:${TEST_NAME}>
    # )

    # add_custom_command(
    #     TARGET ${TEST_NAME}
    #     POST_BUILD
    #     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/expected
    # )
    
    # add_test(
    #     NAME Test_${TEST_NAME} 
    #     COMMAND ${CMAKE_COMMAND} -E compare_files out.txt ${TEST_NAME}.out
    #     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test
    # )

    add_test(
        NAME ${TEST_NAME}_run
        COMMAND $<TARGET_FILE:${TEST_NAME}> # > ${TEST_NAME}.res
    )
    
    add_test(
        NAME ${TEST_NAME}_check
        # COMMAND ${CMAKE_COMMAND} -E compare_files --ignore-eol test-high/${TEST_NAME}.res ${CMAKE_CURRENT_SOURCE_DIR}/expected/${TEST_NAME}.out
        COMMAND ${CMAKE_COMMAND} -E compare_files --ignore-eol ${TEST_NAME}.res ${CMAKE_CURRENT_SOURCE_DIR}/expected/${TEST_NAME}.out
    )


endfunction()


create_pcgtest(check-pcg32)