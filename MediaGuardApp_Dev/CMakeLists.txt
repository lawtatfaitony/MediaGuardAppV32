#####################################################################
#
# @brief	CMakeLists of root project
#
#####################################################################

# cmake ../ -G "Visual Studio 17 2022"
# cmake ../ -G "Unix Makefiles"

# version of cmake required at least
# cmake_minimum_required(VERSION 3.14)
cmake_minimum_required(VERSION 3.8)
 
# project name
set(PROJECT_NAME MediaGuardApp)

project(${PROJECT_NAME})

 message("-------------------------------------------------------------------------------------------")
 message("PROJECT ROOT | CMAKE_CURRENT_SOURCE_DIR : "  ${CMAKE_CURRENT_SOURCE_DIR})
 message("-------------------------------------------------------------------------------------------")

# MediaGuard 运行库
add_subdirectory ("MediaGuard")
 
