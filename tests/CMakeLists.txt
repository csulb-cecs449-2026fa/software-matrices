cmake_minimum_required(VERSION 3.21)

project(InstructorTests LANGUAGES CXX)

# Classroom 50 configures this directory directly and supplies REPO_ROOT.
# Fall back to the repository parent so the regular root build still works.
if(NOT DEFINED REPO_ROOT)
    get_filename_component(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
endif()
file(TO_CMAKE_PATH "${REPO_ROOT}" REPO_ROOT)

foreach(requiredDirectory
    Common/src
    Common/include
    Matrices/src
    Matrices/include
)
    if(NOT IS_DIRECTORY "${REPO_ROOT}/${requiredDirectory}")
        message(FATAL_ERROR
            "REPO_ROOT does not contain ${requiredDirectory}: ${REPO_ROOT}")
    endif()
endforeach()

find_package(GTest CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

file(GLOB COMMON_SOURCES CONFIGURE_DEPENDS
    "${REPO_ROOT}/Common/src/*.cpp"
)
file(GLOB MATRICES_SOURCES CONFIGURE_DEPENDS
    "${REPO_ROOT}/Matrices/src/*.cpp"
)

# The SFML entry point is not part of the code under test.
list(FILTER MATRICES_SOURCES EXCLUDE REGEX "/main\\.cpp$")

if(NOT COMMON_SOURCES)
    message(FATAL_ERROR
        "No supplied implementation files found under ${REPO_ROOT}/Common/src")
endif()
if(NOT MATRICES_SOURCES)
    message(FATAL_ERROR
        "No student implementation files found under ${REPO_ROOT}/Matrices/src")
endif()

add_executable(instructor_tests
    CommonTests/src/framebuffer_tests.cpp
    CommonTests/src/line_tests.cpp
    CommonTests/src/triangle_wireframe_tests.cpp
    MatricesTests/src/matrices_tests.cpp
    ${COMMON_SOURCES}
    ${MATRICES_SOURCES}
)

target_link_libraries(instructor_tests PRIVATE
    GTest::gtest_main
    glm::glm
)
target_include_directories(instructor_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${REPO_ROOT}/Common/include"
    "${REPO_ROOT}/Matrices/include"
)
target_compile_features(instructor_tests PRIVATE cxx_std_20)

enable_testing()
add_test(NAME instructor_tests COMMAND instructor_tests)
