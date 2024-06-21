
#include "MachineInstSelectionX64.h"
#include <algorithm>

MachineFunction* MachineInstSelectionX64::codegen(IRFunction* sfunc)
{
	sfunc->sortBasicBlocks();

	MachineInstSelectionX64 selection(sfunc);
	selection.mfunc = sfunc->context->newMachineFunction(sfunc->name);
	selection.mfunc->fileInfo = sfunc->fileInfo;
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
	selection.mfunc->registers.resize((size_t)RegisterNameX64::vregstart);

	selection.findMaxCallArgsSize();

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
	selection.irbb = sfunc->basicBlocks[0];
	for (size_t i = 0; i < sfunc->stackVars.size(); i++)
	{
		IRInstAlloca* node = sfunc->stackVars[i];

		uint64_t size = node->type->getPointerElementType()->getTypeAllocSize() * selection.getConstantValueInt(node->arraySize);
		size = (size + 15) / 16 * 16;

		MachineOperand dst = selection.newReg(node);
		selection.mfunc->stackvars.push_back({ dst.registerIndex, size, node->name });
	}

	for (size_t i = 0; i < sfunc->basicBlocks.size(); i++)
	{
		IRBasicBlock* bb = sfunc->basicBlocks[i];
		for (IRInst* inst : bb->code)
		{
			IRInstPhi* phi = dynamic_cast<IRInstPhi*>(inst);
			if (!phi)
				break;
			selection.newReg(phi);
		}
	}

	for (size_t i = 0; i < sfunc->basicBlocks.size(); i++)
	{
		IRBasicBlock* bb = sfunc->basicBlocks[i];
		selection.bb = selection.bbMap[bb];
		selection.irbb = bb;
		for (IRInst* node : bb->code)
		{
			selection.debugInfoInst = node;
			node->visit(&selection);
		}
	}

	return selection.mfunc;
}

void MachineInstSelectionX64::inst(IRInstLoad* node)
{
	static const MachineInstOpcodeX64 loadOps[] = { MachineInstOpcodeX64::loadsd, MachineInstOpcodeX64::loadss, MachineInstOpcodeX64::load64, MachineInstOpcodeX64::load32, MachineInstOpcodeX64::load16, MachineInstOpcodeX64::load8 };
	static const MachineInstOpcodeX64 movOps[] = { MachineInstOpcodeX64::movsd, MachineInstOpcodeX64::movss, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::mov8 };

	int dataSizeType = getDataSizeType(node->type);

	if (isConstantInt(node->operand))
	{
		auto srcreg = newTempReg(MachineRegClass::gp);
		emitInst(MachineInstOpcodeX64::mov64, srcreg, node->operand, 2);
		emitInst(loadOps[dataSizeType], newReg(node), srcreg);
	}
	else
	{
		emitInst(loadOps[dataSizeType], newReg(node), node->operand, dataSizeType);
	}
}

void MachineInstSelectionX64::inst(IRInstStore* node)
{
	static const MachineInstOpcodeX64 loadOps[] = { MachineInstOpcodeX64::loadsd, MachineInstOpcodeX64::loadss, MachineInstOpcodeX64::load64, MachineInstOpcodeX64::load32, MachineInstOpcodeX64::load16, MachineInstOpcodeX64::load8 };
	static const MachineInstOpcodeX64 movOps[] = { MachineInstOpcodeX64::movsd, MachineInstOpcodeX64::movss, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::mov8 };
	static const MachineInstOpcodeX64 storeOps[] = { MachineInstOpcodeX64::storesd, MachineInstOpcodeX64::storess, MachineInstOpcodeX64::store64, MachineInstOpcodeX64::store32, MachineInstOpcodeX64::store16, MachineInstOpcodeX64::store8 };

	MachineOperand dstreg;
	if (isConstantInt(node->operand2))
	{
		dstreg = newTempReg(MachineRegClass::gp);
		emitInst(MachineInstOpcodeX64::mov64, dstreg, node->operand2, 2);
	}
	else
	{
		dstreg = instRegister[node->operand2];
	}

	int dataSizeType = getDataSizeType(node->operand2->type->getPointerElementType());

	bool needs64BitImm = (dataSizeType == 2) && isConstantInt(node->operand1) && (getConstantValueInt(node->operand1) < -0x7fffffff || getConstantValueInt(node->operand1) > 0x7fffffff);
	bool isMemToMem = isConstantFP(node->operand1) || isGlobalVariable(node->operand1);
	if (isMemToMem)
	{
		auto srcreg = newTempReg(dataSizeType < 2 ? MachineRegClass::xmm : MachineRegClass::gp);
		emitInst(loadOps[dataSizeType], srcreg, node->operand1, dataSizeType);
		emitInst(storeOps[dataSizeType], node->operand2, dataSizeType, srcreg);
	}
	else if (needs64BitImm)
	{
		auto srcreg = newTempReg(MachineRegClass::gp);
		emitInst(movOps[dataSizeType], srcreg, node->operand1, dataSizeType);
		emitInst(storeOps[dataSizeType], dstreg, srcreg);
	}
	else
	{
		emitInst(storeOps[dataSizeType], dstreg, node->operand1, dataSizeType);
	}
}

void MachineInstSelectionX64::inst(IRInstAdd* node)
{
	static const MachineInstOpcodeX64 ops[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::add64, MachineInstOpcodeX64::add32, MachineInstOpcodeX64::add16, MachineInstOpcodeX64::add8 };
	simpleBinaryInst(node, ops);
}

void MachineInstSelectionX64::inst(IRInstSub* node)
{
	static const MachineInstOpcodeX64 ops[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::sub64, MachineInstOpcodeX64::sub32, MachineInstOpcodeX64::sub16, MachineInstOpcodeX64::sub8 };
	simpleBinaryInst(node, ops);
}

void MachineInstSelectionX64::inst(IRInstFAdd* node)
{
	static const MachineInstOpcodeX64 ops[] = { MachineInstOpcodeX64::addsd, MachineInstOpcodeX64::addss, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop };
	simpleBinaryInst(node, ops);
}

void MachineInstSelectionX64::inst(IRInstFSub* node)
{
	static const MachineInstOpcodeX64 ops[] = { MachineInstOpcodeX64::subsd, MachineInstOpcodeX64::subss, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop };
	simpleBinaryInst(node, ops);
}

void MachineInstSelectionX64::inst(IRInstNot* node)
{
	static const MachineInstOpcodeX64 movOps[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::mov8 };
	static const MachineInstOpcodeX64 notOps[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::not64, MachineInstOpcodeX64::not32, MachineInstOpcodeX64::not16, MachineInstOpcodeX64::not8 };

	int dataSizeType = getDataSizeType(node->type);
	auto dst = newReg(node);
	emitInst(movOps[dataSizeType], dst, node->operand, dataSizeType);
	emitInst(notOps[dataSizeType], dst);
}

void MachineInstSelectionX64::inst(IRInstNeg* node)
{
	static const MachineInstOpcodeX64 movOps[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::mov8 };
	static const MachineInstOpcodeX64 negOps[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::neg64, MachineInstOpcodeX64::neg32, MachineInstOpcodeX64::neg16, MachineInstOpcodeX64::neg8 };

	int dataSizeType = getDataSizeType(node->type);
	auto dst = newReg(node);
	emitInst(movOps[dataSizeType], dst, node->operand, dataSizeType);
	emitInst(negOps[dataSizeType], dst);
}

void MachineInstSelectionX64::inst(IRInstFNeg* node)
{
	static const MachineInstOpcodeX64 xorOps[] = { MachineInstOpcodeX64::xorpd, MachineInstOpcodeX64::xorps, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop };
	static const MachineInstOpcodeX64 subOps[] = { MachineInstOpcodeX64::subsd, MachineInstOpcodeX64::subss, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop };

	int dataSizeType = getDataSizeType(node->type);
	auto dst = newReg(node);
	emitInst(xorOps[dataSizeType], dst, dst);
	emitInst(subOps[dataSizeType], dst, node->operand, dataSizeType);
}

void MachineInstSelectionX64::inst(IRInstMul* node)
{
	static const MachineInstOpcodeX64 ops[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::imul64, MachineInstOpcodeX64::imul32, MachineInstOpcodeX64::imul16, MachineInstOpcodeX64::imul8 };
	static const MachineInstOpcodeX64 movOps[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::mov8 };

	int dataSizeType = getDataSizeType(node->type);
	if (dataSizeType == 5) // 8 bit multiply can only happen in ax register
	{
		auto dst = newReg(node);
		emitInst(MachineInstOpcodeX64::mov8, newPhysReg(RegisterNameX64::rax), node->operand1, dataSizeType);
		emitInst(MachineInstOpcodeX64::imul8, node->operand2, dataSizeType);
		emitInst(MachineInstOpcodeX64::mov8, dst, newPhysReg(RegisterNameX64::rax));
	}
	else
	{
		simpleBinaryInst(node, ops);
	}
}

void MachineInstSelectionX64::inst(IRInstFMul* node)
{
	static const MachineInstOpcodeX64 ops[] = { MachineInstOpcodeX64::mulsd, MachineInstOpcodeX64::mulss, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop };
	simpleBinaryInst(node, ops);
}

void MachineInstSelectionX64::inst(IRInstSDiv* node)
{
	static const MachineInstOpcodeX64 ops[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::idiv64, MachineInstOpcodeX64::idiv32, MachineInstOpcodeX64::idiv16, MachineInstOpcodeX64::idiv8 };
	divBinaryInst(node, ops, false, false);
}

void MachineInstSelectionX64::inst(IRInstUDiv* node)
{
	static const MachineInstOpcodeX64 ops[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::div64, MachineInstOpcodeX64::div32, MachineInstOpcodeX64::div16, MachineInstOpcodeX64::div8 };
	divBinaryInst(node, ops, false, true);
}

void MachineInstSelectionX64::inst(IRInstFDiv* node)
{
	static const MachineInstOpcodeX64 ops[] = { MachineInstOpcodeX64::divsd, MachineInstOpcodeX64::divss, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop };
	simpleBinaryInst(node, ops);
}

void MachineInstSelectionX64::inst(IRInstSRem* node)
{
	static const MachineInstOpcodeX64 ops[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::idiv64, MachineInstOpcodeX64::idiv32, MachineInstOpcodeX64::idiv16, MachineInstOpcodeX64::idiv8 };
	divBinaryInst(node, ops, true, false);
}

void MachineInstSelectionX64::inst(IRInstURem* node)
{
	static const MachineInstOpcodeX64 ops[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::div64, MachineInstOpcodeX64::div32, MachineInstOpcodeX64::div16, MachineInstOpcodeX64::div8 };
	divBinaryInst(node, ops, true, true);
}

void MachineInstSelectionX64::inst(IRInstShl* node)
{
	static const MachineInstOpcodeX64 ops[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::shl64, MachineInstOpcodeX64::shl32, MachineInstOpcodeX64::shl16, MachineInstOpcodeX64::shl8 };
	shiftBinaryInst(node, ops);
}

void MachineInstSelectionX64::inst(IRInstLShr* node)
{
	static const MachineInstOpcodeX64 ops[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::shr64, MachineInstOpcodeX64::shr32, MachineInstOpcodeX64::shr16, MachineInstOpcodeX64::shr8 };
	shiftBinaryInst(node, ops);
}

void MachineInstSelectionX64::inst(IRInstAShr* node)
{
	static const MachineInstOpcodeX64 ops[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::sar64, MachineInstOpcodeX64::sar32, MachineInstOpcodeX64::sar16, MachineInstOpcodeX64::sar8 };
	shiftBinaryInst(node, ops);
}

void MachineInstSelectionX64::inst(IRInstICmpSLT* node)
{
	simpleCompareInst(node, MachineInstOpcodeX64::setl);
}

void MachineInstSelectionX64::inst(IRInstICmpULT* node)
{
	simpleCompareInst(node, MachineInstOpcodeX64::setb);
}

void MachineInstSelectionX64::inst(IRInstFCmpULT* node)
{
	simpleCompareInst(node, MachineInstOpcodeX64::setb, MachineInstOpcodeX64::setnp, MachineInstOpcodeX64::and8);
}

void MachineInstSelectionX64::inst(IRInstICmpSGT* node)
{
	simpleCompareInst(node, MachineInstOpcodeX64::setg);
}

void MachineInstSelectionX64::inst(IRInstICmpUGT* node)
{
	simpleCompareInst(node, MachineInstOpcodeX64::seta);
}

void MachineInstSelectionX64::inst(IRInstFCmpUGT* node)
{
	simpleCompareInst(node, MachineInstOpcodeX64::seta, MachineInstOpcodeX64::setnp, MachineInstOpcodeX64::and8);
}

void MachineInstSelectionX64::inst(IRInstICmpSLE* node)
{
	simpleCompareInst(node, MachineInstOpcodeX64::setle);
}

void MachineInstSelectionX64::inst(IRInstICmpULE* node)
{
	simpleCompareInst(node, MachineInstOpcodeX64::setbe);
}

void MachineInstSelectionX64::inst(IRInstFCmpULE* node)
{
	simpleCompareInst(node, MachineInstOpcodeX64::setbe, MachineInstOpcodeX64::setnp, MachineInstOpcodeX64::and8);
}

void MachineInstSelectionX64::inst(IRInstICmpSGE* node)
{
	simpleCompareInst(node, MachineInstOpcodeX64::setge);
}

void MachineInstSelectionX64::inst(IRInstICmpUGE* node)
{
	simpleCompareInst(node, MachineInstOpcodeX64::setae);
}

void MachineInstSelectionX64::inst(IRInstFCmpUGE* node)
{
	simpleCompareInst(node, MachineInstOpcodeX64::setae, MachineInstOpcodeX64::setnp, MachineInstOpcodeX64::and8);
}

void MachineInstSelectionX64::inst(IRInstICmpEQ* node)
{
	simpleCompareInst(node, MachineInstOpcodeX64::sete);
}

void MachineInstSelectionX64::inst(IRInstFCmpUEQ* node)
{
	simpleCompareInst(node, MachineInstOpcodeX64::sete, MachineInstOpcodeX64::setnp, MachineInstOpcodeX64::and8);
}

void MachineInstSelectionX64::inst(IRInstICmpNE* node)
{
	simpleCompareInst(node, MachineInstOpcodeX64::setne);
}

void MachineInstSelectionX64::inst(IRInstFCmpUNE* node)
{
	simpleCompareInst(node, MachineInstOpcodeX64::setne, MachineInstOpcodeX64::setp, MachineInstOpcodeX64::or8);
}

void MachineInstSelectionX64::inst(IRInstAnd* node)
{
	static const MachineInstOpcodeX64 ops[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::and64, MachineInstOpcodeX64::and32, MachineInstOpcodeX64::and16, MachineInstOpcodeX64::and8 };
	simpleBinaryInst(node, ops);
}

void MachineInstSelectionX64::inst(IRInstOr* node)
{
	static const MachineInstOpcodeX64 ops[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::or64, MachineInstOpcodeX64::or32, MachineInstOpcodeX64::or16, MachineInstOpcodeX64::or8 };
	simpleBinaryInst(node, ops);
}

void MachineInstSelectionX64::inst(IRInstXor* node)
{
	static const MachineInstOpcodeX64 ops[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::xor64, MachineInstOpcodeX64::xor32, MachineInstOpcodeX64::xor16, MachineInstOpcodeX64::xor8 };
	simpleBinaryInst(node, ops);
}

void MachineInstSelectionX64::inst(IRInstTrunc* node)
{
	static const MachineInstOpcodeX64 movOps[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::mov8 };

	int dataSizeType = getDataSizeType(node->type);

	auto dst = newReg(node);
	emitInst(movOps[dataSizeType], dst, node->value, dataSizeType);
}

void MachineInstSelectionX64::inst(IRInstZExt* node)
{
	static const MachineInstOpcodeX64 movsxOps[] =
	{
		MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop,
		MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop,
		MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop,
		MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop,
		MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::movzx16_64, MachineInstOpcodeX64::movzx16_32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::nop,
		MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::movzx8_64, MachineInstOpcodeX64::movzx8_32, MachineInstOpcodeX64::movzx8_16, MachineInstOpcodeX64::mov8,
	};

	int dstDataSizeType = getDataSizeType(node->type);
	int srcDataSizeType = getDataSizeType(node->value->type);
	auto dst = newReg(node);

	if (dstDataSizeType == 2 && srcDataSizeType == 3)
	{
		emitInst(MachineInstOpcodeX64::xor64, dst, dst);
	}

	emitInst(movsxOps[srcDataSizeType * 6 + dstDataSizeType], dst, node->value, srcDataSizeType);
}

void MachineInstSelectionX64::inst(IRInstSExt* node)
{
	static const MachineInstOpcodeX64 movsxOps[] =
	{
		MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop,
		MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop,
		MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop,
		MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::movsx32_64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop,
		MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::movsx16_64, MachineInstOpcodeX64::movsx16_32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::nop,
		MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::movsx8_64, MachineInstOpcodeX64::movsx8_32, MachineInstOpcodeX64::movsx8_16, MachineInstOpcodeX64::mov8,
	};

	int dstDataSizeType = getDataSizeType(node->type);
	int srcDataSizeType = getDataSizeType(node->value->type);
	auto dst = newReg(node);
	emitInst(movsxOps[srcDataSizeType * 6 + dstDataSizeType], dst, node->value, srcDataSizeType);
}

void MachineInstSelectionX64::inst(IRInstFPTrunc* node)
{
	auto dst = newReg(node);
	emitInst(isConstantFP(node->value) ? MachineInstOpcodeX64::movss : MachineInstOpcodeX64::cvtsd2ss, dst, node->value, 1);
}

void MachineInstSelectionX64::inst(IRInstFPExt* node)
{
	auto dst = newReg(node);
	emitInst(isConstantFP(node->value) ? MachineInstOpcodeX64::movsd : MachineInstOpcodeX64::cvtss2sd, dst, node->value, 0);
}

void MachineInstSelectionX64::inst(IRInstFPToUI* node)
{
	if (isConstantFP(node->value))
	{
		static const MachineInstOpcodeX64 movOps[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::mov8 };
		int dstDataSizeType = getDataSizeType(node->type);

		auto dst = newReg(node);
		emitInst(movOps[dstDataSizeType], dst, newImm((uint64_t)getConstantValueDouble(node->value)));
	}
	else
	{
		static const MachineInstOpcodeX64 cvtOps[] = { MachineInstOpcodeX64::cvttsd2si, MachineInstOpcodeX64::cvttsd2si, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop };
		int srcDataSizeType = getDataSizeType(node->value->type);

		auto dst = newReg(node);
		emitInst(cvtOps[srcDataSizeType], dst, node->value, srcDataSizeType);
	}
}

void MachineInstSelectionX64::inst(IRInstFPToSI* node)
{
	if (isConstantFP(node->value))
	{
		static const MachineInstOpcodeX64 movOps[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::mov8 };
		int dstDataSizeType = getDataSizeType(node->type);

		auto dst = newReg(node);
		emitInst(movOps[dstDataSizeType], dst, newImm((uint64_t)(int64_t)getConstantValueDouble(node->value)));
	}
	else
	{
		static const MachineInstOpcodeX64 cvtOps[] = { MachineInstOpcodeX64::cvttsd2si, MachineInstOpcodeX64::cvttss2si, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop };
		int srcDataSizeType = getDataSizeType(node->value->type);

		auto dst = newReg(node);
		emitInst(cvtOps[srcDataSizeType], dst, node->value, srcDataSizeType);
	}
}

void MachineInstSelectionX64::inst(IRInstUIToFP* node)
{
	static const MachineInstOpcodeX64 movOps[] = { MachineInstOpcodeX64::movsd, MachineInstOpcodeX64::movss, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::mov8 };
	static const MachineInstOpcodeX64 cvtOps[] = { MachineInstOpcodeX64::cvtsi2sd, MachineInstOpcodeX64::cvtsi2sd, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop };
	int dstDataSizeType = getDataSizeType(node->type);
	int srcDataSizeType = getDataSizeType(node->value->type);

	if (isConstantInt(node->value))
	{
		auto dst = newReg(node);
		if (dstDataSizeType == 0)
			emitInst(movOps[dstDataSizeType], dst, newConstant((double)getConstantValueInt(node->value)));
		else // if (dstDataSizeType == 1)
			emitInst(movOps[dstDataSizeType], dst, newConstant((float)getConstantValueInt(node->value)));
	}
	else if (srcDataSizeType != 2) // zero extend required
	{
		auto tmp = newTempReg(MachineRegClass::gp);
		auto dst = newReg(node);

		emitInst(MachineInstOpcodeX64::xor64, tmp, tmp);
		emitInst(movOps[srcDataSizeType], tmp, node->value, srcDataSizeType);
		emitInst(cvtOps[dstDataSizeType], dst, tmp);
	}
	else
	{
		auto dst = newReg(node);
		emitInst(cvtOps[dstDataSizeType], dst, node->value, dstDataSizeType);
	}
}

void MachineInstSelectionX64::inst(IRInstSIToFP* node)
{
	static const MachineInstOpcodeX64 movOps[] = { MachineInstOpcodeX64::movsd, MachineInstOpcodeX64::movss, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::mov8 };
	static const MachineInstOpcodeX64 cvtOps[] = { MachineInstOpcodeX64::cvtsi2sd, MachineInstOpcodeX64::cvtsi2ss, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop };
	int dstDataSizeType = getDataSizeType(node->type);
	int srcDataSizeType = getDataSizeType(node->value->type);

	if (isConstantInt(node->value))
	{
		auto dst = newReg(node);
		if (dstDataSizeType == 0)
			emitInst(movOps[dstDataSizeType], dst, newConstant((double)(int64_t)getConstantValueInt(node->value)));
		else // if (dstDataSizeType == 1)
			emitInst(movOps[dstDataSizeType], dst, newConstant((float)(int64_t)getConstantValueInt(node->value)));
	}
	else if (srcDataSizeType != dstDataSizeType + 2) // sign extend required
	{
		static const int bits[] = { 64, 32, 64, 32, 16, 8 };
		int dstbits = bits[dstDataSizeType];
		int srcbits = bits[srcDataSizeType];
		int bitcount = dstbits - srcbits;
		auto count = newImm(bitcount);
		auto tmp = newTempReg(MachineRegClass::gp);
		auto dst = newReg(node);

		emitInst(movOps[srcDataSizeType], tmp, node->value, srcDataSizeType);
		emitInst((dstDataSizeType == 0) ? MachineInstOpcodeX64::shl64 : MachineInstOpcodeX64::shl32, tmp, count);
		emitInst((dstDataSizeType == 0) ? MachineInstOpcodeX64::sar64 : MachineInstOpcodeX64::sar32, tmp, count);
		emitInst(cvtOps[dstDataSizeType], dst, tmp);
	}
	else
	{
		auto dst = newReg(node);
		emitInst(cvtOps[dstDataSizeType], dst, node->value, dstDataSizeType);
	}
}

void MachineInstSelectionX64::inst(IRInstBitCast* node)
{
	if (isConstant(node->value))
	{
		static const MachineInstOpcodeX64 movOps[] = { MachineInstOpcodeX64::movsd, MachineInstOpcodeX64::movss, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::mov8 };

		int dataSizeType = getDataSizeType(node->value->type);
		auto dst = newReg(node);

		emitInst(movOps[dataSizeType], dst, node->value, dataSizeType);
	}
	else
	{
		instRegister[node] = instRegister[node->value];
	}
}

void MachineInstSelectionX64::inst(IRInstCall* node)
{
#ifdef WIN32
	callWin64(node);
#else
	callUnix64(node);
#endif
}

void MachineInstSelectionX64::inst(IRInstGEP* node)
{
	if (!node->instructions.empty())
	{
		for (auto inst : node->instructions)
			inst->visit(this);

		instRegister[node] = instRegister[node->instructions.back()];
	}
	else
	{
		auto dst = newReg(node);
		emitInst(MachineInstOpcodeX64::mov64, dst, node->ptr, getDataSizeType(node->ptr->type));
	}
}

void MachineInstSelectionX64::inst(IRInstBr* node)
{
	emitPhi(node->bb);
	emitInst(MachineInstOpcodeX64::jmp, node->bb);
}

void MachineInstSelectionX64::inst(IRInstCondBr* node)
{
	if (isConstant(node->condition))
	{
		if (getConstantValueInt(node->condition))
		{
			emitPhi(node->bb1);
			emitInst(MachineInstOpcodeX64::jmp, node->bb1);
		}
		else
		{
			emitPhi(node->bb2);
			emitInst(MachineInstOpcodeX64::jmp, node->bb2);
		}
	}
	else
	{
		emitPhi(node->bb1);
		emitPhi(node->bb2);
		emitInst(MachineInstOpcodeX64::cmp8, node->condition, 3, newImm(1));
		emitInst(MachineInstOpcodeX64::jne, node->bb2);
		emitInst(MachineInstOpcodeX64::jmp, node->bb1);
	}
}

void MachineInstSelectionX64::inst(IRInstRet* node)
{
	static const MachineInstOpcodeX64 movOps[] = { MachineInstOpcodeX64::movsd, MachineInstOpcodeX64::movss, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::mov8 };

	int dataSizeType = getDataSizeType(static_cast<IRFunctionType*>(sfunc->type)->returnType);
	MachineOperand dst = newPhysReg(dataSizeType < 2 ? RegisterNameX64::xmm0 : RegisterNameX64::rax);

	emitInst(movOps[dataSizeType], dst, node->operand, dataSizeType);
	emitInst(MachineInstOpcodeX64::jmp, mfunc->epilog);
}

void MachineInstSelectionX64::inst(IRInstRetVoid* node)
{
	emitInst(MachineInstOpcodeX64::jmp, mfunc->epilog);
}

void MachineInstSelectionX64::inst(IRInstAlloca* node)
{
	uint64_t size = node->type->getPointerElementType()->getTypeAllocSize() * getConstantValueInt(node->arraySize);
	size = (size + 15) / 16 * 16;

	// sub rsp,size
	emitInst(MachineInstOpcodeX64::sub64, newPhysReg(RegisterNameX64::rsp), newImm(size));

	MachineOperand dst = newReg(node);
	MachineOperand src = newPhysReg(RegisterNameX64::rsp);

	if (mfunc->maxCallArgsSize == 0)
	{
		// mov vreg,rsp
		emitInst(MachineInstOpcodeX64::mov64, dst, src);
	}
	else
	{
		// lea vreg,ptr[rsp+maxCallArgsSize]
		emitInst(MachineInstOpcodeX64::lea, dst, src, newImm(mfunc->maxCallArgsSize));
	}

	mfunc->dynamicStackAllocations = true;
	mfunc->registers[(int)RegisterNameX64::rbp].cls = MachineRegClass::reserved;
}

void MachineInstSelectionX64::inst(IRInstPhi* node)
{
	// Handled in MachineInstSelectionX64::codegen
}

void MachineInstSelectionX64::emitPhi(IRBasicBlock* target)
{
	static const MachineInstOpcodeX64 movOps[] = { MachineInstOpcodeX64::movsd, MachineInstOpcodeX64::movss, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::mov8 };

	for (IRInst* inst : target->code)
	{
		IRInstPhi* phi = dynamic_cast<IRInstPhi*>(inst);
		if (!phi)
			break;

		int dataSizeType = getDataSizeType(phi->type);

		for (auto& incoming : phi->values)
		{
			if (irbb == incoming.first)
			{
				IRValue* value = incoming.second;

				MachineOperand dst = instRegister[phi];
				emitInst(movOps[dataSizeType], dst, value, dataSizeType);

				break;
			}
		}
	}
}

int MachineInstSelectionX64::getDataSizeType(IRType* type)
{
	if (isInt32(type))
		return 3;
	else if (isFloat(type))
		return 1;
	else if (isDouble(type))
		return 0;
	else if (isInt16(type))
		return 4;
	else if (isInt8(type) || isInt1(type))
		return 5;
	else // if (isInt64(type) || isPointer(type) || isFunction(type))
		return 2;
}

void MachineInstSelectionX64::findMaxCallArgsSize()
{
	int callArgsSize = 4;
	for (size_t i = 0; i < sfunc->basicBlocks.size(); i++)
	{
		IRBasicBlock* bb = sfunc->basicBlocks[i];
		for (IRValue* value : bb->code)
		{
			IRInstCall* node = dynamic_cast<IRInstCall*>(value);
			if (node)
			{
				callArgsSize = std::max(callArgsSize, (int)node->args.size());
			}
		}
	}
	mfunc->maxCallArgsSize = callArgsSize * 8;
}

void MachineInstSelectionX64::callWin64(IRInstCall* node)
{
	static const MachineInstOpcodeX64 loadOps[] = { MachineInstOpcodeX64::loadsd, MachineInstOpcodeX64::loadss, MachineInstOpcodeX64::load64, MachineInstOpcodeX64::load32, MachineInstOpcodeX64::load16, MachineInstOpcodeX64::load8 };
	static const MachineInstOpcodeX64 storeOps[] = { MachineInstOpcodeX64::storesd, MachineInstOpcodeX64::storess, MachineInstOpcodeX64::store64, MachineInstOpcodeX64::store32, MachineInstOpcodeX64::store16, MachineInstOpcodeX64::store8 };
	static const MachineInstOpcodeX64 movOps[] = { MachineInstOpcodeX64::movsd, MachineInstOpcodeX64::movss, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::mov8 };
	static const RegisterNameX64 regvars[] = { RegisterNameX64::rcx, RegisterNameX64::rdx, RegisterNameX64::r8, RegisterNameX64::r9 };

	// Move arguments into stack slots
	for (size_t i = 4, count = node->args.size(); i < count; i++)
	{
		IRValue* arg = node->args[i];

		bool isMemSrc = isConstantFP(arg) || isGlobalVariable(arg);
		int dataSizeType = getDataSizeType(arg->type);
		bool isXmm = dataSizeType < 2;
		bool needs64BitImm = (dataSizeType == 2) && isConstantInt(arg) && (getConstantValueInt(arg) < -0x7fffffff || getConstantValueInt(arg) > 0x7fffffff);

		MachineOperand dst;
		dst.type = MachineOperandType::stackOffset;
		dst.stackOffset = (int)(i * 8);

		if (isMemSrc)
		{
			MachineOperand tmpreg = newPhysReg(isXmm ? RegisterNameX64::xmm0 : RegisterNameX64::rax);
			emitInst(loadOps[dataSizeType], tmpreg, arg, dataSizeType);
			emitInst(storeOps[dataSizeType], dst, tmpreg);
		}
		else if (needs64BitImm)
		{
			MachineOperand tmpreg = newPhysReg(RegisterNameX64::rax);
			emitInst(movOps[dataSizeType], tmpreg, arg, dataSizeType);
			emitInst(storeOps[dataSizeType], dst, tmpreg);
		}
		else
		{
			emitInst(storeOps[dataSizeType], dst, arg, dataSizeType);
		}
	}

	// First four go to registers
	for (size_t i = 0, count = std::min((size_t)4, node->args.size()); i < count; i++)
	{
		IRValue* arg = node->args[i];

		bool isMemSrc = isConstantFP(arg) || isGlobalVariable(arg);
		int dataSizeType = getDataSizeType(arg->type);
		bool isXmm = dataSizeType < 2;

		MachineOperand dst;
		dst.type = MachineOperandType::reg;
		if (isXmm)
			dst.registerIndex = (int)RegisterNameX64::xmm0 + (int)i;
		else
			dst.registerIndex = (int)regvars[i];

		emitInst(isMemSrc ? loadOps[dataSizeType] : movOps[dataSizeType], dst, arg, dataSizeType);
	}

	// Call the function
	MachineOperand target;
	IRFunction* func = dynamic_cast<IRFunction*>(node->func);
	if (func)
	{
		target.type = MachineOperandType::func;
		target.func = func;
	}
	else
	{
		target = newPhysReg(RegisterNameX64::rax);
		emitInst(MachineInstOpcodeX64::mov64, target, instRegister[node->func]);
	}
	emitInst(MachineInstOpcodeX64::call, target);

	// Move return value to virtual register
	if (node->type != context->getVoidTy())
	{
		int dataSizeType = getDataSizeType(node->type);
		bool isXmm = dataSizeType < 2;

		MachineOperand src = newPhysReg(isXmm ? RegisterNameX64::xmm0 : RegisterNameX64::rax);
		auto dst = newReg(node);

		emitInst(movOps[dataSizeType], dst, src);
	}
}

void MachineInstSelectionX64::callUnix64(IRInstCall* node)
{
	static const MachineInstOpcodeX64 loadOps[] = { MachineInstOpcodeX64::loadsd, MachineInstOpcodeX64::loadss, MachineInstOpcodeX64::load64, MachineInstOpcodeX64::load32, MachineInstOpcodeX64::load16, MachineInstOpcodeX64::load8 };
	static const MachineInstOpcodeX64 storeOps[] = { MachineInstOpcodeX64::storesd, MachineInstOpcodeX64::storess, MachineInstOpcodeX64::store64, MachineInstOpcodeX64::store32, MachineInstOpcodeX64::store16, MachineInstOpcodeX64::store8 };
	static const MachineInstOpcodeX64 movOps[] = { MachineInstOpcodeX64::movsd, MachineInstOpcodeX64::movss, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::mov8 };
	static const RegisterNameX64 regvars[] = { RegisterNameX64::rdi, RegisterNameX64::rsi, RegisterNameX64::rdx, RegisterNameX64::rcx, RegisterNameX64::r8, RegisterNameX64::r9 };

	const int numRegisterArgs = 6;
	const int numXmmRegisterArgs = 8;
	int nextRegisterArg = 0;
	int nextXmmRegisterArg = 0;

	// Move arguments into place (stack slots first pass, register slots second pass)
	for (int pass = 0; pass < 2; pass++)
	{
		bool isRegisterPass = (pass == 1);
		for (size_t i = 0; i < node->args.size(); i++)
		{
			IRValue* arg = node->args[i];

			bool isMemSrc = isConstantFP(arg) || isGlobalVariable(arg);
			int dataSizeType = getDataSizeType(arg->type);
			bool isXmm = dataSizeType < 2;
			bool isRegisterArg = (isXmm && nextXmmRegisterArg < numXmmRegisterArgs) || (!isXmm && nextRegisterArg < numRegisterArgs);
			bool needs64BitImm = (dataSizeType == 2) && isConstantInt(arg) && (getConstantValueInt(arg) < -0x7fffffffffff || getConstantValueInt(arg) > 0x7fffffffffff);

			if (isRegisterPass && isRegisterArg)
			{
				MachineOperand dst;
				dst.type = MachineOperandType::reg;
				if (isXmm)
					dst.registerIndex = (int)RegisterNameX64::xmm0 + (nextXmmRegisterArg++);
				else
					dst.registerIndex = (int)regvars[nextRegisterArg++];

				emitInst(isMemSrc ? loadOps[dataSizeType] : movOps[dataSizeType], dst, arg, dataSizeType);
			}
			else if (!isRegisterPass)
			{
				MachineOperand dst;
				dst.type = MachineOperandType::stackOffset;
				dst.stackOffset = (int)(i * 8);

				if (isMemSrc)
				{
					MachineOperand tmpreg = newPhysReg(isXmm ? RegisterNameX64::xmm0 : RegisterNameX64::rax);
					emitInst(loadOps[dataSizeType], tmpreg, arg, dataSizeType);
					emitInst(storeOps[dataSizeType], dst, tmpreg);
				}
				else if (needs64BitImm)
				{
					MachineOperand tmpreg = newPhysReg(RegisterNameX64::rax);
					emitInst(movOps[dataSizeType], tmpreg, arg, dataSizeType);
					emitInst(storeOps[dataSizeType], dst, tmpreg);
				}
				else
				{
					emitInst(storeOps[dataSizeType], dst, arg, dataSizeType);
				}
			}
		}
	}

	// Call the function
	MachineOperand target;
	IRFunction* func = dynamic_cast<IRFunction*>(node->func);
	if (func)
	{
		target.type = MachineOperandType::func;
		target.func = func;
	}
	else
	{
		target = newPhysReg(RegisterNameX64::rax);
		emitInst(MachineInstOpcodeX64::mov64, target, instRegister[node->func]);
	}
	emitInst(MachineInstOpcodeX64::call, target);

	// Move return value to virtual register
	if (node->type != context->getVoidTy())
	{
		int dataSizeType = getDataSizeType(node->type);
		bool isXmm = dataSizeType < 2;

		MachineOperand src = newPhysReg(isXmm ? RegisterNameX64::xmm0 : RegisterNameX64::rax);
		auto dst = newReg(node);

		emitInst(movOps[dataSizeType], dst, src);
	}
}

void MachineInstSelectionX64::simpleCompareInst(IRInstBinary* node, MachineInstOpcodeX64 opSet, MachineInstOpcodeX64 opSet2, MachineInstOpcodeX64 opSet3)
{
	static const MachineInstOpcodeX64 movOps[] = { MachineInstOpcodeX64::movsd, MachineInstOpcodeX64::movss, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::mov8 };
	static const MachineInstOpcodeX64 cmpOps[] = { MachineInstOpcodeX64::ucomisd, MachineInstOpcodeX64::ucomiss, MachineInstOpcodeX64::cmp64, MachineInstOpcodeX64::cmp32, MachineInstOpcodeX64::cmp16, MachineInstOpcodeX64::cmp8 };

	int dataSizeType = getDataSizeType(node->operand1->type);

	// operand1 must be in a register:
	MachineOperand src1;
	if (isConstant(node->operand1))
	{
		src1 = newTempReg(dataSizeType < 2 ? MachineRegClass::xmm : MachineRegClass::gp);
		emitInst(movOps[dataSizeType], src1, node->operand1, dataSizeType);
	}
	else
	{
		src1 = instRegister[node->operand1];
	}

	// Create 8 bit register
	auto dst = newReg(node);

	// Perform comparison

	// 64 bit constants needs to be moved into a register first
	bool needs64BitImm = (dataSizeType == 2) && isConstantInt(node->operand2) && (getConstantValueInt(node->operand2) < -0x7fffffff || getConstantValueInt(node->operand2) > 0x7fffffff);
	if (needs64BitImm)
	{
		auto immreg = newTempReg(MachineRegClass::gp);
		emitInst(movOps[dataSizeType], immreg, node->operand2, dataSizeType);
		emitInst(cmpOps[dataSizeType], src1, immreg);
	}
	else
	{
		emitInst(cmpOps[dataSizeType], src1, node->operand2, dataSizeType);
	}

	if (opSet2 == MachineInstOpcodeX64::nop)
	{
		// Move result flag to register
		emitInst(opSet, dst);
	}
	else
	{
		auto temp = newTempReg(MachineRegClass::gp);
		emitInst(opSet, temp);
		emitInst(opSet2, dst);
		emitInst(opSet3, dst, temp);
	}
}

void MachineInstSelectionX64::simpleBinaryInst(IRInstBinary* node, const MachineInstOpcodeX64* binaryOps)
{
	static const MachineInstOpcodeX64 movOps[] = { MachineInstOpcodeX64::movsd, MachineInstOpcodeX64::movss, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::mov8 };

	int dataSizeType = getDataSizeType(node->type);
	auto dst = newReg(node);

	// Move operand1 to dest
	emitInst(movOps[dataSizeType], dst, node->operand1, dataSizeType);

	// Apply operand2 to dest

	// 64 bit constants needs to be moved into a register first
	bool needs64BitImm = (dataSizeType == 2) && isConstantInt(node->operand2) && (getConstantValueInt(node->operand2) < -0x7fffffff || getConstantValueInt(node->operand2) > 0x7fffffff);
	if (needs64BitImm)
	{
		auto immreg = newTempReg(MachineRegClass::gp);
		emitInst(movOps[dataSizeType], immreg, node->operand2, dataSizeType);
		emitInst(binaryOps[dataSizeType], dst, immreg);
	}
	else
	{
		emitInst(binaryOps[dataSizeType], dst, node->operand2, dataSizeType);
	}
}

void MachineInstSelectionX64::shiftBinaryInst(IRInstBinary* node, const MachineInstOpcodeX64* binaryOps)
{
	static const MachineInstOpcodeX64 movOps[] = { MachineInstOpcodeX64::movsd, MachineInstOpcodeX64::movss, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::mov8 };

	int dataSizeType = getDataSizeType(node->type);
	auto dst = newReg(node);

	if (isConstant(node->operand2))
	{
		// Move operand1 to dest
		emitInst(movOps[dataSizeType], dst, node->operand1, dataSizeType);

		// Apply operand2 to dest
		emitInst(binaryOps[dataSizeType], dst, node->operand2, dataSizeType);
	}
	else
	{
		// Move operand2 to rcx
		emitInst(movOps[dataSizeType], newPhysReg(RegisterNameX64::rcx), node->operand2, dataSizeType);

		// Move operand1 to dest
		emitInst(movOps[dataSizeType], dst, node->operand1, dataSizeType);

		// Apply rcx to dest
		emitInst(binaryOps[dataSizeType], dst, newPhysReg(RegisterNameX64::rcx));
	}
}

void MachineInstSelectionX64::divBinaryInst(IRInstBinary* node, const MachineInstOpcodeX64* binaryOps, bool remainder, bool zeroext)
{
	static const MachineInstOpcodeX64 movOps[] = { MachineInstOpcodeX64::movsd, MachineInstOpcodeX64::movss, MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::mov8 };
	static const MachineInstOpcodeX64 sarOps[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::sar64, MachineInstOpcodeX64::sar32, MachineInstOpcodeX64::sar16, MachineInstOpcodeX64::sar8 };
	static const MachineInstOpcodeX64 xorOps[] = { MachineInstOpcodeX64::nop, MachineInstOpcodeX64::nop, MachineInstOpcodeX64::xor64, MachineInstOpcodeX64::xor32, MachineInstOpcodeX64::xor16, MachineInstOpcodeX64::xor8 };
	static const int shiftcount[] = { 0, 0, 63, 31, 15, 7 };

	int dataSizeType = getDataSizeType(node->type);

	auto dst = newReg(node);

	// Move operand1 to rdx:rax
	emitInst(movOps[dataSizeType], newPhysReg(RegisterNameX64::rax), node->operand1, dataSizeType);
	if (dataSizeType != 5)
	{
		if (zeroext)
		{
			emitInst(xorOps[dataSizeType], newPhysReg(RegisterNameX64::rdx), newPhysReg(RegisterNameX64::rdx));
		}
		else
		{
			emitInst(movOps[dataSizeType], newPhysReg(RegisterNameX64::rdx), newPhysReg(RegisterNameX64::rax));
			emitInst(sarOps[dataSizeType], newPhysReg(RegisterNameX64::rdx), newImm(shiftcount[dataSizeType]));
		}
	}
	else
	{
		emitInst(zeroext ? MachineInstOpcodeX64::movzx8_16 : MachineInstOpcodeX64::movsx8_16, newPhysReg(RegisterNameX64::rax), newPhysReg(RegisterNameX64::rax));
	}

	// Divide
	if (isConstant(node->operand2) || isGlobalVariable(node->operand2))
	{
		emitInst(movOps[dataSizeType], dst, node->operand2, dataSizeType);
		emitInst(binaryOps[dataSizeType], dst);
	}
	else
	{
		emitInst(binaryOps[dataSizeType], node->operand2, dataSizeType);
	}

	// Move rax to dest
	if (dataSizeType != 5 || !remainder)
	{
		emitInst(movOps[dataSizeType], dst, newPhysReg(remainder ? RegisterNameX64::rdx : RegisterNameX64::rax));
	}
	else // for 8 bit the remainder is in ah
	{
		emitInst(MachineInstOpcodeX64::shr16, newPhysReg(RegisterNameX64::rax), newImm(8));
		emitInst(MachineInstOpcodeX64::mov8, dst, newPhysReg(RegisterNameX64::rax));
	}
}

void MachineInstSelectionX64::addDebugInfo(MachineInst* inst)
{
	// Transfer debug info to first instruction emitted
	if (debugInfoInst)
	{
		inst->comment = debugInfoInst->comment;
		inst->fileIndex = debugInfoInst->fileIndex;
		inst->lineNumber = debugInfoInst->lineNumber;
		debugInfoInst = nullptr;
	}
}

void MachineInstSelectionX64::emitInst(MachineInstOpcodeX64 opcode, const MachineOperand& operand1, const MachineOperand& operand2, const MachineOperand& operand3)
{
	auto inst = context->newMachineInst();
	addDebugInfo(inst);
	inst->opcode = (int)opcode;
	inst->operands.push_back(operand1);
	inst->operands.push_back(operand2);
	inst->operands.push_back(operand3);
	bb->code.push_back(inst);
}

void MachineInstSelectionX64::emitInst(MachineInstOpcodeX64 opcode, const MachineOperand& operand1, IRValue* operand2, int dataSizeType, const MachineOperand& operand3)
{
	auto inst = context->newMachineInst();
	addDebugInfo(inst);
	inst->opcode = (int)opcode;
	inst->operands.push_back(operand1);
	pushValueOperand(inst, operand2, dataSizeType);
	inst->operands.push_back(operand3);
	bb->code.push_back(inst);
}

void MachineInstSelectionX64::emitInst(MachineInstOpcodeX64 opcode, const MachineOperand& operand1, const MachineOperand& operand2)
{
	auto inst = context->newMachineInst();
	addDebugInfo(inst);
	inst->opcode = (int)opcode;
	inst->operands.push_back(operand1);
	inst->operands.push_back(operand2);
	bb->code.push_back(inst);
}

void MachineInstSelectionX64::emitInst(MachineInstOpcodeX64 opcode, const MachineOperand& operand1, IRValue* operand2, int dataSizeType)
{
	auto inst = context->newMachineInst();
	addDebugInfo(inst);
	inst->opcode = (int)opcode;
	inst->operands.push_back(operand1);
	pushValueOperand(inst, operand2, dataSizeType);
	bb->code.push_back(inst);
}

void MachineInstSelectionX64::emitInst(MachineInstOpcodeX64 opcode, IRValue* operand1, int dataSizeType, const MachineOperand& operand2)
{
	auto inst = context->newMachineInst();
	addDebugInfo(inst);
	inst->opcode = (int)opcode;
	pushValueOperand(inst, operand1, dataSizeType);
	inst->operands.push_back(operand2);
	bb->code.push_back(inst);
}

void MachineInstSelectionX64::emitInst(MachineInstOpcodeX64 opcode, const MachineOperand& operand)
{
	auto inst = context->newMachineInst();
	addDebugInfo(inst);
	inst->opcode = (int)opcode;
	inst->operands.push_back(operand);
	bb->code.push_back(inst);
}

void MachineInstSelectionX64::emitInst(MachineInstOpcodeX64 opcode, IRValue* operand, int dataSizeType)
{
	auto inst = context->newMachineInst();
	addDebugInfo(inst);
	inst->opcode = (int)opcode;
	pushValueOperand(inst, operand, dataSizeType);
	bb->code.push_back(inst);
}

void MachineInstSelectionX64::emitInst(MachineInstOpcodeX64 opcode, IRBasicBlock* target)
{
	auto inst = context->newMachineInst();
	addDebugInfo(inst);
	inst->opcode = (int)opcode;
	pushBBOperand(inst, target);
	bb->code.push_back(inst);
}

void MachineInstSelectionX64::emitInst(MachineInstOpcodeX64 opcode, MachineBasicBlock* target)
{
	MachineOperand operand;
	operand.type = MachineOperandType::basicblock;
	operand.bb = target;

	auto inst = context->newMachineInst();
	addDebugInfo(inst);
	inst->opcode = (int)opcode;
	inst->operands.push_back(operand);
	bb->code.push_back(inst);
}

void MachineInstSelectionX64::pushValueOperand(MachineInst* inst, IRValue* operand, int dataSizeType)
{
	if (isConstantFP(operand))
	{
		if (dataSizeType == 0)
			inst->operands.push_back(newConstant(getConstantValueDouble(operand)));
		else // if (dataSizeType == 1)
			inst->operands.push_back(newConstant(getConstantValueFloat(operand)));
	}
	else if (isConstantInt(operand))
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

void MachineInstSelectionX64::pushBBOperand(MachineInst* inst, IRBasicBlock* bb)
{
	MachineOperand operand;
	operand.type = MachineOperandType::basicblock;
	operand.bb = bbMap[bb];
	inst->operands.push_back(operand);
}

MachineOperand MachineInstSelectionX64::newReg(IRValue* node)
{
	MachineOperand operand;
	operand.type = MachineOperandType::reg;
	operand.registerIndex = createVirtReg(getDataSizeType(node->type) < 2 ? MachineRegClass::xmm : MachineRegClass::gp);
	instRegister[node] = operand;
	return operand;
}

MachineOperand MachineInstSelectionX64::newPhysReg(RegisterNameX64 name)
{
	MachineOperand operand;
	operand.type = MachineOperandType::reg;
	operand.registerIndex = (int)name;
	return operand;
}

MachineOperand MachineInstSelectionX64::newTempReg(MachineRegClass cls)
{
	MachineOperand operand;
	operand.type = MachineOperandType::reg;
	operand.registerIndex = createVirtReg(cls);
	return operand;
}

int MachineInstSelectionX64::createVirtReg(MachineRegClass cls)
{
	int registerIndex = (int)mfunc->registers.size();
	mfunc->registers.push_back(cls);
	return registerIndex;
}

MachineOperand MachineInstSelectionX64::newConstant(uint64_t address)
{
	return newConstant(&address, sizeof(uint64_t));
}

MachineOperand MachineInstSelectionX64::newConstant(float value)
{
	return newConstant(&value, sizeof(float));
}

MachineOperand MachineInstSelectionX64::newConstant(double value)
{
	return newConstant(&value, sizeof(double));
}

MachineOperand MachineInstSelectionX64::newConstant(const void* data, int size)
{
	int index = (int)mfunc->constants.size();
	mfunc->constants.push_back({ data, size });

	MachineOperand operand;
	operand.type = MachineOperandType::constant;
	operand.constantIndex = index;
	return operand;
}

MachineOperand MachineInstSelectionX64::newImm(uint64_t imm)
{
	MachineOperand operand;
	operand.type = MachineOperandType::imm;
	operand.immvalue = imm;
	return operand;
}

uint64_t MachineInstSelectionX64::getConstantValueInt(IRValue* value)
{
	if (isConstantInt(value))
		return static_cast<IRConstantInt*>(value)->value;
	else
		return 0;
}

float MachineInstSelectionX64::getConstantValueFloat(IRValue* value)
{
	if (isConstantFP(value))
		return static_cast<float>(static_cast<IRConstantFP*>(value)->value);
	else
		return 0.0f;
}

double MachineInstSelectionX64::getConstantValueDouble(IRValue* value)
{
	if (isConstantFP(value))
		return static_cast<IRConstantFP*>(value)->value;
	else
		return 0.0;
}

MachineFunction* MachineInstSelectionX64::dumpinstructions(IRFunction* sfunc)
{
	MachineInstSelectionX64 selection(sfunc);
	selection.mfunc = sfunc->context->newMachineFunction(sfunc->name);
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
	selection.mfunc->registers.resize((size_t)RegisterNameX64::vregstart);

	selection.findMaxCallArgsSize();

	MachineBasicBlock* mbb = sfunc->context->newMachineBasicBlock();
	selection.mfunc->basicBlocks.push_back(mbb);
	selection.bb = mbb;

	std::vector<RegisterNameX64> gpRegs =
	{
		RegisterNameX64::rax, RegisterNameX64::rcx, RegisterNameX64::rdx, RegisterNameX64::rbx,
		RegisterNameX64::rsp, RegisterNameX64::rbp, RegisterNameX64::rsi, RegisterNameX64::rdi,
		RegisterNameX64::r8, RegisterNameX64::r9, RegisterNameX64::r10, RegisterNameX64::r11,
		RegisterNameX64::r12, RegisterNameX64::r13, RegisterNameX64::r14, RegisterNameX64::r15
	};

	std::vector<RegisterNameX64> xmmRegs =
	{
		RegisterNameX64::xmm0, RegisterNameX64::xmm1, RegisterNameX64::xmm2, RegisterNameX64::xmm3,
		RegisterNameX64::xmm4, RegisterNameX64::xmm5, RegisterNameX64::xmm6, RegisterNameX64::xmm7,
		RegisterNameX64::xmm8, RegisterNameX64::xmm9, RegisterNameX64::xmm10, RegisterNameX64::xmm11,
		RegisterNameX64::xmm12, RegisterNameX64::xmm13, RegisterNameX64::xmm14, RegisterNameX64::xmm15
	};

	std::vector<MachineInstOpcodeX64> xmmBinaryOps =
	{
		MachineInstOpcodeX64::movss, MachineInstOpcodeX64::movsd,
		MachineInstOpcodeX64::addss, MachineInstOpcodeX64::addsd,
		MachineInstOpcodeX64::subss, MachineInstOpcodeX64::subsd,
		MachineInstOpcodeX64::mulss, MachineInstOpcodeX64::mulsd,
		MachineInstOpcodeX64::divss, MachineInstOpcodeX64::divsd,
		MachineInstOpcodeX64::ucomiss, MachineInstOpcodeX64::ucomisd
	};

	std::vector<MachineInstOpcodeX64> intBinaryOps =
	{
		MachineInstOpcodeX64::mov64, MachineInstOpcodeX64::mov32, MachineInstOpcodeX64::mov16, MachineInstOpcodeX64::mov8,
		MachineInstOpcodeX64::add64, MachineInstOpcodeX64::add32, MachineInstOpcodeX64::add16, MachineInstOpcodeX64::add8,
		MachineInstOpcodeX64::sub64, MachineInstOpcodeX64::sub32, MachineInstOpcodeX64::sub16, MachineInstOpcodeX64::sub8,
		MachineInstOpcodeX64::and64, MachineInstOpcodeX64::and32, MachineInstOpcodeX64::and16, MachineInstOpcodeX64::and8,
		MachineInstOpcodeX64::or64, MachineInstOpcodeX64::or32, MachineInstOpcodeX64::or16, MachineInstOpcodeX64::or8,
		MachineInstOpcodeX64::xor64, MachineInstOpcodeX64::xor32, MachineInstOpcodeX64::xor16, MachineInstOpcodeX64::xor8,
		MachineInstOpcodeX64::cmp64, MachineInstOpcodeX64::cmp32, MachineInstOpcodeX64::cmp16, MachineInstOpcodeX64::cmp8,
		MachineInstOpcodeX64::imul64, MachineInstOpcodeX64::imul32, MachineInstOpcodeX64::imul16, MachineInstOpcodeX64::imul8,
	};

	std::vector<MachineInstOpcodeX64> intUnaryOps =
	{
		MachineInstOpcodeX64::not64, MachineInstOpcodeX64::not32, MachineInstOpcodeX64::not16, MachineInstOpcodeX64::not8,
		MachineInstOpcodeX64::neg64, MachineInstOpcodeX64::neg32, MachineInstOpcodeX64::neg16, MachineInstOpcodeX64::neg8,
		MachineInstOpcodeX64::setl, MachineInstOpcodeX64::setb, MachineInstOpcodeX64::seta, MachineInstOpcodeX64::setg,
		MachineInstOpcodeX64::setle, MachineInstOpcodeX64::setbe, MachineInstOpcodeX64::setae, MachineInstOpcodeX64::setge,
		MachineInstOpcodeX64::sete, MachineInstOpcodeX64::setne,
	};

	std::vector<MachineInstOpcodeX64> intShiftOps =
	{
		MachineInstOpcodeX64::shl64, MachineInstOpcodeX64::shl32, MachineInstOpcodeX64::shl16, MachineInstOpcodeX64::shl8,
		MachineInstOpcodeX64::shr64, MachineInstOpcodeX64::shr32, MachineInstOpcodeX64::shr16, MachineInstOpcodeX64::shr8,
		MachineInstOpcodeX64::sar64, MachineInstOpcodeX64::sar32, MachineInstOpcodeX64::sar16, MachineInstOpcodeX64::sar8,
	};

	std::vector<MachineInstOpcodeX64> intDivOps =
	{
		MachineInstOpcodeX64::idiv64, MachineInstOpcodeX64::idiv32, MachineInstOpcodeX64::idiv16, MachineInstOpcodeX64::idiv8,
		MachineInstOpcodeX64::div64, MachineInstOpcodeX64::div32, MachineInstOpcodeX64::div16, MachineInstOpcodeX64::div8,
	};

	std::vector<MachineInstOpcodeX64> intExtOps =
	{
		MachineInstOpcodeX64::movsx8_16, MachineInstOpcodeX64::movsx8_32, MachineInstOpcodeX64::movsx8_64,
		MachineInstOpcodeX64::movsx16_32, MachineInstOpcodeX64::movsx16_64,
		MachineInstOpcodeX64::movsx32_64,
		MachineInstOpcodeX64::movzx8_16, MachineInstOpcodeX64::movzx8_32, MachineInstOpcodeX64::movzx8_64,
		MachineInstOpcodeX64::movzx16_32, MachineInstOpcodeX64::movzx16_64,
	};

	// Load/store tests:

	// Register/register tests:

	for (MachineInstOpcodeX64 opcode : { MachineInstOpcodeX64::cvtsd2ss, MachineInstOpcodeX64::cvtss2sd })
	{
		for (RegisterNameX64 regname : xmmRegs)
		{
			MachineOperand operand1, operand2;
			operand1.type = MachineOperandType::reg;
			operand2.type = MachineOperandType::reg;
			operand1.registerIndex = (int)regname;
			operand2.registerIndex = (int)RegisterNameX64::xmm0;
			selection.emitInst(opcode, operand1, operand2);
		}

		for (RegisterNameX64 regname : xmmRegs)
		{
			MachineOperand operand1, operand2;
			operand1.type = MachineOperandType::reg;
			operand2.type = MachineOperandType::reg;
			operand1.registerIndex = (int)RegisterNameX64::xmm0;
			operand2.registerIndex = (int)regname;
			selection.emitInst(opcode, operand1, operand2);
		}
	}

	for (MachineInstOpcodeX64 opcode : { MachineInstOpcodeX64::cvttsd2si, MachineInstOpcodeX64::cvttss2si })
	{
		for (RegisterNameX64 regname : xmmRegs)
		{
			MachineOperand operand1, operand2;
			operand1.type = MachineOperandType::reg;
			operand2.type = MachineOperandType::reg;
			operand1.registerIndex = (int)RegisterNameX64::rax;
			operand2.registerIndex = (int)regname;
			selection.emitInst(opcode, operand1, operand2);
		}

		for (RegisterNameX64 regname : xmmRegs)
		{
			MachineOperand operand1, operand2;
			operand1.type = MachineOperandType::reg;
			operand2.type = MachineOperandType::reg;
			operand1.registerIndex = (int)RegisterNameX64::r15;
			operand2.registerIndex = (int)regname;
			selection.emitInst(opcode, operand1, operand2);
		}

		for (RegisterNameX64 regname : gpRegs)
		{
			MachineOperand operand1, operand2;
			operand1.type = MachineOperandType::reg;
			operand2.type = MachineOperandType::reg;
			operand1.registerIndex = (int)regname;
			operand2.registerIndex = (int)RegisterNameX64::xmm0;
			selection.emitInst(opcode, operand1, operand2);
		}

		for (RegisterNameX64 regname : gpRegs)
		{
			MachineOperand operand1, operand2;
			operand1.type = MachineOperandType::reg;
			operand2.type = MachineOperandType::reg;
			operand1.registerIndex = (int)regname;
			operand2.registerIndex = (int)RegisterNameX64::xmm15;
			selection.emitInst(opcode, operand1, operand2);
		}
	}

	for (MachineInstOpcodeX64 opcode : { MachineInstOpcodeX64::cvtsi2sd, MachineInstOpcodeX64::cvtsi2ss })
	{
		for (RegisterNameX64 regname : xmmRegs)
		{
			MachineOperand operand1, operand2;
			operand1.type = MachineOperandType::reg;
			operand2.type = MachineOperandType::reg;
			operand1.registerIndex = (int)regname;
			operand2.registerIndex = (int)RegisterNameX64::rax;
			selection.emitInst(opcode, operand1, operand2);
		}

		for (RegisterNameX64 regname : xmmRegs)
		{
			MachineOperand operand1, operand2;
			operand1.type = MachineOperandType::reg;
			operand2.type = MachineOperandType::reg;
			operand1.registerIndex = (int)regname;
			operand2.registerIndex = (int)RegisterNameX64::r15;
			selection.emitInst(opcode, operand1, operand2);
		}

		for (RegisterNameX64 regname : gpRegs)
		{
			MachineOperand operand1, operand2;
			operand1.type = MachineOperandType::reg;
			operand2.type = MachineOperandType::reg;
			operand1.registerIndex = (int)RegisterNameX64::xmm0;
			operand2.registerIndex = (int)regname;
			selection.emitInst(opcode, operand1, operand2);
		}

		for (RegisterNameX64 regname : gpRegs)
		{
			MachineOperand operand1, operand2;
			operand1.type = MachineOperandType::reg;
			operand2.type = MachineOperandType::reg;
			operand1.registerIndex = (int)RegisterNameX64::xmm15;
			operand2.registerIndex = (int)regname;
			selection.emitInst(opcode, operand1, operand2);
		}
	}

/*
	for (MachineInstOpcodeX64 opcode : xmmBinaryOps)
	{
		for (RegisterNameX64 regname : xmmRegs)
		{
			MachineOperand operand1, operand2;
			operand1.type = MachineOperandType::reg;
			operand2.type = MachineOperandType::reg;
			operand1.registerIndex = (int)regname;
			operand2.registerIndex = (int)RegisterNameX64::xmm0;
			selection.emitInst(opcode, operand1, operand2);
		}

		for (RegisterNameX64 regname : xmmRegs)
		{
			MachineOperand operand1, operand2;
			operand1.type = MachineOperandType::reg;
			operand2.type = MachineOperandType::reg;
			operand1.registerIndex = (int)RegisterNameX64::xmm0;
			operand2.registerIndex = (int)regname;
			selection.emitInst(opcode, operand1, operand2);
		}
	}

	for (MachineInstOpcodeX64 opcode : intUnaryOps)
	{
		for (RegisterNameX64 regname : gpRegs)
		{
			MachineOperand operand;
			operand.type = MachineOperandType::reg;
			operand.registerIndex = (int)regname;
			selection.emitInst(opcode, operand);
		}
	}

	for (MachineInstOpcodeX64 opcode : intShiftOps)
	{
		for (RegisterNameX64 regname : gpRegs)
		{
			MachineOperand operand1, operand2;
			operand1.type = MachineOperandType::reg;
			operand2.type = MachineOperandType::reg;
			operand1.registerIndex = (int)regname;
			operand2.registerIndex = (int)RegisterNameX64::rcx;
			selection.emitInst(opcode, operand1, operand2);
		}
	}

	for (MachineInstOpcodeX64 opcode : intDivOps)
	{
		for (RegisterNameX64 regname : gpRegs)
		{
			MachineOperand operand;
			operand.type = MachineOperandType::reg;
			operand.registerIndex = (int)regname;
			selection.emitInst(opcode, operand);
		}
	}

	for (MachineInstOpcodeX64 opcode : intBinaryOps)
	{
		for (RegisterNameX64 regname : gpRegs)
		{
			MachineOperand operand1, operand2;
			operand1.type = MachineOperandType::reg;
			operand2.type = MachineOperandType::reg;
			operand1.registerIndex = (int)regname;
			operand2.registerIndex = (int)RegisterNameX64::rax;
			selection.emitInst(opcode, operand1, operand2);
		}

		for (RegisterNameX64 regname : gpRegs)
		{
			MachineOperand operand1, operand2;
			operand1.type = MachineOperandType::reg;
			operand2.type = MachineOperandType::reg;
			operand1.registerIndex = (int)RegisterNameX64::rax;
			operand2.registerIndex = (int)regname;
			selection.emitInst(opcode, operand1, operand2);
		}
	}

	for (MachineInstOpcodeX64 opcode : intExtOps)
	{
		for (RegisterNameX64 regname : gpRegs)
		{
			MachineOperand operand1, operand2;
			operand1.type = MachineOperandType::reg;
			operand2.type = MachineOperandType::reg;
			operand1.registerIndex = (int)regname;
			operand2.registerIndex = (int)RegisterNameX64::rax;
			selection.emitInst(opcode, operand1, operand2);
		}

		for (RegisterNameX64 regname : gpRegs)
		{
			MachineOperand operand1, operand2;
			operand1.type = MachineOperandType::reg;
			operand2.type = MachineOperandType::reg;
			operand1.registerIndex = (int)RegisterNameX64::rax;
			operand2.registerIndex = (int)regname;
			selection.emitInst(opcode, operand1, operand2);
		}
	}

	// Register/constant tests:

	int opcodeindex = 0;
	for (MachineInstOpcodeX64 opcode : xmmBinaryOps)
	{
		MachineOperand operand2 = (opcodeindex++ % 2 == 1) ? selection.newConstant(1.234) : selection.newConstant(1.234f);
		for (RegisterNameX64 regname : xmmRegs)
		{
			MachineOperand operand1;
			operand1.type = MachineOperandType::reg;
			operand1.registerIndex = (int)regname;
			selection.emitInst(opcode, operand1, operand2);
		}
	}

	for (MachineInstOpcodeX64 opcode : intBinaryOps)
	{
		for (RegisterNameX64 regname : gpRegs)
		{
			MachineOperand operand1;
			operand1.type = MachineOperandType::reg;
			operand1.registerIndex = (int)regname;
			MachineOperand operand2 = selection.newImm(0x0123456879abcdef);
			selection.emitInst(opcode, operand1, operand2);
		}
	}

	for (MachineInstOpcodeX64 opcode : intShiftOps)
	{
		for (RegisterNameX64 regname : gpRegs)
		{
			MachineOperand operand1;
			operand1.type = MachineOperandType::reg;
			operand1.registerIndex = (int)regname;
			MachineOperand operand2 = selection.newImm(5);
			selection.emitInst(opcode, operand1, operand2);
		}
	}
*/
	// Register/global tests:


	return selection.mfunc;
}
