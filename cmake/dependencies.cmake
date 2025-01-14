cmake_minimum_required(VERSION 3.10)

find_package(Threads REQUIRED)
find_library(WIRINGPI_LIBRARIES NAMES wiringPi)
include_directories(${WIRINGPI_INCLUDE_DIRS})

include(ExternalProject)

set(source_dir "${CMAKE_BINARY_DIR}/libgraphs-src")
set(build_dir "${CMAKE_BINARY_DIR}/libgraphs-build")
EXTERNALPROJECT_ADD(
  libgraphs
  GIT_REPOSITORY    https://github.com/lukaskaz/lib-graphs.git
  GIT_TAG           main
  PATCH_COMMAND     ""
  PREFIX            libgraphs-workspace
  SOURCE_DIR        ${source_dir}
  BINARY_DIR        ${build_dir}
  CONFIGURE_COMMAND mkdir /${build_dir}/build &> /dev/null
  BUILD_COMMAND     cd ${build_dir}/build && cmake -D BUILD_SHARED_LIBS=ON
                    ${source_dir} && make
  UPDATE_COMMAND    ""
  INSTALL_COMMAND   "" 
  TEST_COMMAND      ""
)
include_directories(${source_dir}/inc)
link_directories(${build_dir}/build)

set(source_dir "${CMAKE_BINARY_DIR}/libshellcmd-src")
set(build_dir "${CMAKE_BINARY_DIR}/libshellcmd-build")
EXTERNALPROJECT_ADD(
  libshellcmd
  GIT_REPOSITORY    https://github.com/lukaskaz/lib-shellcmd.git
  GIT_TAG           main
  PATCH_COMMAND     ${patching_cmd}
  PREFIX            libshellcmd-workspace
  SOURCE_DIR        ${source_dir}
  BINARY_DIR        ${build_dir}
  CONFIGURE_COMMAND mkdir /${build_dir}/build &> /dev/null
  BUILD_COMMAND     cd ${build_dir}/build && cmake -D BUILD_SHARED_LIBS=ON
                    ${source_dir} && make -j 4
  UPDATE_COMMAND    ""
  INSTALL_COMMAND   "" 
  TEST_COMMAND      ""
)
include_directories(${source_dir}/inc)
link_directories(${build_dir}/build)

set(project_dir "${CMAKE_BINARY_DIR}/cava-project")
EXTERNALPROJECT_ADD(
  cavaproject
  GIT_REPOSITORY    https://github.com/karlstav/cava.git 
  GIT_TAG           394c9b1b9b1373db1e1f15587e530a9a3422f5f8
  PATCH_COMMAND     ""
  PREFIX            cava-workspace
  SOURCE_DIR        ${project_dir}
  BINARY_DIR        ${project_dir}
  CONFIGURE_COMMAND cd ${project_dir} && mkdir build && ./autogen.sh &&
  			./configure --prefix=${project_dir}/build
  BUILD_COMMAND     cd ${project_dir} && make -j $(nproc)
  UPDATE_COMMAND    ""
  INSTALL_COMMAND   cd ${project_dir} && make install 
  TEST_COMMAND      ""
)
#include_directories(${project_dir}/inc)
#link_directories(${project_dir}/build)
