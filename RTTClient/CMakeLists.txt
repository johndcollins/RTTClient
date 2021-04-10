# CMakeList.txt : CMake project for RTTClient, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (RTTClient "main.cpp" "CTools.h" "CTools.cpp" "CText.h" "CText.cpp" "CDisplayWindow.h" "CDisplayWindow.cpp" "CNetworkManager.h" "CNetworkManager.cpp" "CSharedMemoryReader.h" "CSharedMemoryReader.cpp" "IntelliVibe.h" "BMSTexture.h"       "CConfigReader.h" "CConfigReader.cpp" "CLogger.h" "CLogger.cpp" "CApplication.h" "CApplication.cpp" "CSharedMemory.h" "CSharedMemory.cpp" "SDL2_gfxPrimitives_font.h" "SDL2_rotozoom.h" "SDL2_rotozoom.c" "SDL2_gfxPrimitives.h" "SDL2_gfxPrimitives.c")

#IF(OS_WINDOWS)
#ELSEIF(OS_LINUX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

FIND_PACKAGE(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
FIND_PACKAGE(GLEW REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(RAKNET REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${RAKNET_INCLUDE_DIRS})
target_link_libraries(RTTClient ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${RAKNET_LIBRARIES} "-lpthread" "-lrt")

#ENDIF(OS_WINDOWS)