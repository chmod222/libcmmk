cmake_minimum_required(VERSION 3.0)
project(libcmmk VERSION 0.2 LANGUAGES C)

#
# Build section
#
add_library(cmmk_obj OBJECT
	src/libcmmk.c)

target_include_directories(cmmk_obj
	PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src)

set_target_properties(cmmk_obj PROPERTIES
	C_STANDARD 99
	PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/libcmmk/libcmmk.h)

IF(NOT WIN32)
	target_compile_options(cmmk_obj PUBLIC "-Wall")
	target_compile_options(cmmk_obj PUBLIC "-Wextra")
ENDIF()

add_library(cmmk_shared SHARED $<TARGET_OBJECTS:cmmk_obj>)
add_library(cmmk_static STATIC $<TARGET_OBJECTS:cmmk_obj>)

IF(WIN32)
	set_target_properties(cmmk_shared PROPERTIES OUTPUT_NAME libcmmk)
	set_target_properties(cmmk_static PROPERTIES OUTPUT_NAME libcmmk)
ELSE()
	set_target_properties(cmmk_shared PROPERTIES OUTPUT_NAME cmmk)
	set_target_properties(cmmk_static PROPERTIES OUTPUT_NAME cmmk)
ENDIF()

#
# Samples and debug
#
add_subdirectory(test)

#
# Install section
#

IF(NOT WIN32)

	include(GNUInstallDirs)

	install(TARGETS cmmk_shared cmmk_static cmmk_obj EXPORT cmmk-targets
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

	#install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

ENDIF()
