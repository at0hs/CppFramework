
set(TARGET ${PROJECT_NAME})

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(${TARGET} STATIC ${SOURCES})

target_compile_features(${TARGET} PUBLIC cxx_std_20)

target_include_directories(${TARGET} PUBLIC ${INCLUDE_DIRECTORY})

target_compile_options(${TARGET} PUBLIC
	-Wall -Wextra
	$<$<CONFIG:Debug>:-O0 -ggdb -g3>
	$<$<CONFIG:TestPlus>:-fsanitize=address --coverage>
)

target_link_libraries(${TARGET} PRIVATE
	atomic
	$<$<CONFIG:TestPlus>:asan>
	$<$<CONFIG:TestPlus>:gcov>
)

# mq_* 関数（PosixMessageQueue）は rt ライブラリが必要。
# ヘッダオンリーのテンプレートなので PUBLIC で消費側にも伝搬させる。
target_link_libraries(${TARGET} PUBLIC rt)

add_subdirectory(Exception)
add_subdirectory(Message)
add_subdirectory(Sync)
add_subdirectory(Task)
add_subdirectory(Templates)
add_subdirectory(Timer)
add_subdirectory(SubProcess)
