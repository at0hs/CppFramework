cmake_minimum_required(VERSION 3.5)
project(Framework CXX)

include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/a7f443b80b105f940225332ed3c31f2790092f47.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
if (WIN32)
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif ()
FetchContent_MakeAvailable(googletest)

#
# definition common variables
#
set(PROJECT_NAME Framework)
set(HOME ${PROJECT_SOURCE_DIR})
set(INCLUDE_DIRECTORY ${HOME}/include)
set(SOURCE_DIRECTORY ${HOME}/lib)
set(TEST_DIRECTORY ${HOME}/tests)

message(STATUS "Project home: " ${HOME})

#
# sub directories
#
add_subdirectory(lib)
add_subdirectory(tests)
