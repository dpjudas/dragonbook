#pragma once

#include <vector>
#include <cstdint>

class MachineFunction;

class UnwindInfoWindowsAarch64
{
public:
	static std::vector<uint16_t> create(MachineFunction* func);
};
