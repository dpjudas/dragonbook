
#include "AssemblyWriter.h"
#include "MachineInst.h"

void AssemblyWriter::codegen()
{
	basicblock(sfunc->prolog);

	for (MachineBasicBlock* bb : sfunc->basicBlocks)
	{
		basicblock(bb);
	}

	basicblock(sfunc->epilog);
}

void AssemblyWriter::basicblock(MachineBasicBlock* bb)
{
	output << "BB:" << std::endl;
	for (MachineInst* inst : bb->code)
	{
		opcode(inst);
	}
}

void AssemblyWriter::opcode(MachineInst* inst)
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

void AssemblyWriter::writeInst(const char* name, MachineInst* inst)
{
	static const char* regname[] = { "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15" };

	output << "    " << name;

	size_t count = strlen(name);
	if (!inst->operands.empty())
	{
		for (size_t i = 4 + count; i < 15; i++)
			output << " ";
	}

	for (const auto& operand : inst->operands)
	{
		output << " ";
		switch (operand.type)
		{
		case MachineOperandType::reg: output << (operand.registerIndex < 32 ? regname[operand.registerIndex] : "vreg"); break;
		case MachineOperandType::constant: output << "constant"; break;
		case MachineOperandType::stack: output << "rsp+" << (sfunc->stackSize - operand.stackOffset); break;
		case MachineOperandType::imm: output << "imm"; break;
		case MachineOperandType::basicblock: output << "basicblock"; break;
		case MachineOperandType::func: output << "func"; break;
		case MachineOperandType::global: output << "global"; break;
		}
	}

	if (!inst->comment.empty())
	{
		output << " ; ";
		output << inst->comment.c_str();
	}

	output << std::endl;
}

void AssemblyWriter::nop(MachineInst* inst)
{
	writeInst("nop", inst);
}

void AssemblyWriter::lea(MachineInst* inst)
{
	writeInst("lea", inst);
}

void AssemblyWriter::loadss(MachineInst* inst)
{
	writeInst("loadss", inst);
}

void AssemblyWriter::loadsd(MachineInst* inst)
{
	writeInst("loadsd", inst);
}

void AssemblyWriter::load64(MachineInst* inst)
{
	writeInst("load64", inst);
}

void AssemblyWriter::load32(MachineInst* inst)
{
	writeInst("load32", inst);
}

void AssemblyWriter::load16(MachineInst* inst)
{
	writeInst("load16", inst);
}

void AssemblyWriter::load8(MachineInst* inst)
{
	writeInst("load8", inst);
}

void AssemblyWriter::storess(MachineInst* inst)
{
	writeInst("storess", inst);
}

void AssemblyWriter::storesd(MachineInst* inst)
{
	writeInst("storesd", inst);
}

void AssemblyWriter::store64(MachineInst* inst)
{
	writeInst("store64", inst);
}

void AssemblyWriter::store32(MachineInst* inst)
{
	writeInst("store32", inst);
}

void AssemblyWriter::store16(MachineInst* inst)
{
	writeInst("store16", inst);
}

void AssemblyWriter::store8(MachineInst* inst)
{
	writeInst("store8", inst);
}

void AssemblyWriter::movss(MachineInst* inst)
{
	writeInst("movss", inst);
}

void AssemblyWriter::movsd(MachineInst* inst)
{
	writeInst("movsd", inst);
}

void AssemblyWriter::mov64(MachineInst* inst)
{
	writeInst("mov64", inst);
}

void AssemblyWriter::mov32(MachineInst* inst)
{
	writeInst("mov32", inst);
}

void AssemblyWriter::mov16(MachineInst* inst)
{
	writeInst("mov16", inst);
}

void AssemblyWriter::mov8(MachineInst* inst)
{
	writeInst("mov8", inst);
}

void AssemblyWriter::addss(MachineInst* inst)
{
	writeInst("addss", inst);
}

void AssemblyWriter::addsd(MachineInst* inst)
{
	writeInst("addsd", inst);
}

void AssemblyWriter::add64(MachineInst* inst)
{
	writeInst("add64", inst);
}

void AssemblyWriter::add32(MachineInst* inst)
{
	writeInst("add32", inst);
}

void AssemblyWriter::add16(MachineInst* inst)
{
	writeInst("add16", inst);
}

void AssemblyWriter::add8(MachineInst* inst)
{
	writeInst("add8", inst);
}

void AssemblyWriter::subss(MachineInst* inst)
{
	writeInst("subss", inst);
}

void AssemblyWriter::subsd(MachineInst* inst)
{
	writeInst("subsd", inst);
}

void AssemblyWriter::sub64(MachineInst* inst)
{
	writeInst("sub64", inst);
}

void AssemblyWriter::sub32(MachineInst* inst)
{
	writeInst("sub32", inst);
}

void AssemblyWriter::sub16(MachineInst* inst)
{
	writeInst("sub16", inst);
}

void AssemblyWriter::sub8(MachineInst* inst)
{
	writeInst("sub8", inst);
}

void AssemblyWriter::not64(MachineInst* inst)
{
	writeInst("not64", inst);
}

void AssemblyWriter::not32(MachineInst* inst)
{
	writeInst("not32", inst);
}

void AssemblyWriter::not16(MachineInst* inst)
{
	writeInst("not16", inst);
}

void AssemblyWriter::not8(MachineInst* inst)
{
	writeInst("not8", inst);
}

void AssemblyWriter::neg64(MachineInst* inst)
{
	writeInst("neg64", inst);
}

void AssemblyWriter::neg32(MachineInst* inst)
{
	writeInst("neg32", inst);
}

void AssemblyWriter::neg16(MachineInst* inst)
{
	writeInst("neg16", inst);
}

void AssemblyWriter::neg8(MachineInst* inst)
{
	writeInst("neg8", inst);
}

void AssemblyWriter::shl64(MachineInst* inst)
{
	writeInst("shl64", inst);
}

void AssemblyWriter::shl32(MachineInst* inst)
{
	writeInst("shl32", inst);
}

void AssemblyWriter::shl16(MachineInst* inst)
{
	writeInst("shl16", inst);
}

void AssemblyWriter::shl8(MachineInst* inst)
{
	writeInst("shl8", inst);
}

void AssemblyWriter::sar64(MachineInst* inst)
{
	writeInst("sar64", inst);
}

void AssemblyWriter::sar32(MachineInst* inst)
{
	writeInst("sar32", inst);
}

void AssemblyWriter::sar16(MachineInst* inst)
{
	writeInst("sar16", inst);
}

void AssemblyWriter::sar8(MachineInst* inst)
{
	writeInst("sar8", inst);
}

void AssemblyWriter::and64(MachineInst* inst)
{
	writeInst("and64", inst);
}

void AssemblyWriter::and32(MachineInst* inst)
{
	writeInst("and32", inst);
}

void AssemblyWriter::and16(MachineInst* inst)
{
	writeInst("and16", inst);
}

void AssemblyWriter::and8(MachineInst* inst)
{
	writeInst("and8", inst);
}

void AssemblyWriter::or64(MachineInst* inst)
{
	writeInst("or64", inst);
}

void AssemblyWriter::or32(MachineInst* inst)
{
	writeInst("or32", inst);
}

void AssemblyWriter::or16(MachineInst* inst)
{
	writeInst("or16", inst);
}

void AssemblyWriter::or8(MachineInst* inst)
{
	writeInst("or8", inst);
}

void AssemblyWriter::xorpd(MachineInst* inst)
{
	writeInst("xorpd", inst);
}

void AssemblyWriter::xorps(MachineInst* inst)
{
	writeInst("xorps", inst);
}

void AssemblyWriter::xor64(MachineInst* inst)
{
	writeInst("xor64", inst);
}

void AssemblyWriter::xor32(MachineInst* inst)
{
	writeInst("xor32", inst);
}

void AssemblyWriter::xor16(MachineInst* inst)
{
	writeInst("xor16", inst);
}

void AssemblyWriter::xor8(MachineInst* inst)
{
	writeInst("xor8", inst);
}

void AssemblyWriter::mulss(MachineInst* inst)
{
	writeInst("mulss", inst);
}

void AssemblyWriter::mulsd(MachineInst* inst)
{
	writeInst("mulsd", inst);
}

void AssemblyWriter::imul64(MachineInst* inst)
{
	writeInst("imul64", inst);
}

void AssemblyWriter::imul32(MachineInst* inst)
{
	writeInst("imul32", inst);
}

void AssemblyWriter::imul16(MachineInst* inst)
{
	writeInst("imul16", inst);
}

void AssemblyWriter::imul8(MachineInst* inst)
{
	writeInst("imul8", inst);
}

void AssemblyWriter::divss(MachineInst* inst)
{
	writeInst("divss", inst);
}

void AssemblyWriter::divsd(MachineInst* inst)
{
	writeInst("divsd", inst);
}

void AssemblyWriter::idiv64(MachineInst* inst)
{
	writeInst("idiv64", inst);
}

void AssemblyWriter::idiv32(MachineInst* inst)
{
	writeInst("idiv32", inst);
}

void AssemblyWriter::idiv16(MachineInst* inst)
{
	writeInst("idiv16", inst);
}

void AssemblyWriter::idiv8(MachineInst* inst)
{
	writeInst("idiv8", inst);
}

void AssemblyWriter::mul64(MachineInst* inst)
{
	writeInst("mul64", inst);
}

void AssemblyWriter::mul32(MachineInst* inst)
{
	writeInst("mul32", inst);
}

void AssemblyWriter::mul16(MachineInst* inst)
{
	writeInst("mul16", inst);
}

void AssemblyWriter::mul8(MachineInst* inst)
{
	writeInst("mul8", inst);
}

void AssemblyWriter::div64(MachineInst* inst)
{
	writeInst("div64", inst);
}

void AssemblyWriter::div32(MachineInst* inst)
{
	writeInst("div32", inst);
}

void AssemblyWriter::div16(MachineInst* inst)
{
	writeInst("div16", inst);
}

void AssemblyWriter::div8(MachineInst* inst)
{
	writeInst("div8", inst);
}

void AssemblyWriter::ucomisd(MachineInst* inst)
{
	writeInst("ucomisd", inst);
}

void AssemblyWriter::ucomiss(MachineInst* inst)
{
	writeInst("ucomiss", inst);
}

void AssemblyWriter::cmp64(MachineInst* inst)
{
	writeInst("cmp64", inst);
}

void AssemblyWriter::cmp32(MachineInst* inst)
{
	writeInst("cmp32", inst);
}

void AssemblyWriter::cmp16(MachineInst* inst)
{
	writeInst("cmp16", inst);
}

void AssemblyWriter::cmp8(MachineInst* inst)
{
	writeInst("cmp8", inst);
}

void AssemblyWriter::setl(MachineInst* inst)
{
	writeInst("setl", inst);
}

void AssemblyWriter::setb(MachineInst* inst)
{
	writeInst("setb", inst);
}

void AssemblyWriter::seta(MachineInst* inst)
{
	writeInst("seta", inst);
}

void AssemblyWriter::setg(MachineInst* inst)
{
	writeInst("setg", inst);
}

void AssemblyWriter::setle(MachineInst* inst)
{
	writeInst("setle", inst);
}

void AssemblyWriter::setbe(MachineInst* inst)
{
	writeInst("setbe", inst);
}

void AssemblyWriter::setae(MachineInst* inst)
{
	writeInst("setae", inst);
}

void AssemblyWriter::setge(MachineInst* inst)
{
	writeInst("setge", inst);
}

void AssemblyWriter::sete(MachineInst* inst)
{
	writeInst("sete", inst);
}

void AssemblyWriter::setne(MachineInst* inst)
{
	writeInst("setne", inst);
}

void AssemblyWriter::cvtsd2ss(MachineInst* inst)
{
	writeInst("cvtsd2ss", inst);
}

void AssemblyWriter::cvtss2sd(MachineInst* inst)
{
	writeInst("cvtss2sd", inst);
}

void AssemblyWriter::cvttsd2si(MachineInst* inst)
{
	writeInst("cvttsd2si", inst);
}

void AssemblyWriter::cvttss2si(MachineInst* inst)
{
	writeInst("cvttss2si", inst);
}

void AssemblyWriter::cvtsi2sd(MachineInst* inst)
{
	writeInst("cvtsi2sd", inst);
}

void AssemblyWriter::cvtsi2ss(MachineInst* inst)
{
	writeInst("cvtsi2ss", inst);
}

void AssemblyWriter::jmp(MachineInst* inst)
{
	writeInst("jmp", inst);
}

void AssemblyWriter::jz(MachineInst* inst)
{
	writeInst("jz", inst);
}

void AssemblyWriter::call(MachineInst* inst)
{
	writeInst("call", inst);
}

void AssemblyWriter::alloca_(MachineInst* inst)
{
	writeInst("alloca", inst);
}

void AssemblyWriter::ret(MachineInst* inst)
{
	writeInst("ret", inst);
}

void AssemblyWriter::push(MachineInst* inst)
{
	writeInst("push", inst);
}

void AssemblyWriter::pop(MachineInst* inst)
{
	writeInst("pop", inst);
}