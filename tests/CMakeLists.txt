set (TEST_SOURCES_FILES
    absorption exp interpolation_table3 nboroptimise simple3 testnormalisation2
    associativite filter interpolation_table patriot simple triangle
    comparison2 interpolateur2 intersect polynome sin unstable comparison
    comparison3 interpolateur comparison4 comparison5 comparison6 comparison7
    intersect_linear relative sqrt3 x2 doublemax2 interpolateur_m kahanmuller
    rump sqrt zero_sept doublemax interpolation_table2 nbornaif simple2 sqrtfonct
    square comparison8 comparison_robustness
)

foreach(file ${TEST_SOURCES_FILES})
    add_executable(${file}_exact ${file}.cpp)
    target_include_directories(${file}_exact PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/..)
    target_link_libraries(${file}_exact FloatDiagnosis)
    target_compile_definitions(${file}_exact PUBLIC
        PROG_NAME=${file}
        FLOAT_DIAGNOSIS
        FLOAT_EXACT
        FLOAT_LOOP_UNSTABLE
        FLOAT_SILENT_COMPUTATIONS
        FLOAT_ATOMIC
        ${FLDLIB_CALLOW_INTERFACE}
    )
    target_compile_options(${file}_exact PUBLIC -include ${CMAKE_SOURCE_DIR}/inc/std_header.h)
    add_test(NAME analyze-exact-${file} COMMAND ${file}_exact )
endforeach()

foreach(file ${TEST_SOURCES_FILES})
    add_executable(${file}_interval ${file}.cpp)
    target_include_directories(${file}_interval PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/..)
    target_link_libraries(${file}_interval FloatDiagnosis)
    target_compile_definitions(${file}_interval PUBLIC
        PROG_NAME=${file}
        FLOAT_DIAGNOSIS
        FLOAT_INTERVAL
        FLOAT_LOOP_UNSTABLE
        FLOAT_SILENT_COMPUTATIONS
        FLOAT_ATOMIC
        ${FLDLIB_CALLOW_INTERFACE}
    )
    target_compile_options(${file}_interval PUBLIC -include ${CMAKE_SOURCE_DIR}/inc/std_header.h)
    add_test(NAME analyze-interval-${file} COMMAND ${file}_interval )
endforeach()

foreach(file ${TEST_SOURCES_FILES})
    add_executable(${file}_affine ${file}.cpp)
    target_include_directories(${file}_affine PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/..)
    target_link_libraries(${file}_affine FloatDiagnosis)
    if (${file} STREQUAL "square")
    target_compile_definitions(${file}_affine PUBLIC
        PROG_NAME=${file}
        FLOAT_DIAGNOSIS
        FLOAT_AFFINE
        FLOAT_LOOP_UNSTABLE
        FLOAT_SILENT_COMPUTATIONS
        FLOAT_ATOMIC
        FLOAT_DEFAULT_AFFINE_OPTION
        FLOAT_TRACK
        ${FLDLIB_CALLOW_INTERFACE}
        ${FLDLIB_CALLOW_AFFINE_OPTION}
        ${FLDLIB_CSUPPORT_INT_DOMAIN}
    )
    elseif (${file} STREQUAL "sqrtfonct")
    target_compile_definitions(${file}_affine PUBLIC
        PROG_NAME=${file}
        FLOAT_DIAGNOSIS
        FLOAT_AFFINE
        FLOAT_LOOP_UNSTABLE
        FLOAT_SILENT_COMPUTATIONS
        FLOAT_ATOMIC
        FLOAT_DEFAULT_AFFINE_OPTION
        FLOAT_LIMIT_NOISE_SYMBOLS_NUMBER=20
        ${FLDLIB_CALLOW_INTERFACE}
        ${FLDLIB_CALLOW_AFFINE_OPTION}
        ${FLDLIB_CSUPPORT_INT_DOMAIN}
    )
    else()
    target_compile_definitions(${file}_affine PUBLIC
        PROG_NAME=${file}
        FLOAT_DIAGNOSIS
        FLOAT_AFFINE
        FLOAT_LOOP_UNSTABLE
        FLOAT_SILENT_COMPUTATIONS
        FLOAT_ATOMIC
        FLOAT_DEFAULT_AFFINE_OPTION
        ${FLDLIB_CALLOW_INTERFACE}
        ${FLDLIB_CALLOW_AFFINE_OPTION}
        ${FLDLIB_CSUPPORT_INT_DOMAIN}
    )
    endif()
    target_compile_options(${file}_affine PUBLIC -include ${CMAKE_SOURCE_DIR}/inc/std_header.h)
    if (NOT FLDLIB_AFFINE_ACCELERATION OR NOT(${file} STREQUAL "sqrtfonct"))
    add_test(NAME analyze-affine-${file} COMMAND ${file}_affine )
    endif()
endforeach()

foreach(file ${TEST_SOURCES_FILES})
    add_executable(${file}_affine_scenario ${file}.cpp)
    target_include_directories(${file}_affine_scenario PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/..)
    target_link_libraries(${file}_affine_scenario FloatDiagnosis)
    target_compile_definitions(${file}_affine_scenario PUBLIC
        PROG_NAME=${file}
        FLOAT_DIAGNOSIS
        FLOAT_AFFINE
        FLOAT_SCENARIO
        FLOAT_LOOP_UNSTABLE
        FLOAT_SILENT_COMPUTATIONS
        FLOAT_ATOMIC
        FLOAT_DEFAULT_AFFINE_OPTION
        ${FLDLIB_CALLOW_INTERFACE}
        ${FLDLIB_CALLOW_AFFINE_OPTION}
        ${FLDLIB_CSUPPORT_INT_DOMAIN}
    )
    target_compile_options(${file}_affine_scenario PUBLIC -include ${CMAKE_SOURCE_DIR}/inc/std_header.h)
    add_test(NAME analyze-affine-scenario-${file} COMMAND ${file}_affine_scenario )
endforeach()

