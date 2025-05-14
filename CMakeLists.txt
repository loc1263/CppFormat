cmake_minimum_required(VERSION 3.14)
project(CppFormat VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Enable parallel builds for MinGW
if(MINGW)
    # Set the number of parallel jobs based on the number of processors
    include(ProcessorCount)
    ProcessorCount(N)
    if(NOT N EQUAL 0)
        set(JOBS ${N})
    else()
        set(JOBS 4)  # Default to 4 jobs if processor count fails
    endif()
    
    # Set the -j flag for make
    set(CMAKE_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM} -j${JOBS}" CACHE STRING "Build command" FORCE)
    
    # Set optimization flags
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
    
    # Enable all warnings
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Windows specific settings
if(WIN32)
    add_compile_definitions(UNICODE _UNICODE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
    
    # For MinGW, use -mwindows flag for GUI applications
    if(MINGW)
        add_compile_options(-mwindows)
    endif()
    
    # Add resource file if it exists
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/app.rc")
        set(RESOURCE_FILE "app.rc")
    endif()
    
    # Enable PDB generation for all configs
    set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pdb)
    set(CMAKE_PDB_NAME ${PROJECT_NAME})
    set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pdb)
    
    # Enable PDB for all configurations
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
        set(CMAKE_PDB_NAME_${OUTPUTCONFIG_UPPER} ${PROJECT_NAME}_${OUTPUTCONFIG})
    endforeach()
endif()

# Add source files
set(SOURCES
    main.cpp
    main_window.cpp
    file_utils.cpp
    ${RESOURCE_FILE}
)

# Download single-header nlohmann/json
set(JSON_URL "https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp")
set(JSON_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/nlohmann")
set(JSON_HEADER "${JSON_INCLUDE_DIR}/json.hpp")

# Create include directory if it doesn't exist
file(MAKE_DIRECTORY ${JSON_INCLUDE_DIR})

# Download the header file if it doesn't exist
if(NOT EXISTS ${JSON_HEADER})
    message(STATUS "Downloading nlohmann/json...")
    file(DOWNLOAD ${JSON_URL} ${JSON_HEADER} SHOW_PROGRESS)
endif()

# Add include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add executable
add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    comctl32
    comdlg32
    gdi32
    ole32
    shell32
    oleaut32
    uuid
    shlwapi
)

# Set compile and link options for MinGW
if(MINGW)
    target_compile_options(${PROJECT_NAME} PRIVATE 
        $<IF:$<CONFIG:Debug>,
            -g -O0,
            -O2 -s
        >
    )
    target_link_options(${PROJECT_NAME} PRIVATE -static -static-libgcc -static-libstdc++)
endif()

# Set Windows version info
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # Add version info if available
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/version.rc.in")
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/version.rc.in
            ${CMAKE_CURRENT_BINARY_DIR}/version.rc
            @ONLY
        )
        target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
    endif()
endif()
