cmake_minimum_required(VERSION 2.8)

#FIXME MODULE_PATH for Windowses
IF(NOT (${CMAKE_SYSTEM_NAME} MATCHES "Windows") )
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/local/lib/cmake")    
    MESSAGE( STATUS "CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH} )    
endif()

project( MackenteinCAM )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

LINK_DIRECTORIES(${OpenCV_LIB_DIR})

find_package( OpenCV REQUIRED )
find_package( aruco REQUIRED )

set(SOURCE
   ${SOURCE}
   findelectrodes.cpp
   twoview.cpp
   main.cpp
)
set(HEADERS
   ${HEADERS}
   findelectrodes.h
   twoview.h
   main.h
)

add_executable( MackenteinCAM findelectrodes.cpp twoview.cpp main.cpp)

target_link_libraries( MackenteinCAM ${OpenCV_LIBS} ${aruco_LIBS} ) 
