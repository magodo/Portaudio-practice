# Let compiler output be verbose
set(CMAKE_VERBOSE_MAKEFILE ON)

# At least this cmake version is needed
cmake_minimum_required (VERSION 2.6)

SET(ADK_INCDIR ${BB_STAGING_DIR_HOST}/usr/include)
SET(ADK_LIBDIR ${BB_STAGING_DIR_HOST}/usr/lib)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

include_directories( ${ADK_INCDIR})
include_directories(/usr/local/include)
link_directories( ${ADK_LIBDIR})
link_directories( ${ADK_LIBDIR}/../bin/aoshuo/bin/ngi2_arm/release/)

# Define name for the shared library,makes life easier below
set(prog pacap)
add_executable(${prog} ${PROJECT_SOURCE_DIR}/pacap.c)
target_link_libraries(${prog} rt pthread asound portaudio m)
