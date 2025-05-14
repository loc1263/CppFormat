cmake_minimum_required(VERSION 3.15)
project(FormatoProcessor VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Windows specific settings
if(WIN32)
    add_definitions(-DUNICODE -D_UNICODE)
    
    # For MinGW, use -mwindows flag for GUI applications
    if(MINGW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows")
    endif()
    
    # Add resource file if it exists
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/app.rc")
        set(RESOURCE_FILE "app.rc")
    endif()
endif()

# Add source files
set(SOURCES
    main.cpp
    main_window.cpp
    csv_parser.cpp
    file_utils.cpp
    ${RESOURCE_FILE}
)

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
)

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
