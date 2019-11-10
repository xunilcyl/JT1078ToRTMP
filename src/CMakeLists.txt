cmake_minimum_required(VERSION 2.8)

project(JT1078ToRTMP)
aux_source_directory(. DIR_SRCS)
set(INC_DIR /usr/local/include/ /home/test/code/boost_1_71_0)
set(LINK_DIR /usr/local/lib /home/test/code/boost_1_71_0/stage/lib)
include_directories(${INC_DIR})
link_directories(${LINK_DIR})
add_definitions(-D BOOST_LOG_DYN_LINK)
add_executable(JT1078ToRTMP ${DIR_SRCS})

target_link_libraries(JT1078ToRTMP microhttpd pthread boost_log_setup boost_log boost_thread)

add_definitions("-Wall -g -std=c++11")

