cmake_minimum_required(VERSION 3.4)
project(glog-download NONE)

include(ExternalProject)
ExternalProject_Add(glog
    GIT_REPOSITORY    https://github.com/google/glog.git
    GIT_TAG           v0.4.0
    SOURCE_DIR        "${CMAKE_CURRENT_BINARY_DIR}/glog"
    BINARY_DIR        ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     ""
    INSTALL_COMMAND   ""
    TEST_COMMAND      ""
    CMAKE_ARGS
    -DCMAKE_INSTALL_MESSAGE=LAZY
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    )
