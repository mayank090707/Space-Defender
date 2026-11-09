cmake_minimum_required(VERSION 3.20)
project(SpaceDefender LANGUAGES CXX)

# Enforce C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Fetch SFML 2.6.2 if not already present
include(FetchContent)
message(STATUS "Configuring SFML via FetchContent...")
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.2
    GIT_SHALLOW TRUE
)

# SFML build configurations (static library build)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)
set(SFML_BUILD_AUDIO ON CACHE BOOL "Build SFML Audio" FORCE)
set(SFML_BUILD_GRAPHICS ON CACHE BOOL "Build SFML Graphics" FORCE)
set(SFML_BUILD_WINDOW ON CACHE BOOL "Build SFML Window" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "Build SFML Network" FORCE)

FetchContent_MakeAvailable(SFML)

# Project executable
add_executable(SpaceDefender
    src/main.cpp
    src/Game.cpp
    src/Player.cpp
    src/Bullet.cpp
    src/Enemy.cpp
    src/Leaderboard.cpp
    src/AudioManager.cpp
    src/VisualEffects.cpp
)

# Include directories
target_include_directories(SpaceDefender PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Link SFML libraries
target_link_libraries(SpaceDefender PRIVATE
    sfml-graphics
    sfml-audio
    sfml-window
    sfml-system
)

# Enforce C++17 compile features and static runtime linking
target_compile_features(SpaceDefender PRIVATE cxx_std_17)
target_link_options(SpaceDefender PRIVATE -static-libgcc -static-libstdc++ -static)

# Test executable for Phase 7 automated tests
add_executable(TestPhase7
    scratch/test_phase7.cpp
    src/AudioManager.cpp
    src/VisualEffects.cpp
    src/Leaderboard.cpp
    src/Bullet.cpp
    src/Enemy.cpp
    src/Player.cpp
)
target_include_directories(TestPhase7 PRIVATE include)
target_link_libraries(TestPhase7 PRIVATE sfml-graphics sfml-audio sfml-window sfml-system)
target_compile_features(TestPhase7 PRIVATE cxx_std_17)
target_link_options(TestPhase7 PRIVATE -static-libgcc -static-libstdc++ -static)

# Master Regression & Benchmark Test executable for Phase 8
add_executable(TestMaster
    scratch/test_master_regression.cpp
    src/Game.cpp
    src/AudioManager.cpp
    src/VisualEffects.cpp
    src/Leaderboard.cpp
    src/Bullet.cpp
    src/Enemy.cpp
    src/Player.cpp
)
target_include_directories(TestMaster PRIVATE include)
target_link_libraries(TestMaster PRIVATE sfml-graphics sfml-audio sfml-window sfml-system)
target_compile_features(TestMaster PRIVATE cxx_std_17)
target_link_options(TestMaster PRIVATE -static-libgcc -static-libstdc++ -static)

# Automatically copy OpenAL DLL on Windows for SFML Audio
if(WIN32)
    add_custom_command(TARGET SpaceDefender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${sfml_SOURCE_DIR}/extlibs/bin/x64/openal32.dll"
        $<TARGET_FILE_DIR:SpaceDefender>
    )
    add_custom_command(TARGET TestPhase7 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${sfml_SOURCE_DIR}/extlibs/bin/x64/openal32.dll"
        $<TARGET_FILE_DIR:TestPhase7>
    )
    add_custom_command(TARGET TestMaster POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${sfml_SOURCE_DIR}/extlibs/bin/x64/openal32.dll"
        $<TARGET_FILE_DIR:TestMaster>
    )
endif()
