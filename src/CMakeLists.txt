cmake_minimum_required(VERSION 2.8)

project(JT1078ToRTMP)
aux_source_directory(. DIR_SRCS)
set(INC_DIR /usr/local/include/ ../include /home/test/ffmpeg_build/include)
set(LINK_DIR /usr/local/lib ../../lib  /home/test/ffmpeg_build/lib)
include_directories(${INC_DIR})
link_directories(${LINK_DIR})
add_definitions(-D BOOST_LOG_DYN_LINK)
add_executable(JT1078ToRTMP ${DIR_SRCS})

install(TARGETS JT1078ToRTMP DESTINATION JT1078ToRTMP/bin)
install(FILES config.json DESTINATION JT1078ToRTMP/bin)
install(FILES config.json.example DESTINATION JT1078ToRTMP/bin)
install(DIRECTORY ../lib/ DESTINATION JT1078ToRTMP/lib)

target_link_libraries(JT1078ToRTMP microhttpd pthread boost_log_setup boost_log boost_thread avformat avcodec avutil z m swresample lzma va va-drm fdk-aac mp3lame opus x264 x265 vpx dl bz2 curl rtmp crypto)

add_definitions("-Wall -g -std=c++11")

