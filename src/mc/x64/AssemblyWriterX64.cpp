
#include "AssemblyWriterX64.h"

void AssemblyWriterX64::codegen()
{
	int nameIndex = 0;
	setBasicBlockName(sfunc->prolog, nameIndex++);
	for (MachineBasicBlock* bb : sfunc->basicBlocks)
	{
		setBasicBlockName(bb, nameIndex++);
	}
	setBasicBlockName(sfunc->epilog, nameIndex++);

	basicblock(sfunc->prolog);

	for (MachineBasicBlock* bb : sfunc->basicBlocks)
	{
		basicblock(bb);
	}

	basicblock(sfunc->epilog);
}

void AssemblyWriterX64::basicblock(MachineBasicBlock* bb)
{
	output << getBasicBlockName(bb) << ":" << std::endl;
	for (MachineInst* inst : bb->code)
	{
		opcode(inst);
	}
}

void AssemblyWriterX64::opcode(MachineInst* inst)
{
	switch ((MachineInstOpcodeX64)inst->opcode)
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

void AssemblyWriterX64::writeInst(const char* name, MachineInst* inst, int ptrindex)
{
	static const char* regname64[] = { "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15" };
	static const char* regname32[] = { "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15" };
	static const char* regname16[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di", "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15" };
	static const char* regname8[] = { "al", "cl", "dl", "bl", "spl", "bpl", "sil", "dil", "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15" };
	static const char** regname[] = { regname64, regname32, regname16, regname8 };

	OperandSize opsize = OperandSize::int64;

	output << "    " << name;

	size_t count = strlen(name);
	if (!inst->operands.empty())
	{
		for (size_t i = 4 + count; i < 15; i++)
			output << " ";
	}

	std::stringstream opoutput;

	int index = 0;
	for (const auto& operand : inst->operands)
	{
		opoutput << (index == 0 ? " " : ", ");
		if (index == ptrindex) opoutput << "ptr[";
		bool dsa;
		int offset;
		switch (operand.type)
		{
		case MachineOperandType::reg: opoutput << (operand.registerIndex < 32 ? regname[(int)opsize][operand.registerIndex] : "vreg"); break;
		case MachineOperandType::constant: opoutput << "constants[" << operand.constantIndex << "]"; break;
		case MachineOperandType::frameOffset:
			dsa = sfunc->dynamicStackAllocations;
			offset = sfunc->frameBaseOffset + operand.frameOffset;
			opoutput << (dsa ? "rbp" : "rsp") << (offset >= 0 ? "+" : "") << std::hex << offset << std::dec << "h";
			break;
		case MachineOperandType::spillOffset:
			dsa = sfunc->dynamicStackAllocations;
			offset = sfunc->spillBaseOffset + operand.spillOffset;
			opoutput << (dsa ? "rbp" : "rsp") << (offset >= 0 ? "+" : "") << std::hex << offset << std::dec << "h";
			break;
		case MachineOperandType::stackOffset:
			offset = operand.stackOffset;
			opoutput << "rsp" << (offset >= 0 ? "+" : "") << std::hex << offset << std::dec << "h";
			break;
		case MachineOperandType::imm: opoutput << std::hex << operand.immvalue << std::dec << "h"; break;
		case MachineOperandType::basicblock: opoutput << getBasicBlockName(operand.bb); break;
		case MachineOperandType::func: opoutput << operand.func->name.c_str(); break;
		case MachineOperandType::global: opoutput << "global"; break;
		}

		if (index == ptrindex) opoutput << "]";
		index++;
	}

	std::string optext = opoutput.str();
	output << optext.c_str();

	if (!inst->comment.empty())
	{
		size_t count = optext.length();
		for (size_t i = 16 + count; i < 48; i++)
			output << " ";

		output << "; ";
		output << inst->comment.c_str();
	}

	output << std::endl;
}

const char* AssemblyWriterX64::getBasicBlockName(MachineBasicBlock* bb)
{
	return bbNames.find(bb)->second.c_str();
}

void AssemblyWriterX64::setBasicBlockName(MachineBasicBlock* bb, int nameIndex)
{
	std::string name = sfunc->name.empty() ? std::string("BB") : sfunc->name;
	name += ".BB" + std::to_string(nameIndex);
	bbNames[bb] = std::move(name);
}

void AssemblyWriterX64::nop(MachineInst* inst)
{
	writeInst("nop", inst);
}

void AssemblyWriterX64::lea(MachineInst* inst)
{
	writeInst("lea", inst);
}

void AssemblyWriterX64::loadss(MachineInst* inst)
{
	writeInst("loadss", inst, 1);
}

void AssemblyWriterX64::loadsd(MachineInst* inst)
{
	writeInst("loadsd", inst, 1);
}

void AssemblyWriterX64::load64(MachineInst* inst)
{
	writeInst("load64", inst, 1);
}

void AssemblyWriterX64::load32(MachineInst* inst)
{
	writeInst("load32", inst, 1);
}

void AssemblyWriterX64::load16(MachineInst* inst)
{
	writeInst("load16", inst, 1);
}

void AssemblyWriterX64::load8(MachineInst* inst)
{
	writeInst("load8", inst, 1);
}

void AssemblyWriterX64::storess(MachineInst* inst)
{
	writeInst("storess", inst, 0);
}

void AssemblyWriterX64::storesd(MachineInst* inst)
{
	writeInst("storesd", inst, 0);
}

void AssemblyWriterX64::store64(MachineInst* inst)
{
	writeInst("store64", inst, 0);
}

void AssemblyWriterX64::store32(MachineInst* inst)
{
	writeInst("store32", inst, 0);
}

void AssemblyWriterX64::store16(MachineInst* inst)
{
	writeInst("store16", inst, 0);
}

void AssemblyWriterX64::store8(MachineInst* inst)
{
	writeInst("store8", inst, 0);
}

void AssemblyWriterX64::movss(MachineInst* inst)
{
	writeInst("movss", inst);
}

void AssemblyWriterX64::movsd(MachineInst* inst)
{
	writeInst("movsd", inst);
}

void AssemblyWriterX64::mov64(MachineInst* inst)
{
	writeInst("mov64", inst);
}

void AssemblyWriterX64::mov32(MachineInst* inst)
{
	writeInst("mov32", inst);
}

void AssemblyWriterX64::mov16(MachineInst* inst)
{
	writeInst("mov16", inst);
}

void AssemblyWriterX64::mov8(MachineInst* inst)
{
	writeInst("mov8", inst);
}

void AssemblyWriterX64::movsx8_16(MachineInst* inst)
{
	writeInst("movsx8_16", inst);
}

void AssemblyWriterX64::movsx8_32(MachineInst* inst)
{
	writeInst("movsx8_32", inst);
}

void AssemblyWriterX64::movsx8_64(MachineInst* inst)
{
	writeInst("movsx8_64", inst);
}

void AssemblyWriterX64::movsx16_32(MachineInst* inst)
{
	writeInst("movsx16_32", inst);
}

void AssemblyWriterX64::movsx16_64(MachineInst* inst)
{
	writeInst("movsx16_64", inst);
}

void AssemblyWriterX64::movsx32_64(MachineInst* inst)
{
	writeInst("movsx32_64", inst);
}

void AssemblyWriterX64::movzx8_16(MachineInst* inst)
{
	writeInst("movzx8_16", inst);
}

void AssemblyWriterX64::movzx8_32(MachineInst* inst)
{
	writeInst("movzx8_32", inst);
}

void AssemblyWriterX64::movzx8_64(MachineInst* inst)
{
	writeInst("movzx8_64", inst);
}

void AssemblyWriterX64::movzx16_32(MachineInst* inst)
{
	writeInst("movzx16_32", inst);
}

void AssemblyWriterX64::movzx16_64(MachineInst* inst)
{
	writeInst("movzx16_64", inst);
}

void AssemblyWriterX64::addss(MachineInst* inst)
{
	writeInst("addss", inst);
}

void AssemblyWriterX64::addsd(MachineInst* inst)
{
	writeInst("addsd", inst);
}

void AssemblyWriterX64::add64(MachineInst* inst)
{
	writeInst("add64", inst);
}

void AssemblyWriterX64::add32(MachineInst* inst)
{
	writeInst("add32", inst);
}

void AssemblyWriterX64::add16(MachineInst* inst)
{
	writeInst("add16", inst);
}

void AssemblyWriterX64::add8(MachineInst* inst)
{
	writeInst("add8", inst);
}

void AssemblyWriterX64::subss(MachineInst* inst)
{
	writeInst("subss", inst);
}

void AssemblyWriterX64::subsd(MachineInst* inst)
{
	writeInst("subsd", inst);
}

void AssemblyWriterX64::sub64(MachineInst* inst)
{
	writeInst("sub64", inst);
}

void AssemblyWriterX64::sub32(MachineInst* inst)
{
	writeInst("sub32", inst);
}

void AssemblyWriterX64::sub16(MachineInst* inst)
{
	writeInst("sub16", inst);
}

void AssemblyWriterX64::sub8(MachineInst* inst)
{
	writeInst("sub8", inst);
}

void AssemblyWriterX64::not64(MachineInst* inst)
{
	writeInst("not64", inst);
}

void AssemblyWriterX64::not32(MachineInst* inst)
{
	writeInst("not32", inst);
}

void AssemblyWriterX64::not16(MachineInst* inst)
{
	writeInst("not16", inst);
}

void AssemblyWriterX64::not8(MachineInst* inst)
{
	writeInst("not8", inst);
}

void AssemblyWriterX64::neg64(MachineInst* inst)
{
	writeInst("neg64", inst);
}

void AssemblyWriterX64::neg32(MachineInst* inst)
{
	writeInst("neg32", inst);
}

void AssemblyWriterX64::neg16(MachineInst* inst)
{
	writeInst("neg16", inst);
}

void AssemblyWriterX64::neg8(MachineInst* inst)
{
	writeInst("neg8", inst);
}

void AssemblyWriterX64::shl64(MachineInst* inst)
{
	writeInst("shl64", inst);
}

void AssemblyWriterX64::shl32(MachineInst* inst)
{
	writeInst("shl32", inst);
}

void AssemblyWriterX64::shl16(MachineInst* inst)
{
	writeInst("shl16", inst);
}

void AssemblyWriterX64::shl8(MachineInst* inst)
{
	writeInst("shl8", inst);
}

void AssemblyWriterX64::shr64(MachineInst* inst)
{
	writeInst("shr64", inst);
}

void AssemblyWriterX64::shr32(MachineInst* inst)
{
	writeInst("shr32", inst);
}

void AssemblyWriterX64::shr16(MachineInst* inst)
{
	writeInst("shr16", inst);
}

void AssemblyWriterX64::shr8(MachineInst* inst)
{
	writeInst("shr8", inst);
}

void AssemblyWriterX64::sar64(MachineInst* inst)
{
	writeInst("sar64", inst);
}

void AssemblyWriterX64::sar32(MachineInst* inst)
{
	writeInst("sar32", inst);
}

void AssemblyWriterX64::sar16(MachineInst* inst)
{
	writeInst("sar16", inst);
}

void AssemblyWriterX64::sar8(MachineInst* inst)
{
	writeInst("sar8", inst);
}

void AssemblyWriterX64::and64(MachineInst* inst)
{
	writeInst("and64", inst);
}

void AssemblyWriterX64::and32(MachineInst* inst)
{
	writeInst("and32", inst);
}

void AssemblyWriterX64::and16(MachineInst* inst)
{
	writeInst("and16", inst);
}

void AssemblyWriterX64::and8(MachineInst* inst)
{
	writeInst("and8", inst);
}

void AssemblyWriterX64::or64(MachineInst* inst)
{
	writeInst("or64", inst);
}

void AssemblyWriterX64::or32(MachineInst* inst)
{
	writeInst("or32", inst);
}

void AssemblyWriterX64::or16(MachineInst* inst)
{
	writeInst("or16", inst);
}

void AssemblyWriterX64::or8(MachineInst* inst)
{
	writeInst("or8", inst);
}

void AssemblyWriterX64::xorpd(MachineInst* inst)
{
	writeInst("xorpd", inst);
}

void AssemblyWriterX64::xorps(MachineInst* inst)
{
	writeInst("xorps", inst);
}

void AssemblyWriterX64::xor64(MachineInst* inst)
{
	writeInst("xor64", inst);
}

void AssemblyWriterX64::xor32(MachineInst* inst)
{
	writeInst("xor32", inst);
}

void AssemblyWriterX64::xor16(MachineInst* inst)
{
	writeInst("xor16", inst);
}

void AssemblyWriterX64::xor8(MachineInst* inst)
{
	writeInst("xor8", inst);
}

void AssemblyWriterX64::mulss(MachineInst* inst)
{
	writeInst("mulss", inst);
}

void AssemblyWriterX64::mulsd(MachineInst* inst)
{
	writeInst("mulsd", inst);
}

void AssemblyWriterX64::imul64(MachineInst* inst)
{
	writeInst("imul64", inst);
}

void AssemblyWriterX64::imul32(MachineInst* inst)
{
	writeInst("imul32", inst);
}

void AssemblyWriterX64::imul16(MachineInst* inst)
{
	writeInst("imul16", inst);
}

void AssemblyWriterX64::imul8(MachineInst* inst)
{
	writeInst("imul8", inst);
}

void AssemblyWriterX64::divss(MachineInst* inst)
{
	writeInst("divss", inst);
}

void AssemblyWriterX64::divsd(MachineInst* inst)
{
	writeInst("divsd", inst);
}

void AssemblyWriterX64::idiv64(MachineInst* inst)
{
	writeInst("idiv64", inst);
}

void AssemblyWriterX64::idiv32(MachineInst* inst)
{
	writeInst("idiv32", inst);
}

void AssemblyWriterX64::idiv16(MachineInst* inst)
{
	writeInst("idiv16", inst);
}

void AssemblyWriterX64::idiv8(MachineInst* inst)
{
	writeInst("idiv8", inst);
}

void AssemblyWriterX64::div64(MachineInst* inst)
{
	writeInst("div64", inst);
}

void AssemblyWriterX64::div32(MachineInst* inst)
{
	writeInst("div32", inst);
}

void AssemblyWriterX64::div16(MachineInst* inst)
{
	writeInst("div16", inst);
}

void AssemblyWriterX64::div8(MachineInst* inst)
{
	writeInst("div8", inst);
}

void AssemblyWriterX64::ucomisd(MachineInst* inst)
{
	writeInst("ucomisd", inst);
}

void AssemblyWriterX64::ucomiss(MachineInst* inst)
{
	writeInst("ucomiss", inst);
}

void AssemblyWriterX64::cmp64(MachineInst* inst)
{
	writeInst("cmp64", inst);
}

void AssemblyWriterX64::cmp32(MachineInst* inst)
{
	writeInst("cmp32", inst);
}

void AssemblyWriterX64::cmp16(MachineInst* inst)
{
	writeInst("cmp16", inst);
}

void AssemblyWriterX64::cmp8(MachineInst* inst)
{
	writeInst("cmp8", inst);
}

void AssemblyWriterX64::setl(MachineInst* inst)
{
	writeInst("setl", inst);
}

void AssemblyWriterX64::setb(MachineInst* inst)
{
	writeInst("setb", inst);
}

void AssemblyWriterX64::seta(MachineInst* inst)
{
	writeInst("seta", inst);
}

void AssemblyWriterX64::setg(MachineInst* inst)
{
	writeInst("setg", inst);
}

void AssemblyWriterX64::setle(MachineInst* inst)
{
	writeInst("setle", inst);
}

void AssemblyWriterX64::setbe(MachineInst* inst)
{
	writeInst("setbe", inst);
}

void AssemblyWriterX64::setae(MachineInst* inst)
{
	writeInst("setae", inst);
}

void AssemblyWriterX64::setge(MachineInst* inst)
{
	writeInst("setge", inst);
}

void AssemblyWriterX64::sete(MachineInst* inst)
{
	writeInst("sete", inst);
}

void AssemblyWriterX64::setne(MachineInst* inst)
{
	writeInst("setne", inst);
}

void AssemblyWriterX64::setp(MachineInst* inst)
{
	writeInst("setp", inst);
}

void AssemblyWriterX64::setnp(MachineInst* inst)
{
	writeInst("setnp", inst);
}

void AssemblyWriterX64::cvtsd2ss(MachineInst* inst)
{
	writeInst("cvtsd2ss", inst);
}

void AssemblyWriterX64::cvtss2sd(MachineInst* inst)
{
	writeInst("cvtss2sd", inst);
}

void AssemblyWriterX64::cvttsd2si(MachineInst* inst)
{
	writeInst("cvttsd2si", inst);
}

void AssemblyWriterX64::cvttss2si(MachineInst* inst)
{
	writeInst("cvttss2si", inst);
}

void AssemblyWriterX64::cvtsi2sd(MachineInst* inst)
{
	writeInst("cvtsi2sd", inst);
}

void AssemblyWriterX64::cvtsi2ss(MachineInst* inst)
{
	writeInst("cvtsi2ss", inst);
}

void AssemblyWriterX64::jmp(MachineInst* inst)
{
	writeInst("jmp", inst);
}

void AssemblyWriterX64::je(MachineInst* inst)
{
	writeInst("je", inst);
}

void AssemblyWriterX64::jne(MachineInst* inst)
{
	writeInst("jne", inst);
}

void AssemblyWriterX64::call(MachineInst* inst)
{
	writeInst("call", inst);
}

void AssemblyWriterX64::ret(MachineInst* inst)
{
	writeInst("ret", inst);
}

void AssemblyWriterX64::push(MachineInst* inst)
{
	writeInst("push", inst);
}

void AssemblyWriterX64::pop(MachineInst* inst)
{
	writeInst("pop", inst);
}

void AssemblyWriterX64::movdqa(MachineInst* inst)
{
	writeInst("movdqa", inst);
}
