find_package(glm CONFIG REQUIRED)

add_library(matrices_core STATIC
    src/matrices.cpp
)

target_link_libraries(matrices_core PUBLIC
    common_drawing
    glm::glm
)
target_include_directories(matrices_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_compile_features(matrices_core PUBLIC cxx_std_20)

if(BUILD_GRAPHICS)
    find_package(SFML COMPONENTS System Window Graphics CONFIG REQUIRED)
    find_package(assimp CONFIG REQUIRED)

    add_executable(Matrices
        src/main.cpp
    )

    target_link_libraries(Matrices PRIVATE
        matrices_core
        SFML::System
        SFML::Window
        SFML::Graphics
        assimp::assimp
    )
    target_compile_features(Matrices PRIVATE cxx_std_20)

    add_custom_command(TARGET Matrices POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:Matrices>/models"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/models/bunny.obj"
            "$<TARGET_FILE_DIR:Matrices>/models/bunny.obj"
        COMMENT "Copying the Bunny model beside the Matrices executable"
    )
endif()
