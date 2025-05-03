add_executable(BitLib_Example1)

target_sources(BitLib_Example1 PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src/example1.cpp"
)
target_link_libraries(BitLib_Example1 PRIVATE bitlib::bitlib)

add_executable(BitLib_Example2)
target_sources(BitLib_Example2 PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src/example2.cpp"
)
target_link_libraries(BitLib_Example2 PRIVATE bitlib::bitlib bitlib_utils)
