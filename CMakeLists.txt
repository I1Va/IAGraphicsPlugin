cmake_minimum_required(VERSION 3.20)
project(AIPlugin LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_library(SDL2_GFX_LIB SDL2_gfx)

option(SANITIZE "Enable compiler sanitizers" OFF)
if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(
        -Wall
        -Wextra
        -Werror
         $<$<BOOL:${SANITIZE}>:-fsanitize=address,undefined>
    )
    add_link_options(
         $<$<BOOL:${SANITIZE}>:-fsanitize=address,undefined>
    )
endif()

#add_subdirectory(external/gui-interface)

add_library(${PROJECT_NAME} SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Common.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Plugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SDLRAII.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Drawable.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/gui-interface/include
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE SDL2::SDL2 SDL2_image::SDL2_image
    PRIVATE SDL2_ttf::SDL2_ttf
    PRIVATE ${SDL2_GFX_LIB} 
)


                      