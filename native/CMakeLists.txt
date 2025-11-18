    cmake_minimum_required(VERSION 3.10)
    project(chess_engine LANGUAGES CXX)

    # Use C++20
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    # ASAN
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -fsanitize=address,undefined -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -fsanitize=address,undefined -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ggdb -fsanitize=address,undefined -fno-omit-frame-pointer")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ggdb -fsanitize=address,undefined -fno-omit-frame-pointer")

    #GTest
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # Include directories
    include_directories(include)

    # Source files
    set(SOURCES
        src/main.cpp
        src/board.cpp
        src/utils.cpp
    )

    #GTest Executable
    enable_testing()

    add_executable(
        board_test
        src/board_test.cc
        src/board.cpp
        src/utils.cpp
    )

    target_link_libraries(
        board_test
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(board_test)
    
    # Create executable
    add_executable(chess_engine ${SOURCES}) 
