cmake_minimum_required(VERSION 3.3.2)

set(LIB_SOURCES
	ir/IR.h
	ir/IRContext.cpp
	ir/IRContext.h
	ir/IRBuilder.cpp
	ir/IRBuilder.h
	ir/IRValue.h
	ir/IRType.h
	ir/IRInst.h
	ir/IRInstVisitor.h
	ir/IRBasicBlock.h
	ir/IRFunction.cpp
	ir/IRFunction.h
	ir/IRInstValidator.cpp
	ir/IRInstValidator.h
	jit/JITRuntime.cpp
	jit/JITRuntime.h
	jit/NativeSymbolResolver.cpp
	jit/NativeSymbolResolver.h
	jit/StackTrace.cpp
	jit/StackTrace.h
	mc/RegisterAllocator.cpp
	mc/RegisterAllocator.h
	mc/MachineInstSelection.cpp
	mc/MachineInstSelection.h
	mc/MachineInst.h
	mc/MachineCodeHolder.cpp
	mc/MachineCodeHolder.h
	mc/MachineCodeWriter.cpp
	mc/MachineCodeWriter.h
	mc/AssemblyWriter.cpp
	mc/AssemblyWriter.h
	mc/UnwindInfoUnix.cpp
	mc/UnwindInfoUnix.h
	mc/UnwindInfoWindows.cpp
	mc/UnwindInfoWindows.h
)

set(LIB_LIBS
)

include_directories(.)

set(NOT_COMPILED_SOURCES
)

if (WIN32)
	add_definitions(-DWIN32 -D_WIN32)
endif()

# Set up flags for MSVC
if (MSVC)
	set(CMAKE_CXX_FLAGS "/MP ${CMAKE_CXX_FLAGS}") # Use all cores for compilation
else()
	set(CMAKE_CXX_FLAGS "-std=gnu++14 ${CMAKE_CXX_FLAGS}")
endif()

add_library(dragonbook STATIC ${LIB_SOURCES} ${NOT_COMPILED_SOURCES})

target_link_libraries(dragonbook ${LIB_LIBS})

target_compile_definitions(dragonbook PUBLIC "$<$<CONFIG:RELEASE>:RELEASE>")
target_compile_definitions(dragonbook PUBLIC "$<$<CONFIG:MINSIZEREL>:RELEASE>")
target_compile_definitions(dragonbook PUBLIC "$<$<CONFIG:RELWITHDEBINFO>:RELEASE>")

target_compile_definitions(dragonbook PUBLIC "$<$<CONFIG:RELEASE>:NDEBUG>")
target_compile_definitions(dragonbook PUBLIC "$<$<CONFIG:MINSIZEREL>:NDEBUG>")
target_compile_definitions(dragonbook PUBLIC "$<$<CONFIG:RELWITHDEBINFO>:NDEBUG>")

target_compile_definitions(dragonbook PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")

set_source_files_properties(${NOT_COMPILED_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group("ir" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/ir/.+")
source_group("jit" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/jit/.+")
source_group("mc" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/mc/.+")
