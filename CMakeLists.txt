cmake_minimum_required (VERSION 3.15) 
project (fldlib
        VERSION 0.95
        DESCRIPTION "guaranteed propagation of round-off errors"
        LANGUAGES CXX) 

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(utils algorithms applications)

set(FLDLIB_GENERICITY "LONG" CACHE STRING "Whether or not mantissa bits are stored in a generic array of uint64_t or uint32_t")
set_property(CACHE FLDLIB_GENERICITY PROPERTY STRINGS "OFF" "LONG" "UNSIGNED")

if (FLDLIB_GENERICITY STREQUAL "LONG")
set(FLDLIBGENERIC_LONG ON)
elseif (FLDLIB_GENERICITY STREQUAL "UNSIGNED")
set(FLDLIBGENERIC_UNSIGNED ON)
endif()
# option(FLDLIB_GENERIC_LONG "Whether or not mantissa bits are stored in a generic array of uint64_t (instead of a custom array of uint32_t) - this excludes FLDLIB_GENERIC_UNSIGNED" ON)
# option(FLDLIB_GENERIC_UNSIGNED "Whether or not mantissa bits are stored in a generic array of uint32_t (instead of a custom array of uint32_t - this excludes FLDLIB_GENERIC_LONG)" OFF)

option(FLDLIB_ALLOW_INTERFACE "Whether or not fldlib uses a dynamic library or inline instrumentation" OFF)
option(FLDLIB_ALLOW_READ_EXCEPTION "Whether or not fldlib allows read exception if file_in (unstable branching in mode non unloop analysis) is altered" OFF)
option(FLDLIB_LONG_WRITE "Whether or not fldlib prints real numbers in decimal or in hexa" ON)
set(FLDLIB_REAL_BITS_NUMBER "123" CACHE STRING "number of bits for the mantissa of real numbers (avoid power of 32) - default is 123")
option(FLDLIB_ZONOTOPE_ALLOW_SIMPLEX "Whether or not fldlib pushes the constraints onto a stack to apply them in an optimal but costly way" OFF)
option(FLDLIB_ZONOTOPE_DOES_ABSORB_HIGH_LEVEL "Whether or not fldlib systematically absorbs high level symbols into standard noise symbols" OFF)
option(FLDLIB_ZONOTOPE_DOES_EXCLUDE_CONSTANT_FROM_SYMBOL_ABSORPTION "Whether or not fldlib excludes the constant part in the definition of negligeable symbols for their absorption into standard noise symbols" OFF)
set(FLDLIB_ZONOTOPE_LIMIT_SYMBOL_ABSORPTION "0" CACHE STRING "define the limit in binary digits from which a symbol is absorbed into a standard noise symbol")
set(FLDLIB_ZONOTOPE_LIMIT_NUMBER_OF_SYMBOLS "0" CACHE STRING "define a limit for the number of symbols in every affine form")
option(FLDLIB_CONCRETE "Whether or not a concrete value is propagated for fldlib internal verification" ON)
option(FLDLIB_AFFINE_OPTION "Whether or fldlib enables optional affine forms mixed with concrete values" OFF)
option(FLDLIB_AFFINE_ACCELERATION "Whether or fldlib use double instead of conservative coefficients for affine forms to accelerate analysis feedback" OFF)
option(FLDLIB_SUPPORT_INT_DOMAIN "Whether or fldlib supports conditional domains mixed with affine forms" OFF)

set(FLOAT_GENERIC_BASE_LONG ${FLDLIBGENERIC_LONG})
set(FLOAT_GENERIC_BASE_UNSIGNED ${FLDLIBGENERIC_UNSIGNED})
set(FLOAT_INTERFACE ${FLDLIB_ALLOW_INTERFACE})
if (FLDLIB_ALLOW_INTERFACE)
set(FLOAT_ALLOW_INTERFACE "-DFLOAT_INTERFACE")
set(FLDLIB_CALLOW_INTERFACE "-DFLOAT_INTERFACE")
set(FLDLIBLDFLAGS "-L\$\{FLOATDIAGNOSIS_LIB\} -Wl,-rpath,\$\{FLOATDIAGNOSIS_LIB\} -lm -lFloatDiagnosis" ${CMAKE_SHARED_LINKER_FLAGS})
set(FLDLIBAUTO_LDFLAGS ${FLDLIBLDFLAGS})
else(FLDLIB_ALLOW_INTERFACE)
set(FLDLIBLDFLAGS "-L\$\{FLOATDIAGNOSIS_LIB\}/fldlib -lm -lFloatDiagnosis" ${CMAKE_STATIC_LINKER_FLAGS})
set(FLDLIBAUTO_LDFLAGS "-L\$\{FLOATDIAGNOSIS_LIB\} -lm -lFloatDiagnosis" ${CMAKE_STATIC_LINKER_FLAGS})
endif(FLDLIB_ALLOW_INTERFACE)
if (FLDLIB_AFFINE_OPTION)
set(FLDLIB_CALLOW_AFFINE_OPTION "-DFLOAT_AFFINE_OPTION")
endif()
if (FLDLIB_SUPPORT_INT_DOMAIN)
set(FLDLIB_CSUPPORT_INT_DOMAIN "-DFLOAT_SUPPORT_INT_DOMAIN")
endif()
set(FLOAT_ALLOW_READ_EXCEPTION ${FLDLIB_ALLOW_READ_EXCEPTION})
set(FLOAT_LONG_WRITE ${FLDLIB_LONG_WRITE})
set(FLOAT_CONCRETE ${FLDLIB_CONCRETE})
set(FLOAT_AFFINE_ACCELERATION ${FLDLIB_AFFINE_ACCELERATION})

if (FLDLIB_REAL_BITS_NUMBER)
set(FLOAT_REAL_BITS_NUMBER ${FLDLIB_REAL_BITS_NUMBER})
else()
set(FLOAT_REAL_BITS_NUMBER 123)
endif()

set(FLOAT_ZONOTOPE_ALLOW_SIMPLEX ${FLDLIB_ZONOTOPE_ALLOW_SIMPLEX})
set(FLOAT_ZONOTOPE_DOES_ABSORB_HIGH_LEVEL ${FLDLIB_ZONOTOPE_DOES_ABSORB_HIGH_LEVEL})
set(FLOAT_ZONOTOPE_DOES_EXCLUDE_CONSTANT_FROM_SYMBOL_ABSORPTION ${FLDLIB_ZONOTOPE_DOES_EXCLUDE_CONSTANT_FROM_SYMBOL_ABSORPTION})
set(FLOAT_ZONOTOPE_LIMIT_SYMBOL_ABSORPTION ${FLDLIB_ZONOTOPE_LIMIT_SYMBOL_ABSORPTION})
set(FLOAT_ZONOTOPE_LIMIT_NUMBER_OF_SYMBOLS ${FLDLIB_ZONOTOPE_LIMIT_NUMBER_OF_SYMBOLS})
set(FLOAT_AFFINE_OPTION ${FLDLIB_AFFINE_OPTION})

set(CMAKE_HAVE_CLANG 0)
set(CMAKE_HAVE_GNUC 0)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
set(CMAKE_HAVE_CLANG 1)
set(FLDLIBCXX clang++)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
set(FLDLIBCXX g++)
endif()

configure_file(inc/fldlib_config.h.in fldlib_config.h)
configure_file(tests/comp_float_diagnosis.sh.in comp_float_diagnosis.sh @ONLY)

option(FLDLIB_ENABLE_TESTS "Whether or not fldlib tests are run" OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

function(dump_cmake_variables)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        if ((NOT DEFINED ARGV0) OR _variableName MATCHES ${ARGV0})
            message(STATUS "${_variableName}=${${_variableName}}")
        endif()
    endforeach()
endfunction()

dump_cmake_variables(FLDLIB_)

set (FLDLIB_UTILS_STANDARD_CLASSES_PUBLIC_HEADERS
    utils/StandardClasses/StandardMessage.h
    utils/StandardClasses/StandardClasses.hpp
    utils/StandardClasses/StandardClasses.macro
    utils/StandardClasses/IOStreamMethods.inch
    utils/StandardClasses/Persistence.h
    utils/StandardClasses/DefineNew.h
    utils/StandardClasses/UndefineNew.h
)

set (FLDLIB_UTILS_STANDARD_CLASSES_SRC_DEPENDENCIES
    utils/StandardClasses/IOStreamMethods.incc
)

set (FLDLIB_UTILS_STANDARD_CLASSES_SRC
    utils/StandardClasses/StandardClasses.cpp
    utils/StandardClasses/Persistence.cpp
)

set (FLDLIB_UTILS_POINTER_SRC
    utils/Pointer/Pointer.cpp
    utils/Pointer/MngPointer.cpp
    utils/Pointer/ImplArray.cpp
    utils/Pointer/ImplList.cpp
    utils/Pointer/SharedPointer.cpp
    utils/Pointer/SharedCollection.cpp
    utils/Pointer/Binary.cpp
)

set (FLDLIB_UTILS_COLLECTION_SRC
    utils/Collection/VirtualCollection/VirtualCollection.cpp
    utils/Collection/VirtualCollection/VirtualTree.cpp
    utils/Collection/Implementation/BinaryParent.cpp
    utils/Collection/Implementation/ImplTree.cpp
    utils/Collection/ConcreteCollection/BasicList.cpp
    utils/Collection/ConcreteCollection/List.cpp
    utils/Collection/ConcreteCollection/Array.cpp
    utils/Collection/ConcreteCollection/SortedArray.cpp
    utils/Collection/ConcreteCollection/MultiArray.cpp
    utils/Collection/ConcreteCollection/BasicSortedAVL.cpp
    utils/Collection/ConcreteCollection/SortedAVL.cpp
    utils/Collection/ConcreteCollection/Special.cpp
    utils/Collection/ConcreteCollection/Tree.cpp
)

set (FLDLIB_ALGORITHMS_NUMERICS_SRC
    algorithms/Numerics/HostFloating.cpp
    algorithms/Numerics/Integer.cpp
)

set (FLDLIB_NUMERICAL_ANALYSIS_SRC
    applications/NumericalAnalysis/BackTrace.cpp
)

set (FLDLIB_FLOAT_INSTRUMENTATION_SRC
    src/FloatAffineTypes.cpp
    src/FloatExactTypes.cpp
    src/FloatIntervalTypes.cpp
    src/FloatAffine.cpp
    src/FloatExact.cpp
    src/FloatInterval.cpp
)

set (FLDLIB_FLOAT_INSTRUMENTATION_INTERFACE_SRC
    src/FloatAffineTypes.cpp
    src/FloatExactTypes.cpp
    src/FloatIntervalTypes.cpp
    src/FloatAffineInterface.cpp
    src/FloatExactInterface.cpp
    src/FloatIntervalInterface.cpp
)

if(FLDLIB_ALLOW_INTERFACE)

add_executable(TypesSizes
    ${FLDLIB_UTILS_STANDARD_CLASSES_SRC}
    ${FLDLIB_UTILS_POINTER_SRC}
    ${FLDLIB_UTILS_COLLECTION_SRC}
    ${FLDLIB_ALGORITHMS_NUMERICS_SRC}
    ${FLDLIB_NUMERICAL_ANALYSIS_SRC}
    src/TypesSizes.cpp
)

target_include_directories(TypesSizes PUBLIC inc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
if(CMAKE_BUILD_TYPE MATCHES Release)
add_definitions(-DDefineDebugLevel=1)
endif()

add_custom_command(
    OUTPUT
        obj_interface/AffineTypesSize.h
        obj_interface/ExactTypesSize.h
        obj_interface/IntervalTypesSize.h
    COMMAND mkdir -p obj_interface & ${CMAKE_CURRENT_BINARY_DIR}/TypesSizes
    DEPENDS TypesSizes
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(TypesSizesInterface
    DEPENDS
        obj_interface/AffineTypesSize.h
        obj_interface/ExactTypesSize.h
        obj_interface/IntervalTypesSize.h
)

add_library(FloatDiagnosis SHARED
    ${FLDLIB_UTILS_STANDARD_CLASSES_SRC}
    ${FLDLIB_UTILS_POINTER_SRC}
    ${FLDLIB_UTILS_COLLECTION_SRC}
    ${FLDLIB_ALGORITHMS_NUMERICS_SRC}
    ${FLDLIB_NUMERICAL_ANALYSIS_SRC}
    ${FLDLIB_FLOAT_INSTRUMENTATION_INTERFACE_SRC}
)
add_dependencies(FloatDiagnosis TypesSizesInterface)

else(FLDLIB_ALLOW_INTERFACE)

if (FLDLIB_AFFINE_OPTION)
add_library(FloatDiagnosis
    ${FLDLIB_UTILS_STANDARD_CLASSES_SRC}
    ${FLDLIB_UTILS_POINTER_SRC}
    ${FLDLIB_UTILS_COLLECTION_SRC}
    ${FLDLIB_ALGORITHMS_NUMERICS_SRC}
    ${FLDLIB_NUMERICAL_ANALYSIS_SRC}
    ${FLDLIB_FLOAT_INSTRUMENTATION_SRC}
    src/FloatAffineOption.cpp
)
else(FLDLIB_AFFINE_OPTION)
add_library(FloatDiagnosis
    ${FLDLIB_UTILS_STANDARD_CLASSES_SRC}
    ${FLDLIB_UTILS_POINTER_SRC}
    ${FLDLIB_UTILS_COLLECTION_SRC}
    ${FLDLIB_ALGORITHMS_NUMERICS_SRC}
    ${FLDLIB_NUMERICAL_ANALYSIS_SRC}
    ${FLDLIB_FLOAT_INSTRUMENTATION_SRC}
)
endif(FLDLIB_AFFINE_OPTION)

endif(FLDLIB_ALLOW_INTERFACE)

if(FLDLIB_ALLOW_INTERFACE)
file(GLOB_RECURSE FLOATDIAGNOSIS_INCLUDE_FILES "inc/*Interface.h"
        "inc/float_diagnosis.h" "inc/std_header.h")
else(FLDLIB_ALLOW_INTERFACE)
set(FLOATDIAGNOSIS_INCLUDE_FILES
        "inc/FloatExact.h" "inc/FloatInterval.h" "inc/FloatAffine.h" "inc/FloatAffineOption.h"
        "inc/float_diagnosis.h" "inc/std_header.h")
endif(FLDLIB_ALLOW_INTERFACE)

target_include_directories(FloatDiagnosis PUBLIC inc PRIVATE ${CMAKE_CURRENT_BINARY_DIR} )
set_target_properties(FloatDiagnosis
    PROPERTIES
    PUBLIC_HEADER "${FLOATDIAGNOSIS_INCLUDE_FILES}"
)

if(FLDLIB_ENABLE_TESTS)
    enable_testing()
    include(CTest)

    add_subdirectory(tests)
endif()

## Install
install(TARGETS FloatDiagnosis
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/fldlib
        PUBLIC_HEADER DESTINATION include/fldlib)

if (NOT FLDLIB_ALLOW_INTERFACE)
install(DIRECTORY utils DESTINATION include/fldlib
        REGEX "/[^/]*.cpp" EXCLUDE
        REGEX "/[^/]*.incc" EXCLUDE
        REGEX "/[^/]*.template" EXCLUDE
        REGEX "/[^/]*.inctemplate" EXCLUDE)

install(DIRECTORY algorithms DESTINATION include/fldlib
        REGEX "/[^/]*.cpp" EXCLUDE
        REGEX "/[^/]*.incc" EXCLUDE
        REGEX "/[^/]*.template" EXCLUDE
        REGEX "/[^/]*.inctemplate" EXCLUDE)

install(DIRECTORY applications DESTINATION include/fldlib
        REGEX "/[^/]*.cpp" EXCLUDE
        REGEX "/[^/]*.incc" EXCLUDE
        REGEX "/[^/]*.template" EXCLUDE
        REGEX "/[^/]*.inctemplate" EXCLUDE)
else(NOT FLDLIB_ALLOW_INTERFACE)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/obj_interface DESTINATION include/fldlib)
endif(NOT FLDLIB_ALLOW_INTERFACE)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fldlib_config.h DESTINATION include/fldlib)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/comp_float_diagnosis.sh DESTINATION bin)
