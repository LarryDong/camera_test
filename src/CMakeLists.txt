
ADD_EXECUTABLE(trigger main.cpp)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/libs)
 
TARGET_LINK_LIBRARIES(trigger arducam_mipicamera opencv_core opencv_highgui opencv_imgproc opencv_imgcodecs opencv_videoio)
