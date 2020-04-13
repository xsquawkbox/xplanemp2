cmake_minimum_required(VERSION 3.1)

project(xpsdk LANGUAGES C CXX VERSION 2.0)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules" ${CMAKE_MODULE_PATH})
include(CMakeDependentOption)
find_package(XPSDK REQUIRED)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
	set(XPMP_DEFINES ${XPMP_DEFINES} DEBUG=1)
	set(XPMP_DEBUG ON)
else()
	set(XPMP_DEBUG OFF)
endif()
cmake_dependent_option(XPMP_DEBUG_OPENGL "Install OpenGL Debug hooks and debug info" ON "XPMP_DEBUG" OFF)
if(XPMP_DEBUG_OPENGL)
	set(XPMP_DEFINES ${XPMP_DEFINES} DEBUG_GL=1)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(XPMP_DEFINES ${XPMP_DEFINES} LIN=1)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(XPMP_DEFINES ${XPMP_DEFINES} IBM=1)
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
	set(XPMP_DEFINES ${XPMP_DEFINES} APL=1)
endif()

add_library(xplanemp
	${XPMP_PLATFORM_SOURCES}
	src/CSL.cpp
	src/CSL.h
	src/CullInfo.cpp
	src/CullInfo.h
	src/MapRendering.cpp
	src/MapRendering.h
	src/PlaneType.cpp
	src/PlaneType.h
	src/Renderer.cpp
	src/Renderer.h
	src/TCASHack.cpp
	src/TCASHack.h
	src/XPMPMultiplayer.cpp
	src/CSLLibrary.cpp
	src/CSLLibrary.h
	src/XPMPMultiplayerVars.cpp
	src/XPMPMultiplayerVars.h
	src/XPMPPlane.cpp
	src/XPMPPlane.h
	src/XStringUtils.cpp
	src/XStringUtils.h
	src/XUtils.cpp

	src/obj8/Obj8CSL.cpp
	src/obj8/Obj8CSL.h
	src/obj8/Obj8Attachment.cpp
	src/obj8/Obj8Attachment.h
	src/obj8/Obj8InstanceData.cpp
	src/obj8/Obj8InstanceData.h
	)
target_include_directories(xplanemp
	PUBLIC 
		${XPSDK_INCLUDE_DIRS}
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(xplanemp
	PRIVATE ${XPSDK_XPLM_LIBRARIES}
	${PNG_LIBRARY}
	${XPMP_PLATFORM_LIBRARIES})
target_compile_definitions(xplanemp
		PRIVATE ${XPMP_DEFINES} XPLM200=1 XPLM210=1 XPLM300=1)
set_property(TARGET xplanemp PROPERTY CXX_STANDARD_REQUIRED 11)
set_property(TARGET xplanemp PROPERTY CXX_STANDARD 14)
