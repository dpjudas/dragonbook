#pragma once

#include "../MachineInst.h"

enum class MachineInstOpcodeX64 : int
{
	nop,
	lea,
	loadss, loadsd, load64, load32, load16, load8, // mov reg,ptr
	storess, storesd, store64, store32, store16, store8, // mov ptr,reg
	movss, movsd, mov64, mov32, mov16, mov8,
	movsx8_16, movsx8_32, movsx8_64, movsx16_32, movsx16_64, movsx32_64,
	movzx8_16, movzx8_32, movzx8_64, movzx16_32, movzx16_64,
	addss, addsd, add64, add32, add16, add8,
	subss, subsd, sub64, sub32, sub16, sub8,
	not64, not32, not16, not8,
	neg64, neg32, neg16, neg8,
	shl64, shl32, shl16, shl8,
	shr64, shr32, shr16, shr8,
	sar64, sar32, sar16, sar8,
	and64, and32, and16, and8,
	or64, or32, or16, or8,
	xorpd, xorps, xor64, xor32, xor16, xor8,
	mulss, mulsd, imul64, imul32, imul16, imul8,
	divss, divsd, idiv64, idiv32, idiv16, idiv8,
	div64, div32, div16, div8,
	ucomisd, ucomiss, cmp64, cmp32, cmp16, cmp8,
	setl, setb, seta, setg, setle, setbe, setae, setge, sete, setne, setp, setnp,
	cvtsd2ss, cvtss2sd,
	cvttsd2si, cvttss2si,
	cvtsi2sd, cvtsi2ss,
	jmp, je, jne,
	call, ret, push, pop, movdqa
};

enum class RegisterNameX64 : int
{
	rax,
	rcx,
	rdx,
	rbx,
	rsp,
	rbp,
	rsi,
	rdi,
	r8,
	r9,
	r10,
	r11,
	r12,
	r13,
	r14,
	r15,
	xmm0,
	xmm1,
	xmm2,
	xmm3,
	xmm4,
	xmm5,
	xmm6,
	xmm7,
	xmm8,
	xmm9,
	xmm10,
	xmm11,
	xmm12,
	xmm13,
	xmm14,
	xmm15,
	vregstart = 100
};
