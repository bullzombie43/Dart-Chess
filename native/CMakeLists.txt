    cmake_minimum_required(VERSION 3.10)
    project(chess_engine LANGUAGES CXX)

    # Use C++20
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    # ASAN
    # Conditional compilation flags based on build type
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        # Debug: Enable ASan and debug symbols
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -fsanitize=address,undefined -fno-omit-frame-pointer")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -fsanitize=address,undefined -fno-omit-frame-pointer")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address,undefined")
        message(STATUS "Building in DEBUG mode with ASan")
    else()
        # Release: Optimize aggressively, no debug info, no ASan
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -DNDEBUG")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -march=native -DNDEBUG")
        message(STATUS "Building in RELEASE mode with optimizations")
    endif()

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
        src/engine.cpp
    )

    #Bridge source files (NEW!)
    set(BRIDGE_SOURCES
        bridge/chess_bridge.cpp
        src/board.cpp
        src/utils.cpp
        src/engine.cpp
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

    add_executable(
        engine_test
        src/engine_test.cc
        src/board.cpp
        src/utils.cpp
        src/engine.cpp
    )

    target_link_libraries(
        engine_test
        GTest::gtest_main
    )

    #Bridge test executable (NEW!)
    add_executable(
        bridge_test
        bridge/chess_bridge_test.cpp
        ${BRIDGE_SOURCES}
    )

    target_link_libraries(
        bridge_test
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(board_test)
    gtest_discover_tests(engine_test)
    gtest_discover_tests(bridge_test)
    
    # Create executable
    add_executable(chess_engine ${SOURCES}) 

    # ============================================================================
    # SHARED LIBRARY FOR FLUTTER FFI (NEW!)
    # ============================================================================

    # Build bridge as a shared library
    add_library(chess_bridge SHARED ${BRIDGE_SOURCES})

    # Set library output name (will create libchess_bridge.dylib on macOS, etc.)
    set_target_properties(chess_bridge PROPERTIES
        OUTPUT_NAME "chess_bridge"
        VERSION 1.0.0
        SOVERSION 1
    )

    # On macOS, set the install name for the dylib
    if(APPLE)
        set_target_properties(chess_bridge PROPERTIES
            MACOSX_RPATH TRUE
            INSTALL_NAME_DIR "@rpath"
        )
    endif()

    # Install the library (optional, for now just build it)
    install(TARGETS chess_bridge
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )