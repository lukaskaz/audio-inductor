cmake_minimum_required(VERSION 3.10)

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
  PATCH_COMMAND     ""
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

set(source_dir "${CMAKE_BINARY_DIR}/liblogger-src")
set(build_dir "${CMAKE_BINARY_DIR}/liblogger-build")

EXTERNALPROJECT_ADD(
  liblogger
  GIT_REPOSITORY    https://github.com/lukaskaz/lib-logger.git
  GIT_TAG           main
  PATCH_COMMAND     ""
  PREFIX            liblogger-workspace
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

set(source_dir "${CMAKE_BINARY_DIR}/libdisplays-src")
set(build_dir "${CMAKE_BINARY_DIR}/libdisplays-build")
EXTERNALPROJECT_ADD(
  libdisplays
  GIT_REPOSITORY    https://github.com/lukaskaz/lib-displays.git
  GIT_TAG           main
  PATCH_COMMAND     ""
  PREFIX            libdisplays-workspace
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

set(source_dir "${CMAKE_BINARY_DIR}/libpwm-src")
set(build_dir "${CMAKE_BINARY_DIR}/libpwm-build")
EXTERNALPROJECT_ADD(
  libpwm
  GIT_REPOSITORY    https://github.com/lukaskaz/lib-pwm.git
  GIT_TAG           main
  PATCH_COMMAND     ""
  PREFIX            libpwm-workspace
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

set(source_dir "${CMAKE_BINARY_DIR}/libadc-src")
set(build_dir "${CMAKE_BINARY_DIR}/libadc-build")
EXTERNALPROJECT_ADD(
  libadc
  GIT_REPOSITORY    https://github.com/lukaskaz/lib-adc.git
  GIT_TAG           main
  PATCH_COMMAND     ""
  PREFIX            libadc-workspace
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

set(source_dir "${CMAKE_BINARY_DIR}/libservo-src")
set(build_dir "${CMAKE_BINARY_DIR}/libservo-build")
EXTERNALPROJECT_ADD(
  libservo
  GIT_REPOSITORY    https://github.com/lukaskaz/lib-servo.git
  GIT_TAG           main
  PATCH_COMMAND     ""
  PREFIX            libservo-workspace
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

set(source_dir "${CMAKE_BINARY_DIR}/libledrgb-src")
set(build_dir "${CMAKE_BINARY_DIR}/libledrgb-build")
EXTERNALPROJECT_ADD(
  libledrgb
  GIT_REPOSITORY    https://github.com/lukaskaz/lib-ledrgb.git
  GIT_TAG           main
  PATCH_COMMAND     ""
  PREFIX            libledrgb-workspace
  SOURCE_DIR        ${source_dir}
  BINARY_DIR        ${build_dir}
  CONFIGURE_COMMAND mkdir /${build_dir}/build &> /dev/null
  BUILD_COMMAND     cd ${build_dir}/build && cmake -D BUILD_SHARED_LIBS=ON
                    -D USE_RPI5=ON ${source_dir} && make -j 4
  UPDATE_COMMAND    ""
  INSTALL_COMMAND   "" 
  TEST_COMMAND      ""
)
include_directories(${source_dir}/inc)
link_directories(${build_dir}/build)

set(source_dir "${CMAKE_BINARY_DIR}/libgpio-src")
set(build_dir "${CMAKE_BINARY_DIR}/libgpio-build")
EXTERNALPROJECT_ADD(
  libgpio
  GIT_REPOSITORY    https://github.com/lukaskaz/lib-gpio.git
  GIT_TAG           main
  PATCH_COMMAND     ""
  PREFIX            libgpio-workspace
  SOURCE_DIR        ${source_dir}
  BINARY_DIR        ${build_dir}
  CONFIGURE_COMMAND mkdir /${build_dir}/build &> /dev/null
  BUILD_COMMAND     cd ${build_dir}/build && cmake -D BUILD_SHARED_LIBS=ON
                    -D USE_RPI5=ON ${source_dir} && make -j 4
  UPDATE_COMMAND    ""
  INSTALL_COMMAND   "" 
  TEST_COMMAND      ""
)
include_directories(${source_dir}/inc)
link_directories(${build_dir}/build)

set(source_dir "${CMAKE_BINARY_DIR}/libtts-src")
set(build_dir "${CMAKE_BINARY_DIR}/libtts-build")
EXTERNALPROJECT_ADD(
  libtts
  GIT_REPOSITORY    https://github.com/lukaskaz/lib-tts.git
  GIT_TAG           main
  PATCH_COMMAND     ""
  PREFIX            libtts-workspace
  SOURCE_DIR        ${source_dir}
  BINARY_DIR        ${build_dir}
  CONFIGURE_COMMAND mkdir /${build_dir}/build &> /dev/null
  BUILD_COMMAND     cd ${build_dir}/build && cmake -D BUILD_SHARED_LIBS=ON
                    -D USE_RPI5=ON ${source_dir} && make -j 4
  UPDATE_COMMAND    ""
  INSTALL_COMMAND   "" 
  TEST_COMMAND      ""
)
include_directories(${source_dir}/inc)
link_directories(${build_dir}/build)


set(source_dir "${CMAKE_BINARY_DIR}/libstt-src")
set(build_dir "${CMAKE_BINARY_DIR}/libstt-build")
EXTERNALPROJECT_ADD(
  libstt
  GIT_REPOSITORY    https://github.com/lukaskaz/lib-stt.git
  GIT_TAG           main
  PATCH_COMMAND     ""
  PREFIX            libstt-workspace
  SOURCE_DIR        ${source_dir}
  BINARY_DIR        ${build_dir}
  CONFIGURE_COMMAND mkdir /${build_dir}/build &> /dev/null
  BUILD_COMMAND     cd ${build_dir}/build && cmake -D BUILD_SHARED_LIBS=ON
                    -D USE_RPI5=ON ${source_dir} && make -j 4
  UPDATE_COMMAND    ""
  INSTALL_COMMAND   "" 
  TEST_COMMAND      ""
)
include_directories(${source_dir}/inc)
link_directories(${build_dir}/build)
