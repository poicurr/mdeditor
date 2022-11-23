cmake_minimum_required(VERSION 2.8)

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project(md2html CXX)

add_subdirectory(src)

