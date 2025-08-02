project(BitLib_Examples)
cmake_minimum_required(VERSION 3.20)

if(NOT TARGET bitlib::bitlib)
  add_subdirectory(../ bitlib_dir)
endif()

add_executable(BitLib_Example1 EXCLUDE_FROM_ALL)

target_sources(BitLib_Example1 PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src/example1.cpp"
)
target_link_libraries(BitLib_Example1 PRIVATE bitlib::bitlib)

add_executable(BitLib_Example3 EXCLUDE_FROM_ALL)
target_sources(BitLib_Example3 PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src/example3.cpp"
)
target_link_libraries(BitLib_Example3 PRIVATE bitlib::bitlib)