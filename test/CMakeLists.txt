add_executable(cmmk-test test.c)
add_executable(cmmk-debug debug.c)

find_package(HIDAPI QUIET COMPONENTS libusb)
add_definitions(-DCMMK_TRACE)

IF (WIN32)
	target_link_libraries(cmmk-test cmmk_shared)
	target_link_libraries(cmmk-debug cmmk_shared)
ELSE()
	target_link_libraries(cmmk-test cmmk_shared hidapi-libusb)
	target_link_libraries(cmmk-debug cmmk_shared hidapi-libusb)
ENDIF()

target_include_directories(cmmk-test PRIVATE
	${libcmmk_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(cmmk-debug PRIVATE
	${libcmmk_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR})

IF (NOT WIN32)
	install(TARGETS cmmk-test cmmk-debug
		CONFIGURATIONS Debug
		RUNTIME DESTINATION bin/debug)
	install(TARGETS cmmk-test cmmk-debug
		CONFIGURATIONS Release
		RUNTIME DESTINATION bin/release)
ENDIF()
