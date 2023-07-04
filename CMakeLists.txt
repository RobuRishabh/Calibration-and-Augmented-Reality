cmake_minimum_required(VERSION 3.16)
project(Calibration_and_Augmented_Reality)

# enable C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

# set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(include)

add_executable(main src/main.cpp src/utils.cpp)
add_executable(ar src/ar.cpp src/utils.cpp)
add_executable(feature src/feature.cpp src/utils.cpp)


# linking
target_link_libraries(main ${OpenCV_LIBS})
target_link_libraries(ar ${OpenCV_LIBS})
target_link_libraries(feature ${OpenCV_LIBS})
