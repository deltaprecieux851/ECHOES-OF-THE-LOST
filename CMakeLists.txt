cmake_minimum_required(VERSION 3.20)
project(EchoesOfTheLost VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive- /EHsc)
    add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(Dependencies)

set(EOL_SOURCES
    src/main.cpp
    src/core/Application.cpp
    src/core/JobSystem.cpp
    src/core/Logger.cpp
    src/game/CombatSystem.cpp
    src/game/EchoSystem.cpp
    src/game/Enemy.cpp
    src/game/HydraulicPuzzle.cpp
    src/game/LevelManager.cpp
    src/game/PlayerController.cpp
    src/platform/Camera.cpp
    src/platform/Input.cpp
    src/platform/Renderer.cpp
    src/platform/Window.cpp
    src/networking/NetworkStub.cpp
)

if(WIN32)
    list(APPEND EOL_SOURCES resources/EchoesOfTheLost.rc)
endif()

add_executable(EchoesOfTheLost WIN32 ${EOL_SOURCES})

target_include_directories(EchoesOfTheLost PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

eol_link_optional(EchoesOfTheLost)

add_custom_command(TARGET EchoesOfTheLost POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:EchoesOfTheLost>/assets
    COMMENT "Copying assets to output directory"
)

install(TARGETS EchoesOfTheLost RUNTIME DESTINATION .)
install(DIRECTORY assets/ DESTINATION assets)
