cmake_minimum_required(VERSION 2.8)

project(gameserver CXX C)

set(CMAKE_BUILD_TYPE Debug) # 设置编译类型
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O2 -Wall ") # 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -Wall ")  # 设置 cxx编译选项

if (WIN32)
	message(STATUS "IS windows.")
endif(WIN32)

if (UNIX)
	message(STATUS "IS unix.")
endif(UNIX)

if (LINUX)
	message(STATUS "IS linux.")
endif(LINUX)

find_library(zlib_found NAMES z)
if (zlib_found) 
	message(STATUS "libz found ${zlib_found}")
else (zlib_found)
	message(STATUS "libz not found")
endif(zlib_found)

find_package(ZLIB)
if (ZLIB_FOUND)
	message(STATUS "found zlib")
endif(ZLIB_FOUND)


find_package (BZip2)
if (BZIP2_FOUND)
	message(STATUS "found Bzip2")
endif(BZIP2_FOUND)

find_library(gtest_found NAMES gtest)
if (gtest_found)
	message(STATUS "libluajit found ${luajit_found}")
else (gtest_found)
	message(STATUS "libluajit not found ${luajit_found}")
endif(gtest_found)


set(name chenbo)


# 显示环境变量 PATH
if (name STREQUAL chenbo)
	message(STATUS $ENV{PATH}) 
endif(name STREQUAL chenbo)

set(SRC 
	connectpool.cpp
	interface_c.cpp
	interface.cpp
	llist.c
	log.cpp
	lua_interface.cpp
	main.cpp
	MemcachedHandler.cc
	mtimer.c
	mysql_part.cpp
	net.cpp
	protocal.cpp
	redis.cpp
	socket.cpp
	timer.c
	timer_event.cpp
	timewheel.cc
	)
#这两项需要放在add_executable前面，否则link的时候早不到库
include_directories(. /usr/local/mysql/include/ /home/AustinChen/lib/include/hiredis /home/AustinChen/lib/include) # -I
link_directories(/usr/local/mysql/lib /home/AustinChen/lib
/home/AustinChen/tools/luajit-2.0/src) # -L

add_executable(gameserver ${SRC})

add_definitions(-DHEADER_SIZE=9 -DTCP_BUFFER_SIZE=16384
-DBeginHandlerNums=100) # 设置宏变量
#add_definitions(-g -O2 -Wall)

target_link_libraries(gameserver dl m pthread mysqlclient z rt uuid memcached
libtolua++.a libhiredis.a libluajit-2.1.a) # 需要连接的库

#set_target_properties(gameserver PROPERTIES LINK_FLAGS -Wl,-E) #设置连接选项
