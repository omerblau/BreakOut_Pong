cmake_minimum_required(VERSION 3.7)
project(BreakoutPong)

set(CMAKE_CXX_STANDARD 17)

add_executable(
        ${PROJECT_NAME}
        main.cpp
        include/bagel_cfg.h
        include/components.h
        include/atlas_bricks_ball.h
        src/game.cpp
        include/game.h
)

set(SDL_STATIC ON)
set(SDL_SHARED OFF)
add_subdirectory(lib/SDL)
target_link_libraries(${PROJECT_NAME} PUBLIC SDL3-static)

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(lib/SDL_image)
target_link_libraries(${PROJECT_NAME} PUBLIC SDL3_image-static)

set(BOX2D_SAMPLES OFF)
set(BOX2D_BENCHMARKS OFF)
set(BOX2D_DOCS OFF)
set(BOX2D_PROFILE OFF)
set(BOX2D_VALIDATE OFF)
set(BOX2D_UNIT_TESTS OFF)
add_subdirectory(lib/box2d)
target_link_libraries(${PROJECT_NAME} PUBLIC box2d)

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E
        copy_directory_if_different
            "${PROJECT_SOURCE_DIR}/res"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/res"
)