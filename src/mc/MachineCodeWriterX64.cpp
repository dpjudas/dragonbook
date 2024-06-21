
#include "MachineCodeWriterX64.h"
#include "MachineCodeHolder.h"

void MachineCodeWriterX64::codegen()
{
	codeholder->fileInfo = sfunc->fileInfo;

	funcBeginAddress = codeholder->code.size();

	basicblock(sfunc->prolog);

	for (MachineBasicBlock* bb : sfunc->basicBlocks)
	{
		basicblock(bb);
	}

	basicblock(sfunc->epilog);
}

void MachineCodeWriterX64::basicblock(MachineBasicBlock* bb)
{
	codeholder->bbOffsets[bb] = codeholder->code.size();
	for (MachineInst* inst : bb->code)
	{
		opcode(inst);
		inst->unwindOffset = (int)(codeholder->code.size() - funcBeginAddress);
	}
}

void MachineCodeWriterX64::opcode(MachineInst* inst)
{
	switch (inst->opcode)
	{
	default:
	case MachineInstOpcodeX64::nop: nop(inst); break;
	case MachineInstOpcodeX64::lea: lea(inst); break;
	case MachineInstOpcodeX64::loadss: loadss(inst); break;
	case MachineInstOpcodeX64::loadsd: loadsd(inst); break;
	case MachineInstOpcodeX64::load64: load64(inst); break;
	case MachineInstOpcodeX64::load32: load32(inst); break;
	case MachineInstOpcodeX64::load16: load16(inst); break;
	case MachineInstOpcodeX64::load8: load8(inst); break;
	case MachineInstOpcodeX64::storess: storess(inst); break;
	case MachineInstOpcodeX64::storesd: storesd(inst); break;
	case MachineInstOpcodeX64::store64: store64(inst); break;
	case MachineInstOpcodeX64::store32: store32(inst); break;
	case MachineInstOpcodeX64::store16: store16(inst); break;
	case MachineInstOpcodeX64::store8: store8(inst); break;
	case MachineInstOpcodeX64::movss: movss(inst); break;
	case MachineInstOpcodeX64::movsd: movsd(inst); break;
	case MachineInstOpcodeX64::mov64: mov64(inst); break;
	case MachineInstOpcodeX64::mov32: mov32(inst); break;
	case MachineInstOpcodeX64::mov16: mov16(inst); break;
	case MachineInstOpcodeX64::mov8: mov8(inst); break;
	case MachineInstOpcodeX64::movsx8_16: movsx8_16(inst); break;
	case MachineInstOpcodeX64::movsx8_32: movsx8_32(inst); break;
	case MachineInstOpcodeX64::movsx8_64: movsx8_64(inst); break;
	case MachineInstOpcodeX64::movsx16_32: movsx16_32(inst); break;
	case MachineInstOpcodeX64::movsx16_64: movsx16_64(inst); break;
	case MachineInstOpcodeX64::movsx32_64: movsx32_64(inst); break;
	case MachineInstOpcodeX64::movzx8_16: movzx8_16(inst); break;
	case MachineInstOpcodeX64::movzx8_32: movzx8_32(inst); break;
	case MachineInstOpcodeX64::movzx8_64: movzx8_64(inst); break;
	case MachineInstOpcodeX64::movzx16_32: movzx16_32(inst); break;
	case MachineInstOpcodeX64::movzx16_64: movzx16_64(inst); break;
	case MachineInstOpcodeX64::addss: addss(inst); break;
	case MachineInstOpcodeX64::addsd: addsd(inst); break;
	case MachineInstOpcodeX64::add64: add64(inst); break;
	case MachineInstOpcodeX64::add32: add32(inst); break;
	case MachineInstOpcodeX64::add16: add16(inst); break;
	case MachineInstOpcodeX64::add8: add8(inst); break;
	case MachineInstOpcodeX64::subss: subss(inst); break;
	case MachineInstOpcodeX64::subsd: subsd(inst); break;
	case MachineInstOpcodeX64::sub64: sub64(inst); break;
	case MachineInstOpcodeX64::sub32: sub32(inst); break;
	case MachineInstOpcodeX64::sub16: sub16(inst); break;
	case MachineInstOpcodeX64::sub8: sub8(inst); break;
	case MachineInstOpcodeX64::not64: not64(inst); break;
	case MachineInstOpcodeX64::not32: not32(inst); break;
	case MachineInstOpcodeX64::not16: not16(inst); break;
	case MachineInstOpcodeX64::not8: not8(inst); break;
	case MachineInstOpcodeX64::neg64: neg64(inst); break;
	case MachineInstOpcodeX64::neg32: neg32(inst); break;
	case MachineInstOpcodeX64::neg16: neg16(inst); break;
	case MachineInstOpcodeX64::neg8: neg8(inst); break;
	case MachineInstOpcodeX64::shl64: shl64(inst); break;
	case MachineInstOpcodeX64::shl32: shl32(inst); break;
	case MachineInstOpcodeX64::shl16: shl16(inst); break;
	case MachineInstOpcodeX64::shl8: shl8(inst); break;
	case MachineInstOpcodeX64::shr64: shr64(inst); break;
	case MachineInstOpcodeX64::shr32: shr32(inst); break;
	case MachineInstOpcodeX64::shr16: shr16(inst); break;
	case MachineInstOpcodeX64::shr8: shr8(inst); break;
	case MachineInstOpcodeX64::sar64: sar64(inst); break;
	case MachineInstOpcodeX64::sar32: sar32(inst); break;
	case MachineInstOpcodeX64::sar16: sar16(inst); break;
	case MachineInstOpcodeX64::sar8: sar8(inst); break;
	case MachineInstOpcodeX64::and64: and64(inst); break;
	case MachineInstOpcodeX64::and32: and32(inst); break;
	case MachineInstOpcodeX64::and16: and16(inst); break;
	case MachineInstOpcodeX64::and8: and8(inst); break;
	case MachineInstOpcodeX64::or64: or64(inst); break;
	case MachineInstOpcodeX64::or32: or32(inst); break;
	case MachineInstOpcodeX64::or16: or16(inst); break;
	case MachineInstOpcodeX64::or8: or8(inst); break;
	case MachineInstOpcodeX64::xorpd: xorpd(inst); break;
	case MachineInstOpcodeX64::xorps: xorps(inst); break;
	case MachineInstOpcodeX64::xor64: xor64(inst); break;
	case MachineInstOpcodeX64::xor32: xor32(inst); break;
	case MachineInstOpcodeX64::xor16: xor16(inst); break;
	case MachineInstOpcodeX64::xor8: xor8(inst); break;
	case MachineInstOpcodeX64::mulss: mulss(inst); break;
	case MachineInstOpcodeX64::mulsd: mulsd(inst); break;
	case MachineInstOpcodeX64::imul64: imul64(inst); break;
	case MachineInstOpcodeX64::imul32: imul32(inst); break;
	case MachineInstOpcodeX64::imul16: imul16(inst); break;
	case MachineInstOpcodeX64::imul8: imul8(inst); break;
	case MachineInstOpcodeX64::divss: divss(inst); break;
	case MachineInstOpcodeX64::divsd: divsd(inst); break;
	case MachineInstOpcodeX64::idiv64: idiv64(inst); break;
	case MachineInstOpcodeX64::idiv32: idiv32(inst); break;
	case MachineInstOpcodeX64::idiv16: idiv16(inst); break;
	case MachineInstOpcodeX64::idiv8: idiv8(inst); break;
	case MachineInstOpcodeX64::div64: div64(inst); break;
	case MachineInstOpcodeX64::div32: div32(inst); break;
	case MachineInstOpcodeX64::div16: div16(inst); break;
	case MachineInstOpcodeX64::div8: div8(inst); break;
	case MachineInstOpcodeX64::ucomisd: ucomisd(inst); break;
	case MachineInstOpcodeX64::ucomiss: ucomiss(inst); break;
	case MachineInstOpcodeX64::cmp64: cmp64(inst); break;
	case MachineInstOpcodeX64::cmp32: cmp32(inst); break;
	case MachineInstOpcodeX64::cmp16: cmp16(inst); break;
	case MachineInstOpcodeX64::cmp8: cmp8(inst); break;
	case MachineInstOpcodeX64::setl: setl(inst); break;
	case MachineInstOpcodeX64::setb: setb(inst); break;
	case MachineInstOpcodeX64::seta: seta(inst); break;
	case MachineInstOpcodeX64::setg: setg(inst); break;
	case MachineInstOpcodeX64::setle: setle(inst); break;
	case MachineInstOpcodeX64::setbe: setbe(inst); break;
	case MachineInstOpcodeX64::setae: setae(inst); break;
	case MachineInstOpcodeX64::setge: setge(inst); break;
	case MachineInstOpcodeX64::sete: sete(inst); break;
	case MachineInstOpcodeX64::setne: setne(inst); break;
	case MachineInstOpcodeX64::setp: setp(inst); break;
	case MachineInstOpcodeX64::setnp: setnp(inst); break;
	case MachineInstOpcodeX64::cvtsd2ss: cvtsd2ss(inst); break;
	case MachineInstOpcodeX64::cvtss2sd: cvtss2sd(inst); break;
	case MachineInstOpcodeX64::cvttsd2si: cvttsd2si(inst); break;
	case MachineInstOpcodeX64::cvttss2si: cvttss2si(inst); break;
	case MachineInstOpcodeX64::cvtsi2sd: cvtsi2sd(inst); break;
	case MachineInstOpcodeX64::cvtsi2ss: cvtsi2ss(inst); break;
	case MachineInstOpcodeX64::jmp: jmp(inst); break;
	case MachineInstOpcodeX64::je: je(inst); break;
	case MachineInstOpcodeX64::jne: jne(inst); break;
	case MachineInstOpcodeX64::call: call(inst); break;
	case MachineInstOpcodeX64::ret: ret(inst); break;
	case MachineInstOpcodeX64::push: push(inst); break;
	case MachineInstOpcodeX64::pop: pop(inst); break;
	case MachineInstOpcodeX64::movdqa: movdqa(inst); break;
	}
}

void MachineCodeWriterX64::nop(MachineInst* inst)
{
	emitInstZO(OpFlags::NP, 0x90);
}

void MachineCodeWriterX64::lea(MachineInst* inst)
{
	X64Instruction x64inst;
	setR(x64inst, inst->operands[0]);

	if (inst->operands.size() == 3)
	{
		x64inst.disp = (int)inst->operands[2].immvalue;
		x64inst.dispsize = (x64inst.disp > -127 && x64inst.disp < 127) ? 8 : 32;
		if (inst->operands[1].registerIndex == (int)RegisterNameX64::rsp || inst->operands[1].registerIndex == (int)RegisterNameX64::r12)
		{
			x64inst.mod = (x64inst.dispsize == 8) ? 1 : 2;
			x64inst.rm = 4;
			x64inst.scale = 0;
			x64inst.index = 4;
			x64inst.base = getPhysReg(inst->operands[1]);
			x64inst.sib = true;
		}
		else
		{
			x64inst.mod = (x64inst.dispsize == 8) ? 1 : 2;
			x64inst.rm = getPhysReg(inst->operands[1]);
		}
	}
	else
	{
		setM(x64inst, inst->operands[1], true);
	}

	writeInst(OpFlags::RexW, { 0x8d }, x64inst, inst);
}

void MachineCodeWriterX64::loadss(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf3, 0x0f, 0x10 }, inst, true);
}

void MachineCodeWriterX64::loadsd(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf2, 0x0f, 0x10 }, inst, true);
}

void MachineCodeWriterX64::load64(MachineInst* inst)
{
	emitInstRM(OpFlags::RexW, 0x8b, inst, true);
}

void MachineCodeWriterX64::load32(MachineInst* inst)
{
	emitInstRM(0, 0x8b, inst, true);
}

void MachineCodeWriterX64::load16(MachineInst* inst)
{
	emitInstRM(OpFlags::SizeOverride, 0x8b, inst, true);
}

void MachineCodeWriterX64::load8(MachineInst* inst)
{
	emitInstRM(OpFlags::Rex, 0x8a, inst, true);
}

void MachineCodeWriterX64::storess(MachineInst* inst)
{
	emitInstSSE_MR(0, { 0xf3, 0x0f, 0x11 }, inst, true);
}

void MachineCodeWriterX64::storesd(MachineInst* inst)
{
	emitInstSSE_MR(0, { 0xf2, 0x0f, 0x11 }, inst, true);
}

void MachineCodeWriterX64::store64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::RexW, 0xc7, 32, inst, true);
	else
		emitInstMR(OpFlags::RexW, 0x89, inst, true);
}

void MachineCodeWriterX64::store32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(0, 0xc7, 32, inst, true);
	else
		emitInstMR(0, 0x89, inst, true);
}

void MachineCodeWriterX64::store16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::SizeOverride, 0xc7, 16, inst, true);
	else
		emitInstMR(OpFlags::SizeOverride, 0x89, inst, true);
}

void MachineCodeWriterX64::store8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::Rex, 0xc6, 8, inst, true);
	else
		emitInstMR(OpFlags::Rex, 0x88, inst, true);
}

void MachineCodeWriterX64::movss(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf3, 0x0f, 0x10 }, inst);
}

void MachineCodeWriterX64::movsd(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf2, 0x0f, 0x10 }, inst);
}

void MachineCodeWriterX64::mov64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstOI(OpFlags::RexW, 0xb8, 64, inst);
	else
		emitInstRM(OpFlags::RexW, 0x8b, inst);
}

void MachineCodeWriterX64::mov32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstOI(0, 0xb8, 32, inst);
	else
		emitInstRM(0, 0x8b, inst);
}

void MachineCodeWriterX64::mov16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstOI(OpFlags::SizeOverride, 0xb8, 16, inst);
	else
		emitInstRM(OpFlags::SizeOverride, 0x8b, inst);
}

void MachineCodeWriterX64::mov8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstOI(OpFlags::Rex, 0xb0, 8, inst);
	else
		emitInstRM(OpFlags::Rex, 0x8a, inst);
}

void MachineCodeWriterX64::movsx8_16(MachineInst* inst)
{
	emitInstRM(OpFlags::Rex | OpFlags::SizeOverride, { 0x0f, 0xbe }, inst);
}

void MachineCodeWriterX64::movsx8_32(MachineInst* inst)
{
	emitInstRM(OpFlags::Rex, { 0x0f, 0xbe }, inst);
}

void MachineCodeWriterX64::movsx8_64(MachineInst* inst)
{
	emitInstRM(OpFlags::RexW, { 0x0f, 0xbe }, inst);
}

void MachineCodeWriterX64::movsx16_32(MachineInst* inst)
{
	emitInstRM(0, { 0x0f, 0xbf }, inst);
}

void MachineCodeWriterX64::movsx16_64(MachineInst* inst)
{
	emitInstRM(OpFlags::RexW, { 0x0f, 0xbf }, inst);
}

void MachineCodeWriterX64::movsx32_64(MachineInst* inst)
{
	emitInstRM(OpFlags::RexW, { 0x63 }, inst);
}

void MachineCodeWriterX64::movzx8_16(MachineInst* inst)
{
	emitInstRM(OpFlags::Rex | OpFlags::SizeOverride, { 0x0f, 0xb6 }, inst);
}

void MachineCodeWriterX64::movzx8_32(MachineInst* inst)
{
	emitInstRM(OpFlags::Rex, { 0x0f, 0xb6 }, inst);
}

void MachineCodeWriterX64::movzx8_64(MachineInst* inst)
{
	emitInstRM(OpFlags::RexW, { 0x0f, 0xb6 }, inst);
}

void MachineCodeWriterX64::movzx16_32(MachineInst* inst)
{
	emitInstRM(0, { 0x0f, 0xb7 }, inst);
}

void MachineCodeWriterX64::movzx16_64(MachineInst* inst)
{
	emitInstRM(OpFlags::RexW, { 0x0f, 0xb7 }, inst);
}

void MachineCodeWriterX64::addss(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf3, 0x0f, 0x58 }, inst);
}

void MachineCodeWriterX64::addsd(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf2, 0x0f, 0x58 }, inst);
}

void MachineCodeWriterX64::add64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::RexW, 0x81, 32, inst);
	else
		emitInstRM(OpFlags::RexW, 0x03, inst);
}

void MachineCodeWriterX64::add32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(0, 0x81, 32, inst);
	else
		emitInstRM(0, 0x03, inst);
}

void MachineCodeWriterX64::add16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::SizeOverride, 0x81, 16, inst);
	else
		emitInstRM(OpFlags::SizeOverride, 0x03, inst);
}

void MachineCodeWriterX64::add8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::Rex, 0x80, 8, inst);
	else
		emitInstRM(OpFlags::Rex, 0x02, inst);
}

void MachineCodeWriterX64::subss(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf3, 0x0f, 0x5c }, inst);
}

void MachineCodeWriterX64::subsd(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf2, 0x0f, 0x5c }, inst);
}

void MachineCodeWriterX64::sub64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::RexW, 0x81, 5, 32, inst);
	else
		emitInstRM(OpFlags::RexW, 0x2b, inst);
}

void MachineCodeWriterX64::sub32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(0, 0x81, 5, 32, inst);
	else
		emitInstRM(0, 0x2b, inst);
}

void MachineCodeWriterX64::sub16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::SizeOverride, 0x81, 5, 16, inst);
	else
		emitInstRM(OpFlags::SizeOverride, 0x2b, inst);
}

void MachineCodeWriterX64::sub8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::Rex, 0x80, 5, 8, inst);
	else
		emitInstRM(OpFlags::Rex, 0x2a, inst);
}

void MachineCodeWriterX64::not64(MachineInst* inst)
{
	emitInstM(OpFlags::RexW, 0xf7, 2, inst);
}

void MachineCodeWriterX64::not32(MachineInst* inst)
{
	emitInstM(0, 0xf7, 2, inst);
}

void MachineCodeWriterX64::not16(MachineInst* inst)
{
	emitInstM(OpFlags::SizeOverride, 0xf7, 2, inst);
}

void MachineCodeWriterX64::not8(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, 0xf6, 2, inst);
}

void MachineCodeWriterX64::neg64(MachineInst* inst)
{
	emitInstM(OpFlags::RexW, 0xf7, 3, inst);
}

void MachineCodeWriterX64::neg32(MachineInst* inst)
{
	emitInstM(0, 0xf7, 3, inst);
}

void MachineCodeWriterX64::neg16(MachineInst* inst)
{
	emitInstM(OpFlags::SizeOverride, 0xf7, 3, inst);
}

void MachineCodeWriterX64::neg8(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, 0xf6, 3, inst);
}

void MachineCodeWriterX64::shl64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::RexW, 0xc1, 4, 8, inst);
	else
		emitInstMC(OpFlags::RexW, 0xd3, 4, inst);
}

void MachineCodeWriterX64::shl32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(0, 0xc1, 4, 8, inst);
	else
		emitInstMC(0, 0xd3, 4, inst);
}

void MachineCodeWriterX64::shl16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::SizeOverride, 0xc1, 4, 8, inst);
	else
		emitInstMC(OpFlags::SizeOverride, 0xd3, 4, inst);
}

void MachineCodeWriterX64::shl8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::Rex, 0xc0, 4, 8, inst);
	else
		emitInstMC(OpFlags::Rex, 0xd2, 4, inst);
}

void MachineCodeWriterX64::shr64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::RexW, 0xc1, 5, 8, inst);
	else
		emitInstMC(OpFlags::RexW, 0xd3, 5, inst);
}

void MachineCodeWriterX64::shr32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(0, 0xc1, 5, 8, inst);
	else
		emitInstMC(0, 0xd3, 5, inst);
}

void MachineCodeWriterX64::shr16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::SizeOverride, 0xc1, 5, 8, inst);
	else
		emitInstMC(OpFlags::SizeOverride, 0xd3, 5, inst);
}

void MachineCodeWriterX64::shr8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::Rex, 0xc0, 5, 8, inst);
	else
		emitInstMC(OpFlags::Rex, 0xd2, 5, inst);
}

void MachineCodeWriterX64::sar64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::RexW, 0xc1, 7, 8, inst);
	else
		emitInstMC(OpFlags::RexW, 0xd3, 7, inst);
}

void MachineCodeWriterX64::sar32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(0, 0xc1, 7, 8, inst);
	else
		emitInstMC(0, 0xd3, 7, inst);
}

void MachineCodeWriterX64::sar16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::SizeOverride, 0xc1, 7, 8, inst);
	else
		emitInstMC(OpFlags::SizeOverride, 0xd3, 7, inst);
}

void MachineCodeWriterX64::sar8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::Rex, 0xc0, 7, 8, inst);
	else
		emitInstMC(OpFlags::Rex, 0xd2, 7, inst);
}

void MachineCodeWriterX64::and64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::RexW, 0x81, 4, 32, inst);
	else
		emitInstRM(OpFlags::RexW, 0x23, inst);
}

void MachineCodeWriterX64::and32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(0, 0x81, 4, 32, inst);
	else
		emitInstRM(0, 0x23, inst);
}

void MachineCodeWriterX64::and16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::SizeOverride, 0x81, 4, 16, inst);
	else
		emitInstRM(OpFlags::SizeOverride, 0x23, inst);
}

void MachineCodeWriterX64::and8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::Rex, 0x80, 4, 8, inst);
	else
		emitInstRM(OpFlags::Rex, 0x22, inst);
}

void MachineCodeWriterX64::or64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::RexW, 0x81, 1, 32, inst);
	else
		emitInstRM(OpFlags::RexW, 0x0b, inst);
}

void MachineCodeWriterX64::or32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(0, 0x81, 1, 32, inst);
	else
		emitInstRM(0, 0x0b, inst);
}

void MachineCodeWriterX64::or16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::SizeOverride, 0x81, 1, 16, inst);
	else
		emitInstRM(OpFlags::SizeOverride, 0x0b, inst);
}

void MachineCodeWriterX64::or8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::Rex, 0x80, 1, 8, inst);
	else
		emitInstRM(OpFlags::Rex, 0x0a, inst);
}

void MachineCodeWriterX64::xorpd(MachineInst* inst)
{
	emitInstSSE_RM(OpFlags::SizeOverride, { 0x0f, 0x57 }, inst);
}

void MachineCodeWriterX64::xorps(MachineInst* inst)
{
	emitInstSSE_RM(OpFlags::NP, { 0x0f, 0x57 }, inst);
}

void MachineCodeWriterX64::xor64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::RexW, 0x81, 6, 32, inst);
	else
		emitInstRM(OpFlags::RexW, 0x33, inst);
}

void MachineCodeWriterX64::xor32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(0, 0x81, 6, 32, inst);
	else
		emitInstRM(0, 0x33, inst);
}

void MachineCodeWriterX64::xor16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::SizeOverride, 0x81, 6, 16, inst);
	else
		emitInstRM(OpFlags::SizeOverride, 0x33, inst);
}

void MachineCodeWriterX64::xor8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::Rex, 0x80, 6, 8, inst);
	else
		emitInstRM(OpFlags::Rex, 0x32, inst);
}

void MachineCodeWriterX64::mulss(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf3, 0x0f, 0x59 }, inst);
}

void MachineCodeWriterX64::mulsd(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf2, 0x0f, 0x59 }, inst);
}

void MachineCodeWriterX64::imul64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstRMI(OpFlags::RexW, 0x69, 32, inst);
	else
		emitInstRM(OpFlags::RexW, { 0x0f, 0xaf }, inst);
}

void MachineCodeWriterX64::imul32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstRMI(0, 0x69, 32, inst);
	else
		emitInstRM(0, { 0x0f, 0xaf }, inst);
}

void MachineCodeWriterX64::imul16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstRMI(OpFlags::SizeOverride, 0x69, 16, inst);
	else
		emitInstRM(OpFlags::SizeOverride, { 0x0f, 0xaf }, inst);
}

void MachineCodeWriterX64::imul8(MachineInst* inst)
{
	// Doesn't have an immediate multiply. AX <- AL * r/m byte
	emitInstM(OpFlags::Rex, 0xf6, 5, inst);
}

void MachineCodeWriterX64::divss(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf3, 0x0f, 0x5e }, inst);
}

void MachineCodeWriterX64::divsd(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf2, 0x0f, 0x5e }, inst);
}

void MachineCodeWriterX64::idiv64(MachineInst* inst)
{
	// Doesn't have an immediate divide. Always uses RAX and RDX.
	emitInstM(OpFlags::RexW, 0xf7, 7, inst);
}

void MachineCodeWriterX64::idiv32(MachineInst* inst)
{
	// Doesn't have an immediate divide. Always uses EAX and EDX.
	emitInstM(0, 0xf7, 7, inst);
}

void MachineCodeWriterX64::idiv16(MachineInst* inst)
{
	// Doesn't have an immediate divide. Always uses AX and DX.
	emitInstM(OpFlags::SizeOverride, 0xf7, 7, inst);
}

void MachineCodeWriterX64::idiv8(MachineInst* inst)
{
	// Doesn't have an immediate divide. Always uses AX.
	emitInstM(OpFlags::Rex, 0xf6, 7, inst);
}

void MachineCodeWriterX64::div64(MachineInst* inst)
{
	// Doesn't have an immediate divide. Always uses RAX and RDX.
	emitInstM(OpFlags::RexW, 0xf7, 6, inst);
}

void MachineCodeWriterX64::div32(MachineInst* inst)
{
	// Doesn't have an immediate divide. Always uses EAX and EDX.
	emitInstM(0, 0xf7, 6, inst);
}

void MachineCodeWriterX64::div16(MachineInst* inst)
{
	// Doesn't have an immediate divide. Always uses AX and DX.
	emitInstM(OpFlags::SizeOverride, 0xf7, 6, inst);
}

void MachineCodeWriterX64::div8(MachineInst* inst)
{
	// Doesn't have an immediate divide. Always uses AX.
	emitInstM(OpFlags::Rex, 0xf6, 6, inst);
}

void MachineCodeWriterX64::ucomisd(MachineInst* inst)
{
	emitInstSSE_RM(OpFlags::SizeOverride, { 0x0f, 0x2e }, inst);
}

void MachineCodeWriterX64::ucomiss(MachineInst* inst)
{
	emitInstSSE_RM(OpFlags::NP, { 0x0f, 0x2e }, inst);
}

void MachineCodeWriterX64::cmp64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::RexW, 0x81, 7, 32, inst);
	else
		emitInstRM(OpFlags::RexW, 0x3b, inst);
}

void MachineCodeWriterX64::cmp32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(0, 0x81, 7, 32, inst);
	else
		emitInstRM(0, 0x3b, inst);
}

void MachineCodeWriterX64::cmp16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::SizeOverride, 0x81, 7, 16, inst);
	else
		emitInstRM(OpFlags::SizeOverride, 0x3b, inst);
}

void MachineCodeWriterX64::cmp8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::Rex, 0x80, 7, 8, inst);
	else
		emitInstRM(OpFlags::Rex, 0x3a, inst);
}

void MachineCodeWriterX64::setl(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x9c }, 8, inst);
}

void MachineCodeWriterX64::setb(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x92 }, 8, inst);
}

void MachineCodeWriterX64::seta(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x97 }, 8, inst);
}

void MachineCodeWriterX64::setg(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x9f }, 8, inst);
}

void MachineCodeWriterX64::setle(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x9e }, 8, inst);
}

void MachineCodeWriterX64::setbe(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x96 }, 8, inst);
}

void MachineCodeWriterX64::setae(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x93 }, 8, inst);
}

void MachineCodeWriterX64::setge(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x9d }, 8, inst);
}

void MachineCodeWriterX64::sete(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x94 }, 8, inst);
}

void MachineCodeWriterX64::setne(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x95 }, 8, inst);
}

void MachineCodeWriterX64::setp(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x9a }, 8, inst);
}

void MachineCodeWriterX64::setnp(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x9b }, 8, inst);
}

void MachineCodeWriterX64::cvtsd2ss(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf2, 0x0f, 0x5a }, inst);
}

void MachineCodeWriterX64::cvtss2sd(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf3, 0x0f, 0x5a }, inst);
}

void MachineCodeWriterX64::cvttsd2si(MachineInst* inst)
{
	emitInstSSE_RM(OpFlags::RexW, { 0xf2, 0x0f, 0x2c }, inst);
}

void MachineCodeWriterX64::cvttss2si(MachineInst* inst)
{
	emitInstSSE_RM(OpFlags::RexW, { 0xf3, 0x0f, 0x2c }, inst); // Always QWORD output
}

void MachineCodeWriterX64::cvtsi2sd(MachineInst* inst)
{
	emitInstSSE_RM(OpFlags::RexW, { 0xf2, 0x0f, 0x2a }, inst);
}

void MachineCodeWriterX64::cvtsi2ss(MachineInst* inst)
{
	emitInstSSE_RM(OpFlags::RexW, { 0xf3, 0x0f, 0x2a }, inst); // Always QWORD output
}

void MachineCodeWriterX64::jmp(MachineInst* inst)
{
	writeOpcode(0, { 0xe9 }, 0, 0, 0);
	writeImm(32, 0xffffffff);
	codeholder->bbRelocateInfo.push_back({ codeholder->code.size() - 4, inst->operands[0].bb });
}

void MachineCodeWriterX64::je(MachineInst* inst)
{
	writeOpcode(0, { 0x0f, 0x84 }, 0, 0, 0);
	writeImm(32, 0xffffffff);
	codeholder->bbRelocateInfo.push_back({ codeholder->code.size() - 4, inst->operands[0].bb });
}

void MachineCodeWriterX64::jne(MachineInst* inst)
{
	writeOpcode(0, { 0x0f, 0x85 }, 0, 0, 0);
	writeImm(32, 0xffffffff);
	codeholder->bbRelocateInfo.push_back({ codeholder->code.size() - 4, inst->operands[0].bb });
}

void MachineCodeWriterX64::call(MachineInst* inst)
{
	int regindex = (int)RegisterNameX64::rax;

	// mov rax, imm64
	if (inst->operands[0].type == MachineOperandType::func)
	{
		int opcode = 0xb8;
		opcode |= (regindex & 7);

		writeOpcode(OpFlags::RexW, { opcode }, 0, 0, regindex >> 3);
		writeImm(64, 0xffff'ffff'ffff'ffff);
		codeholder->callRelocateInfo.push_back({ codeholder->code.size() - 8, inst->operands[0].func });
	}

	// call rax
	writeOpcode(0, { 0xff }, 0, 0, regindex >> 3);
	writeModRM(3, 2, regindex);
}

void MachineCodeWriterX64::ret(MachineInst* inst)
{
	emitInstZO(0, 0xc3);
}

void MachineCodeWriterX64::push(MachineInst* inst)
{
	emitInstO(0, 0x50, inst);
}

void MachineCodeWriterX64::pop(MachineInst* inst)
{
	emitInstO(0, 0x58, inst);
}

void MachineCodeWriterX64::movdqa(MachineInst* inst)
{
	if (inst->operands[0].type == MachineOperandType::reg)
	{
		emitInstSSE_RM(OpFlags::SizeOverride, { 0x0f, 0x6f }, inst);
	}
	else
	{
		emitInstSSE_MR(OpFlags::SizeOverride, { 0x0f, 0x7f }, inst);
	}
}

void MachineCodeWriterX64::emitInstZO(int flags, int opcode)
{
	writeOpcode(flags, { opcode }, 0, 0, 0);
}

void MachineCodeWriterX64::emitInstO(int flags, int opcode, MachineInst* inst)
{
	int regindex = getPhysReg(inst->operands[0]);
	opcode |= (regindex & 7);

	writeOpcode(flags, { opcode }, 0, 0, regindex >> 3);
}

void MachineCodeWriterX64::emitInstOI(int flags, int opcode, int immsize, MachineInst* inst)
{
	int regindex = getPhysReg(inst->operands[0]);
	opcode |= (regindex & 7);

	writeOpcode(flags, { opcode }, 0, 0, regindex >> 3);
	writeImm(immsize, inst->operands[1].immvalue);
}

void MachineCodeWriterX64::emitInstMI(int flags, int opcode, int immsize, MachineInst* inst, bool memptr)
{
	emitInstMI(flags, opcode, 0, immsize, inst, memptr);
}

void MachineCodeWriterX64::emitInstMI(int flags, int opcode, int modopcode, int immsize, MachineInst* inst, bool memptr)
{
	X64Instruction x64inst;
	setR(x64inst, modopcode);
	setM(x64inst, inst->operands[0], memptr);
	setI(x64inst, immsize, inst->operands[1]);
	writeInst(flags, { opcode }, x64inst, inst);
}

void MachineCodeWriterX64::emitInstMR(int flags, int opcode, MachineInst* inst, bool memptr)
{
	X64Instruction x64inst;
	setM(x64inst, inst->operands[0], memptr);
	setR(x64inst, inst->operands[1]);
	writeInst(flags, { opcode }, x64inst, inst);
}

void MachineCodeWriterX64::emitInstM(int flags, int opcode, MachineInst* inst, bool memptr)
{
	emitInstM(flags, { opcode }, 0, inst, memptr);
}

void MachineCodeWriterX64::emitInstM(int flags, int opcode, int modopcode, MachineInst* inst, bool memptr)
{
	emitInstM(flags, { opcode }, modopcode, inst, memptr);
}

void MachineCodeWriterX64::emitInstM(int flags, std::initializer_list<int> opcode, MachineInst* inst, bool memptr)
{
	emitInstM(flags, opcode, 0, inst, memptr);
}

void MachineCodeWriterX64::emitInstM(int flags, std::initializer_list<int> opcode, int modopcode, MachineInst* inst, bool memptr)
{
	X64Instruction x64inst;
	setR(x64inst, modopcode);
	setM(x64inst, inst->operands[0], memptr);
	writeInst(flags, opcode, x64inst, inst);
}

void MachineCodeWriterX64::emitInstMC(int flags, int opcode, MachineInst* inst, bool memptr)
{
	emitInstMC(flags, opcode, 0, inst, memptr);
}

void MachineCodeWriterX64::emitInstMC(int flags, int opcode, int modopcode, MachineInst* inst, bool memptr)
{
	// register in inst->operands[1] must be CL

	X64Instruction x64inst;
	setR(x64inst, modopcode);
	setM(x64inst, inst->operands[0], memptr);
	writeInst(flags, { opcode }, x64inst, inst);
}

void MachineCodeWriterX64::emitInstRM(int flags, int opcode, MachineInst* inst, bool memptr)
{
	emitInstRM(flags, { opcode }, inst, memptr);
}

void MachineCodeWriterX64::emitInstRM(int flags, std::initializer_list<int> opcode, MachineInst* inst, bool memptr)
{
	X64Instruction x64inst;
	setR(x64inst, inst->operands[0]);
	setM(x64inst, inst->operands[1], memptr);
	writeInst(flags, opcode, x64inst, inst);
}

void MachineCodeWriterX64::emitInstRMI(int flags, int opcode, int immsize, MachineInst* inst, bool memptr)
{
	X64Instruction x64inst;
	setR(x64inst, inst->operands[0]);
	setM(x64inst, inst->operands[0], memptr);
	setI(x64inst, immsize, inst->operands[1]);
	writeInst(flags, { opcode }, x64inst, inst);
}

void MachineCodeWriterX64::emitInstSSE_RM(int flags, std::initializer_list<int> opcode, MachineInst* inst, bool memptr)
{
	X64Instruction x64inst;
	setR(x64inst, inst->operands[0]);
	setM(x64inst, inst->operands[1], memptr);
	writeInst(flags, opcode, x64inst, inst);

	if (inst->operands[1].type == MachineOperandType::constant)
	{
		size_t codepos = codeholder->code.size() - 4;
		size_t datapos = codeholder->data.size();
		MachineConstant* constant = &sfunc->constants[inst->operands[1].constantIndex];
		codeholder->data.insert(codeholder->data.end(), constant->data, constant->data + constant->size);
		codeholder->dataRelocateInfo.push_back({ codepos, datapos });
	}
}

void MachineCodeWriterX64::emitInstSSE_MR(int flags, std::initializer_list<int> opcode, MachineInst* inst, bool memptr)
{
	X64Instruction x64inst;
	setM(x64inst, inst->operands[0], memptr);
	setR(x64inst, inst->operands[1]);
	writeInst(flags, opcode, x64inst, inst);
}

int MachineCodeWriterX64::getPhysReg(const MachineOperand& operand)
{
	if (operand.registerIndex < (int)RegisterNameX64::xmm0)
		return operand.registerIndex;
	else
		return operand.registerIndex - (int)RegisterNameX64::xmm0;
}

void MachineCodeWriterX64::setM(X64Instruction& x64inst, const MachineOperand& operand, bool memptr)
{
	if (operand.type == MachineOperandType::reg)
	{
		if (memptr)
		{
			if (operand.registerIndex == (int)RegisterNameX64::rsp || operand.registerIndex == (int)RegisterNameX64::r12)
			{
				x64inst.rm = (int)RegisterNameX64::rsp;
				x64inst.mod = 0;
				x64inst.scale = 0;
				x64inst.index = 4;
				x64inst.base = getPhysReg(operand);
				x64inst.sib = true;
			}
			else if (operand.registerIndex == (int)RegisterNameX64::rbp || operand.registerIndex == (int)RegisterNameX64::r13)
			{
				x64inst.rm = getPhysReg(operand);
				x64inst.mod = 1;
				x64inst.disp = 0;
				x64inst.dispsize = 8;
			}
			else
			{
				x64inst.rm = getPhysReg(operand);
				x64inst.mod = 0;
			}
		}
		else
		{
			x64inst.rm = getPhysReg(operand);
			x64inst.mod = 3;
		}

	}
	else if (operand.type == MachineOperandType::frameOffset)
	{
		bool dsa = sfunc->dynamicStackAllocations;
		x64inst.disp = sfunc->frameBaseOffset + operand.frameOffset;
		x64inst.dispsize = (x64inst.disp > -127 && x64inst.disp < 127) ? 8 : 32;
		x64inst.mod = (x64inst.dispsize == 8) ? 1 : 2;
		x64inst.rm = 4;
		x64inst.scale = 0;
		x64inst.index = 4;
		x64inst.base = (int)(dsa ? RegisterNameX64::rbp : RegisterNameX64::rsp);
		x64inst.sib = true;
	}
	else if (operand.type == MachineOperandType::spillOffset)
	{
		bool dsa = sfunc->dynamicStackAllocations;
		x64inst.disp = sfunc->spillBaseOffset + operand.spillOffset;
		x64inst.dispsize = (x64inst.disp > -127 && x64inst.disp < 127) ? 8 : 32;
		x64inst.mod = (x64inst.dispsize == 8) ? 1 : 2;
		x64inst.rm = 4;
		x64inst.scale = 0;
		x64inst.index = 4;
		x64inst.base = (int)(dsa ? RegisterNameX64::rbp : RegisterNameX64::rsp);
		x64inst.sib = true;
	}
	else if (operand.type == MachineOperandType::stackOffset)
	{
		x64inst.disp = operand.stackOffset;
		x64inst.dispsize = (x64inst.disp > -127 && x64inst.disp < 127) ? 8 : 32;
		x64inst.mod = (x64inst.dispsize == 8) ? 1 : 2;
		x64inst.rm = 4;
		x64inst.scale = 0;
		x64inst.index = 4;
		x64inst.base = (int)RegisterNameX64::rsp;
		x64inst.sib = true;
	}
	else if (operand.type == MachineOperandType::constant)
	{
		x64inst.disp = -1;
		x64inst.dispsize = 32;
		x64inst.mod = 0;
		x64inst.rm = 5;
	}
	else if (operand.type == MachineOperandType::imm)
	{
		x64inst.disp = (int32_t)operand.immvalue;
		x64inst.dispsize = 32;
		x64inst.mod = 0;
		x64inst.rm = 5;
	}
}

void MachineCodeWriterX64::setI(X64Instruction& x64inst, int immsize, const MachineOperand& operand)
{
	x64inst.immsize = immsize;
	x64inst.imm = operand.immvalue;
}

void MachineCodeWriterX64::setR(X64Instruction& x64inst, const MachineOperand& operand)
{
	if (operand.type == MachineOperandType::reg)
	{
		x64inst.modreg = getPhysReg(operand);
	}
}

void MachineCodeWriterX64::setR(X64Instruction& x64inst, int modopcode)
{
	x64inst.modreg = modopcode;
}

void MachineCodeWriterX64::writeInst(int flags, std::initializer_list<int> opcode, const X64Instruction& x64inst, MachineInst* debugInfo)
{
	if (debugInfo->lineNumber != -1)
	{
		MachineCodeHolder::DebugInfo info;
		info.offset = codeholder->code.size();
		info.fileIndex = debugInfo->fileIndex;
		info.lineNumber = debugInfo->lineNumber;
		codeholder->debugInfo.push_back(info);
	}

	writeOpcode(flags, opcode, x64inst.modreg >> 3, x64inst.index >> 3, (x64inst.rm | x64inst.base) >> 3);
	writeModRM(x64inst.mod, x64inst.modreg, x64inst.rm);
	if (x64inst.sib) writeSIB(x64inst.scale, x64inst.index, x64inst.base);
	if (x64inst.dispsize > 0) writeImm(x64inst.dispsize, x64inst.disp);
	if (x64inst.immsize > 0) writeImm(x64inst.immsize, x64inst.imm);
}

void MachineCodeWriterX64::writeOpcode(int flags, std::initializer_list<int> opcode, int r, int x, int b)
{
	if (flags & OpFlags::SizeOverride)
		codeholder->code.push_back(0x66);

	auto it = opcode.begin();
	if (*it == 0xf2 || *it == 0xf3)
	{
		codeholder->code.push_back(*it);
		++it;
	}

	int rexcode = 0;
	if (flags & OpFlags::RexW)
		rexcode |= 1 << 3;
	rexcode |= r << 2; // Extension of the ModR/M reg field
	rexcode |= x << 1; // Extension of the SIB index field
	rexcode |= b; // Extension of the ModR/M r/m field, SIB base field, or Opcode reg field

	if (rexcode != 0 || (flags & OpFlags::Rex))
		codeholder->code.push_back(0x40 | rexcode);

	while (it != opcode.end())
	{
		codeholder->code.push_back(*it);
		++it;
	}
}

void MachineCodeWriterX64::writeModRM(int mod, int modreg, int rm)
{
	modreg &= 7;
	rm &= 7;
	codeholder->code.push_back((mod << 6) | (modreg << 3) | rm);
}

void MachineCodeWriterX64::writeSIB(int scale, int index, int base)
{
	index &= 7;
	base &= 7;
	codeholder->code.push_back((scale << 6) | (index << 3) | base);
}

void MachineCodeWriterX64::writeImm(int immsize, uint64_t value)
{
	for (int i = 0; i < immsize; i += 8)
	{
		codeholder->code.push_back(value & 0xff);
		value >>= 8;
	}
}
