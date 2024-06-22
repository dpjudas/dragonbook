#pragma once

#include "../MachineInst.h"

enum class MachineInstOpcodeAArch64 : int
{
	nop,
	loadss, loadsd, load64, load32, load16, load8,
	storess, storesd, store64, store32, store16, store8,
	movss, movsd, mov64, mov32,
	movsx8_32, movsx8_64, movsx16_32, movsx16_64, movsx32_64,
	movzx8_32, movzx8_64, movzx16_32, movzx16_64,
	addss, addsd, add64, add32,
	subss, subsd, sub64, sub32,
	not64, not32,
	neg64, neg32,
	shl64, shl32,
	shr64, shr32,
	sar64, sar32,
	and64, and32,
	or64, or32,
	xorpd, xorps, xor64, xor32,
	mulss, mulsd, mul64, mul32,
	divss, divsd, sdiv64, sdiv32,
	udiv64, udiv32,
};

enum class RegisterNameAArch64 : int
{
	// 31 General purpose X0..X30 / W0..W30
	x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, x30,

	// meaning depends on instruction
	x31,

	// struct return value pointer
	xr = x8,

	// intra-procedure-call
	ip0 = x16,
	ip1 = x17,
	pr = x18,

	// frame pointer, link register
	fp = x29,
	lr = x30,

	// zero register (zxr/wzr), stack pointer
	zxr = x31,
	wzr = x31,
	sp = x31,

	// 32 float/vector registers Vx (Bx 8bit, Hx 16bit, Sx 32bit, Dx 64bit, Qx 128bit)
	xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15,
	xmm16, xmm17, xmm18, xmm19, xmm20, xmm21, xmm22, xmm23, xmm24, xmm25, xmm26, xmm27, xmm28, xmm29, xmm30, xmm31,

	vregstart = 100
};
