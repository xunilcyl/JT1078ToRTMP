cmake_minimum_required(VERSION 2.8)

project(JT1078ToRTMP)
aux_source_directory(. DIR_SRCS)
set(INC_DIR /usr/local/include/ ../include /home/test/ffmpeg_build/include)
set(LINK_DIR /usr/local/lib ../../lib  /home/test/ffmpeg_build/lib)
include_directories(${INC_DIR})
link_directories(${LINK_DIR})
add_definitions(-D BOOST_LOG_DYN_LINK)
add_executable(JT1078ToRTMP ${DIR_SRCS})

target_link_libraries(JT1078ToRTMP microhttpd pthread boost_log_setup boost_log boost_thread avformat avcodec avutil z m swresample lzma va va-drm fdk-aac mp3lame opus x264 x265 vpx dl bz2 curl)

add_definitions("-Wall -g -std=c++11")

