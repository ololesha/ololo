cmake_minimum_required (VERSION 2.6)

project (wget_clone)
add_executable(wget_clone main.cpp parser.cpp utils.cpp writer.cpp connection.cpp)
set_property(TARGET wget_clone PROPERTY CXX_STANDARD 17)
