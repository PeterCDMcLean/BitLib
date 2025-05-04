add_executable(BitLib_Example1 EXCLUDE_FROM_ALL)

target_sources(BitLib_Example1 PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src/example1.cpp"
)
target_link_libraries(BitLib_Example1 PRIVATE bitlib::bitlib)
