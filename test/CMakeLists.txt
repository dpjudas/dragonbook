cmake_minimum_required(VERSION 3.11)

set(TEST_SOURCES
	Main.cpp
)

set(TEST_LIBS
	IRCompiler
)

include_directories(. ../src)

if (WIN32)
	add_definitions(-DWIN32 -D_WIN32)
endif()

# Set up flags for MSVC
if (MSVC)
	set(CMAKE_CXX_FLAGS "/MP ${CMAKE_CXX_FLAGS}") # Use all cores for compilation
endif()

add_executable(IRTest ${TEST_SOURCES})

target_link_libraries(IRTest ${TEST_LIBS})

target_compile_definitions(IRTest PUBLIC "$<$<CONFIG:RELEASE>:RELEASE>")
target_compile_definitions(IRTest PUBLIC "$<$<CONFIG:MINSIZEREL>:RELEASE>")
target_compile_definitions(IRTest PUBLIC "$<$<CONFIG:RELWITHDEBINFO>:RELEASE>")

target_compile_definitions(IRTest PUBLIC "$<$<CONFIG:RELEASE>:NDEBUG>")
target_compile_definitions(IRTest PUBLIC "$<$<CONFIG:MINSIZEREL>:NDEBUG>")
target_compile_definitions(IRTest PUBLIC "$<$<CONFIG:RELWITHDEBINFO>:NDEBUG>")

target_compile_definitions(IRTest PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")
