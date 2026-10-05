cmake_minimum_required(VERSION 3.21)

option(BUILD_GRAPHICS "Build the interactive SFML application" ON)
option(BUILD_TESTING "Build the unit tests" ON)

# Select vcpkg dependencies before project() loads the vcpkg toolchain.
if(BUILD_GRAPHICS)
    list(APPEND VCPKG_MANIFEST_FEATURES "graphics")
endif()
if(BUILD_TESTING)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()

project(SoftwareMatrices LANGUAGES CXX)

add_subdirectory(Common)
add_subdirectory(Matrices)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
