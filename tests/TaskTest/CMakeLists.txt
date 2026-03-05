set(TARGET TaskTest)

add_executable(${TARGET}
	${CMAKE_CURRENT_SOURCE_DIR}/EventAggregatorTest.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MessageRequestTest.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MessageTaskTest.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/StatementTask.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BackGroundWorkerTest.cpp
	# TaskPoolTest.cpp  # 無効
)

target_compile_features(${TARGET} PUBLIC cxx_std_20)

target_include_directories(${TARGET} PRIVATE
	${INCLUDE_DIRECTORY}
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${TARGET} PRIVATE
	${PROJECT_NAME}
	${TESTING_FRAMEWORK}
	$<$<CONFIG:Coverage>:gcov>
	$<$<CONFIG:AddressSanitizer>:asan>
)

target_compile_options(${TARGET} PRIVATE
	${TEST_OPTIONS}
	$<$<CONFIG:Coverage>:--coverage>
	$<$<CONFIG:AddressSanitizer>:-fsanitize=address>
)

add_test(NAME ${TARGET} COMMAND ${TARGET})

enable_testing()
