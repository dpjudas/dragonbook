
#include "MachineCodeWriterAArch64.h"
#include "../MachineCodeHolder.h"
#include <stdexcept>

void MachineCodeWriterAArch64::codegen()
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

void MachineCodeWriterAArch64::basicblock(MachineBasicBlock* bb)
{
	codeholder->bbOffsets[bb] = codeholder->code.size();
	for (MachineInst* inst : bb->code)
	{
		opcode(inst);
		inst->unwindOffset = (int)(codeholder->code.size() - funcBeginAddress);
	}
}

void MachineCodeWriterAArch64::opcode(MachineInst* inst)
{
	switch ((MachineInstOpcodeAArch64)inst->opcode)
	{
	default:
	case MachineInstOpcodeAArch64::nop: nop(inst); break;
	case MachineInstOpcodeAArch64::loadss: loadss(inst); break;
	case MachineInstOpcodeAArch64::loadsd: loadsd(inst); break;
	case MachineInstOpcodeAArch64::load64: load64(inst); break;
	case MachineInstOpcodeAArch64::load32: load32(inst); break;
	case MachineInstOpcodeAArch64::load16: load16(inst); break;
	case MachineInstOpcodeAArch64::load8: load8(inst); break;
	case MachineInstOpcodeAArch64::storess: storess(inst); break;
	case MachineInstOpcodeAArch64::storesd: storesd(inst); break;
	case MachineInstOpcodeAArch64::store64: store64(inst); break;
	case MachineInstOpcodeAArch64::store32: store32(inst); break;
	case MachineInstOpcodeAArch64::store16: store16(inst); break;
	case MachineInstOpcodeAArch64::store8: store8(inst); break;
	case MachineInstOpcodeAArch64::movss: movss(inst); break;
	case MachineInstOpcodeAArch64::movsd: movsd(inst); break;
	case MachineInstOpcodeAArch64::mov64: mov64(inst); break;
	case MachineInstOpcodeAArch64::mov32: mov32(inst); break;
	case MachineInstOpcodeAArch64::movsx8_32: movsx8_32(inst); break;
	case MachineInstOpcodeAArch64::movsx8_64: movsx8_64(inst); break;
	case MachineInstOpcodeAArch64::movsx16_32: movsx16_32(inst); break;
	case MachineInstOpcodeAArch64::movsx16_64: movsx16_64(inst); break;
	case MachineInstOpcodeAArch64::movsx32_64: movsx32_64(inst); break;
	case MachineInstOpcodeAArch64::movzx8_32: movzx8_32(inst); break;
	case MachineInstOpcodeAArch64::movzx8_64: movzx8_64(inst); break;
	case MachineInstOpcodeAArch64::movzx16_32: movzx16_32(inst); break;
	case MachineInstOpcodeAArch64::movzx16_64: movzx16_64(inst); break;
	case MachineInstOpcodeAArch64::addss: addss(inst); break;
	case MachineInstOpcodeAArch64::addsd: addsd(inst); break;
	case MachineInstOpcodeAArch64::add64: add64(inst); break;
	case MachineInstOpcodeAArch64::add32: add32(inst); break;
	case MachineInstOpcodeAArch64::subss: subss(inst); break;
	case MachineInstOpcodeAArch64::subsd: subsd(inst); break;
	case MachineInstOpcodeAArch64::sub64: sub64(inst); break;
	case MachineInstOpcodeAArch64::sub32: sub32(inst); break;
	case MachineInstOpcodeAArch64::not64: not64(inst); break;
	case MachineInstOpcodeAArch64::not32: not32(inst); break;
	case MachineInstOpcodeAArch64::negss: negss(inst); break;
	case MachineInstOpcodeAArch64::negsd: negsd(inst); break;
	case MachineInstOpcodeAArch64::neg64: neg64(inst); break;
	case MachineInstOpcodeAArch64::neg32: neg32(inst); break;
	case MachineInstOpcodeAArch64::shl64: shl64(inst); break;
	case MachineInstOpcodeAArch64::shl32: shl32(inst); break;
	case MachineInstOpcodeAArch64::shr64: shr64(inst); break;
	case MachineInstOpcodeAArch64::shr32: shr32(inst); break;
	case MachineInstOpcodeAArch64::sar64: sar64(inst); break;
	case MachineInstOpcodeAArch64::sar32: sar32(inst); break;
	case MachineInstOpcodeAArch64::and64: and64(inst); break;
	case MachineInstOpcodeAArch64::and32: and32(inst); break;
	case MachineInstOpcodeAArch64::or64: or64(inst); break;
	case MachineInstOpcodeAArch64::or32: or32(inst); break;
	case MachineInstOpcodeAArch64::xor64: xor64(inst); break;
	case MachineInstOpcodeAArch64::xor32: xor32(inst); break;
	case MachineInstOpcodeAArch64::mulss: mulss(inst); break;
	case MachineInstOpcodeAArch64::mulsd: mulsd(inst); break;
	case MachineInstOpcodeAArch64::mul64: mul64(inst); break;
	case MachineInstOpcodeAArch64::mul32: mul32(inst); break;
	case MachineInstOpcodeAArch64::divss: divss(inst); break;
	case MachineInstOpcodeAArch64::divsd: divsd(inst); break;
	case MachineInstOpcodeAArch64::sdiv64: sdiv64(inst); break;
	case MachineInstOpcodeAArch64::sdiv32: sdiv32(inst); break;
	case MachineInstOpcodeAArch64::udiv64: udiv64(inst); break;
	case MachineInstOpcodeAArch64::udiv32: udiv32(inst); break;
	case MachineInstOpcodeAArch64::cvtsd2ss: cvtsd2ss(inst); break;
	case MachineInstOpcodeAArch64::cvtss2sd: cvtss2sd(inst); break;
	case MachineInstOpcodeAArch64::cvttsd2si: cvttsd2si(inst); break;
	case MachineInstOpcodeAArch64::cvttss2si: cvttss2si(inst); break;
	case MachineInstOpcodeAArch64::cvtsi2sd: cvtsi2sd(inst); break;
	case MachineInstOpcodeAArch64::cvtsi2ss: cvtsi2ss(inst); break;
	case MachineInstOpcodeAArch64::fcmpsd: fcmpsd(inst); break;
	case MachineInstOpcodeAArch64::fcmpss: fcmpss(inst); break;
	case MachineInstOpcodeAArch64::cmp32_sx8: cmp32_sx8(inst); break;
	case MachineInstOpcodeAArch64::cmp32_sx16: cmp32_sx16(inst); break;
	case MachineInstOpcodeAArch64::cmp32_sx32: cmp32_sx32(inst); break;
	case MachineInstOpcodeAArch64::cmp32_sx64: cmp32_sx64(inst); break;
	case MachineInstOpcodeAArch64::cmp32_zx8: cmp32_zx8(inst); break;
	case MachineInstOpcodeAArch64::cmp32_zx16: cmp32_zx16(inst); break;
	case MachineInstOpcodeAArch64::cmp32_zx32: cmp32_zx32(inst); break;
	case MachineInstOpcodeAArch64::cmp32_zx64: cmp32_zx64(inst); break;
	case MachineInstOpcodeAArch64::cmp64_sx8: cmp64_sx8(inst); break;
	case MachineInstOpcodeAArch64::cmp64_sx16: cmp64_sx16(inst); break;
	case MachineInstOpcodeAArch64::cmp64_sx32: cmp64_sx32(inst); break;
	case MachineInstOpcodeAArch64::cmp64_sx64: cmp64_sx64(inst); break;
	case MachineInstOpcodeAArch64::cmp64_zx8: cmp64_zx8(inst); break;
	case MachineInstOpcodeAArch64::cmp64_zx16: cmp64_zx16(inst); break;
	case MachineInstOpcodeAArch64::cmp64_zx32: cmp64_zx32(inst); break;
	case MachineInstOpcodeAArch64::cmp64_zx64: cmp64_zx64(inst); break;
	case MachineInstOpcodeAArch64::cseteq: cseteq(inst); break;
	case MachineInstOpcodeAArch64::csetne: csetne(inst); break;
	case MachineInstOpcodeAArch64::csetcs: csetcs(inst); break;
	case MachineInstOpcodeAArch64::csetcc: csetcc(inst); break;
	case MachineInstOpcodeAArch64::csetmi: csetmi(inst); break;
	case MachineInstOpcodeAArch64::csetpl: csetpl(inst); break;
	case MachineInstOpcodeAArch64::csetvs: csetvs(inst); break;
	case MachineInstOpcodeAArch64::csetvc: csetvc(inst); break;
	case MachineInstOpcodeAArch64::csethi: csethi(inst); break;
	case MachineInstOpcodeAArch64::csetls: csetls(inst); break;
	case MachineInstOpcodeAArch64::csetge: csetge(inst); break;
	case MachineInstOpcodeAArch64::csetlt: csetlt(inst); break;
	case MachineInstOpcodeAArch64::csetgt: csetgt(inst); break;
	case MachineInstOpcodeAArch64::csetle: csetle(inst); break;
	case MachineInstOpcodeAArch64::b: b(inst); break;
	case MachineInstOpcodeAArch64::beq: beq(inst); break;
	case MachineInstOpcodeAArch64::bne: bne(inst); break;
	case MachineInstOpcodeAArch64::bl: bl(inst); break;
	case MachineInstOpcodeAArch64::blr: blr(inst); break;
	case MachineInstOpcodeAArch64::ret: ret(inst); break;
	}
}

void MachineCodeWriterAArch64::nop(MachineInst* inst)
{
	writeOpcode(0b11010101000000110010000000011111, inst);
}

void MachineCodeWriterAArch64::loadss(MachineInst* inst)
{
	auto srcOffset = getImmediateOffset(inst->operands[1]);

	// LDR (immediate):
	uint32_t opcode = 0b10'111101'01'000000000000'00000'00000;
	opcode |= (srcOffset.second & ((1 << 12) - 1)) << 10; // imm12
	opcode |= getPhysReg(srcOffset.first) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::loadsd(MachineInst* inst)
{
	auto srcOffset = getImmediateOffset(inst->operands[1]);

	// LDR (immediate):
	uint32_t opcode = 0b11'111101'01'000000000000'00000'00000;
	opcode |= (srcOffset.second & ((1 << 12) - 1)) << 10; // imm12
	opcode |= getPhysReg(srcOffset.first) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::load64(MachineInst* inst)
{
	auto srcOffset = getImmediateOffset(inst->operands[1]);

	// LDR (immediate):
	uint32_t opcode = 0b1111100101'000000000000'00000'00000;
	opcode |= (srcOffset.second & ((1 << 12) - 1)) << 10; // imm12
	opcode |= getPhysReg(srcOffset.first) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::load32(MachineInst* inst)
{
	auto srcOffset = getImmediateOffset(inst->operands[1]);

	// LDR (immediate):
	uint32_t opcode = 0b1011100101'000000000000'00000'00000;
	opcode |= (srcOffset.second & ((1 << 12) - 1)) << 10; // imm12
	opcode |= getPhysReg(srcOffset.first) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::load16(MachineInst* inst)
{
	auto srcOffset = getImmediateOffset(inst->operands[1]);

	// LDRH (immediate):
	uint32_t opcode = 0b0111100101'000000000000'00000'00000;
	opcode |= (srcOffset.second & ((1 << 12) - 1)) << 10; // imm12
	opcode |= getPhysReg(srcOffset.first) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::load8(MachineInst* inst)
{
	auto srcOffset = getImmediateOffset(inst->operands[1]);

	// LDRB (immediate):
	uint32_t opcode = 0b0011100101'000000000000'00000'00000;
	opcode |= (srcOffset.second & ((1 << 12) - 1)) << 10; // imm12
	opcode |= getPhysReg(srcOffset.first) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::storess(MachineInst* inst)
{
	auto dstOffset = getImmediateOffset(inst->operands[0]);

	// STR (immediate):
	uint32_t opcode = 0b10'111101'00'000000000000'00000'00000;
	opcode |= (dstOffset.second & ((1 << 12) - 1)) << 10; // imm12
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(dstOffset.first); // Rt
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::storesd(MachineInst* inst)
{
	auto dstOffset = getImmediateOffset(inst->operands[0]);

	// STR (immediate):
	uint32_t opcode = 0b11'111101'00'000000000000'00000'00000;
	opcode |= (dstOffset.second & ((1 << 12) - 1)) << 10; // imm12
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(dstOffset.first); // Rt
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::store64(MachineInst* inst)
{
	auto dstOffset = getImmediateOffset(inst->operands[0]);

	// STR (immediate):
	uint32_t opcode = 0b1111100100'000000000000'00000'00000;
	opcode |= (dstOffset.second & ((1 << 12) - 1)) << 10; // imm12
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(dstOffset.first); // Rt
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::store32(MachineInst* inst)
{
	auto dstOffset = getImmediateOffset(inst->operands[0]);

	// STR (immediate):
	uint32_t opcode = 0b1011100100'000000000000'00000'00000;
	opcode |= (dstOffset.second & ((1 << 12) - 1)) << 10; // imm12
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(dstOffset.first); // Rt
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::store16(MachineInst* inst)
{
	auto dstOffset = getImmediateOffset(inst->operands[0]);

	// STRH (immediate):
	uint32_t opcode = 0b0111100100'000000000000'00000'00000;
	opcode |= (dstOffset.second & ((1 << 12) - 1)) << 10; // imm12
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(dstOffset.first); // Rt
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::store8(MachineInst* inst)
{
	auto dstOffset = getImmediateOffset(inst->operands[0]);

	// STRB (immediate):
	uint32_t opcode = 0b0011100100'000000000000'00000'00000;
	opcode |= (dstOffset.second & ((1 << 12) - 1)) << 10; // imm12
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(dstOffset.first); // Rt
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::movss(MachineInst* inst)
{
	// FMOV (register)
	uint32_t opcode = 0b00011110'00'100000010000'00000'00000;
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::movsd(MachineInst* inst)
{
	// FMOV (register)
	uint32_t opcode = 0b00011110'01'100000010000'00000'00000;
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::mov64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
	{
		// MOVZ
		uint32_t opcode = 0b110100101'00'0000000000000000'00000;
		opcode |= (inst->operands[1].immvalue & 0xffff) << 5; // imm16
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
	else
	{
		// MOV (register)
		uint32_t opcode = 0b10101010000'00000'000000'11111'00000;
		opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
}

void MachineCodeWriterAArch64::mov32(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
	{
		// MOVZ
		uint32_t opcode = 0b010100101'00'0000000000000000'00000;
		opcode |= (inst->operands[1].immvalue & 0xffff) << 5; // imm16
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
	else
	{
		// MOV (register)
		uint32_t opcode = 0b00101010000'00000'000000'11111'00000;
		opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
}

void MachineCodeWriterAArch64::movsx8_32(MachineInst* inst)
{
	// SXTB:
	uint32_t opcode = 0b0001001100'000000000111'00000'00000;
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::movsx8_64(MachineInst* inst)
{
	// SXTB:
	uint32_t opcode = 0b1001001101'000000000111'00000'00000;
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::movsx16_32(MachineInst* inst)
{
	// SXTH:
	uint32_t opcode = 0b0001001100'000000001111'00000'00000;
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::movsx16_64(MachineInst* inst)
{
	// SXTH:
	uint32_t opcode = 0b1001001101'000000001111'00000'00000;
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::movsx32_64(MachineInst* inst)
{
	// SXTW:
	uint32_t opcode = 0b1001001101'000000011111'00000'00000;
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::movzx8_32(MachineInst* inst)
{
	// UTXB:
	uint32_t opcode = 0b0101001100'000000000111'00000'00000;
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::movzx8_64(MachineInst* inst)
{
	// UBFM: (same as UTXB with sf and N set to 1 - why isn't this an official alias?)
	uint32_t opcode = 0b1101001101'000000000111'00000'00000;
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::movzx16_32(MachineInst* inst)
{
	// UTXH:
	uint32_t opcode = 0b0101001100'000000001111'00000'00000;
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::movzx16_64(MachineInst* inst)
{
	// UBFM: (same as UTXH with sf and N set to 1 - why isn't this an official alias?)
	uint32_t opcode = 0b1101001101'000000001111'00000'00000;
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::addss(MachineInst* inst)
{
	// FADD (scalar)
	uint32_t opcode = 0b00011110'00'1'00000'001010'00000'00000;
	opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::addsd(MachineInst* inst)
{
	// FADD (scalar)
	uint32_t opcode = 0b00011110'01'1'00000'001010'00000'00000;
	opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::add64(MachineInst* inst)
{
	if (inst->operands[2].type == MachineOperandType::imm)
	{
		// ADD (immediate)
		uint32_t opcode = 0b1001000100'000000000000'00000'00000;
		opcode |= (inst->operands[2].immvalue & ((1 << 12) - 1)) << 10; // imm12
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
	else
	{
		// ADD (extended register)
		uint32_t opcode = 0b10001011001'00000'011'000'00000'00000;
		opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
}

void MachineCodeWriterAArch64::add32(MachineInst* inst)
{
	if (inst->operands[2].type == MachineOperandType::imm)
	{
		// ADD (immediate)
		uint32_t opcode = 0b0001000100'000000000000'00000'00000;
		opcode |= (inst->operands[2].immvalue & ((1 << 12) - 1)) << 10; // imm12
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
	else
	{
		// ADD (extended register)
		uint32_t opcode = 0b00001011001'00000'010'000'00000'00000;
		opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
}

void MachineCodeWriterAArch64::subss(MachineInst* inst)
{
	// FSUB (scalar)
	uint32_t opcode = 0b00011110'00'1'00000'001110'00000'00000;
	opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::subsd(MachineInst* inst)
{
	// FSUB (scalar)
	uint32_t opcode = 0b00011110'01'1'00000'001110'00000'00000;
	opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::sub64(MachineInst* inst)
{
	if (inst->operands[2].type == MachineOperandType::imm)
	{
		// SUB (immediate)
		uint32_t opcode = 0b1101000100'000000000000'00000'00000;
		opcode |= (inst->operands[2].immvalue & ((1 << 12) - 1)) << 10; // imm12
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
	else
	{
		// SUB (extended register)
		uint32_t opcode = 0b11001011001'00000'011'000'00000'00000;
		opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
}

void MachineCodeWriterAArch64::sub32(MachineInst* inst)
{
	if (inst->operands[2].type == MachineOperandType::imm)
	{
		// SUB (immediate)
		uint32_t opcode = 0b0101000100'000000000000'00000'00000;
		opcode |= (inst->operands[2].immvalue & ((1 << 12) - 1)) << 10; // imm12
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
	else
	{
		// SUB (extended register)
		uint32_t opcode = 0b01001011001'00000'010'000'00000'00000;
		opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
}

void MachineCodeWriterAArch64::not64(MachineInst* inst)
{
	// MVN (bitwise NOT)
	uint32_t opcode = 0b10101010'00'1'00000'000000'11111'00000;
	opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::not32(MachineInst* inst)
{
	// MVN (bitwise NOT)
	uint32_t opcode = 0b00101010'00'1'00000'000000'11111'00000;
	opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::neg64(MachineInst* inst)
{
	// NEG (shifted register)
	uint32_t opcode = 0b11001011'00'0'00000'000000'11111'00000;
	opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::neg32(MachineInst* inst)
{
	// NEG (shifted register)
	uint32_t opcode = 0b01001011'00'0'00000'000000'11111'00000;
	opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::shl64(MachineInst* inst)
{
	if (inst->operands[2].type == MachineOperandType::imm)
	{
		// LSL (immediate)
		uint32_t opcode = 0b1101001101'000000'000000'00000'00000;
		opcode |= ((-(int)inst->operands[2].immvalue % 32) & ((1 << 6) - 1)) << 16; // immr
		opcode |= ((31 - inst->operands[2].immvalue) & ((1 << 6) - 1)) << 10; // imms
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
	else
	{
		// LSL (register)
		uint32_t opcode = 0b10011010110'00000'001000'00000'00000;
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
}

void MachineCodeWriterAArch64::shl32(MachineInst* inst)
{
	if (inst->operands[2].type == MachineOperandType::imm)
	{
		// LSL (immediate)
		uint32_t opcode = 0b0101001100'000000'000000'00000'00000;
		opcode |= ((-(int)inst->operands[2].immvalue % 32) & ((1 << 6) - 1)) << 16; // immr
		opcode |= ((31 - inst->operands[2].immvalue) & ((1 << 6) - 1)) << 10; // imms
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
	else
	{
		// LSL (register)
		uint32_t opcode = 0b00011010110'00000'001000'00000'00000;
		opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
}

void MachineCodeWriterAArch64::shr64(MachineInst* inst)
{
	if (inst->operands[2].type == MachineOperandType::imm)
	{
		// LSR (immediate)
		uint32_t opcode = 0b1101001101'000000'111111'00000'00000;
		opcode |= (inst->operands[2].immvalue & ((1 << 6) - 1)) << 16; // immr
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
	else
	{
		// LSR (register)
		uint32_t opcode = 0b10011010110'00000'001001'00000'00000;
		opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
}

void MachineCodeWriterAArch64::shr32(MachineInst* inst)
{
	if (inst->operands[2].type == MachineOperandType::imm)
	{
		// LSR (immediate)
		uint32_t opcode = 0b0101001100'000000'011111'00000'00000;
		opcode |= (inst->operands[2].immvalue & ((1 << 6) - 1)) << 16; // immr
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
	else
	{
		// LSR (register)
		uint32_t opcode = 0b00011010110'00000'001001'00000'00000;
		opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
}

void MachineCodeWriterAArch64::sar64(MachineInst* inst)
{
	if (inst->operands[2].type == MachineOperandType::imm)
	{
		// ASR (immediate)
		uint32_t opcode = 0b1001001101'000000'111111'00000'00000;
		opcode |= (inst->operands[2].immvalue & ((1 << 6) - 1)) << 16; // immr
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
	else
	{
		// ASR (register)
		uint32_t opcode = 0b10011010110'00000'001010'00000'00000;
		opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
}

void MachineCodeWriterAArch64::sar32(MachineInst* inst)
{
	if (inst->operands[2].type == MachineOperandType::imm)
	{
		// ASR (immediate)
		uint32_t opcode = 0b0001001100'000000'011111'00000'00000;
		opcode |= (inst->operands[2].immvalue & ((1 << 6) - 1)) << 16; // immr
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
	else
	{
		// ASR (register)
		uint32_t opcode = 0b00011010110'00000'001010'00000'00000;
		opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
}

void MachineCodeWriterAArch64::and64(MachineInst* inst)
{
	// AND (shifted register)
	uint32_t opcode = 0b10001010'00'0'00000'000000'00000'00000;
	opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::and32(MachineInst* inst)
{
	// AND (shifted register)
	uint32_t opcode = 0b00001010'00'0'00000'000000'00000'00000;
	opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::or64(MachineInst* inst)
{
	// ORR (shifted register)
	uint32_t opcode = 0b10101010'00'0'00000'000000'00000'00000;
	opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::or32(MachineInst* inst)
{
	// ORR (shifted register)
	uint32_t opcode = 0b00101010'00'0'00000'000000'00000'00000;
	opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::negss(MachineInst* inst)
{
	// FNEG (scalar)
	uint32_t opcode = 0b00011110'00'100001010000'00000'00000;
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::negsd(MachineInst* inst)
{
	// FNEG (scalar)
	uint32_t opcode = 0b00011110'01'100001010000'00000'00000;
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::xor64(MachineInst* inst)
{
	// EOR (shifted register)
	uint32_t opcode = 0b11001010'00'0'00000'000000'00000'00000;
	opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::xor32(MachineInst* inst)
{
	// EOR (shifted register)
	uint32_t opcode = 0b01001010'00'0'00000'000000'00000'00000;
	opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::mulss(MachineInst* inst)
{
	// FMUL (scalar)
	uint32_t opcode = 0b00011110'00'1'00000'000010'00000'00000;
	opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::mulsd(MachineInst* inst)
{
	// FMUL (scalar)
	uint32_t opcode = 0b00011110'01'1'00000'000010'00000'00000;
	opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::mul64(MachineInst* inst)
{
	// MUL
	uint32_t opcode = 0b10011011000'00000'011111'00000'00000;
	opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::mul32(MachineInst* inst)
{
	// MUL
	uint32_t opcode = 0b00011011000'00000'011111'00000'00000;
	opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::divss(MachineInst* inst)
{
	// FDIV (scalar)
	uint32_t opcode = 0b00011110'00'1'00000'000110'00000'00000;
	opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::divsd(MachineInst* inst)
{
	// FDIV (scalar)
	uint32_t opcode = 0b00011110'01'1'00000'000110'00000'00000;
	opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::sdiv64(MachineInst* inst)
{
	// SDIV
	uint32_t opcode = 0b10011010110'00000'000011'00000'00000;
	opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::sdiv32(MachineInst* inst)
{
	// SDIV
	uint32_t opcode = 0b00011010110'00000'000011'00000'00000;
	opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::udiv64(MachineInst* inst)
{
	// UDIV
	uint32_t opcode = 0b10011010110'00000'000010'00000'00000;
	opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::udiv32(MachineInst* inst)
{
	// UDIV
	uint32_t opcode = 0b00011010110'00000'000010'00000'00000;
	opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cvtsd2ss(MachineInst* inst)
{
	// FCVT (scalar)
	uint32_t opcode = 0b00011110'01'10001'00'10000'00000'00000;
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cvtss2sd(MachineInst* inst)
{
	// FCVT (scalar)
	uint32_t opcode = 0b00011110'00'10001'01'10000'00000'00000;
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cvttsd2si(MachineInst* inst)
{
	// FCVTAS (scalar)
	uint32_t opcode = 0b10011110'00'100100000000'00000'00000;
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cvttss2si(MachineInst* inst)
{
	// FCVTAS (scalar)
	uint32_t opcode = 0b10011110'01'100100000000'00000'00000;
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cvtsi2sd(MachineInst* inst)
{
	// SCVTF (scalar, integer)
	uint32_t opcode = 0b00011110'01'100010000000'00000'00000;
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cvtsi2ss(MachineInst* inst)
{
	// SCVTF (scalar, integer)
	uint32_t opcode = 0b00011110'00'100010000000'00000'00000;
	opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::fcmpsd(MachineInst* inst)
{
	// FCMP
	if (inst->operands[1].type == MachineOperandType::imm) // Note: imm must be zero
	{
		uint32_t opcode = 0b00011110'01'1'00000'001000'00000'01000;
		opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
		writeOpcode(opcode, inst);
	}
	else
	{
		uint32_t opcode = 0b00011110'01'1'00000'001000'00000'00000;
		opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
		opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
		writeOpcode(opcode, inst);
	}
}

void MachineCodeWriterAArch64::fcmpss(MachineInst* inst)
{
	// FCMP
	if (inst->operands[1].type == MachineOperandType::imm) // Note: imm must be zero
	{
		uint32_t opcode = 0b00011110'00'1'00000'001000'00000'01000;
		opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
		writeOpcode(opcode, inst);
	}
	else
	{
		uint32_t opcode = 0b00011110'00'1'00000'001000'00000'00000;
		opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
		opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
		writeOpcode(opcode, inst);
	}
}

void MachineCodeWriterAArch64::cmp32_sx8(MachineInst* inst)
{
	// CMP (extended register)
	uint32_t opcode = 0b01101011001'00000'000'000'00000'11111;
	opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cmp32_sx16(MachineInst* inst)
{
	// CMP (extended register)
	uint32_t opcode = 0b01101011001'00000'001'000'00000'11111;
	opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cmp32_sx32(MachineInst* inst)
{
	// CMP (extended register)
	uint32_t opcode = 0b01101011001'00000'010'000'00000'11111;
	opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cmp32_sx64(MachineInst* inst)
{
	// CMP (extended register)
	uint32_t opcode = 0b01101011001'00000'011'000'00000'11111;
	opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cmp32_zx8(MachineInst* inst)
{
	// CMP (extended register)
	uint32_t opcode = 0b01101011001'00000'100'000'00000'11111;
	opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cmp32_zx16(MachineInst* inst)
{
	// CMP (extended register)
	uint32_t opcode = 0b01101011001'00000'101'000'00000'11111;
	opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cmp32_zx32(MachineInst* inst)
{
	// CMP (extended register)
	uint32_t opcode = 0b01101011001'00000'110'000'00000'11111;
	opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cmp32_zx64(MachineInst* inst)
{
	// CMP (extended register)
	uint32_t opcode = 0b01101011001'00000'111'000'00000'11111;
	opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cmp64_sx8(MachineInst* inst)
{
	// CMP (extended register)
	uint32_t opcode = 0b11101011001'00000'000'000'00000'11111;
	opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cmp64_sx16(MachineInst* inst)
{
	// CMP (extended register)
	uint32_t opcode = 0b11101011001'00000'001'000'00000'11111;
	opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cmp64_sx32(MachineInst* inst)
{
	// CMP (extended register)
	uint32_t opcode = 0b11101011001'00000'010'000'00000'11111;
	opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cmp64_sx64(MachineInst* inst)
{
	// CMP (extended register)
	uint32_t opcode = 0b11101011001'00000'011'000'00000'11111;
	opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cmp64_zx8(MachineInst* inst)
{
	// CMP (extended register)
	uint32_t opcode = 0b11101011001'00000'100'000'00000'11111;
	opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cmp64_zx16(MachineInst* inst)
{
	// CMP (extended register)
	uint32_t opcode = 0b11101011001'00000'101'000'00000'11111;
	opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cmp64_zx32(MachineInst* inst)
{
	// CMP (extended register)
	uint32_t opcode = 0b11101011001'00000'110'000'00000'11111;
	opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cmp64_zx64(MachineInst* inst)
{
	// CMP (extended register)
	uint32_t opcode = 0b11101011001'00000'111'000'00000'11111;
	opcode |= getPhysReg(inst->operands[1]) << 16; // Rm
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::cseteq(MachineInst* inst)
{
	// CSET invcond NE
	uint32_t opcode = 0b0001101010011111'0000'0111111'00000;
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::csetne(MachineInst* inst)
{
	// CSET invcond EQ
	uint32_t opcode = 0b0001101010011111'0001'0111111'00000;
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::csetcs(MachineInst* inst)
{
	// CSET invcond CC
	uint32_t opcode = 0b0001101010011111'0010'0111111'00000;
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::csetcc(MachineInst* inst)
{
	// CSET invcond CS
	uint32_t opcode = 0b0001101010011111'0011'0111111'00000;
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::csetmi(MachineInst* inst)
{
	// CSET invcond PL
	uint32_t opcode = 0b0001101010011111'0100'0111111'00000;
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::csetpl(MachineInst* inst)
{
	// CSET invcond MI
	uint32_t opcode = 0b0001101010011111'0101'0111111'00000;
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::csetvs(MachineInst* inst)
{
	// CSET invcond VC
	uint32_t opcode = 0b0001101010011111'0110'0111111'00000;
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::csetvc(MachineInst* inst)
{
	// CSET invcond VS
	uint32_t opcode = 0b0001101010011111'0111'0111111'00000;
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::csethi(MachineInst* inst)
{
	// CSET invcond LS
	uint32_t opcode = 0b0001101010011111'1000'0111111'00000;
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::csetls(MachineInst* inst)
{
	// CSET invcond HI
	uint32_t opcode = 0b0001101010011111'1001'0111111'00000;
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::csetge(MachineInst* inst)
{
	// CSET invcond LT
	uint32_t opcode = 0b0001101010011111'1010'0111111'00000;
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::csetlt(MachineInst* inst)
{
	// CSET invcond GE
	uint32_t opcode = 0b0001101010011111'1011'0111111'00000;
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::csetgt(MachineInst* inst)
{
	// CSET invcond LE
	uint32_t opcode = 0b0001101010011111'1100'0111111'00000;
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::csetle(MachineInst* inst)
{
	// CSET invcond GT
	uint32_t opcode = 0b0001101010011111'1101'0111111'00000;
	opcode |= getPhysReg(inst->operands[0]); // Rd
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::b(MachineInst* inst)
{
	// B
	uint32_t opcode = 0b000101'11111111111111111111111111;
	writeOpcode(opcode, inst);
	codeholder->bbRelocateInfo.push_back({ codeholder->code.size() - 4, inst->operands[0].bb, (1 << 26) - 1, 0 });
}

void MachineCodeWriterAArch64::beq(MachineInst* inst)
{
	// B.cond
	uint32_t opcode = 0b01010100'1111111111111111111'0'0000;
	writeOpcode(opcode, inst);
	codeholder->bbRelocateInfo.push_back({ codeholder->code.size() - 4, inst->operands[0].bb, (1 << 19) - 1, 5 });
}

void MachineCodeWriterAArch64::bne(MachineInst* inst)
{
	// B.cond
	uint32_t opcode = 0b01010100'1111111111111111111'0'0001;
	writeOpcode(opcode, inst);
	codeholder->bbRelocateInfo.push_back({ codeholder->code.size() - 4, inst->operands[0].bb, (1 << 19) - 1, 5 });
}

void MachineCodeWriterAArch64::bl(MachineInst* inst)
{
	// BL
	// uint32_t opcode = 0b100101'11111111111111111111111111;
	// writeOpcode(opcode, inst);
	// codeholder->callRelocateInfo.push_back({ codeholder->code.size() - 4, inst->operands[0].func, (1 << 26) - 1, 0 });
}

void MachineCodeWriterAArch64::blr(MachineInst* inst)
{
	// BLR
	uint32_t opcode = 0b1101011000111111000000'00000'00000;
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::ret(MachineInst* inst)
{
	// RET
	uint32_t opcode = 0b1101011001011111000000'00000'00000;
	opcode |= getPhysReg(RegisterNameAArch64::lr) << 5; // Rn
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::writeOpcode(uint32_t opcode, MachineInst* debugInfo)
{
	if (debugInfo->lineNumber != -1)
	{
		MachineCodeHolder::DebugInfo info;
		info.offset = codeholder->code.size();
		info.fileIndex = debugInfo->fileIndex;
		info.lineNumber = debugInfo->lineNumber;
		codeholder->debugInfo.push_back(info);
	}

	codeholder->code.push_back((uint8_t)opcode);
	codeholder->code.push_back((uint8_t)(opcode >> 8));
	codeholder->code.push_back((uint8_t)(opcode >> 16));
	codeholder->code.push_back((uint8_t)(opcode >> 24));
}

std::pair<RegisterNameAArch64, uint32_t> MachineCodeWriterAArch64::getImmediateOffset(const MachineOperand& operand)
{
	if (operand.type == MachineOperandType::reg)
	{
		RegisterNameAArch64 base = (RegisterNameAArch64)operand.registerIndex;
		uint32_t offset = 0;
		return { base, offset };
	}
	if (operand.type == MachineOperandType::stackOffset)
	{
		RegisterNameAArch64 base = RegisterNameAArch64::sp;
		uint32_t offset = operand.stackOffset;
		return { base, offset };
	}
	else if (operand.type == MachineOperandType::frameOffset)
	{
		RegisterNameAArch64 base = sfunc->dynamicStackAllocations ? RegisterNameAArch64::fp : RegisterNameAArch64::sp;
		uint32_t offset = sfunc->frameBaseOffset + operand.frameOffset;
		return { base, offset };
	}
	else if (operand.type == MachineOperandType::spillOffset)
	{
		RegisterNameAArch64 base = sfunc->dynamicStackAllocations ? RegisterNameAArch64::fp : RegisterNameAArch64::sp;
		uint32_t offset = sfunc->spillBaseOffset + operand.spillOffset;
		return { base, offset };
	}
	else
	{
		throw std::runtime_error("Unsupported operand type in getImmediateOffset");
	}
}

uint32_t MachineCodeWriterAArch64::getPhysReg(const MachineOperand& operand)
{
	return getPhysReg((RegisterNameAArch64)operand.registerIndex);
}

uint32_t MachineCodeWriterAArch64::getPhysReg(RegisterNameAArch64 name)
{
	int index = (int)name;
	if (index < (int)RegisterNameAArch64::xmm0)
		return index;
	else
		return index - (int)RegisterNameAArch64::xmm0;
}
