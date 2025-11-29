# packages
find_package(Boost REQUIRED COMPONENTS HEADERS)
enable_testing()
find_package(GTest REQUIRED)

function(add_unit_test TEST_FILENAME_WITHOUT_CPP)
    add_executable(${TEST_FILENAME_WITHOUT_CPP} ${TEST_FILENAME_WITHOUT_CPP}.cpp)

    set_target_properties(${TEST_FILENAME_WITHOUT_CPP}
        PROPERTIES
            CXX_STANDARD_REQUIRED       ON
            CXX_STANDARD                23
            CXX_EXTENSIONS              OFF
            LINKER_LANGUAGE             CXX
            RUNTIME_OUTPUT_DIRECTORY    "${PROJECT_SOURCE_DIR}/build/bin"
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
            # -fconcepts-diagnostics-depth=5
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
            boost::boost
            GTest::gtest_main
    )
endfunction()

add_unit_test(test_reflection)
