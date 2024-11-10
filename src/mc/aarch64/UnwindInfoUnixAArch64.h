#pragma once

#include <vector>
#include <cstdint>

class MachineFunction;

class UnwindInfoUnixAArch64
{
public:
	static std::vector<uint8_t> create(MachineFunction* func, unsigned int& functionStart);
};
