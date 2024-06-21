#pragma once

#include "../MachineInst.h"

enum class MachineInstOpcodeAArch64 : int
{
	nop
};

enum class RegisterNameAArch64 : int
{
	x0, x1, x2, x3, x4, x5, x6, x7,
	x8, x9, x10, x11, x12, x13, x14, x15,
	x16, x17, x18, x19, x20, x21, x22, x23,
	x24, x25, x26, x27, x28, x29, x30, x31,
	xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7,
	xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15,
	vregstart = 100
};
