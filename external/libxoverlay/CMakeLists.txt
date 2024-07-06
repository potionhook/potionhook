# This builds a 32-bit version of library, building 64 bit is not supported yet.

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type")
endif()
set(CMAKE_BUILD_TYPE_VALUES "Debug;Release" CACHE INTERNAL "List of supported build")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_BUILD_TYPE_VALUES})

cmake_minimum_required(VERSION 3.0)

project(xoverlay LANGUAGES C VERSION 0.0.1)

set(export_dest "lib/${PROJECT_NAME}-${PROJECT_VERSION}")
set(include_dest "include/${PROJECT_NAME}-${PROJECT_VERSION}")
set(lib_dest "${export_dest}/${CMAKE_BUILD_TYPE}")

find_package(X11 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

if(NOT (X11_Xext_FOUND AND X11_Xfixes_FOUND))
    message(FATAL_ERROR "Please install Xext and Xfixes")  
endif()

add_library(${PROJECT_NAME} STATIC "")

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-m32 -fPIC" LINK_FLAGS "-m32 -fPIC")

target_include_directories(${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${GLEW_INCLUDE_DIRS}>
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${include_dest}>
)

target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES} m rt pthread ${X11_X11_LIB} ${X11_Xext_LIB} ${X11_Xfixes_LIB})

add_subdirectory(include)
add_subdirectory(src)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME} DESTINATION "${lib_dest}")
install(FILES "include/xoverlay.hpp" DESTINATION "${include_dest}")
install(EXPORT ${PROJECT_NAME} DESTINATION "${lib_dest}")
install(FILES ${PROJECT_NAME}-config.cmake DESTINATION ${export_dest})