add_executable(cmmk-test test.c)
add_executable(cmmk-debug debug.c)

target_link_libraries(cmmk-test cmmk_shared usb-1.0)
target_link_libraries(cmmk-debug cmmk_shared usb-1.0)

target_include_directories(cmmk-test PRIVATE
	${libcmmk_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(cmmk-debug PRIVATE
	${libcmmk_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS cmmk-test cmmk-debug
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
