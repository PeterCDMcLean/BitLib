# set output directory of builds
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# set build type
set(CMAKE_BUILD_TYPE RelWithDebInfo)

# Add targets
file(GLOB PROFILE_SOURCES "src/*.cpp")
add_executable(bitlib-profile ${PROFILE_SOURCES})

target_link_libraries(bitlib-profile PRIVATE bitlib_utils)

target_compile_options(bitlib-profile PUBLIC -O2 -ggdb -Wpedantic)
install(TARGETS bitlib-profile DESTINATION .)
