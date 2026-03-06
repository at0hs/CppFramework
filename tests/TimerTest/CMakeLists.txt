set(TARGET TimerTest)

add_executable(${TARGET}
	${CMAKE_CURRENT_SOURCE_DIR}/TimerTest.cpp
)

target_compile_features(${TARGET} PUBLIC cxx_std_20)

target_include_directories(${TARGET} PRIVATE
	${INCLUDE_DIRECTORY}
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${TARGET} PRIVATE
	${PROJECT_NAME}
	${TESTING_FRAMEWORK}
	$<$<CONFIG:TestPlus>:gcov>
	$<$<CONFIG:TestPlus>:asan>
)

target_compile_options(${TARGET} PRIVATE
	${TEST_OPTIONS}
	$<$<CONFIG:TestPlus>:--coverage>
	$<$<CONFIG:TestPlus>:-fsanitize=address>
)

add_test(NAME ${TARGET} COMMAND ${TARGET})

enable_testing()
