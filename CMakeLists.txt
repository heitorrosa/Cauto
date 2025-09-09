cmake_minimum_required(VERSION 4.0.3)

project(Cauto VERSION 0.0.0 LANGUAGES C)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

# Set C standard
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Create the main executable
add_executable(Cauto 
    src/main.c
    src/clicker/clicker.c
    src/player/player.c
    src/recorder/recorder.c
    src/utils/utils.c
)

# Include directories
target_include_directories(Cauto PRIVATE src)

# Basic Windows libraries
target_link_libraries(Cauto winmm advapi32)

# Set compiler flags for Windows
if(WIN32)
    target_compile_definitions(Cauto PRIVATE 
        _WIN32_WINNT=0x0601  # Windows 7 minimum
        WIN32_LEAN_AND_MEAN
        _CRT_SECURE_NO_WARNINGS  # Disable security warnings
        WITH_WEBSOCKETS  # Always enable websockets
    )
    
    # Set compiler-specific flags
    if(MSVC)
        # MSVC compiler flags
        target_compile_options(Cauto PRIVATE /W3 /wd4996 /wd4267 /wd4473)
    else()
        # GCC/MinGW compiler flags
        target_compile_options(Cauto PRIVATE -Wall -Wno-deprecated-declarations -Wno-unused-function)
    endif()
endif()

# Check if libwebsockets source exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/external/libwebsockets-4.4.1/CMakeLists.txt")
    # Configure libwebsockets with minimal features
    set(LWS_WITH_SSL OFF CACHE BOOL "Disable SSL")
    set(LWS_WITHOUT_TESTAPPS ON CACHE BOOL "No test apps")
    set(LWS_WITH_SHARED OFF CACHE BOOL "Static library only")
    set(LWS_WITH_STATIC ON CACHE BOOL "Build static library")
    set(LWS_WITHOUT_SERVER ON CACHE BOOL "Client only")
    set(LWS_WITH_HTTP2 OFF CACHE BOOL "No HTTP2")
    set(LWS_WITH_MINIMAL_EXAMPLES OFF CACHE BOOL "No examples")
    set(LWS_WITH_LWSWS OFF CACHE BOOL "No lwsws")
    
    # Disable warnings for libwebsockets build
    if(MSVC)
        set(CMAKE_C_FLAGS_BACKUP ${CMAKE_C_FLAGS})
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /w")  # Disable all warnings for libwebsockets
    else()
        # For GCC/MinGW, disable warnings that cause build failures
        set(CMAKE_C_FLAGS_BACKUP ${CMAKE_C_FLAGS})
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-label -Wno-unused-function -Wno-unused-variable")
    endif()
    
    add_subdirectory(src/external/libwebsockets-4.4.1)
    
    # Restore original flags
    if(MSVC OR CMAKE_COMPILER_IS_GNUCC)
        set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS_BACKUP})
    endif()
    
    # Add websocket support to our executable
    target_include_directories(Cauto PRIVATE 
        src/external/libwebsockets-4.4.1/include
        ${CMAKE_BINARY_DIR}/src/external/libwebsockets-4.4.1
    )
    target_link_libraries(Cauto websockets ws2_32)
    
    # Ensure proper build order
    add_dependencies(Cauto websockets)
    
else()
    message(FATAL_ERROR "libwebsockets source not found in src/external/libwebsockets-4.4.1. This is required for the build.")
endif()

install(TARGETS Cauto RUNTIME DESTINATION bin)