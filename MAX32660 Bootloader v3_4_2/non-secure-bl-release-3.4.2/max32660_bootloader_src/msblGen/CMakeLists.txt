cmake_minimum_required(VERSION 2.8)
project(msblGen)

#include_directories(${CMAKE_SOURCE_DIR}/Include)
include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../bootloader)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

# define source files to be compiled
set(SOURCES main.c crc.c)
add_executable(msblGen ${SOURCES})

#set executable path to current directory
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})

#check if open ssl libraries are installed
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
	if(DEFINED CROSS_COMPILE_TO_WIN32)
		target_link_libraries(${PROJECT_NAME} crypto-1_1)
	else()
		target_link_libraries(${PROJECT_NAME} OpenSSL::Crypto)
	endif()
else()
	#warn the user
	message(WARNING "Please install OPENSSL libraries!")
endif()

if(DEFINED CROSS_COMPILE_TO_WIN32)
	add_definitions(-DWIN32)
	set(TOOLCHAIN_PREFIX i686-w64-mingw32)
	set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
	set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
	set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)
	set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
	set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
	set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "msblGen.exe")
else()
	#define needed variables for MSBLGen to work correctly
	if(WIN32)
		add_definitions(-DWIN32)
	elseif(UNIX)
		add_definitions(-DUNIX)
	endif()
endif()
