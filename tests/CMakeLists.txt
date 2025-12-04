# packages


enable_testing()
find_package(GTest REQUIRED)
find_package(yaml-cpp REQUIRED)

function(add_unit_test TEST_FILENAME_WITHOUT_CPP)
    add_executable(${TEST_FILENAME_WITHOUT_CPP} ${TEST_FILENAME_WITHOUT_CPP}.cpp)

    set_target_properties(${TEST_FILENAME_WITHOUT_CPP}
        PROPERTIES
            CXX_STANDARD_REQUIRED       ON
            CXX_STANDARD                23
            CXX_EXTENSIONS              OFF
            LINKER_LANGUAGE             CXX
    )

    # compiler flags
    target_compile_options(${TEST_FILENAME_WITHOUT_CPP}
        PRIVATE
            -Wall
            -Wextra
            -Werror
            -Wcast-align
            -Wcast-qual
            -Wuninitialized
            -Wmissing-include-dirs
            -Wuseless-cast
            -Wstring-compare
            -Wconversion

            -fexceptions
            -fstrict-aliasing
            -fdiagnostics-color=always
            -fconcepts-diagnostics-depth=5
    )

        # optimization flags
    target_compile_options(${TEST_FILENAME_WITHOUT_CPP}
        PRIVATE
            $<$<CONFIG:Debug>:-O0 -ggdb>
            $<$<CONFIG:Release>:-O3>
            $<$<CONFIG:RelWithDebInfo>:-O2 -g>
    )

    target_link_libraries(${TEST_FILENAME_WITHOUT_CPP}
        PRIVATE
            yaml-cpp
            GTest::gtest
            GTest::gtest_main
            pthread
            -l:libgmock.a # TODO: Resolve without specifying this library
    )

    add_test(${TEST_FILENAME_WITHOUT_CPP} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_FILENAME_WITHOUT_CPP}")
endfunction()

add_unit_test(test_reflection)
add_unit_test(test_with_yaml)
