
#include "MachineInstSelection.h"
#include <algorithm>

MachineFunction* MachineInstSelection::codegen(IRFunction* sfunc)
{
	MachineInstSelection selection(sfunc);
	selection.mfunc = sfunc->context->newMachineFunction();
	selection.mfunc->type = dynamic_cast<IRFunctionType*>(sfunc->type);
	selection.mfunc->prolog = sfunc->context->newMachineBasicBlock();
	selection.mfunc->epilog = sfunc->context->newMachineBasicBlock();
	selection.mfunc->registers.push_back(MachineRegClass::gp); // rax
	selection.mfunc->registers.push_back(MachineRegClass::gp); // rcx
	selection.mfunc->registers.push_back(MachineRegClass::gp); // rdx
	selection.mfunc->registers.push_back(MachineRegClass::gp); // rbx
	selection.mfunc->registers.push_back(MachineRegClass::reserved); // rsp
	selection.mfunc->registers.push_back(MachineRegClass::gp); // rbp
	selection.mfunc->registers.push_back(MachineRegClass::gp); // rsi
	selection.mfunc->registers.push_back(MachineRegClass::gp); // rdi
	selection.mfunc->registers.push_back(MachineRegClass::gp); // r8
	selection.mfunc->registers.push_back(MachineRegClass::gp); // r9
	selection.mfunc->registers.push_back(MachineRegClass::gp); // r10
	selection.mfunc->registers.push_back(MachineRegClass::gp); // r11
	selection.mfunc->registers.push_back(MachineRegClass::gp); // r12
	selection.mfunc->registers.push_back(MachineRegClass::gp); // r13
	selection.mfunc->registers.push_back(MachineRegClass::gp); // r14
	selection.mfunc->registers.push_back(MachineRegClass::gp); // r15
	selection.mfunc->registers.push_back(MachineRegClass::xmm); // xmm0
	selection.mfunc->registers.push_back(MachineRegClass::xmm); // xmm1
	selection.mfunc->registers.push_back(MachineRegClass::xmm); // xmm2
	selection.mfunc->registers.push_back(MachineRegClass::xmm); // xmm3
	selection.mfunc->registers.push_back(MachineRegClass::xmm); // xmm4
	selection.mfunc->registers.push_back(MachineRegClass::xmm); // xmm5
	selection.mfunc->registers.push_back(MachineRegClass::xmm); // xmm6
	selection.mfunc->registers.push_back(MachineRegClass::xmm); // xmm7
	selection.mfunc->registers.push_back(MachineRegClass::xmm); // xmm8
	selection.mfunc->registers.push_back(MachineRegClass::xmm); // xmm9
	selection.mfunc->registers.push_back(MachineRegClass::xmm); // xmm10
	selection.mfunc->registers.push_back(MachineRegClass::xmm); // xmm11
	selection.mfunc->registers.push_back(MachineRegClass::xmm); // xmm12
	selection.mfunc->registers.push_back(MachineRegClass::xmm); // xmm13
	selection.mfunc->registers.push_back(MachineRegClass::xmm); // xmm14
	selection.mfunc->registers.push_back(MachineRegClass::xmm); // xmm15
	selection.mfunc->registers.resize((size_t)RegisterName::vregstart);

	for (IRValue* value : sfunc->args)
	{
		selection.newReg(value);
	}

	for (size_t i = 0; i < sfunc->basicBlocks.size(); i++)
	{
		IRBasicBlock* bb = sfunc->basicBlocks[i];
		MachineBasicBlock* mbb = sfunc->context->newMachineBasicBlock();
		selection.mfunc->basicBlocks.push_back(mbb);
		selection.bbMap[bb] = mbb;
	}

	selection.bb = selection.bbMap[sfunc->basicBlocks[0]];
	for (size_t i = 0; i < sfunc->stackVars.size(); i++)
	{
		selection.inst(sfunc->stackVars[i]);
	}

	for (size_t i = 0; i < sfunc->basicBlocks.size(); i++)
	{
		IRBasicBlock* bb = sfunc->basicBlocks[i];
		selection.bb = selection.bbMap[bb];
		bb->visit(&selection);
	}

	return selection.mfunc;
}

void MachineInstSelection::inst(IRInstLoad* node)
{
	static const MachineInstOpcode loadOps[] = { MachineInstOpcode::loadsd, MachineInstOpcode::loadss, MachineInstOpcode::load64, MachineInstOpcode::load32, MachineInstOpcode::load16, MachineInstOpcode::load8 };

	int dataSizeType = getDataSizeType(node->type);

	auto inst = context->newMachineInst();
	inst->opcode = loadOps[dataSizeType];
	inst->operands.push_back(newReg(node));
	pushValueOperand(inst, node->operand, dataSizeType);
	bb->code.push_back(inst);
}

void MachineInstSelection::inst(IRInstStore* node)
{
	static const MachineInstOpcode loadOps[] = { MachineInstOpcode::loadsd, MachineInstOpcode::loadss, MachineInstOpcode::load64, MachineInstOpcode::load32, MachineInstOpcode::load16, MachineInstOpcode::load8 };
	static const MachineInstOpcode storeOps[] = { MachineInstOpcode::storesd, MachineInstOpcode::storess, MachineInstOpcode::store64, MachineInstOpcode::store32, MachineInstOpcode::store16, MachineInstOpcode::store8 };

	int dataSizeType = getDataSizeType(node->operand2->type->getPointerElementType());

	bool isMemToMem = isConstantFP(node->operand1) || isGlobalVariable(node->operand1);
	if (isMemToMem)
	{
		auto srcreg = newTempReg(dataSizeType < 2 ? MachineRegClass::xmm : MachineRegClass::gp);

		auto loadinst = context->newMachineInst();
		loadinst->opcode = loadOps[dataSizeType];
		loadinst->operands.push_back(srcreg);
		pushValueOperand(loadinst, node->operand1, dataSizeType);
		bb->code.push_back(loadinst);

		auto storeinst = context->newMachineInst();
		storeinst->opcode = storeOps[dataSizeType];
		pushValueOperand(storeinst, node->operand2, dataSizeType);
		storeinst->operands.push_back(srcreg);
		bb->code.push_back(storeinst);
	}
	else
	{
		auto inst = context->newMachineInst();
		inst->opcode = storeOps[dataSizeType];
		pushValueOperand(inst, node->operand2, dataSizeType);
		pushValueOperand(inst, node->operand1, dataSizeType);
		bb->code.push_back(inst);
	}
}

void MachineInstSelection::inst(IRInstAdd* node)
{
	static const MachineInstOpcode ops[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::add64, MachineInstOpcode::add32, MachineInstOpcode::add16, MachineInstOpcode::add8 };
	simpleBinaryInst(node, ops);
}

void MachineInstSelection::inst(IRInstSub* node)
{
	static const MachineInstOpcode ops[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::sub64, MachineInstOpcode::sub32, MachineInstOpcode::sub16, MachineInstOpcode::sub8 };
	simpleBinaryInst(node, ops);
}

void MachineInstSelection::inst(IRInstFAdd* node)
{
	static const MachineInstOpcode ops[] = { MachineInstOpcode::addsd, MachineInstOpcode::addss, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop };
	simpleBinaryInst(node, ops);
}

void MachineInstSelection::inst(IRInstFSub* node)
{
	static const MachineInstOpcode ops[] = { MachineInstOpcode::subsd, MachineInstOpcode::subss, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop };
	simpleBinaryInst(node, ops);
}

void MachineInstSelection::inst(IRInstNot* node)
{
	static const MachineInstOpcode movOps[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::mov64, MachineInstOpcode::mov32, MachineInstOpcode::mov16, MachineInstOpcode::mov8 };
	static const MachineInstOpcode notOps[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::not64, MachineInstOpcode::not32, MachineInstOpcode::not16, MachineInstOpcode::not8 };

	int dataSizeType = getDataSizeType(node->type);
	auto dst = newReg(node);
	{
		auto inst = context->newMachineInst();
		inst->opcode = movOps[dataSizeType];
		inst->operands.push_back(dst);
		pushValueOperand(inst, node->operand, dataSizeType);
		bb->code.push_back(inst);
	}
	{
		auto inst = context->newMachineInst();
		inst->opcode = notOps[dataSizeType];
		inst->operands.push_back(dst);
		bb->code.push_back(inst);
	}
}

void MachineInstSelection::inst(IRInstNeg* node)
{
	static const MachineInstOpcode movOps[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::mov64, MachineInstOpcode::mov32, MachineInstOpcode::mov16, MachineInstOpcode::mov8 };
	static const MachineInstOpcode negOps[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::neg64, MachineInstOpcode::neg32, MachineInstOpcode::neg16, MachineInstOpcode::neg8 };

	int dataSizeType = getDataSizeType(node->type);
	auto dst = newReg(node);
	{
		auto inst = context->newMachineInst();
		inst->opcode = movOps[dataSizeType];
		inst->operands.push_back(dst);
		pushValueOperand(inst, node->operand, dataSizeType);
		bb->code.push_back(inst);
	}
	{
		auto inst = context->newMachineInst();
		inst->opcode = negOps[dataSizeType];
		inst->operands.push_back(dst);
		bb->code.push_back(inst);
	}
}

void MachineInstSelection::inst(IRInstFNeg* node)
{
	static const MachineInstOpcode xorOps[] = { MachineInstOpcode::xorpd, MachineInstOpcode::xorps, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop };
	static const MachineInstOpcode subOps[] = { MachineInstOpcode::subsd, MachineInstOpcode::subss, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop };

	int dataSizeType = getDataSizeType(node->type);
	auto dst = newReg(node);
	{
		auto inst = context->newMachineInst();
		inst->opcode = xorOps[dataSizeType];
		inst->operands.push_back(dst);
		inst->operands.push_back(dst);
		bb->code.push_back(inst);
	}
	{
		auto inst = context->newMachineInst();
		inst->opcode = subOps[dataSizeType];
		inst->operands.push_back(dst);
		pushValueOperand(inst, node->operand, dataSizeType);
		bb->code.push_back(inst);
	}
}

void MachineInstSelection::inst(IRInstMul* node)
{
	static const MachineInstOpcode ops[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::imul64, MachineInstOpcode::imul32, MachineInstOpcode::imul16, MachineInstOpcode::imul8 };
	simpleBinaryInst(node, ops);
}

void MachineInstSelection::inst(IRInstFMul* node)
{
	static const MachineInstOpcode ops[] = { MachineInstOpcode::mulsd, MachineInstOpcode::mulss, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop };
	simpleBinaryInst(node, ops);
}

void MachineInstSelection::inst(IRInstSDiv* node)
{
	static const MachineInstOpcode ops[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::idiv64, MachineInstOpcode::idiv32, MachineInstOpcode::idiv16, MachineInstOpcode::idiv8 };
	divBinaryInst(node, ops, false, false);
}

void MachineInstSelection::inst(IRInstUDiv* node)
{
	static const MachineInstOpcode ops[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::div64, MachineInstOpcode::div32, MachineInstOpcode::div16, MachineInstOpcode::div8 };
	divBinaryInst(node, ops, false, true);
}

void MachineInstSelection::inst(IRInstFDiv* node)
{
	static const MachineInstOpcode ops[] = { MachineInstOpcode::divsd, MachineInstOpcode::divss, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop };
	simpleBinaryInst(node, ops);
}

void MachineInstSelection::inst(IRInstSRem* node)
{
	static const MachineInstOpcode ops[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::idiv64, MachineInstOpcode::idiv32, MachineInstOpcode::idiv16, MachineInstOpcode::idiv8 };
	divBinaryInst(node, ops, true, false);
}

void MachineInstSelection::inst(IRInstURem* node)
{
	static const MachineInstOpcode ops[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::div64, MachineInstOpcode::div32, MachineInstOpcode::div16, MachineInstOpcode::div8 };
	divBinaryInst(node, ops, true, true);
}

void MachineInstSelection::inst(IRInstShl* node)
{
	static const MachineInstOpcode ops[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::shl64, MachineInstOpcode::shl32, MachineInstOpcode::shl16, MachineInstOpcode::shl8 };
	shiftBinaryInst(node, ops);
}

void MachineInstSelection::inst(IRInstLShr* node)
{
	static const MachineInstOpcode ops[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::shr64, MachineInstOpcode::shr32, MachineInstOpcode::shr16, MachineInstOpcode::shr8 };
	shiftBinaryInst(node, ops);
}

void MachineInstSelection::inst(IRInstAShr* node)
{
	static const MachineInstOpcode ops[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::sar64, MachineInstOpcode::sar32, MachineInstOpcode::sar16, MachineInstOpcode::sar8 };
	shiftBinaryInst(node, ops);
}

void MachineInstSelection::inst(IRInstICmpSLT* node)
{
	simpleCompareInst(node, MachineInstOpcode::setl);
}

void MachineInstSelection::inst(IRInstICmpULT* node)
{
	simpleCompareInst(node, MachineInstOpcode::setb);
}

void MachineInstSelection::inst(IRInstFCmpULT* node)
{
	simpleCompareInst(node, MachineInstOpcode::seta);
}

void MachineInstSelection::inst(IRInstICmpSGT* node)
{
	simpleCompareInst(node, MachineInstOpcode::setg);
}

void MachineInstSelection::inst(IRInstICmpUGT* node)
{
	simpleCompareInst(node, MachineInstOpcode::seta);
}

void MachineInstSelection::inst(IRInstFCmpUGT* node)
{
	simpleCompareInst(node, MachineInstOpcode::seta);
}

void MachineInstSelection::inst(IRInstICmpSLE* node)
{
	simpleCompareInst(node, MachineInstOpcode::setle);
}

void MachineInstSelection::inst(IRInstICmpULE* node)
{
	simpleCompareInst(node, MachineInstOpcode::setbe);
}

void MachineInstSelection::inst(IRInstFCmpULE* node)
{
	simpleCompareInst(node, MachineInstOpcode::setae);
}

void MachineInstSelection::inst(IRInstICmpSGE* node)
{
	simpleCompareInst(node, MachineInstOpcode::setge);
}

void MachineInstSelection::inst(IRInstICmpUGE* node)
{
	simpleCompareInst(node, MachineInstOpcode::setae);
}

void MachineInstSelection::inst(IRInstFCmpUGE* node)
{
	simpleCompareInst(node, MachineInstOpcode::setae);
}

void MachineInstSelection::inst(IRInstICmpEQ* node)
{
	simpleCompareInst(node, MachineInstOpcode::sete);
}

void MachineInstSelection::inst(IRInstFCmpUEQ* node)
{
	simpleCompareInst(node, MachineInstOpcode::sete);
}

void MachineInstSelection::inst(IRInstICmpNE* node)
{
	simpleCompareInst(node, MachineInstOpcode::setne);
}

void MachineInstSelection::inst(IRInstFCmpUNE* node)
{
	simpleCompareInst(node, MachineInstOpcode::setne);
}

void MachineInstSelection::inst(IRInstAnd* node)
{
	static const MachineInstOpcode ops[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::and64, MachineInstOpcode::and32, MachineInstOpcode::and16, MachineInstOpcode::and8 };
	simpleBinaryInst(node, ops);
}

void MachineInstSelection::inst(IRInstOr* node)
{
	static const MachineInstOpcode ops[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::or64, MachineInstOpcode::or32, MachineInstOpcode::or16, MachineInstOpcode::or8 };
	simpleBinaryInst(node, ops);
}

void MachineInstSelection::inst(IRInstXor* node)
{
	static const MachineInstOpcode ops[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::xor64, MachineInstOpcode::xor32, MachineInstOpcode::xor16, MachineInstOpcode::xor8 };
	simpleBinaryInst(node, ops);
}

void MachineInstSelection::inst(IRInstTrunc* node)
{
	static const MachineInstOpcode movOps[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::mov64, MachineInstOpcode::mov32, MachineInstOpcode::mov16, MachineInstOpcode::mov8 };

	int dataSizeType = getDataSizeType(node->type);

	auto dst = newReg(node);
	auto inst = context->newMachineInst();
	inst->opcode = movOps[dataSizeType];
	inst->operands.push_back(dst);
	pushValueOperand(inst, node->value, dataSizeType);
	bb->code.push_back(inst);
}

void MachineInstSelection::inst(IRInstZExt* node)
{
	static const MachineInstOpcode movsxOps[] =
	{
		MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop,
		MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop,
		MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::mov64, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop,
		MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::mov32, MachineInstOpcode::mov32, MachineInstOpcode::nop, MachineInstOpcode::nop,
		MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::movzx16_64, MachineInstOpcode::movzx16_32, MachineInstOpcode::mov16, MachineInstOpcode::nop,
		MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::movzx8_64, MachineInstOpcode::movzx8_32, MachineInstOpcode::movzx8_16, MachineInstOpcode::mov8,
	};

	int dstDataSizeType = getDataSizeType(node->type);
	int srcDataSizeType = getDataSizeType(node->value->type);
	auto dst = newReg(node);

	if (dstDataSizeType == 2 && srcDataSizeType == 3)
	{
		auto inst = context->newMachineInst();
		inst->opcode = MachineInstOpcode::xor64;
		inst->operands.push_back(dst);
		inst->operands.push_back(dst);
		bb->code.push_back(inst);
	}

	auto inst = context->newMachineInst();
	inst->opcode = movsxOps[dstDataSizeType * 6 + srcDataSizeType];
	inst->operands.push_back(dst);
	pushValueOperand(inst, node->value, srcDataSizeType);
	bb->code.push_back(inst);
}

void MachineInstSelection::inst(IRInstSExt* node)
{
	static const MachineInstOpcode movsxOps[] =
	{
		MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop,
		MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop,
		MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::mov64, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop,
		MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::movsx32_64, MachineInstOpcode::mov32, MachineInstOpcode::nop, MachineInstOpcode::nop,
		MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::movsx16_64, MachineInstOpcode::movsx16_32, MachineInstOpcode::mov16, MachineInstOpcode::nop,
		MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::movsx8_64, MachineInstOpcode::movsx8_32, MachineInstOpcode::movsx8_16, MachineInstOpcode::mov8,
	};

	int dstDataSizeType = getDataSizeType(node->type);
	int srcDataSizeType = getDataSizeType(node->value->type);
	auto dst = newReg(node);
	auto inst = context->newMachineInst();
	inst->opcode = movsxOps[dstDataSizeType * 6 + srcDataSizeType];
	inst->operands.push_back(dst);
	pushValueOperand(inst, node->value, srcDataSizeType);
	bb->code.push_back(inst);
}

void MachineInstSelection::inst(IRInstFPTrunc* node)
{
	auto dst = newReg(node);
	auto inst = context->newMachineInst();
	inst->opcode = isConstantFP(node->value) ? MachineInstOpcode::movss : MachineInstOpcode::cvtsd2ss;
	inst->operands.push_back(dst);
	pushValueOperand(inst, node->value, 1);
	bb->code.push_back(inst);
}

void MachineInstSelection::inst(IRInstFPExt* node)
{
	auto dst = newReg(node);
	auto inst = context->newMachineInst();
	inst->opcode = isConstantFP(node->value) ? MachineInstOpcode::movsd : MachineInstOpcode::cvtss2sd;
	inst->operands.push_back(dst);
	pushValueOperand(inst, node->value, 0);
	bb->code.push_back(inst);
}

void MachineInstSelection::inst(IRInstFPToUI* node)
{
	if (isConstantFP(node->value))
	{
		static const MachineInstOpcode movOps[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::mov64, MachineInstOpcode::mov32, MachineInstOpcode::mov16, MachineInstOpcode::mov8 };
		int dstDataSizeType = getDataSizeType(node->type);

		auto dst = newReg(node);
		auto inst = context->newMachineInst();
		inst->opcode = movOps[dstDataSizeType];
		inst->operands.push_back(dst);
		inst->operands.push_back(newImm((uint64_t)getConstantValueDouble(node->value)));
		bb->code.push_back(inst);
	}
	else
	{
		static const MachineInstOpcode cvtOps[] = { MachineInstOpcode::cvttsd2si, MachineInstOpcode::cvttsd2si, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop };
		int srcDataSizeType = getDataSizeType(node->value->type);

		auto dst = newReg(node);
		auto inst = context->newMachineInst();
		inst->opcode = cvtOps[srcDataSizeType];
		inst->operands.push_back(dst);
		pushValueOperand(inst, node->value, srcDataSizeType);
		bb->code.push_back(inst);
	}
}

void MachineInstSelection::inst(IRInstFPToSI* node)
{
	if (isConstantFP(node->value))
	{
		static const MachineInstOpcode movOps[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::mov64, MachineInstOpcode::mov32, MachineInstOpcode::mov16, MachineInstOpcode::mov8 };
		int dstDataSizeType = getDataSizeType(node->type);

		auto dst = newReg(node);
		auto inst = context->newMachineInst();
		inst->opcode = movOps[dstDataSizeType];
		inst->operands.push_back(dst);
		inst->operands.push_back(newImm((uint64_t)(int64_t)getConstantValueDouble(node->value)));
		bb->code.push_back(inst);
	}
	else
	{
		static const MachineInstOpcode cvtOps[] = { MachineInstOpcode::cvttsd2si, MachineInstOpcode::cvttss2si, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop };
		int srcDataSizeType = getDataSizeType(node->value->type);

		auto dst = newReg(node);
		auto inst = context->newMachineInst();
		inst->opcode = cvtOps[srcDataSizeType];
		inst->operands.push_back(dst);
		pushValueOperand(inst, node->value, srcDataSizeType);
		bb->code.push_back(inst);
	}
}

void MachineInstSelection::inst(IRInstUIToFP* node)
{
	static const MachineInstOpcode movOps[] = { MachineInstOpcode::movsd, MachineInstOpcode::movss, MachineInstOpcode::mov64, MachineInstOpcode::mov32, MachineInstOpcode::mov16, MachineInstOpcode::mov8 };
	static const MachineInstOpcode cvtOps[] = { MachineInstOpcode::cvtsi2sd, MachineInstOpcode::cvtsi2sd, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop };
	int dstDataSizeType = getDataSizeType(node->type);
	int srcDataSizeType = getDataSizeType(node->value->type);

	if (isConstantInt(node->value))
	{
		auto dst = newReg(node);
		auto inst = context->newMachineInst();
		inst->opcode = movOps[dstDataSizeType];
		inst->operands.push_back(dst);
		pushValueOperand(inst, node->value, dstDataSizeType);
		bb->code.push_back(inst);
	}
	else if (srcDataSizeType != 2) // zero extend required
	{
		auto tmp = newTempReg(MachineRegClass::gp);

		{
			auto inst = context->newMachineInst();
			inst->opcode = MachineInstOpcode::xor64;
			inst->operands.push_back(tmp);
			inst->operands.push_back(tmp);
			bb->code.push_back(inst);
		}

		{
			auto inst = context->newMachineInst();
			inst->opcode = movOps[srcDataSizeType];
			inst->operands.push_back(tmp);
			pushValueOperand(inst, node->value, srcDataSizeType);
			bb->code.push_back(inst);
		}

		{
			auto dst = newReg(node);
			auto inst = context->newMachineInst();
			inst->opcode = cvtOps[dstDataSizeType];
			inst->operands.push_back(dst);
			inst->operands.push_back(tmp);
			bb->code.push_back(inst);
		}
	}
	else
	{
		auto dst = newReg(node);
		auto inst = context->newMachineInst();
		inst->opcode = cvtOps[dstDataSizeType];
		inst->operands.push_back(dst);
		pushValueOperand(inst, node->value, dstDataSizeType);
		bb->code.push_back(inst);
	}
}

void MachineInstSelection::inst(IRInstSIToFP* node)
{
	static const MachineInstOpcode movOps[] = { MachineInstOpcode::movsd, MachineInstOpcode::movss, MachineInstOpcode::mov64, MachineInstOpcode::mov32, MachineInstOpcode::mov16, MachineInstOpcode::mov8 };
	static const MachineInstOpcode cvtOps[] = { MachineInstOpcode::cvtsi2sd, MachineInstOpcode::cvtsi2ss, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::nop };
	int dstDataSizeType = getDataSizeType(node->type);
	int srcDataSizeType = getDataSizeType(node->value->type);

	if (isConstantInt(node->value))
	{
		auto dst = newReg(node);
		auto inst = context->newMachineInst();
		inst->opcode = movOps[dstDataSizeType];
		inst->operands.push_back(dst);
		pushValueOperand(inst, node->value, dstDataSizeType);
		bb->code.push_back(inst);
	}
	else if (srcDataSizeType != dstDataSizeType + 2) // sign extend required
	{
		auto tmp = newTempReg(MachineRegClass::gp);

		static const int bits[] = { 64, 32, 64, 32, 16, 8 };
		int dstbits = bits[dstDataSizeType];
		int srcbits = bits[srcDataSizeType];
		int bitcount = dstbits - srcbits;
		auto count = newImm(bitcount);

		{
			auto inst = context->newMachineInst();
			inst->opcode = movOps[srcDataSizeType];
			inst->operands.push_back(tmp);
			pushValueOperand(inst, node->value, srcDataSizeType);
			bb->code.push_back(inst);
		}

		{
			auto inst = context->newMachineInst();
			inst->opcode = (dstDataSizeType == 0) ? MachineInstOpcode::shl64 : MachineInstOpcode::shl32;
			inst->operands.push_back(tmp);
			inst->operands.push_back(count);
			bb->code.push_back(inst);
		}

		{
			auto inst = context->newMachineInst();
			inst->opcode = (dstDataSizeType == 0) ? MachineInstOpcode::sar64 : MachineInstOpcode::sar32;
			inst->operands.push_back(tmp);
			inst->operands.push_back(count);
			bb->code.push_back(inst);
		}

		{
			auto dst = newReg(node);
			auto inst = context->newMachineInst();
			inst->opcode = cvtOps[dstDataSizeType];
			inst->operands.push_back(dst);
			inst->operands.push_back(tmp);
			bb->code.push_back(inst);
		}
	}
	else
	{
		auto dst = newReg(node);
		auto inst = context->newMachineInst();
		inst->opcode = cvtOps[dstDataSizeType];
		inst->operands.push_back(dst);
		pushValueOperand(inst, node->value, dstDataSizeType);
		bb->code.push_back(inst);
	}
}

void MachineInstSelection::inst(IRInstBitCast* node)
{
	if (isConstant(node->value))
	{
		throw std::runtime_error("BitCast on constant not allowed");
	}
	else
	{
		instRegister[node] = instRegister[node->value];
	}
}

void MachineInstSelection::inst(IRInstCall* node)
{
#ifdef WIN32
	callWin64(node);
#else
	callUnix64(node);
#endif
}

void MachineInstSelection::inst(IRInstGEP* node)
{
	int offset = 0;
	IRType* curType = node->ptr->type;
	for (IRValue* index : node->indices)
	{
		int cindex = (int)getConstantValueInt(index);

		if (isPointer(curType))
		{
			offset += curType->getPointerElementType()->getTypeAllocSize() * cindex;
			curType = curType->getPointerElementType();
		}
		else if (isStruct(curType))
		{
			IRStructType* stype = static_cast<IRStructType*>(curType);
			for (int i = 0; i < cindex; i++)
			{
				offset += (stype->elements[i]->getTypeAllocSize() + 7) / 8 * 8;
			}
			curType = stype->elements[cindex];
		}
		else
		{
			throw std::runtime_error("Invalid arguments to GEP instruction");
		}
	}

	auto dst = newReg(node);

	MachineOperand offsetoperand = newImm((int64_t)offset);

	auto inst = context->newMachineInst();
	inst->opcode = MachineInstOpcode::lea;
	inst->operands.push_back(dst);
	pushValueOperand(inst, node->ptr, getDataSizeType(node->ptr->type));
	inst->operands.push_back(offsetoperand);
	bb->code.push_back(inst);
}

void MachineInstSelection::inst(IRInstBr* node)
{
	auto inst = context->newMachineInst();
	inst->opcode = MachineInstOpcode::jmp;
	pushBBOperand(inst, node->bb);
	bb->code.push_back(inst);
}

void MachineInstSelection::inst(IRInstCondBr* node)
{
	if (isConstant(node->condition))
	{
		auto inst = context->newMachineInst();
		inst->opcode = MachineInstOpcode::jmp;
		if (getConstantValueInt(node->condition))
			pushBBOperand(inst, node->bb1);
		else
			pushBBOperand(inst, node->bb2);
		bb->code.push_back(inst);
	}
	else
	{
		{
			auto inst = context->newMachineInst();
			inst->opcode = MachineInstOpcode::cmp32;
			pushValueOperand(inst, node->condition, 3);
			inst->operands.push_back(newImm(1));
			bb->code.push_back(inst);
		}
		{
			auto inst = context->newMachineInst();
			inst->opcode = MachineInstOpcode::jz;
			pushBBOperand(inst, node->bb1);
			bb->code.push_back(inst);
		}
		{
			auto inst = context->newMachineInst();
			inst->opcode = MachineInstOpcode::jmp;
			pushBBOperand(inst, node->bb2);
			bb->code.push_back(inst);
		}
	}
}

void MachineInstSelection::inst(IRInstRet* node)
{
	static const MachineInstOpcode movOps[] = { MachineInstOpcode::movsd, MachineInstOpcode::movss, MachineInstOpcode::mov64, MachineInstOpcode::mov32, MachineInstOpcode::mov16, MachineInstOpcode::mov8 };

	{
		int dataSizeType = getDataSizeType(static_cast<IRFunctionType*>(sfunc->type)->returnType);
		auto inst = context->newMachineInst();
		inst->opcode = movOps[dataSizeType];
		inst->operands.push_back(newPhysReg(dataSizeType < 2 ? RegisterName::xmm0 : RegisterName::rax));
		pushValueOperand(inst, node->operand, dataSizeType);
		bb->code.push_back(inst);
	}
	{
		MachineOperand operand;
		operand.type = MachineOperandType::basicblock;
		operand.bb = mfunc->epilog;

		auto inst = context->newMachineInst();
		inst->opcode = MachineInstOpcode::jmp;
		inst->operands.push_back(operand);
		bb->code.push_back(inst);
	}
}

void MachineInstSelection::inst(IRInstRetVoid* node)
{
	MachineOperand operand;
	operand.type = MachineOperandType::basicblock;
	operand.bb = mfunc->epilog;

	auto inst = context->newMachineInst();
	inst->opcode = MachineInstOpcode::jmp;
	inst->operands.push_back(operand);
	bb->code.push_back(inst);
}

void MachineInstSelection::inst(IRInstAlloca* node)
{
	uint64_t size = node->type->getPointerElementType()->getTypeAllocSize() * getConstantValueInt(node->arraySize);
	size = (size + 15) / 16 * 16;

	// add rsp,size
	{
		MachineOperand src = newImm(size);
		MachineOperand dst = newPhysReg(RegisterName::rsp);

		auto inst = context->newMachineInst();
		inst->opcode = MachineInstOpcode::add64;
		inst->operands.push_back(dst);
		inst->operands.push_back(src);
		bb->code.push_back(inst);
	}

	MachineOperand dst = newReg(node);

	// mov vreg,rsp
	{
		MachineOperand src = newPhysReg(RegisterName::rsp);

		auto inst = context->newMachineInst();
		inst->opcode = MachineInstOpcode::mov64;
		inst->operands.push_back(dst);
		inst->operands.push_back(src);
		bb->code.push_back(inst);
	}

	// add vreg,maxCallArgsSize
	{
		MachineOperand src = newImm(mfunc->maxCallArgsSize); // To do: maxCallArgsSize must be calculated first for the entire function

		auto inst = context->newMachineInst();
		inst->opcode = MachineInstOpcode::add64;
		inst->operands.push_back(dst);
		inst->operands.push_back(src);
		bb->code.push_back(inst);
	}

	mfunc->dynamicStackAllocations = true;
	mfunc->registers[(int)RegisterName::rbp].cls = MachineRegClass::reserved;
}

int MachineInstSelection::getDataSizeType(IRType* type)
{
	if (isInt32(type) || isInt1(type))
		return 3;
	else if (isFloat(type))
		return 1;
	else if (isDouble(type))
		return 0;
	else if (isInt16(type))
		return 4;
	else if (isInt8(type))
		return 5;
	else // if (isInt64(type) || isPointer(type) || isFunction(type))
		return 2;
}

void MachineInstSelection::callWin64(IRInstCall* node)
{
	static const MachineInstOpcode loadOps[] = { MachineInstOpcode::loadsd, MachineInstOpcode::loadss, MachineInstOpcode::load64, MachineInstOpcode::load32, MachineInstOpcode::load16, MachineInstOpcode::load8 };
	static const MachineInstOpcode storeOps[] = { MachineInstOpcode::storesd, MachineInstOpcode::storess, MachineInstOpcode::store64, MachineInstOpcode::store32, MachineInstOpcode::store16, MachineInstOpcode::store8 };
	static const MachineInstOpcode movOps[] = { MachineInstOpcode::movsd, MachineInstOpcode::movss, MachineInstOpcode::mov64, MachineInstOpcode::mov32, MachineInstOpcode::mov16, MachineInstOpcode::mov8 };
	static const RegisterName regvars[] = { RegisterName::rcx, RegisterName::rdx, RegisterName::r8, RegisterName::r9 };

	int callArgsSize = 0;

	// Move arguments into place
	for (size_t i = 0; i < node->args.size(); i++)
	{
		IRValue* arg = node->args[i];

		bool isMemSrc = isConstantFP(arg) || isGlobalVariable(arg);
		int dataSizeType = getDataSizeType(arg->type);
		bool isXmm = dataSizeType < 2;

		// First four go to registers
		if (i < 4)
		{
			MachineOperand dst;
			dst.type = MachineOperandType::reg;
			if (isXmm)
				dst.registerIndex = (int)RegisterName::xmm0 + (int)i;
			else
				dst.registerIndex = (int)regvars[i];

			auto inst = context->newMachineInst();
			inst->opcode = isMemSrc ? loadOps[dataSizeType] : movOps[dataSizeType];
			inst->operands.push_back(dst);
			pushValueOperand(inst, arg, dataSizeType);
			bb->code.push_back(inst);
		}
		else
		{
			MachineOperand dst;
			dst.type = MachineOperandType::stackOffset;
			dst.stackOffset = (int)(i * 8);

			if (isMemSrc)
			{
				MachineOperand tmpreg;
				tmpreg.type = MachineOperandType::reg;
				tmpreg.registerIndex = isXmm ? (int)RegisterName::xmm0 : (int)RegisterName::rax;

				auto loadinst = context->newMachineInst();
				loadinst->opcode = loadOps[dataSizeType];
				loadinst->operands.push_back(tmpreg);
				pushValueOperand(loadinst, arg, dataSizeType);
				bb->code.push_back(loadinst);

				auto storeinst = context->newMachineInst();
				storeinst->opcode = storeOps[dataSizeType];
				storeinst->operands.push_back(dst);
				storeinst->operands.push_back(tmpreg);
				bb->code.push_back(storeinst);
			}
			else
			{
				auto inst = context->newMachineInst();
				inst->opcode = storeOps[dataSizeType];
				inst->operands.push_back(dst);
				pushValueOperand(inst, arg, dataSizeType);
				bb->code.push_back(inst);
			}
		}

		callArgsSize += 8;
	}

	mfunc->maxCallArgsSize = std::max(mfunc->maxCallArgsSize, callArgsSize);

	// Call the function
	{
		MachineOperand dst;

		IRFunction* func = dynamic_cast<IRFunction*>(node->func);
		if (func)
		{
			dst.type = MachineOperandType::func;
			dst.func = func;
		}
		else
		{
			dst.type = MachineOperandType::reg;
			dst.registerIndex = (int)RegisterName::rax;

			auto inst = context->newMachineInst();
			inst->opcode = MachineInstOpcode::mov64;
			inst->operands.push_back(dst);
			inst->operands.push_back(instRegister[node->func]);
			bb->code.push_back(inst);
		}

		auto inst = context->newMachineInst();
		inst->opcode = MachineInstOpcode::call;
		inst->operands.push_back(dst);
		bb->code.push_back(inst);
	}

	// Move return value to virtual register
	if (node->type != context->getVoidTy())
	{
		int dataSizeType = getDataSizeType(node->type);
		bool isXmm = dataSizeType < 2;

		MachineOperand src;
		src.type = MachineOperandType::reg;
		src.registerIndex = isXmm ? (int)RegisterName::xmm0 : (int)RegisterName::rax;

		auto dst = newReg(node);

		auto inst = context->newMachineInst();
		inst->opcode = movOps[dataSizeType];
		inst->operands.push_back(dst);
		inst->operands.push_back(src);
		bb->code.push_back(inst);
	}
}

void MachineInstSelection::callUnix64(IRInstCall* node)
{
	static const MachineInstOpcode loadOps[] = { MachineInstOpcode::loadsd, MachineInstOpcode::loadss, MachineInstOpcode::load64, MachineInstOpcode::load32, MachineInstOpcode::load16, MachineInstOpcode::load8 };
	static const MachineInstOpcode storeOps[] = { MachineInstOpcode::storesd, MachineInstOpcode::storess, MachineInstOpcode::store64, MachineInstOpcode::store32, MachineInstOpcode::store16, MachineInstOpcode::store8 };
	static const MachineInstOpcode movOps[] = { MachineInstOpcode::movsd, MachineInstOpcode::movss, MachineInstOpcode::mov64, MachineInstOpcode::mov32, MachineInstOpcode::mov16, MachineInstOpcode::mov8 };
	static const RegisterName regvars[] = { RegisterName::rdi, RegisterName::rsi, RegisterName::rdx, RegisterName::rcx, RegisterName::r8, RegisterName::r9 };

	const int numRegisterArgs = 6;
	const int numXmmRegisterArgs = 8;
	int nextRegisterArg = 0;
	int nextXmmRegisterArg = 0;
	int callArgsSize = 0;

	// Move arguments into place
	for (size_t i = 0; i < node->args.size(); i++)
	{
		IRValue* arg = node->args[i];

		bool isMemSrc = isConstantFP(arg) || isGlobalVariable(arg);
		int dataSizeType = getDataSizeType(arg->type);
		bool isXmm = dataSizeType < 2;
		bool isRegisterArg = (isXmm && nextXmmRegisterArg < numXmmRegisterArgs) || (!isXmm && nextRegisterArg < numRegisterArgs);

		if (isRegisterArg)
		{
			MachineOperand dst;
			dst.type = MachineOperandType::reg;
			if (isXmm)
				dst.registerIndex = (int)RegisterName::xmm0 + (nextXmmRegisterArg++);
			else
				dst.registerIndex = (int)regvars[nextRegisterArg++];

			auto inst = context->newMachineInst();
			inst->opcode = isMemSrc ? loadOps[dataSizeType] : movOps[dataSizeType];
			inst->operands.push_back(dst);
			pushValueOperand(inst, arg, dataSizeType);
			bb->code.push_back(inst);
		}
		else
		{
			MachineOperand dst;
			dst.type = MachineOperandType::stackOffset;
			dst.stackOffset = (int)(i * 8);

			if (isMemSrc)
			{
				MachineOperand tmpreg;
				tmpreg.type = MachineOperandType::reg;
				tmpreg.registerIndex = isXmm ? (int)RegisterName::xmm0 : (int)RegisterName::rax;

				auto loadinst = context->newMachineInst();
				loadinst->opcode = loadOps[dataSizeType];
				loadinst->operands.push_back(tmpreg);
				pushValueOperand(loadinst, arg, dataSizeType);
				bb->code.push_back(loadinst);

				auto storeinst = context->newMachineInst();
				storeinst->opcode = storeOps[dataSizeType];
				storeinst->operands.push_back(dst);
				storeinst->operands.push_back(tmpreg);
				bb->code.push_back(storeinst);
			}
			else
			{
				auto inst = context->newMachineInst();
				inst->opcode = storeOps[dataSizeType];
				inst->operands.push_back(dst);
				pushValueOperand(inst, arg, dataSizeType);
				bb->code.push_back(inst);
			}

			callArgsSize += 8;
		}
	}

	mfunc->maxCallArgsSize = std::max(mfunc->maxCallArgsSize, callArgsSize);

	// Call the function
	{
		MachineOperand dst;

		IRFunction* func = dynamic_cast<IRFunction*>(node->func);
		if (func)
		{
			dst.type = MachineOperandType::func;
			dst.func = func;
		}
		else
		{
			dst.type = MachineOperandType::reg;
			dst.registerIndex = (int)RegisterName::rax;

			auto inst = context->newMachineInst();
			inst->opcode = MachineInstOpcode::mov64;
			inst->operands.push_back(dst);
			inst->operands.push_back(instRegister[node->func]);
			bb->code.push_back(inst);
		}

		auto inst = context->newMachineInst();
		inst->opcode = MachineInstOpcode::call;
		inst->operands.push_back(dst);
		bb->code.push_back(inst);
	}

	// Move return value to virtual register
	if (node->type != context->getVoidTy())
	{
		int dataSizeType = getDataSizeType(node->type);
		bool isXmm = dataSizeType < 2;

		MachineOperand src;
		src.type = MachineOperandType::reg;
		src.registerIndex = isXmm ? (int)RegisterName::xmm0 : (int)RegisterName::rax;

		auto dst = newReg(node);

		auto inst = context->newMachineInst();
		inst->opcode = movOps[dataSizeType];
		inst->operands.push_back(dst);
		inst->operands.push_back(src);
		bb->code.push_back(inst);
	}
}

void MachineInstSelection::simpleCompareInst(IRInstBinary* node, MachineInstOpcode opSet)
{
	static const MachineInstOpcode movOps[] = { MachineInstOpcode::movsd, MachineInstOpcode::movss, MachineInstOpcode::mov64, MachineInstOpcode::mov32, MachineInstOpcode::mov16, MachineInstOpcode::mov8 };
	static const MachineInstOpcode cmpOps[] = { MachineInstOpcode::ucomisd, MachineInstOpcode::ucomiss, MachineInstOpcode::cmp64, MachineInstOpcode::cmp32, MachineInstOpcode::cmp16, MachineInstOpcode::cmp8 };

	int dataSizeType = getDataSizeType(node->type);

	// operand1 must be in a register:
	MachineOperand src1;
	if (isConstant(node->operand1))
	{
		src1 = newTempReg(dataSizeType < 2 ? MachineRegClass::xmm : MachineRegClass::gp);

		auto inst = context->newMachineInst();
		inst->opcode = movOps[dataSizeType];
		inst->operands.push_back(src1);
		pushValueOperand(inst, node->operand1, dataSizeType);

		bb->code.push_back(inst);
	}
	else
	{
		src1 = instRegister[node->operand1];
	}

	// Create 32 bit register, cleared to zero (move instruction below only moves 8 bits)
	auto dst = newReg(node);
	{
		auto inst = context->newMachineInst();
		inst->opcode = MachineInstOpcode::xor32;
		inst->operands.push_back(dst);
		inst->operands.push_back(dst);
		bb->code.push_back(inst);
	}

	// Perform comparison
	{
		auto inst = context->newMachineInst();
		inst->opcode = cmpOps[dataSizeType];
		inst->operands.push_back(src1);
		pushValueOperand(inst, node->operand2, dataSizeType);

		bb->code.push_back(inst);
	}

	// Move result flag to register
	{
		auto inst = context->newMachineInst();
		inst->opcode = opSet;
		inst->operands.push_back(dst);
		bb->code.push_back(inst);
	}
}

void MachineInstSelection::simpleBinaryInst(IRInstBinary* node, const MachineInstOpcode* binaryOps)
{
	static const MachineInstOpcode movOps[] = { MachineInstOpcode::movsd, MachineInstOpcode::movss, MachineInstOpcode::mov64, MachineInstOpcode::mov32, MachineInstOpcode::mov16, MachineInstOpcode::mov8 };

	int dataSizeType = getDataSizeType(node->type);

	auto dst = newReg(node);

	// Move operand1 to dest
	{
		auto inst = context->newMachineInst();
		inst->opcode = movOps[dataSizeType];
		inst->operands.push_back(dst);
		pushValueOperand(inst, node->operand1, dataSizeType);
		bb->code.push_back(inst);
	}

	// Apply operand2 to dest
	{
		auto inst = context->newMachineInst();
		inst->opcode = binaryOps[dataSizeType];
		inst->operands.push_back(dst);
		pushValueOperand(inst, node->operand2, dataSizeType);
		bb->code.push_back(inst);
	}
}

void MachineInstSelection::shiftBinaryInst(IRInstBinary* node, const MachineInstOpcode* binaryOps)
{
	static const MachineInstOpcode movOps[] = { MachineInstOpcode::movsd, MachineInstOpcode::movss, MachineInstOpcode::mov64, MachineInstOpcode::mov32, MachineInstOpcode::mov16, MachineInstOpcode::mov8 };

	int dataSizeType = getDataSizeType(node->type);

	auto dst = newReg(node);

	if (isConstant(node->operand2))
	{
		// Move operand1 to dest
		auto inst = context->newMachineInst();
		inst->opcode = movOps[dataSizeType];
		inst->operands.push_back(dst);
		pushValueOperand(inst, node->operand1, dataSizeType);
		bb->code.push_back(inst);

		// Apply operand2 to dest
		inst = context->newMachineInst();
		inst->opcode = binaryOps[dataSizeType];
		inst->operands.push_back(dst);
		pushValueOperand(inst, node->operand2, dataSizeType);
		bb->code.push_back(inst);
	}
	else
	{
		// Move operand2 to rcx
		auto inst = context->newMachineInst();
		inst->opcode = movOps[dataSizeType];
		inst->operands.push_back(newPhysReg(RegisterName::rcx));
		pushValueOperand(inst, node->operand2, dataSizeType);
		bb->code.push_back(inst);

		// Move operand1 to dest
		inst = context->newMachineInst();
		inst->opcode = movOps[dataSizeType];
		inst->operands.push_back(dst);
		pushValueOperand(inst, node->operand1, dataSizeType);
		bb->code.push_back(inst);

		// Apply rcx to dest
		inst = context->newMachineInst();
		inst->opcode = binaryOps[dataSizeType];
		inst->operands.push_back(dst);
		inst->operands.push_back(newPhysReg(RegisterName::rcx));
		bb->code.push_back(inst);
	}
}

void MachineInstSelection::divBinaryInst(IRInstBinary* node, const MachineInstOpcode* binaryOps, bool remainder, bool zeroext)
{
	static const MachineInstOpcode movOps[] = { MachineInstOpcode::movsd, MachineInstOpcode::movss, MachineInstOpcode::mov64, MachineInstOpcode::mov32, MachineInstOpcode::mov16, MachineInstOpcode::mov8 };
	static const MachineInstOpcode sarOps[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::sar64, MachineInstOpcode::sar32, MachineInstOpcode::sar16, MachineInstOpcode::sar8 };
	static const MachineInstOpcode xorOps[] = { MachineInstOpcode::nop, MachineInstOpcode::nop, MachineInstOpcode::xor64, MachineInstOpcode::xor32, MachineInstOpcode::xor16, MachineInstOpcode::xor8 };
	static const int shiftcount[] = { 0, 0, 63, 31, 15, 7 };

	int dataSizeType = getDataSizeType(node->type);

	auto dst = newReg(node);

	// Move operand1 to rdx:rax
	{
		auto inst = context->newMachineInst();
		inst->opcode = movOps[dataSizeType];
		inst->operands.push_back(newPhysReg(RegisterName::rax));
		pushValueOperand(inst, node->operand1, dataSizeType);
		bb->code.push_back(inst);

		if (dataSizeType != 5)
		{
			if (zeroext)
			{
				auto inst2 = context->newMachineInst();
				inst2->opcode = xorOps[dataSizeType];
				inst2->operands.push_back(newPhysReg(RegisterName::rdx));
				inst2->operands.push_back(newPhysReg(RegisterName::rdx));
				bb->code.push_back(inst2);
			}
			else
			{
				auto inst2 = context->newMachineInst();
				inst2->opcode = movOps[dataSizeType];
				inst2->operands.push_back(newPhysReg(RegisterName::rdx));
				inst2->operands.push_back(newPhysReg(RegisterName::rax));
				bb->code.push_back(inst2);

				auto inst3 = context->newMachineInst();
				inst3->opcode = sarOps[dataSizeType];
				inst3->operands.push_back(newPhysReg(RegisterName::rdx));
				inst3->operands.push_back(newImm(shiftcount[dataSizeType]));
				bb->code.push_back(inst3);
			}
		}
		else
		{
			auto inst2 = context->newMachineInst();
			inst2->opcode = zeroext ? MachineInstOpcode::movzx8_16 : MachineInstOpcode::movsx8_16;
			inst2->operands.push_back(newPhysReg(RegisterName::rax));
			inst2->operands.push_back(newPhysReg(RegisterName::rax));
			bb->code.push_back(inst2);
		}
	}

	// Divide
	if (isConstant(node->operand2) || isGlobalVariable(node->operand2))
	{
		auto inst = context->newMachineInst();
		inst->opcode = movOps[dataSizeType];
		inst->operands.push_back(dst);
		pushValueOperand(inst, node->operand2, dataSizeType);
		bb->code.push_back(inst);

		inst = context->newMachineInst();
		inst->opcode = binaryOps[dataSizeType];
		inst->operands.push_back(dst);
		bb->code.push_back(inst);
	}
	else
	{
		auto inst = context->newMachineInst();
		inst->opcode = binaryOps[dataSizeType];
		pushValueOperand(inst, node->operand2, dataSizeType);
		bb->code.push_back(inst);
	}

	// Move rax to dest
	{
		auto inst = context->newMachineInst();
		inst->opcode = movOps[dataSizeType];
		inst->operands.push_back(dst);
		inst->operands.push_back(newPhysReg(remainder ? RegisterName::rdx : RegisterName::rax));
		bb->code.push_back(inst);
	}
}

void MachineInstSelection::pushValueOperand(MachineInst* inst, IRValue* operand, int dataSizeType)
{
	if (isConstantFP(operand))
	{
		if (dataSizeType == 0)
			inst->operands.push_back(newConstant(getConstantValueDouble(operand)));
		else // if (dataSizeType == 1)
			inst->operands.push_back(newConstant(getConstantValueFloat(operand)));
	}
	if (isConstantInt(operand))
	{
		inst->operands.push_back(newImm(getConstantValueInt(operand)));
	}
	else if (isGlobalVariable(operand))
	{
		MachineOperand mcoperand;
		mcoperand.type = MachineOperandType::global;
		mcoperand.global = static_cast<IRGlobalVariable*>(operand);
		inst->operands.push_back(mcoperand);
	}
	else
	{
		inst->operands.push_back(instRegister[operand]);
	}
}

void MachineInstSelection::pushBBOperand(MachineInst* inst, IRBasicBlock* bb)
{
	MachineOperand operand;
	operand.type = MachineOperandType::basicblock;
	operand.bb = bbMap[bb];
	inst->operands.push_back(operand);
}

MachineOperand MachineInstSelection::newReg(IRValue* node)
{
	MachineOperand operand;
	operand.type = MachineOperandType::reg;
	operand.registerIndex = createVirtReg(getDataSizeType(node->type) < 2 ? MachineRegClass::xmm : MachineRegClass::gp);
	instRegister[node] = operand;
	return operand;
}

MachineOperand MachineInstSelection::newPhysReg(RegisterName name)
{
	MachineOperand operand;
	operand.type = MachineOperandType::reg;
	operand.registerIndex = (int)name;
	return operand;
}

MachineOperand MachineInstSelection::newTempReg(MachineRegClass cls)
{
	MachineOperand operand;
	operand.type = MachineOperandType::reg;
	operand.registerIndex = createVirtReg(cls);
	return operand;
}

int MachineInstSelection::createVirtReg(MachineRegClass cls)
{
	int registerIndex = (int)mfunc->registers.size();
	mfunc->registers.push_back(cls);
	return registerIndex;
}

MachineOperand MachineInstSelection::newConstant(uint64_t address)
{
	return newConstant(&address, sizeof(uint64_t));
}

MachineOperand MachineInstSelection::newConstant(float value)
{
	return newConstant(&value, sizeof(float));
}

MachineOperand MachineInstSelection::newConstant(double value)
{
	return newConstant(&value, sizeof(double));
}

MachineOperand MachineInstSelection::newConstant(const void* data, int size)
{
	int index = (int)mfunc->constants.size();
	mfunc->constants.push_back({ data, size });

	MachineOperand operand;
	operand.type = MachineOperandType::constant;
	operand.constantIndex = index;
	return operand;
}

MachineOperand MachineInstSelection::newImm(uint64_t imm)
{
	MachineOperand operand;
	operand.type = MachineOperandType::imm;
	operand.immvalue = imm;
	return operand;
}

uint64_t MachineInstSelection::getConstantValueInt(IRValue* value)
{
	if (isConstantInt(value))
		return static_cast<IRConstantInt*>(value)->value;
	else
		return 0;
}

float MachineInstSelection::getConstantValueFloat(IRValue* value)
{
	if (isConstantFP(value))
		return static_cast<float>(static_cast<IRConstantFP*>(value)->value);
	else
		return 0.0f;
}

double MachineInstSelection::getConstantValueDouble(IRValue* value)
{
	if (isConstantFP(value))
		return static_cast<IRConstantFP*>(value)->value;
	else
		return 0.0;
}
