
#include "MachineCodeWriter.h"
#include "MachineCodeHolder.h"
#include "RegisterAllocator.h"
#include "UnwindInfoWindows.h"
#include "UnwindInfoUnix.h"

void MachineCodeWriter::codegen()
{
	funcBeginAddress = codeholder->code.size();

	basicblock(sfunc->prolog);

	for (MachineBasicBlock* bb : sfunc->basicBlocks)
	{
		basicblock(bb);
	}

	basicblock(sfunc->epilog);
}

void MachineCodeWriter::basicblock(MachineBasicBlock* bb)
{
	codeholder->bbOffsets[bb] = codeholder->code.size();
	for (MachineInst* inst : bb->code)
	{
		opcode(inst);
		inst->unwindOffset = (int)(codeholder->code.size() - funcBeginAddress);
	}
}

void MachineCodeWriter::opcode(MachineInst* inst)
{
	switch (inst->opcode)
	{
	default:
	case MachineInstOpcode::nop: nop(inst); break;
	case MachineInstOpcode::lea: lea(inst); break;
	case MachineInstOpcode::loadss: loadss(inst); break;
	case MachineInstOpcode::loadsd: loadsd(inst); break;
	case MachineInstOpcode::load64: load64(inst); break;
	case MachineInstOpcode::load32: load32(inst); break;
	case MachineInstOpcode::load16: load16(inst); break;
	case MachineInstOpcode::load8: load8(inst); break;
	case MachineInstOpcode::storess: storess(inst); break;
	case MachineInstOpcode::storesd: storesd(inst); break;
	case MachineInstOpcode::store64: store64(inst); break;
	case MachineInstOpcode::store32: store32(inst); break;
	case MachineInstOpcode::store16: store16(inst); break;
	case MachineInstOpcode::store8: store8(inst); break;
	case MachineInstOpcode::movss: movss(inst); break;
	case MachineInstOpcode::movsd: movsd(inst); break;
	case MachineInstOpcode::mov64: mov64(inst); break;
	case MachineInstOpcode::mov32: mov32(inst); break;
	case MachineInstOpcode::mov16: mov16(inst); break;
	case MachineInstOpcode::mov8: mov8(inst); break;
	case MachineInstOpcode::addss: addss(inst); break;
	case MachineInstOpcode::addsd: addsd(inst); break;
	case MachineInstOpcode::add64: add64(inst); break;
	case MachineInstOpcode::add32: add32(inst); break;
	case MachineInstOpcode::add16: add16(inst); break;
	case MachineInstOpcode::add8: add8(inst); break;
	case MachineInstOpcode::subss: subss(inst); break;
	case MachineInstOpcode::subsd: subsd(inst); break;
	case MachineInstOpcode::sub64: sub64(inst); break;
	case MachineInstOpcode::sub32: sub32(inst); break;
	case MachineInstOpcode::sub16: sub16(inst); break;
	case MachineInstOpcode::sub8: sub8(inst); break;
	case MachineInstOpcode::not64: not64(inst); break;
	case MachineInstOpcode::not32: not32(inst); break;
	case MachineInstOpcode::not16: not16(inst); break;
	case MachineInstOpcode::not8: not8(inst); break;
	case MachineInstOpcode::neg64: neg64(inst); break;
	case MachineInstOpcode::neg32: neg32(inst); break;
	case MachineInstOpcode::neg16: neg16(inst); break;
	case MachineInstOpcode::neg8: neg8(inst); break;
	case MachineInstOpcode::shl64: shl64(inst); break;
	case MachineInstOpcode::shl32: shl32(inst); break;
	case MachineInstOpcode::shl16: shl16(inst); break;
	case MachineInstOpcode::shl8: shl8(inst); break;
	case MachineInstOpcode::sar64: sar64(inst); break;
	case MachineInstOpcode::sar32: sar32(inst); break;
	case MachineInstOpcode::sar16: sar16(inst); break;
	case MachineInstOpcode::sar8: sar8(inst); break;
	case MachineInstOpcode::and64: and64(inst); break;
	case MachineInstOpcode::and32: and32(inst); break;
	case MachineInstOpcode::and16: and16(inst); break;
	case MachineInstOpcode::and8: and8(inst); break;
	case MachineInstOpcode::or64: or64(inst); break;
	case MachineInstOpcode::or32: or32(inst); break;
	case MachineInstOpcode::or16: or16(inst); break;
	case MachineInstOpcode::or8: or8(inst); break;
	case MachineInstOpcode::xorpd: xorpd(inst); break;
	case MachineInstOpcode::xorps: xorps(inst); break;
	case MachineInstOpcode::xor64: xor64(inst); break;
	case MachineInstOpcode::xor32: xor32(inst); break;
	case MachineInstOpcode::xor16: xor16(inst); break;
	case MachineInstOpcode::xor8: xor8(inst); break;
	case MachineInstOpcode::mulss: mulss(inst); break;
	case MachineInstOpcode::mulsd: mulsd(inst); break;
	case MachineInstOpcode::imul64: imul64(inst); break;
	case MachineInstOpcode::imul32: imul32(inst); break;
	case MachineInstOpcode::imul16: imul16(inst); break;
	case MachineInstOpcode::imul8: imul8(inst); break;
	case MachineInstOpcode::divss: divss(inst); break;
	case MachineInstOpcode::divsd: divsd(inst); break;
	case MachineInstOpcode::idiv64: idiv64(inst); break;
	case MachineInstOpcode::idiv32: idiv32(inst); break;
	case MachineInstOpcode::idiv16: idiv16(inst); break;
	case MachineInstOpcode::idiv8: idiv8(inst); break;
	case MachineInstOpcode::mul64: mul64(inst); break;
	case MachineInstOpcode::mul32: mul32(inst); break;
	case MachineInstOpcode::mul16: mul16(inst); break;
	case MachineInstOpcode::mul8: mul8(inst); break;
	case MachineInstOpcode::div64: div64(inst); break;
	case MachineInstOpcode::div32: div32(inst); break;
	case MachineInstOpcode::div16: div16(inst); break;
	case MachineInstOpcode::div8: div8(inst); break;
	case MachineInstOpcode::ucomisd: ucomisd(inst); break;
	case MachineInstOpcode::ucomiss: ucomiss(inst); break;
	case MachineInstOpcode::cmp64: cmp64(inst); break;
	case MachineInstOpcode::cmp32: cmp32(inst); break;
	case MachineInstOpcode::cmp16: cmp16(inst); break;
	case MachineInstOpcode::cmp8: cmp8(inst); break;
	case MachineInstOpcode::setl: setl(inst); break;
	case MachineInstOpcode::setb: setb(inst); break;
	case MachineInstOpcode::seta: seta(inst); break;
	case MachineInstOpcode::setg: setg(inst); break;
	case MachineInstOpcode::setle: setle(inst); break;
	case MachineInstOpcode::setbe: setbe(inst); break;
	case MachineInstOpcode::setae: setae(inst); break;
	case MachineInstOpcode::setge: setge(inst); break;
	case MachineInstOpcode::sete: sete(inst); break;
	case MachineInstOpcode::setne: setne(inst); break;
	case MachineInstOpcode::cvtsd2ss: cvtsd2ss(inst); break;
	case MachineInstOpcode::cvtss2sd: cvtss2sd(inst); break;
	case MachineInstOpcode::cvttsd2si: cvttsd2si(inst); break;
	case MachineInstOpcode::cvttss2si: cvttss2si(inst); break;
	case MachineInstOpcode::cvtsi2sd: cvtsi2sd(inst); break;
	case MachineInstOpcode::cvtsi2ss: cvtsi2ss(inst); break;
	case MachineInstOpcode::jmp: jmp(inst); break;
	case MachineInstOpcode::jz: jz(inst); break;
	case MachineInstOpcode::call: call(inst); break;
	case MachineInstOpcode::alloca_: alloca_(inst); break;
	case MachineInstOpcode::ret: ret(inst); break;
	case MachineInstOpcode::push: push(inst); break;
	case MachineInstOpcode::pop: pop(inst); break;
	}
}

void MachineCodeWriter::nop(MachineInst* inst)
{
	emitInstZO(OpFlags::NP, 0x90);
}

void MachineCodeWriter::lea(MachineInst* inst)
{
	// ModR/M:
	int mod = 0;
	int modreg = 0;
	int rm = 0;

	// SIB:
	int scale = 0;
	int index = 0;
	int base = 0;
	bool sib = false;

	// Disp:
	int32_t disp = 0;
	int dispsize = 0;

	modreg = getPhysReg(inst->operands[0]);

	disp = (int32_t)inst->operands[2].immvalue;
	dispsize = (disp > -127 && disp < 127) ? 8 : 32;
	mod = (dispsize == 8) ? 1 : 2;
	rm = getPhysReg(inst->operands[1]);

	writeOpcode(OpFlags::RexW, { 0x8d }, modreg >> 3, index >> 3, rm >> 3);
	writeModRM(mod, modreg, rm);
	if (sib) writeSIB(scale, index, base);
	if (dispsize > 0) writeImm(dispsize, disp);
}

void MachineCodeWriter::loadss(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf3, 0x0f, 0x10 }, inst);
}

void MachineCodeWriter::loadsd(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf2, 0x0f, 0x10 }, inst);
}

void MachineCodeWriter::load64(MachineInst* inst)
{
	emitInstRM(OpFlags::RexW, 0x8b, inst);
}

void MachineCodeWriter::load32(MachineInst* inst)
{
	emitInstRM(0, 0x8b, inst);
}

void MachineCodeWriter::load16(MachineInst* inst)
{
	emitInstRM(OpFlags::SizeOverride, 0x8b, inst);
}

void MachineCodeWriter::load8(MachineInst* inst)
{
	emitInstRM(OpFlags::Rex, 0x8a, inst);
}

void MachineCodeWriter::storess(MachineInst* inst)
{
	emitInstSSE_MR(0, { 0xf3, 0x0f, 0x11 }, inst);
}

void MachineCodeWriter::storesd(MachineInst* inst)
{
	emitInstSSE_MR(0, { 0xf2, 0x0f, 0x11 }, inst);
}

void MachineCodeWriter::store64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::RexW, 0xb8, 64, inst);
	else
		emitInstMR(OpFlags::RexW, 0x89, inst);
}

void MachineCodeWriter::store32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(0, 0xb8, 32, inst);
	else
		emitInstMR(0, 0x89, inst);
}

void MachineCodeWriter::store16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::SizeOverride, 0xb8, 16, inst);
	else
		emitInstMR(OpFlags::SizeOverride, 0x89, inst);
}

void MachineCodeWriter::store8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::Rex, 0xc6, 8, inst);
	else
		emitInstMR(OpFlags::Rex, 0x88, inst);
}

void MachineCodeWriter::movss(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf3, 0x0f, 0x10 }, inst);
}

void MachineCodeWriter::movsd(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf2, 0x0f, 0x10 }, inst);
}

void MachineCodeWriter::mov64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstOI(OpFlags::RexW, 0xb8, 64, inst);
	else
		emitInstRM(OpFlags::RexW, 0x8b, inst);
}

void MachineCodeWriter::mov32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstOI(0, 0xb8, 32, inst);
	else
		emitInstRM(0, 0x8b, inst);
}

void MachineCodeWriter::mov16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstOI(OpFlags::SizeOverride, 0xb8, 16, inst);
	else
		emitInstRM(OpFlags::SizeOverride, 0x8b, inst);
}

void MachineCodeWriter::mov8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstOI(OpFlags::Rex, 0xb0, 8, inst);
	else
		emitInstRM(OpFlags::Rex, 0x8a, inst);
}

void MachineCodeWriter::addss(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf3, 0x0f, 0x58 }, inst);
}

void MachineCodeWriter::addsd(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf2, 0x0f, 0x58 }, inst);
}

void MachineCodeWriter::add64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::RexW, 0x81, 32, inst);
	else
		emitInstRM(OpFlags::RexW, 0x03, inst);
}

void MachineCodeWriter::add32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(0, 0x81, 32, inst);
	else
		emitInstRM(0, 0x03, inst);
}

void MachineCodeWriter::add16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::SizeOverride, 0x81, 16, inst);
	else
		emitInstRM(OpFlags::SizeOverride, 0x03, inst);
}

void MachineCodeWriter::add8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::Rex, 0x80, 8, inst);
	else
		emitInstRM(OpFlags::Rex, 0x02, inst);
}

void MachineCodeWriter::subss(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf3, 0x0f, 0x5c }, inst);
}

void MachineCodeWriter::subsd(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf2, 0x0f, 0x5c }, inst);
}

void MachineCodeWriter::sub64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::RexW, 0x81, 5, 32, inst);
	else
		emitInstRM(OpFlags::RexW, 0x2b, inst);
}

void MachineCodeWriter::sub32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(0, 0x81, 5, 32, inst);
	else
		emitInstRM(0, 0x2b, inst);
}

void MachineCodeWriter::sub16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::SizeOverride, 0x81, 5, 16, inst);
	else
		emitInstRM(OpFlags::SizeOverride, 0x2b, inst);
}

void MachineCodeWriter::sub8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::Rex, 0x80, 5, 8, inst);
	else
		emitInstRM(OpFlags::Rex, 0x2a, inst);
}

void MachineCodeWriter::not64(MachineInst* inst)
{
	emitInstM(OpFlags::RexW, 0xf7, 2, inst);
}

void MachineCodeWriter::not32(MachineInst* inst)
{
	emitInstM(0, 0xf7, 2, inst);
}

void MachineCodeWriter::not16(MachineInst* inst)
{
	emitInstM(OpFlags::SizeOverride, 0xf7, 2, inst);
}

void MachineCodeWriter::not8(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, 0xf6, 2, inst);
}

void MachineCodeWriter::neg64(MachineInst* inst)
{
	emitInstM(OpFlags::RexW, 0xf7, 3, inst);
}

void MachineCodeWriter::neg32(MachineInst* inst)
{
	emitInstM(0, 0xf7, 3, inst);
}

void MachineCodeWriter::neg16(MachineInst* inst)
{
	emitInstM(OpFlags::SizeOverride, 0xf7, 3, inst);
}

void MachineCodeWriter::neg8(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, 0xf6, 3, inst);
}

void MachineCodeWriter::shl64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::RexW, 0xc1, 4, 8, inst);
	else
		emitInstMC(OpFlags::RexW, 0xd3, 4, inst); // register must be CL
}

void MachineCodeWriter::shl32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(0, 0xc1, 4, 8, inst);
	else
		emitInstMC(0, 0xd3, 4, inst); // register must be CL
}

void MachineCodeWriter::shl16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::SizeOverride, 0xc1, 4, 8, inst);
	else
		emitInstMC(OpFlags::SizeOverride, 0xd3, 4, inst); // register must be CL
}

void MachineCodeWriter::shl8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::Rex, 0xc0, 4, 8, inst);
	else
		emitInstMC(OpFlags::Rex, 0xd2, 4, inst); // register must be CL
}

void MachineCodeWriter::sar64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::RexW, 0xc1, 7, 8, inst);
	else
		emitInstMC(OpFlags::RexW, 0xd3, 7, inst); // register must be CL
}

void MachineCodeWriter::sar32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(0, 0xc1, 7, 8, inst);
	else
		emitInstMC(0, 0xd3, 7, inst); // register must be CL
}

void MachineCodeWriter::sar16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::SizeOverride, 0xc1, 7, 8, inst);
	else
		emitInstMC(OpFlags::SizeOverride, 0xd3, 7, inst); // register must be CL
}

void MachineCodeWriter::sar8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::Rex, 0xc0, 7, 8, inst);
	else
		emitInstMC(OpFlags::Rex, 0xd2, 7, inst); // register must be CL
}

void MachineCodeWriter::and64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::RexW, 0x81, 4, 32, inst);
	else
		emitInstRM(OpFlags::RexW, 0x23, inst);
}

void MachineCodeWriter::and32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(0, 0x81, 4, 32, inst);
	else
		emitInstRM(0, 0x23, inst);
}

void MachineCodeWriter::and16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::SizeOverride, 0x81, 4, 16, inst);
	else
		emitInstRM(OpFlags::SizeOverride, 0x23, inst);
}

void MachineCodeWriter::and8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::Rex, 0x80, 4, 8, inst);
	else
		emitInstRM(OpFlags::Rex, 0x22, inst);
}

void MachineCodeWriter::or64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::RexW, 0x81, 1, 32, inst);
	else
		emitInstRM(OpFlags::RexW, 0x0b, inst);
}

void MachineCodeWriter::or32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(0, 0x81, 1, 32, inst);
	else
		emitInstRM(0, 0x0b, inst);
}

void MachineCodeWriter::or16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::SizeOverride, 0x81, 1, 16, inst);
	else
		emitInstRM(OpFlags::SizeOverride, 0x0b, inst);
}

void MachineCodeWriter::or8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::Rex, 0x80, 1, 8, inst);
	else
		emitInstRM(OpFlags::Rex, 0x08, inst);
}

void MachineCodeWriter::xorpd(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0x66, 0x0f, 0x57 }, inst);
}

void MachineCodeWriter::xorps(MachineInst* inst)
{
	emitInstSSE_RM(OpFlags::NP, { 0x0f, 0x57 }, inst);
}

void MachineCodeWriter::xor64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::RexW, 0x81, 6, 32, inst);
	else
		emitInstRM(OpFlags::RexW, 0x33, inst);
}

void MachineCodeWriter::xor32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(0, 0x81, 6, 32, inst);
	else
		emitInstRM(0, 0x33, inst);
}

void MachineCodeWriter::xor16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::SizeOverride, 0x81, 6, 16, inst);
	else
		emitInstRM(OpFlags::SizeOverride, 0x33, inst);
}

void MachineCodeWriter::xor8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::Rex, 0x80, 6, 8, inst);
	else
		emitInstRM(OpFlags::Rex, 0x32, inst);
}

void MachineCodeWriter::mulss(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf3, 0x0f, 0x59 }, inst);
}

void MachineCodeWriter::mulsd(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf2, 0x0f, 0x59 }, inst);
}

void MachineCodeWriter::imul64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstRMI(OpFlags::RexW, 0x69, 32, inst);
	else
		emitInstRM(OpFlags::RexW, { 0x0f, 0xaf }, inst);
}

void MachineCodeWriter::imul32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstRMI(0, 0x69, 32, inst);
	else
		emitInstRM(0, { 0x0f, 0xaf }, inst);
}

void MachineCodeWriter::imul16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstRMI(OpFlags::SizeOverride, 0x69, 16, inst);
	else
		emitInstRM(OpFlags::SizeOverride, { 0x0f, 0xaf }, inst);
}

void MachineCodeWriter::imul8(MachineInst* inst)
{
	// Doesn't have an immediate multiply. AX <- AL * r/m byte
	emitInstM(0, 0xf6, 5, inst);
}

void MachineCodeWriter::divss(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf3, 0x0f, 0x5e }, inst);
}

void MachineCodeWriter::divsd(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf2, 0x0f, 0x5e }, inst);
}

void MachineCodeWriter::idiv64(MachineInst* inst)
{
	// Doesn't have an immediate divide. Always uses RAX and RDX.
	emitInstM(OpFlags::RexW, 0xf7, 7, inst);
}

void MachineCodeWriter::idiv32(MachineInst* inst)
{
	// Doesn't have an immediate divide. Always uses EAX and EDX.
	emitInstM(0, 0xf7, 7, inst);
}

void MachineCodeWriter::idiv16(MachineInst* inst)
{
	// Doesn't have an immediate divide. Always uses AX and DX.
	emitInstM(OpFlags::SizeOverride, 0xf7, 7, inst);
}

void MachineCodeWriter::idiv8(MachineInst* inst)
{
	// Doesn't have an immediate divide. Always uses AX.
	emitInstM(OpFlags::Rex, 0xf6, 7, inst);
}

void MachineCodeWriter::mul64(MachineInst* inst)
{
	// Doesn't have an immediate multiply. Always uses RAX and RDX.
	emitInstM(OpFlags::RexW, 0xf7, 4, inst);
}

void MachineCodeWriter::mul32(MachineInst* inst)
{
	// Doesn't have an immediate multiply. Always uses EAX and EDX.
	emitInstM(0, 0xf7, 4, inst);
}

void MachineCodeWriter::mul16(MachineInst* inst)
{
	// Doesn't have an immediate multiply. Always uses AX and DX.
	emitInstM(OpFlags::SizeOverride, 0xf7, 4, inst);
}

void MachineCodeWriter::mul8(MachineInst* inst)
{
	// Doesn't have an immediate multiply. Always uses AX.
	emitInstM(OpFlags::Rex, 0xf6, 4, inst);
}

void MachineCodeWriter::div64(MachineInst* inst)
{
	// Doesn't have an immediate divide. Always uses RAX and RDX.
	emitInstM(OpFlags::RexW, 0xf7, 6, inst);
}

void MachineCodeWriter::div32(MachineInst* inst)
{
	// Doesn't have an immediate divide. Always uses EAX and EDX.
	emitInstM(0, 0xf7, 6, inst);
}

void MachineCodeWriter::div16(MachineInst* inst)
{
	// Doesn't have an immediate divide. Always uses AX and DX.
	emitInstM(OpFlags::SizeOverride, 0xf7, 6, inst);
}

void MachineCodeWriter::div8(MachineInst* inst)
{
	// Doesn't have an immediate divide. Always uses AX.
	emitInstM(OpFlags::Rex, 0xf6, 6, inst);
}

void MachineCodeWriter::ucomisd(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0x66, 0x0f, 0x2e }, inst);
}

void MachineCodeWriter::ucomiss(MachineInst* inst)
{
	emitInstSSE_RM(OpFlags::NP, { 0x0f, 0x2e }, inst);
}

void MachineCodeWriter::cmp64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::RexW, 0x81, 7, 32, inst);
	else
		emitInstRM(OpFlags::RexW, 0x3b, inst);
}

void MachineCodeWriter::cmp32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(0, 0x81, 7, 32, inst);
	else
		emitInstRM(0, 0x3b, inst);
}

void MachineCodeWriter::cmp16(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::SizeOverride, 0x81, 7, 16, inst);
	else
		emitInstRM(OpFlags::SizeOverride, 0x3b, inst);
}

void MachineCodeWriter::cmp8(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
		emitInstMI(OpFlags::Rex, 0x80, 7, 8, inst);
	else
		emitInstRM(OpFlags::Rex, 0x3a, inst);
}

void MachineCodeWriter::setl(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x9c }, 8, inst);
}

void MachineCodeWriter::setb(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x92 }, 8, inst);
}

void MachineCodeWriter::seta(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x97 }, 8, inst);
}

void MachineCodeWriter::setg(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x9f }, 8, inst);
}

void MachineCodeWriter::setle(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x9e }, 8, inst);
}

void MachineCodeWriter::setbe(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x96 }, 8, inst);
}

void MachineCodeWriter::setae(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x93 }, 8, inst);
}

void MachineCodeWriter::setge(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x9d }, 8, inst);
}

void MachineCodeWriter::sete(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x94 }, 8, inst);
}

void MachineCodeWriter::setne(MachineInst* inst)
{
	emitInstM(OpFlags::Rex, { 0x0f, 0x95 }, 8, inst);
}

void MachineCodeWriter::cvtsd2ss(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf2, 0x0f, 0x5a }, inst);
}

void MachineCodeWriter::cvtss2sd(MachineInst* inst)
{
	emitInstSSE_RM(0, { 0xf3, 0x0f, 0x5a }, inst);
}

void MachineCodeWriter::cvttsd2si(MachineInst* inst)
{
	emitInstSSE_RM(OpFlags::RexW, { 0xf2, 0x0f, 0x2c }, inst);
}

void MachineCodeWriter::cvttss2si(MachineInst* inst)
{
	emitInstSSE_RM(OpFlags::RexW, { 0xf3, 0x0f, 0x2c }, inst); // Always QWORD output
}

void MachineCodeWriter::cvtsi2sd(MachineInst* inst)
{
	emitInstSSE_RM(OpFlags::RexW, { 0xf2, 0x0f, 0x2a }, inst);
}

void MachineCodeWriter::cvtsi2ss(MachineInst* inst)
{
	emitInstSSE_RM(OpFlags::RexW, { 0xf3, 0x0f, 0x2a }, inst); // Always QWORD output
}

void MachineCodeWriter::jmp(MachineInst* inst)
{
	writeOpcode(0, { 0xe9 }, 0, 0, 0);
	writeImm(32, 0xffffffff);
	codeholder->bbRelocateInfo.push_back({ codeholder->code.size() - 4, inst->operands[0].bb });
}

void MachineCodeWriter::jz(MachineInst* inst)
{
	writeOpcode(0, { 0x0f, 0x84 }, 0, 0, 0);
	writeImm(32, 0xffffffff);
	codeholder->bbRelocateInfo.push_back({ codeholder->code.size() - 4, inst->operands[0].bb });
}

void MachineCodeWriter::call(MachineInst* inst)
{
	int regindex = (int)RegisterName::rax;

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

void MachineCodeWriter::alloca_(MachineInst* inst)
{
	//cc.lea(reg_i64(inst->operands[0]), cc.newStack((uint32_t)imm(inst->operands[1]), 8));
}

void MachineCodeWriter::ret(MachineInst* inst)
{
	emitInstZO(0, 0xc3);
}

void MachineCodeWriter::push(MachineInst* inst)
{
	emitInstO(0, 0x50, inst);
}

void MachineCodeWriter::pop(MachineInst* inst)
{
	emitInstO(0, 0x58, inst);
}

void MachineCodeWriter::emitInstZO(int flags, int opcode)
{
	writeOpcode(flags, { opcode }, 0, 0, 0);
}

void MachineCodeWriter::emitInstO(int flags, int opcode, MachineInst* inst)
{
	int regindex = getPhysReg(inst->operands[0]);
	opcode |= (regindex & 7);

	writeOpcode(flags, { opcode }, 0, 0, regindex >> 3);
}

void MachineCodeWriter::emitInstOI(int flags, int opcode, int immsize, MachineInst* inst)
{
	int regindex = getPhysReg(inst->operands[0]);
	opcode |= (regindex & 7);

	writeOpcode(flags, { opcode }, 0, 0, regindex >> 3);
	writeImm(immsize, inst->operands[1].immvalue);
}

void MachineCodeWriter::emitInstMI(int flags, int opcode, int immsize, MachineInst* inst)
{
	emitInstMI(flags, opcode, 0, immsize, inst);
}

void MachineCodeWriter::emitInstMI(int flags, int opcode, int modopcode, int immsize, MachineInst* inst)
{
	// ModR/M:
	int mod = 0;
	int modreg = modopcode;
	int rm = 0;

	// SIB:
	int scale = 0;
	int index = 0;
	int base = 0;

	if (inst->operands[0].type == MachineOperandType::reg)
	{
		rm = getPhysReg(inst->operands[0]);
		mod = 3;
	}

	writeOpcode(flags, { opcode }, modreg >> 3, index >> 3, rm >> 3);
	writeModRM(mod, modreg, rm);
	writeImm(immsize, inst->operands[1].immvalue);
}

void MachineCodeWriter::emitInstMR(int flags, int opcode, MachineInst* inst)
{
	// ModR/M:
	int mod = 0;
	int modreg = 0;
	int rm = 0;

	// SIB:
	int scale = 0;
	int index = 0;
	int base = 0;

	if (inst->operands[0].type == MachineOperandType::reg)
	{
		rm = getPhysReg(inst->operands[0]);
		mod = 3;
	}

	if (inst->operands[1].type == MachineOperandType::reg)
	{
		modreg = getPhysReg(inst->operands[1]);
	}

	writeOpcode(flags, { opcode }, modreg >> 3, index >> 3, rm >> 3);
	writeModRM(mod, modreg, rm);
}

void MachineCodeWriter::emitInstM(int flags, int opcode, MachineInst* inst)
{
	emitInstM(flags, { opcode }, 0, inst);
}

void MachineCodeWriter::emitInstM(int flags, int opcode, int modopcode, MachineInst* inst)
{
	emitInstM(flags, { opcode }, modopcode, inst);
}

void MachineCodeWriter::emitInstM(int flags, std::initializer_list<int> opcode, MachineInst* inst)
{
	emitInstM(flags, opcode, 0, inst);
}

void MachineCodeWriter::emitInstM(int flags, std::initializer_list<int> opcode, int modopcode, MachineInst* inst)
{
	// ModR/M:
	int mod = 0;
	int modreg = modopcode;
	int rm = 0;

	// SIB:
	int scale = 0;
	int index = 0;
	int base = 0;

	if (inst->operands[0].type == MachineOperandType::reg)
	{
		rm = getPhysReg(inst->operands[0]);
		mod = 3;
	}

	writeOpcode(flags, opcode, modreg >> 3, index >> 3, rm >> 3);
	writeModRM(mod, modreg, rm);
}

void MachineCodeWriter::emitInstMC(int flags, int opcode, MachineInst* inst)
{
	emitInstMC(flags, opcode, 0, inst);
}

void MachineCodeWriter::emitInstMC(int flags, int opcode, int modopcode, MachineInst* inst)
{
	// ModR/M:
	int mod = 0;
	int modreg = modopcode;
	int rm = 0;

	// SIB:
	int scale = 0;
	int index = 0;
	int base = 0;

	if (inst->operands[0].type == MachineOperandType::reg)
	{
		rm = getPhysReg(inst->operands[0]);
		mod = 3;
	}

	writeOpcode(flags, { opcode }, modreg >> 3, index >> 3, rm >> 3);
	writeModRM(mod, modreg, rm);
}

void MachineCodeWriter::emitInstRM(int flags, int opcode, MachineInst* inst)
{
	emitInstRM(flags, { opcode }, inst);
}

void MachineCodeWriter::emitInstRM(int flags, std::initializer_list<int> opcode, MachineInst* inst)
{
	// ModR/M:
	int mod = 0;
	int modreg = 0;
	int rm = 0;

	// SIB:
	int scale = 0;
	int index = 0;
	int base = 0;
	bool sib = false;

	// Disp:
	int32_t disp = 0;
	int dispsize = 0;

	if (inst->operands[0].type == MachineOperandType::reg)
	{
		modreg = getPhysReg(inst->operands[0]);
	}

	if (inst->operands[1].type == MachineOperandType::reg)
	{
		rm = getPhysReg(inst->operands[1]);
		mod = 3;
	}
	else if (inst->operands[1].type == MachineOperandType::stack)
	{
		disp = sfunc->stackSize - inst->operands[1].stackOffset;
		dispsize = (disp > -127 && disp < 127) ? 8 : 32;
		mod = (dispsize == 8) ? 1 : 2;
		rm = 4;
		scale = 0;
		index = 4;
		base = (int)RegisterName::rsp;
		sib = true;
	}

	writeOpcode(flags, opcode, modreg >> 3, index >> 3, rm >> 3);
	writeModRM(mod, modreg, rm);
	if (sib) writeSIB(scale, index, base);
	if (dispsize > 0) writeImm(dispsize, disp);
}

void MachineCodeWriter::emitInstRMI(int flags, int opcode, int immsize, MachineInst* inst)
{
	// ModR/M:
	int mod = 0;
	int modreg = 0;
	int rm = 0;

	// SIB:
	int scale = 0;
	int index = 0;
	int base = 0;

	if (inst->operands[0].type == MachineOperandType::reg)
	{
		modreg = getPhysReg(inst->operands[0]);
		rm = getPhysReg(inst->operands[0]);
		mod = 3;
	}

	writeOpcode(flags, { opcode }, modreg >> 3, index >> 3, rm >> 3);
	writeModRM(mod, modreg, rm);
	writeImm(immsize, inst->operands[1].immvalue);
}

void MachineCodeWriter::emitInstSSE_RM(int flags, std::initializer_list<int> opcode, MachineInst* inst)
{
	// ModR/M:
	int mod = 0;
	int modreg = 0;
	int rm = 0;

	// SIB:
	int scale = 0;
	int index = 0;
	int base = 0;
	bool sib = false;

	// Disp:
	int32_t disp = 0;
	int dispsize = 0;

	if (inst->operands[0].type == MachineOperandType::reg)
	{
		modreg = getPhysReg(inst->operands[0]);
	}

	if (inst->operands[1].type == MachineOperandType::reg)
	{
		rm = getPhysReg(inst->operands[1]);
		mod = 3;
	}
	else if (inst->operands[1].type == MachineOperandType::stack)
	{
		disp = sfunc->stackSize - inst->operands[1].stackOffset;
		dispsize = (disp > -127 && disp < 127) ? 8 : 32;
		mod = (dispsize == 8) ? 1 : 2;
		rm = 4;
		scale = 0;
		index = 4;
		base = (int)RegisterName::rsp;
		sib = true;
	}

	writeOpcode(flags, opcode, modreg >> 3, index >> 3, rm >> 3);
	writeModRM(mod, modreg, rm);
	if (sib) writeSIB(scale, index, base);
	if (dispsize > 0) writeImm(dispsize, disp);
}

void MachineCodeWriter::emitInstSSE_MR(int flags, std::initializer_list<int> opcode, MachineInst* inst)
{
	// ModR/M:
	int mod = 0;
	int modreg = 0;
	int rm = 0;

	// SIB:
	int scale = 0;
	int index = 0;
	int base = 0;
	bool sib = false;

	// Disp:
	int32_t disp = 0;
	int dispsize = 0;

	if (inst->operands[0].type == MachineOperandType::reg)
	{
		rm = getPhysReg(inst->operands[0]);
		mod = 3;
	}
	else if (inst->operands[0].type == MachineOperandType::stack)
	{
		disp = sfunc->stackSize - inst->operands[0].stackOffset;
		dispsize = (disp > -127 && disp < 127) ? 8 : 32;
		mod = (dispsize == 8) ? 1 : 2;
		rm = 4;
		scale = 0;
		index = 4;
		base = (int)RegisterName::rsp;
		sib = true;
	}

	if (inst->operands[1].type == MachineOperandType::reg)
	{
		modreg = getPhysReg(inst->operands[1]);
	}

	writeOpcode(flags, opcode, modreg >> 3, index >> 3, rm >> 3);
	writeModRM(mod, modreg, rm);
	if (sib) writeSIB(scale, index, base);
	if (dispsize > 0) writeImm(dispsize, disp);
}

int MachineCodeWriter::getPhysReg(const MachineOperand& operand)
{
	if (operand.registerIndex < (int)RegisterName::xmm0)
		return operand.registerIndex;
	else
		return operand.registerIndex - (int)RegisterName::xmm0;
}

void MachineCodeWriter::writeOpcode(int flags, std::initializer_list<int> opcode, int r, int x, int b)
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

void MachineCodeWriter::writeModRM(int mod, int modreg, int rm)
{
	modreg &= 7;
	rm &= 7;
	codeholder->code.push_back((mod << 6) | (modreg << 3) | rm);
}

void MachineCodeWriter::writeSIB(int scale, int index, int base)
{
	index &= 7;
	codeholder->code.push_back((scale << 6) | (index << 3) | base);
}

void MachineCodeWriter::writeImm(int immsize, uint64_t value)
{
	for (int i = 0; i < immsize; i += 8)
	{
		codeholder->code.push_back(value & 0xff);
		value >>= 8;
	}
}
