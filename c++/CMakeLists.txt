cmake_minimum_required(VERSION 3.4...3.18)
project(patrol_car)

set (CMAKE_CXX_STANDARD 17)


#---------------------------------------------------------------------
#       [ Include、Lib  : Define ] ==> []
#---------------------------------------------------------------------

set(COMMON_LIB_DIR "${PROJECT_SOURCE_DIR}/lib/")

set(COMMON_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")


link_directories(${COMMON_LIB_DIR})
include_directories( ${COMMON_INCLUDE_DIR} )

#---------------------------------------------------------------------
#       [ Include、Lib  : Path ] ==> [ glib opencv ]
#---------------------------------------------------------------------
find_package(PkgConfig)
pkg_search_module(GLIB REQUIRED glib-2.0)
include_directories(${GLIB_INCLUDE_DIRS})

find_package( OpenCV REQUIRED )    
include_directories( ${OpenCV_INCLUDE_DIRS} )

find_package(PaddleLite QUIET)
include_directories(${PADDLELITE_INCLUDE_DIR})
LINK_DIRECTORIES("/usr/local/lib/paddle_lite/")
message(${PADDLELITE_LIBRARY})

#---------------------------------------------------------------------
#               [ bin ] ==> [ demo ]
#---------------------------------------------------------------------
set(PREDICT_PROJECT_NAME "predict")
set(PREDICT_PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/demo/predict.cpp)
add_executable(${PREDICT_PROJECT_NAME} ${PREDICT_PROJECT_SOURCES})
target_link_libraries(${PREDICT_PROJECT_NAME} PRIVATE pthread )
target_link_libraries(${PREDICT_PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
target_link_libraries(${PREDICT_PROJECT_NAME} PRIVATE paddle_full_api_shared)



set(CAMERA_DISPLAY_PROJECT_NAME "camera_display")
set(CAMERA_DISPLAY_PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/demo/camera_display.cpp)
add_executable(${CAMERA_DISPLAY_PROJECT_NAME} ${CAMERA_DISPLAY_PROJECT_SOURCES})
target_link_libraries(${CAMERA_DISPLAY_PROJECT_NAME} PRIVATE pthread )
target_link_libraries(${CAMERA_DISPLAY_PROJECT_NAME} PRIVATE ${OpenCV_LIBS})


set(USB_DEVICE_TEST_PROJECT_NAME "usb_device_test")
set(USB_DEVICE_TEST_PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/demo/usb_device_test.cpp)
add_executable(${USB_DEVICE_TEST_PROJECT_NAME} ${USB_DEVICE_TEST_PROJECT_SOURCES})
target_link_libraries(${USB_DEVICE_TEST_PROJECT_NAME} PRIVATE pthread )
target_link_libraries(${USB_DEVICE_TEST_PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
target_link_libraries(${USB_DEVICE_TEST_PROJECT_NAME} PRIVATE serial)



set(CAMERA2VIDEO_PROJECT_NAME "camera2video")
set(CAMERA2VIDEO_PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/demo/camera2video.cpp)
add_executable(${CAMERA2VIDEO_PROJECT_NAME} ${CAMERA2VIDEO_PROJECT_SOURCES})
target_link_libraries(${CAMERA2VIDEO_PROJECT_NAME} PRIVATE pthread )
target_link_libraries(${CAMERA2VIDEO_PROJECT_NAME} PRIVATE ${OpenCV_LIBS})


set(RGB2GRAY_ARRAY_NAME "rgb_mat2gray_array")
set(RGB2GRAY_ARRAY_SOURCES ${PROJECT_SOURCE_DIR}/demo/rgb_mat2gray_array.cpp)
add_executable(${RGB2GRAY_ARRAY_NAME} ${RGB2GRAY_ARRAY_SOURCES})
target_link_libraries(${RGB2GRAY_ARRAY_NAME} PRIVATE ${OpenCV_LIBS})

set(UART_SEND_PROJECT_NAME "uart_send")
set(UART_SEND_PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/demo/uart_send.cpp)
add_executable(${UART_SEND_PROJECT_NAME} ${UART_SEND_PROJECT_SOURCES})
target_link_libraries(${UART_SEND_PROJECT_NAME} PRIVATE pthread )
target_link_libraries(${UART_SEND_PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
target_link_libraries(${UART_SEND_PROJECT_NAME} PRIVATE serial)

set(UART_RECV_PROJECT_NAME "uart_recv")
set(UART_RECV_PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/demo/uart_recv.cpp)
add_executable(${UART_RECV_PROJECT_NAME} ${UART_RECV_PROJECT_SOURCES})
target_link_libraries(${UART_RECV_PROJECT_NAME} PRIVATE pthread )
target_link_libraries(${UART_RECV_PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
target_link_libraries(${UART_RECV_PROJECT_NAME} PRIVATE serial)

set(UART_SEND_PROJECT_NAME "usb_uart_send")
set(UART_SEND_PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/demo/usb_uart_send.cpp)
add_executable(${UART_SEND_PROJECT_NAME} ${UART_SEND_PROJECT_SOURCES})
target_link_libraries(${UART_SEND_PROJECT_NAME} PRIVATE pthread )
target_link_libraries(${UART_SEND_PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
target_link_libraries(${UART_SEND_PROJECT_NAME} PRIVATE serial)

set(UART_RECV_PROJECT_NAME "usb_uart_recv")
set(UART_RECV_PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/demo/usb_uart_recv.cpp)
add_executable(${UART_RECV_PROJECT_NAME} ${UART_RECV_PROJECT_SOURCES})
target_link_libraries(${UART_RECV_PROJECT_NAME} PRIVATE pthread )
target_link_libraries(${UART_RECV_PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
target_link_libraries(${UART_RECV_PROJECT_NAME} PRIVATE serial)

set(RS485_SEND_PROJECT_NAME "rs485_send")
set(RS485_SEND_PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/demo/rs485_send.cpp)
link_directories("/usr/lib/aarch64-linux-gnu/")
add_executable(${RS485_SEND_PROJECT_NAME} ${RS485_SEND_PROJECT_SOURCES})
target_link_libraries(${RS485_SEND_PROJECT_NAME} PRIVATE pthread )
target_link_libraries(${RS485_SEND_PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
target_link_libraries(${RS485_SEND_PROJECT_NAME} PRIVATE serial)
target_link_libraries(${RS485_SEND_PROJECT_NAME} PRIVATE gpiod)

set(RS485_RECV_PROJECT_NAME "rs485_recv")
set(RS485_RECV_PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/demo/rs485_recv.cpp)
link_directories("/usr/lib/aarch64-linux-gnu/")
add_executable(${RS485_RECV_PROJECT_NAME} ${RS485_RECV_PROJECT_SOURCES})
target_link_libraries(${RS485_RECV_PROJECT_NAME} PRIVATE pthread )
target_link_libraries(${RS485_RECV_PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
target_link_libraries(${RS485_RECV_PROJECT_NAME} PRIVATE serial)
target_link_libraries(${RS485_RECV_PROJECT_NAME} PRIVATE gpiod)

set(GPIO_PROJECT_NAME "gpioDemo")
set(GPIO_PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/demo/gpioDemo.cpp)
link_directories("/usr/lib/aarch64-linux-gnu/")
add_executable(${GPIO_PROJECT_NAME} ${GPIO_PROJECT_SOURCES})
target_link_libraries(${GPIO_PROJECT_NAME} PRIVATE pthread )
target_link_libraries(${GPIO_PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
target_link_libraries(${GPIO_PROJECT_NAME} PRIVATE serial)
target_link_libraries(${GPIO_PROJECT_NAME} PRIVATE gpiod)
