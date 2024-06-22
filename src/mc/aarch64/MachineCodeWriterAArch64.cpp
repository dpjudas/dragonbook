
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
	case MachineInstOpcodeAArch64::movsx8_16: movsx8_16(inst); break;
	case MachineInstOpcodeAArch64::movsx8_32: movsx8_32(inst); break;
	case MachineInstOpcodeAArch64::movsx8_64: movsx8_64(inst); break;
	case MachineInstOpcodeAArch64::movsx16_32: movsx16_32(inst); break;
	case MachineInstOpcodeAArch64::movsx16_64: movsx16_64(inst); break;
	case MachineInstOpcodeAArch64::movsx32_64: movsx32_64(inst); break;
	case MachineInstOpcodeAArch64::movzx8_16: movzx8_16(inst); break;
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
	}
}

void MachineCodeWriterAArch64::nop(MachineInst* inst)
{
	writeOpcode(0b11010101000000110010000000011111, inst);
}

void MachineCodeWriterAArch64::loadss(MachineInst* inst)
{
}

void MachineCodeWriterAArch64::loadsd(MachineInst* inst)
{
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
}

void MachineCodeWriterAArch64::storesd(MachineInst* inst)
{
}

void MachineCodeWriterAArch64::store64(MachineInst* inst)
{
	auto dstOffset = getImmediateOffset(inst->operands[0]);

	// STR (immediate):
	uint32_t opcode = 0b1111100100'000000000000'00000'00000;
	opcode |= (dstOffset.second & ((1 << 12) - 1)) << 10; // imm12
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	opcode |= getPhysReg(dstOffset.first); // Rt
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::store32(MachineInst* inst)
{
	auto dstOffset = getImmediateOffset(inst->operands[0]);

	// STR (immediate):
	uint32_t opcode = 0b1011100100'000000000000'00000'00000;
	opcode |= (dstOffset.second & ((1 << 12) - 1)) << 10; // imm12
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	opcode |= getPhysReg(dstOffset.first); // Rt
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::store16(MachineInst* inst)
{
	auto dstOffset = getImmediateOffset(inst->operands[0]);

	// STRH (immediate):
	uint32_t opcode = 0b0111100100'000000000000'00000'00000;
	opcode |= (dstOffset.second & ((1 << 12) - 1)) << 10; // imm12
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	opcode |= getPhysReg(dstOffset.first); // Rt
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::store8(MachineInst* inst)
{
	auto dstOffset = getImmediateOffset(inst->operands[0]);

	// STRB (immediate):
	uint32_t opcode = 0b0011100100'000000000000'00000'00000;
	opcode |= (dstOffset.second & ((1 << 12) - 1)) << 10; // imm12
	opcode |= getPhysReg(inst->operands[0]) << 5; // Rn
	opcode |= getPhysReg(dstOffset.first); // Rt
	writeOpcode(opcode, inst);
}

void MachineCodeWriterAArch64::movss(MachineInst* inst)
{
}

void MachineCodeWriterAArch64::movsd(MachineInst* inst)
{
}

void MachineCodeWriterAArch64::mov64(MachineInst* inst)
{
	if (inst->operands[1].type == MachineOperandType::imm)
	{
		uint32_t opcode = 0b110100101'00'0000000000000000'00000;
		opcode |= (inst->operands[1].immvalue & 0xffff) << 5; // imm16
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
	else
	{
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
		uint32_t opcode = 0b010100101'00'0000000000000000'00000;
		opcode |= (inst->operands[1].immvalue & 0xffff) << 5; // imm16
		opcode |= getPhysReg(inst->operands[0]); // Rd
		writeOpcode(opcode, inst);
	}
	else
	{
		uint32_t opcode = 0b00101010000'00000'000000'11111'00000;
		opcode |= getPhysReg(inst->operands[0]) << 16; // Rm
		opcode |= getPhysReg(inst->operands[1]); // Rd
		writeOpcode(opcode, inst);
	}
}

void MachineCodeWriterAArch64::movsx8_16(MachineInst* inst)
{
}

void MachineCodeWriterAArch64::movsx8_32(MachineInst* inst)
{
}

void MachineCodeWriterAArch64::movsx8_64(MachineInst* inst)
{
}

void MachineCodeWriterAArch64::movsx16_32(MachineInst* inst)
{
}

void MachineCodeWriterAArch64::movsx16_64(MachineInst* inst)
{
}

void MachineCodeWriterAArch64::movsx32_64(MachineInst* inst)
{
}

void MachineCodeWriterAArch64::movzx8_16(MachineInst* inst)
{
}

void MachineCodeWriterAArch64::movzx8_32(MachineInst* inst)
{
}

void MachineCodeWriterAArch64::movzx8_64(MachineInst* inst)
{
}

void MachineCodeWriterAArch64::movzx16_32(MachineInst* inst)
{
}

void MachineCodeWriterAArch64::movzx16_64(MachineInst* inst)
{
}

void MachineCodeWriterAArch64::addss(MachineInst* inst)
{
}

void MachineCodeWriterAArch64::addsd(MachineInst* inst)
{
}

void MachineCodeWriterAArch64::add64(MachineInst* inst)
{
	if (inst->operands[2].type == MachineOperandType::imm)
	{
		uint32_t opcode = 0b1001000100'000000000000'00000'00000;
		opcode |= (inst->operands[2].immvalue & ((1 << 12) - 1)) << 10; // imm12
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
	}
	else
	{
		uint32_t opcode = 0b10001011001'00000'011'000'00000'00000;
		opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
	}
}

void MachineCodeWriterAArch64::add32(MachineInst* inst)
{
	if (inst->operands[2].type == MachineOperandType::imm)
	{
		uint32_t opcode = 0b0001000100'000000000000'00000'00000;
		opcode |= (inst->operands[2].immvalue & ((1 << 12) - 1)) << 10; // imm12
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
	}
	else
	{
		uint32_t opcode = 0b00001011001'00000'010'000'00000'00000;
		opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
	}
}

void MachineCodeWriterAArch64::subss(MachineInst* inst)
{
}

void MachineCodeWriterAArch64::subsd(MachineInst* inst)
{
}

void MachineCodeWriterAArch64::sub64(MachineInst* inst)
{
	if (inst->operands[2].type == MachineOperandType::imm)
	{
		uint32_t opcode = 0b101000100'000000000000'00000'00000;
		opcode |= (inst->operands[2].immvalue & ((1 << 12) - 1)) << 10; // imm12
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
	}
	else
	{
		uint32_t opcode = 0b11001011001'00000'010'000'00000'00000;
		opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
	}
}

void MachineCodeWriterAArch64::sub32(MachineInst* inst)
{
	if (inst->operands[2].type == MachineOperandType::imm)
	{
		uint32_t opcode = 0b101000100'000000000000'00000'00000;
		opcode |= (inst->operands[2].immvalue & ((1 << 12) - 1)) << 10; // imm12
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
	}
	else
	{
		uint32_t opcode = 0b01001011001'00000'010'000'00000'00000;
		opcode |= getPhysReg(inst->operands[2]) << 16; // Rm
		opcode |= getPhysReg(inst->operands[1]) << 5; // Rn
		opcode |= getPhysReg(inst->operands[0]); // Rd
	}
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
