cmake_minimum_required(VERSION 2.8)
project(POSE_GRAPH_CERES_PLUS)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
ENDIF()

MESSAGE("Build type: " ${CMAKE_BUILD_TYPE})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -Wall  -O3 -march=native ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall   -O3 -march=native")

# Check C++11 or C++0x support
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
   add_definitions(-DCOMPILEDWITHC11)
   message(STATUS "Using flag -std=c++11.")
elseif(COMPILER_SUPPORTS_CXX0X)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
   add_definitions(-DCOMPILEDWITHC0X)
   message(STATUS "Using flag -std=c++0x.")
else()
   message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

#LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)



find_package(OpenCV 3.0 QUIET)
if(NOT OpenCV_FOUND)
   find_package(OpenCV 2.4.3 QUIET)
   if(NOT OpenCV_FOUND)
      message(FATAL_ERROR "OpenCV > 2.4.3 not found.")
   endif()
endif()

include_directories("/usr/include/eigen3")
#find_package(Eigen3 3.1.0 REQUIRED)

# Sophus
#find_package( Sophus REQUIRED )
#include_directories( ${Sophus_INCLUDE_DIRS} )

# ceres
find_package(Ceres REQUIRED)
include_directories(${CERES_INCLUDE_DIRS})

include_directories(
${PROJECT_SOURCE_DIR}
${PROJECT_SOURCE_DIR}/include
${PROJECT_SOURCE_DIR}/test
${EIGEN3_INCLUDE_DIR}
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

add_library(${PROJECT_NAME} SHARED
    src/config.cc
    src/converter.cc
    src/GroundTruth.cc
    src/Frame.cc
    src/MapPoint.cc
    src/ORBextractor.cc
    src/ORBmatcher.cc
    src/SequenceRun.cc
    src/ImageRead.cc
    src/MotionEstimate.cc
    src/camera.cc

)

target_link_libraries(${PROJECT_NAME}
${OpenCV_LIBS}
${EIGEN3_LIBS}
${CERES_LIBRARIES}
)

# Build
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_executable(pose_graph_ceres test/pose_graph_ceres.cpp )
target_link_libraries(pose_graph_ceres ${PROJECT_NAME})

add_executable(frame_test test/frame_test.cpp )
target_link_libraries(frame_test ${PROJECT_NAME})

add_executable(fake_loop_generate test/fake_loop_generate.cpp )
target_link_libraries(fake_loop_generate ${PROJECT_NAME})

add_executable(match_test test/match_test.cpp )
target_link_libraries(match_test ${PROJECT_NAME})

add_executable(match_test2 test/match_test2.cpp )
target_link_libraries(match_test2 ${PROJECT_NAME})

add_executable(motionestimate test/motionestimate.cpp )
target_link_libraries(motionestimate ${PROJECT_NAME}) 

add_executable(motionestimate_wholesequence test/motionestimate_wholesequence.cpp )
target_link_libraries(motionestimate_wholesequence ${PROJECT_NAME}) 

add_executable(pose_graph_ceres_plus test/pose_graph_ceres_plus.cpp )
target_link_libraries(pose_graph_ceres_plus ${PROJECT_NAME})

add_executable(generate_edges_from_trajectory_origion test/generate_edges_from_trajectory_origion.cpp )
target_link_libraries(generate_edges_from_trajectory_origion ${PROJECT_NAME}) 

add_executable(read_loop_edges_test test/read_loop_edges_test.cpp )
target_link_libraries(read_loop_edges_test ${PROJECT_NAME})

add_executable(pose_graph_ceres_plus_finial test/pose_graph_ceres_plus_finial.cpp )
target_link_libraries(pose_graph_ceres_plus_finial ${PROJECT_NAME}) 





