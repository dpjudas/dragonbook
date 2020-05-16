#pragma once

#include <string>
#include "JITRuntime.h"

class NativeSymbolResolver
{
public:
	NativeSymbolResolver();
	~NativeSymbolResolver();

	JITStackFrame GetName(void* frame);

private:
	NativeSymbolResolver(const NativeSymbolResolver&) = delete;
	NativeSymbolResolver& operator=(const NativeSymbolResolver&) = delete;
};
