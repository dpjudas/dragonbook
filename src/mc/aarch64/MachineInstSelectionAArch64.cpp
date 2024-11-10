
#include "MachineInstSelectionAArch64.h"
#include <algorithm>

MachineFunction* MachineInstSelectionAArch64::codegen(IRFunction* sfunc)
{
	sfunc->sortBasicBlocks();

	MachineInstSelectionAArch64 selection(sfunc);
	selection.mfunc = sfunc->context->newMachineFunction(sfunc->name);
	selection.mfunc->fileInfo = sfunc->fileInfo;
	selection.mfunc->type = dynamic_cast<IRFunctionType*>(sfunc->type);
	selection.mfunc->prolog = sfunc->context->newMachineBasicBlock();
	selection.mfunc->epilog = sfunc->context->newMachineBasicBlock();
	for (int i = 0; i < 30; i++) // X0..X29
		selection.mfunc->registers.push_back(MachineRegClass::gp);
	selection.mfunc->registers.push_back(MachineRegClass::reserved); // lr (tbd: should we use this register and then restore it before calling RET instead?)
	selection.mfunc->registers.push_back(MachineRegClass::reserved); // sp/zxr/wzr
	for (int i = 0; i < 32; i++) // D0..D31
		selection.mfunc->registers.push_back(MachineRegClass::xmm);
	selection.mfunc->registers.resize((size_t)RegisterNameAArch64::vregstart);

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

void MachineInstSelectionAArch64::inst(IRInstLoad* node)
{
	static const MachineInstOpcodeAArch64 loadOps[] = { MachineInstOpcodeAArch64::loadsd, MachineInstOpcodeAArch64::loadss, MachineInstOpcodeAArch64::load64, MachineInstOpcodeAArch64::load32, MachineInstOpcodeAArch64::load16, MachineInstOpcodeAArch64::load8 };
	static const MachineInstOpcodeAArch64 movOps[] = { MachineInstOpcodeAArch64::movsd, MachineInstOpcodeAArch64::movss, MachineInstOpcodeAArch64::mov64, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32 };

	int dataSizeType = getDataSizeType(node->type);

	if (isConstantInt(node->operand))
	{
		auto srcreg = newTempReg(MachineRegClass::gp);
		emitInst(MachineInstOpcodeAArch64::mov64, srcreg, node->operand, 2);
		emitInst(loadOps[dataSizeType], newReg(node), srcreg);
	}
	else
	{
		emitInst(loadOps[dataSizeType], newReg(node), node->operand, dataSizeType);
	}
}

void MachineInstSelectionAArch64::inst(IRInstStore* node)
{
	static const MachineInstOpcodeAArch64 loadOps[] = { MachineInstOpcodeAArch64::loadsd, MachineInstOpcodeAArch64::loadss, MachineInstOpcodeAArch64::load64, MachineInstOpcodeAArch64::load32, MachineInstOpcodeAArch64::load16, MachineInstOpcodeAArch64::load8 };
	static const MachineInstOpcodeAArch64 movOps[] = { MachineInstOpcodeAArch64::movsd, MachineInstOpcodeAArch64::movss, MachineInstOpcodeAArch64::mov64, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32 };
	static const MachineInstOpcodeAArch64 storeOps[] = { MachineInstOpcodeAArch64::storesd, MachineInstOpcodeAArch64::storess, MachineInstOpcodeAArch64::store64, MachineInstOpcodeAArch64::store32, MachineInstOpcodeAArch64::store16, MachineInstOpcodeAArch64::store8 };

	MachineOperand dstreg;
	if (isConstantInt(node->operand2))
	{
		dstreg = newTempReg(MachineRegClass::gp);
		emitInst(MachineInstOpcodeAArch64::mov64, dstreg, node->operand2, 2);
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

void MachineInstSelectionAArch64::inst(IRInstAdd* node)
{
	static const MachineInstOpcodeAArch64 ops[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::add64, MachineInstOpcodeAArch64::add32, MachineInstOpcodeAArch64::add32, MachineInstOpcodeAArch64::add32 };
	simpleBinaryInst(node, ops);
}

void MachineInstSelectionAArch64::inst(IRInstSub* node)
{
	static const MachineInstOpcodeAArch64 ops[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::sub64, MachineInstOpcodeAArch64::sub32, MachineInstOpcodeAArch64::sub32, MachineInstOpcodeAArch64::sub32 };
	simpleBinaryInst(node, ops);
}

void MachineInstSelectionAArch64::inst(IRInstFAdd* node)
{
	static const MachineInstOpcodeAArch64 ops[] = { MachineInstOpcodeAArch64::addsd, MachineInstOpcodeAArch64::addss, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop };
	simpleBinaryInst(node, ops);
}

void MachineInstSelectionAArch64::inst(IRInstFSub* node)
{
	static const MachineInstOpcodeAArch64 ops[] = { MachineInstOpcodeAArch64::subsd, MachineInstOpcodeAArch64::subss, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop };
	simpleBinaryInst(node, ops);
}

void MachineInstSelectionAArch64::inst(IRInstNot* node)
{
	static const MachineInstOpcodeAArch64 movOps[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::mov64, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32 };
	static const MachineInstOpcodeAArch64 notOps[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::not64, MachineInstOpcodeAArch64::not32, MachineInstOpcodeAArch64::not32, MachineInstOpcodeAArch64::not32 };

	int dataSizeType = getDataSizeType(node->type);
	auto dst = newReg(node);
	emitInst(movOps[dataSizeType], dst, node->operand, dataSizeType);
	emitInst(notOps[dataSizeType], dst);
}

void MachineInstSelectionAArch64::inst(IRInstNeg* node)
{
	static const MachineInstOpcodeAArch64 movOps[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::mov64, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32 };
	static const MachineInstOpcodeAArch64 negOps[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::neg64, MachineInstOpcodeAArch64::neg32, MachineInstOpcodeAArch64::neg32, MachineInstOpcodeAArch64::neg32 };

	int dataSizeType = getDataSizeType(node->type);
	auto dst = newReg(node);
	emitInst(movOps[dataSizeType], dst, node->operand, dataSizeType);
	emitInst(negOps[dataSizeType], dst);
}

void MachineInstSelectionAArch64::inst(IRInstFNeg* node)
{
	/*
	static const MachineInstOpcodeAArch64 xorOps[] = { MachineInstOpcodeAArch64::xorpd, MachineInstOpcodeAArch64::xorps, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop };
	static const MachineInstOpcodeAArch64 subOps[] = { MachineInstOpcodeAArch64::subsd, MachineInstOpcodeAArch64::subss, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop };

	int dataSizeType = getDataSizeType(node->type);
	auto dst = newReg(node);
	emitInst(xorOps[dataSizeType], dst, dst);
	emitInst(subOps[dataSizeType], dst, node->operand, dataSizeType);
	*/
}

void MachineInstSelectionAArch64::inst(IRInstMul* node)
{
	static const MachineInstOpcodeAArch64 ops[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::mul64, MachineInstOpcodeAArch64::mul32, MachineInstOpcodeAArch64::mul32, MachineInstOpcodeAArch64::mul32 };
	simpleBinaryInst(node, ops);
}

void MachineInstSelectionAArch64::inst(IRInstFMul* node)
{
	static const MachineInstOpcodeAArch64 ops[] = { MachineInstOpcodeAArch64::mulsd, MachineInstOpcodeAArch64::mulss, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop };
	simpleBinaryInst(node, ops);
}

void MachineInstSelectionAArch64::inst(IRInstSDiv* node)
{
	static const MachineInstOpcodeAArch64 ops[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::sdiv64, MachineInstOpcodeAArch64::sdiv32, MachineInstOpcodeAArch64::sdiv32, MachineInstOpcodeAArch64::udiv32 };
	divBinaryInst(node, ops, false, false);
}

void MachineInstSelectionAArch64::inst(IRInstUDiv* node)
{
	static const MachineInstOpcodeAArch64 ops[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::udiv64, MachineInstOpcodeAArch64::udiv32, MachineInstOpcodeAArch64::udiv32, MachineInstOpcodeAArch64::udiv32 };
	divBinaryInst(node, ops, false, true);
}

void MachineInstSelectionAArch64::inst(IRInstFDiv* node)
{
	static const MachineInstOpcodeAArch64 ops[] = { MachineInstOpcodeAArch64::divsd, MachineInstOpcodeAArch64::divss, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop };
	simpleBinaryInst(node, ops);
}

void MachineInstSelectionAArch64::inst(IRInstSRem* node)
{
	static const MachineInstOpcodeAArch64 ops[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::sdiv64, MachineInstOpcodeAArch64::sdiv32, MachineInstOpcodeAArch64::sdiv32, MachineInstOpcodeAArch64::sdiv32 };
	divBinaryInst(node, ops, true, false);
}

void MachineInstSelectionAArch64::inst(IRInstURem* node)
{
	static const MachineInstOpcodeAArch64 ops[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::udiv64, MachineInstOpcodeAArch64::udiv32, MachineInstOpcodeAArch64::udiv32, MachineInstOpcodeAArch64::udiv32 };
	divBinaryInst(node, ops, true, true);
}

void MachineInstSelectionAArch64::inst(IRInstShl* node)
{
	static const MachineInstOpcodeAArch64 ops[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::shl64, MachineInstOpcodeAArch64::shl32, MachineInstOpcodeAArch64::shl32, MachineInstOpcodeAArch64::shl32 };
	shiftBinaryInst(node, ops);
}

void MachineInstSelectionAArch64::inst(IRInstLShr* node)
{
	static const MachineInstOpcodeAArch64 ops[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::shr64, MachineInstOpcodeAArch64::shr32, MachineInstOpcodeAArch64::shr32, MachineInstOpcodeAArch64::shr32 };
	shiftBinaryInst(node, ops);
}

void MachineInstSelectionAArch64::inst(IRInstAShr* node)
{
	static const MachineInstOpcodeAArch64 ops[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::sar64, MachineInstOpcodeAArch64::sar32, MachineInstOpcodeAArch64::sar32, MachineInstOpcodeAArch64::sar32 };
	shiftBinaryInst(node, ops);
}

void MachineInstSelectionAArch64::inst(IRInstICmpSLT* node)
{
	// simpleCompareInst(node, MachineInstOpcodeAArch64::setl);
}

void MachineInstSelectionAArch64::inst(IRInstICmpULT* node)
{
	// simpleCompareInst(node, MachineInstOpcodeAArch64::setb);
}

void MachineInstSelectionAArch64::inst(IRInstFCmpULT* node)
{
	// simpleCompareInst(node, MachineInstOpcodeAArch64::setb, MachineInstOpcodeAArch64::setnp, MachineInstOpcodeAArch64::and8);
}

void MachineInstSelectionAArch64::inst(IRInstICmpSGT* node)
{
	// simpleCompareInst(node, MachineInstOpcodeAArch64::setg);
}

void MachineInstSelectionAArch64::inst(IRInstICmpUGT* node)
{
	// simpleCompareInst(node, MachineInstOpcodeAArch64::seta);
}

void MachineInstSelectionAArch64::inst(IRInstFCmpUGT* node)
{
	// simpleCompareInst(node, MachineInstOpcodeAArch64::seta, MachineInstOpcodeAArch64::setnp, MachineInstOpcodeAArch64::and8);
}

void MachineInstSelectionAArch64::inst(IRInstICmpSLE* node)
{
	// simpleCompareInst(node, MachineInstOpcodeAArch64::setle);
}

void MachineInstSelectionAArch64::inst(IRInstICmpULE* node)
{
	// simpleCompareInst(node, MachineInstOpcodeAArch64::setbe);
}

void MachineInstSelectionAArch64::inst(IRInstFCmpULE* node)
{
	// simpleCompareInst(node, MachineInstOpcodeAArch64::setbe, MachineInstOpcodeAArch64::setnp, MachineInstOpcodeAArch64::and8);
}

void MachineInstSelectionAArch64::inst(IRInstICmpSGE* node)
{
	// simpleCompareInst(node, MachineInstOpcodeAArch64::setge);
}

void MachineInstSelectionAArch64::inst(IRInstICmpUGE* node)
{
	// simpleCompareInst(node, MachineInstOpcodeAArch64::setae);
}

void MachineInstSelectionAArch64::inst(IRInstFCmpUGE* node)
{
	// simpleCompareInst(node, MachineInstOpcodeAArch64::setae, MachineInstOpcodeAArch64::setnp, MachineInstOpcodeAArch64::and8);
}

void MachineInstSelectionAArch64::inst(IRInstICmpEQ* node)
{
	// simpleCompareInst(node, MachineInstOpcodeAArch64::sete);
}

void MachineInstSelectionAArch64::inst(IRInstFCmpUEQ* node)
{
	// simpleCompareInst(node, MachineInstOpcodeAArch64::sete, MachineInstOpcodeAArch64::setnp, MachineInstOpcodeAArch64::and8);
}

void MachineInstSelectionAArch64::inst(IRInstICmpNE* node)
{
	// simpleCompareInst(node, MachineInstOpcodeAArch64::setne);
}

void MachineInstSelectionAArch64::inst(IRInstFCmpUNE* node)
{
	// simpleCompareInst(node, MachineInstOpcodeAArch64::setne, MachineInstOpcodeAArch64::setp, MachineInstOpcodeAArch64::or8);
}

void MachineInstSelectionAArch64::inst(IRInstAnd* node)
{
	static const MachineInstOpcodeAArch64 ops[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::and64, MachineInstOpcodeAArch64::and32, MachineInstOpcodeAArch64::and32, MachineInstOpcodeAArch64::and32 };
	simpleBinaryInst(node, ops);
}

void MachineInstSelectionAArch64::inst(IRInstOr* node)
{
	static const MachineInstOpcodeAArch64 ops[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::or64, MachineInstOpcodeAArch64::or32, MachineInstOpcodeAArch64::or32, MachineInstOpcodeAArch64::or32 };
	simpleBinaryInst(node, ops);
}

void MachineInstSelectionAArch64::inst(IRInstXor* node)
{
	static const MachineInstOpcodeAArch64 ops[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::xor64, MachineInstOpcodeAArch64::xor32, MachineInstOpcodeAArch64::xor32, MachineInstOpcodeAArch64::xor32 };
	simpleBinaryInst(node, ops);
}

void MachineInstSelectionAArch64::inst(IRInstTrunc* node)
{
	static const MachineInstOpcodeAArch64 movOps[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::mov64, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32 };

	int dataSizeType = getDataSizeType(node->type);

	auto dst = newReg(node);
	emitInst(movOps[dataSizeType], dst, node->value, dataSizeType);
}

void MachineInstSelectionAArch64::inst(IRInstZExt* node)
{
	static const MachineInstOpcodeAArch64 movsxOps[] =
	{
		MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop,
		MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop,
		MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::mov64, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop,
		MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop,
		MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::movzx16_64, MachineInstOpcodeAArch64::movzx16_32, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::nop,
		MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::movzx8_64, MachineInstOpcodeAArch64::movzx8_32, MachineInstOpcodeAArch64::movzx8_32, MachineInstOpcodeAArch64::mov32,
	};

	int dstDataSizeType = getDataSizeType(node->type);
	int srcDataSizeType = getDataSizeType(node->value->type);
	auto dst = newReg(node);

	if (dstDataSizeType == 2 && srcDataSizeType == 3)
	{
		emitInst(MachineInstOpcodeAArch64::xor64, dst, dst);
	}

	emitInst(movsxOps[srcDataSizeType * 6 + dstDataSizeType], dst, node->value, srcDataSizeType);
}

void MachineInstSelectionAArch64::inst(IRInstSExt* node)
{
	static const MachineInstOpcodeAArch64 movsxOps[] =
	{
		MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop,
		MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop,
		MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::mov64, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop,
		MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::movsx32_64, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop,
		MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::movsx16_64, MachineInstOpcodeAArch64::movsx16_32, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::nop,
		MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::movsx8_64, MachineInstOpcodeAArch64::movsx8_32, MachineInstOpcodeAArch64::movsx8_32, MachineInstOpcodeAArch64::mov32,
	};

	int dstDataSizeType = getDataSizeType(node->type);
	int srcDataSizeType = getDataSizeType(node->value->type);
	auto dst = newReg(node);
	emitInst(movsxOps[srcDataSizeType * 6 + dstDataSizeType], dst, node->value, srcDataSizeType);
}

void MachineInstSelectionAArch64::inst(IRInstFPTrunc* node)
{
	auto dst = newReg(node);
	emitInst(isConstantFP(node->value) ? MachineInstOpcodeAArch64::movss : MachineInstOpcodeAArch64::cvtsd2ss, dst, node->value, 1);
}

void MachineInstSelectionAArch64::inst(IRInstFPExt* node)
{
	auto dst = newReg(node);
	emitInst(isConstantFP(node->value) ? MachineInstOpcodeAArch64::movsd : MachineInstOpcodeAArch64::cvtss2sd, dst, node->value, 0);
}

void MachineInstSelectionAArch64::inst(IRInstFPToUI* node)
{
	if (isConstantFP(node->value))
	{
		static const MachineInstOpcodeAArch64 movOps[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::mov64, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32 };
		int dstDataSizeType = getDataSizeType(node->type);

		auto dst = newReg(node);
		emitInst(movOps[dstDataSizeType], dst, newImm((uint64_t)getConstantValueDouble(node->value)));
	}
	else
	{
		static const MachineInstOpcodeAArch64 cvtOps[] = { MachineInstOpcodeAArch64::cvttsd2si, MachineInstOpcodeAArch64::cvttsd2si, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop };
		int srcDataSizeType = getDataSizeType(node->value->type);

		auto dst = newReg(node);
		emitInst(cvtOps[srcDataSizeType], dst, node->value, srcDataSizeType);
	}
}

void MachineInstSelectionAArch64::inst(IRInstFPToSI* node)
{
	if (isConstantFP(node->value))
	{
		static const MachineInstOpcodeAArch64 movOps[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::mov64, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32 };
		int dstDataSizeType = getDataSizeType(node->type);

		auto dst = newReg(node);
		emitInst(movOps[dstDataSizeType], dst, newImm((uint64_t)(int64_t)getConstantValueDouble(node->value)));
	}
	else
	{
		static const MachineInstOpcodeAArch64 cvtOps[] = { MachineInstOpcodeAArch64::cvttsd2si, MachineInstOpcodeAArch64::cvttss2si, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop };
		int srcDataSizeType = getDataSizeType(node->value->type);

		auto dst = newReg(node);
		emitInst(cvtOps[srcDataSizeType], dst, node->value, srcDataSizeType);
	}
}

void MachineInstSelectionAArch64::inst(IRInstUIToFP* node)
{
	static const MachineInstOpcodeAArch64 movOps[] = { MachineInstOpcodeAArch64::movsd, MachineInstOpcodeAArch64::movss, MachineInstOpcodeAArch64::mov64, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32 };
	static const MachineInstOpcodeAArch64 cvtOps[] = { MachineInstOpcodeAArch64::cvtsi2sd, MachineInstOpcodeAArch64::cvtsi2sd, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop };
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

		emitInst(MachineInstOpcodeAArch64::xor64, tmp, tmp);
		emitInst(movOps[srcDataSizeType], tmp, node->value, srcDataSizeType);
		emitInst(cvtOps[dstDataSizeType], dst, tmp);
	}
	else
	{
		auto dst = newReg(node);
		emitInst(cvtOps[dstDataSizeType], dst, node->value, dstDataSizeType);
	}
}

void MachineInstSelectionAArch64::inst(IRInstSIToFP* node)
{
	static const MachineInstOpcodeAArch64 movOps[] = { MachineInstOpcodeAArch64::movsd, MachineInstOpcodeAArch64::movss, MachineInstOpcodeAArch64::mov64, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32 };
	static const MachineInstOpcodeAArch64 cvtOps[] = { MachineInstOpcodeAArch64::cvtsi2sd, MachineInstOpcodeAArch64::cvtsi2ss, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop };
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
		emitInst((dstDataSizeType == 0) ? MachineInstOpcodeAArch64::shl64 : MachineInstOpcodeAArch64::shl32, tmp, count);
		emitInst((dstDataSizeType == 0) ? MachineInstOpcodeAArch64::sar64 : MachineInstOpcodeAArch64::sar32, tmp, count);
		emitInst(cvtOps[dstDataSizeType], dst, tmp);
	}
	else
	{
		auto dst = newReg(node);
		emitInst(cvtOps[dstDataSizeType], dst, node->value, dstDataSizeType);
	}
}

void MachineInstSelectionAArch64::inst(IRInstBitCast* node)
{
	if (isConstant(node->value))
	{
		static const MachineInstOpcodeAArch64 movOps[] = { MachineInstOpcodeAArch64::movsd, MachineInstOpcodeAArch64::movss, MachineInstOpcodeAArch64::mov64, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32 };

		int dataSizeType = getDataSizeType(node->value->type);
		auto dst = newReg(node);

		emitInst(movOps[dataSizeType], dst, node->value, dataSizeType);
	}
	else
	{
		instRegister[node] = instRegister[node->value];
	}
}

void MachineInstSelectionAArch64::inst(IRInstCall* node)
{
#ifdef WIN32
	callWin64(node);
#else
	callUnix64(node);
#endif
}

void MachineInstSelectionAArch64::inst(IRInstGEP* node)
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
		emitInst(MachineInstOpcodeAArch64::mov64, dst, node->ptr, getDataSizeType(node->ptr->type));
	}
}

void MachineInstSelectionAArch64::inst(IRInstBr* node)
{
	emitPhi(node->bb);
	emitInst(MachineInstOpcodeAArch64::b, node->bb);
}

void MachineInstSelectionAArch64::inst(IRInstCondBr* node)
{
	if (isConstant(node->condition))
	{
		if (getConstantValueInt(node->condition))
		{
			emitPhi(node->bb1);
			emitInst(MachineInstOpcodeAArch64::b, node->bb1);
		}
		else
		{
			emitPhi(node->bb2);
			emitInst(MachineInstOpcodeAArch64::b, node->bb2);
		}
	}
	else
	{
		emitPhi(node->bb1);
		emitPhi(node->bb2);
		emitInst(MachineInstOpcodeAArch64::cseteq, node->condition, 3, newImm(1));
		emitInst(MachineInstOpcodeAArch64::bne, node->bb2);
		emitInst(MachineInstOpcodeAArch64::b, node->bb1);
	}
}

void MachineInstSelectionAArch64::inst(IRInstRet* node)
{
	static const MachineInstOpcodeAArch64 movOps[] = { MachineInstOpcodeAArch64::movsd, MachineInstOpcodeAArch64::movss, MachineInstOpcodeAArch64::mov64, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32 };

	int dataSizeType = getDataSizeType(static_cast<IRFunctionType*>(sfunc->type)->returnType);
	MachineOperand dst = newPhysReg(dataSizeType < 2 ? RegisterNameAArch64::xmm0 : RegisterNameAArch64::x0);

	emitInst(movOps[dataSizeType], dst, node->operand, dataSizeType);
	emitInst(MachineInstOpcodeAArch64::b, mfunc->epilog);
}

void MachineInstSelectionAArch64::inst(IRInstRetVoid* node)
{
	emitInst(MachineInstOpcodeAArch64::b, mfunc->epilog);
}

void MachineInstSelectionAArch64::inst(IRInstAlloca* node)
{
	uint64_t size = node->type->getPointerElementType()->getTypeAllocSize() * getConstantValueInt(node->arraySize);
	size = (size + 15) / 16 * 16;

	// sub rsp,size
	emitInst(MachineInstOpcodeAArch64::sub64, newPhysReg(RegisterNameAArch64::sp), newImm(size));

	MachineOperand dst = newReg(node);
	MachineOperand src = newPhysReg(RegisterNameAArch64::sp);

	if (mfunc->maxCallArgsSize == 0)
	{
		// mov vreg,rsp
		emitInst(MachineInstOpcodeAArch64::mov64, dst, src);
	}
	else
	{
		// lea vreg,ptr[rsp+maxCallArgsSize]
		emitInst(MachineInstOpcodeAArch64::add64, dst, src, newImm(mfunc->maxCallArgsSize));
	}

	mfunc->dynamicStackAllocations = true;
	mfunc->registers[(int)RegisterNameAArch64::fp].cls = MachineRegClass::reserved;
}

void MachineInstSelectionAArch64::inst(IRInstPhi* node)
{
	// Handled in MachineInstSelectionAArch64::codegen
}

void MachineInstSelectionAArch64::emitPhi(IRBasicBlock* target)
{
	static const MachineInstOpcodeAArch64 movOps[] = { MachineInstOpcodeAArch64::movsd, MachineInstOpcodeAArch64::movss, MachineInstOpcodeAArch64::mov64, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32 };

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

int MachineInstSelectionAArch64::getDataSizeType(IRType* type)
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

void MachineInstSelectionAArch64::findMaxCallArgsSize()
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

void MachineInstSelectionAArch64::callWin64(IRInstCall* node)
{
}

void MachineInstSelectionAArch64::callUnix64(IRInstCall* node)
{
}

void MachineInstSelectionAArch64::simpleCompareInst(IRInstBinary* node, MachineInstOpcodeAArch64 opSet, MachineInstOpcodeAArch64 opSet2, MachineInstOpcodeAArch64 opSet3)
{
	static const MachineInstOpcodeAArch64 movOps[] = { MachineInstOpcodeAArch64::movsd, MachineInstOpcodeAArch64::movss, MachineInstOpcodeAArch64::mov64, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32 };
	static const MachineInstOpcodeAArch64 cmpOps[] = { MachineInstOpcodeAArch64::fcmpsd, MachineInstOpcodeAArch64::fcmpss, MachineInstOpcodeAArch64::cmp64_zx64, MachineInstOpcodeAArch64::cmp32_zx32, MachineInstOpcodeAArch64::cmp32_zx16, MachineInstOpcodeAArch64::cmp32_zx8 };

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

	if (opSet2 == MachineInstOpcodeAArch64::nop)
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

void MachineInstSelectionAArch64::simpleBinaryInst(IRInstBinary* node, const MachineInstOpcodeAArch64* binaryOps)
{
	static const MachineInstOpcodeAArch64 movOps[] = { MachineInstOpcodeAArch64::movsd, MachineInstOpcodeAArch64::movss, MachineInstOpcodeAArch64::mov64, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32 };

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

void MachineInstSelectionAArch64::shiftBinaryInst(IRInstBinary* node, const MachineInstOpcodeAArch64* binaryOps)
{
	simpleBinaryInst(node, binaryOps);
}

void MachineInstSelectionAArch64::divBinaryInst(IRInstBinary* node, const MachineInstOpcodeAArch64* binaryOps, bool remainder, bool zeroext)
{
	/*
	static const MachineInstOpcodeAArch64 movOps[] = { MachineInstOpcodeAArch64::movsd, MachineInstOpcodeAArch64::movss, MachineInstOpcodeAArch64::mov64, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov32, MachineInstOpcodeAArch64::mov8 };
	static const MachineInstOpcodeAArch64 sarOps[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::sar64, MachineInstOpcodeAArch64::sar32, MachineInstOpcodeAArch64::sar16, MachineInstOpcodeAArch64::sar8 };
	static const MachineInstOpcodeAArch64 xorOps[] = { MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::nop, MachineInstOpcodeAArch64::xor64, MachineInstOpcodeAArch64::xor32, MachineInstOpcodeAArch64::xor16, MachineInstOpcodeAArch64::xor8 };
	static const int shiftcount[] = { 0, 0, 63, 31, 15, 7 };

	int dataSizeType = getDataSizeType(node->type);

	auto dst = newReg(node);

	// Move operand1 to rdx:rax
	emitInst(movOps[dataSizeType], newPhysReg(RegisterNameAArch64::rax), node->operand1, dataSizeType);
	if (dataSizeType != 5)
	{
		if (zeroext)
		{
			emitInst(xorOps[dataSizeType], newPhysReg(RegisterNameAArch64::rdx), newPhysReg(RegisterNameAArch64::rdx));
		}
		else
		{
			emitInst(movOps[dataSizeType], newPhysReg(RegisterNameAArch64::rdx), newPhysReg(RegisterNameAArch64::rax));
			emitInst(sarOps[dataSizeType], newPhysReg(RegisterNameAArch64::rdx), newImm(shiftcount[dataSizeType]));
		}
	}
	else
	{
		emitInst(zeroext ? MachineInstOpcodeAArch64::movzx8_16 : MachineInstOpcodeAArch64::movsx8_16, newPhysReg(RegisterNameAArch64::rax), newPhysReg(RegisterNameAArch64::rax));
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
		emitInst(movOps[dataSizeType], dst, newPhysReg(remainder ? RegisterNameAArch64::rdx : RegisterNameAArch64::rax));
	}
	else // for 8 bit the remainder is in ah
	{
		emitInst(MachineInstOpcodeAArch64::shr16, newPhysReg(RegisterNameAArch64::rax), newImm(8));
		emitInst(MachineInstOpcodeAArch64::mov8, dst, newPhysReg(RegisterNameAArch64::rax));
	}
	*/
}

void MachineInstSelectionAArch64::addDebugInfo(MachineInst* inst)
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

void MachineInstSelectionAArch64::emitInst(MachineInstOpcodeAArch64 opcode, const MachineOperand& operand1, const MachineOperand& operand2, const MachineOperand& operand3)
{
	auto inst = context->newMachineInst();
	addDebugInfo(inst);
	inst->opcode = (int)opcode;
	inst->operands.push_back(operand1);
	inst->operands.push_back(operand2);
	inst->operands.push_back(operand3);
	bb->code.push_back(inst);
}

void MachineInstSelectionAArch64::emitInst(MachineInstOpcodeAArch64 opcode, const MachineOperand& operand1, IRValue* operand2, int dataSizeType, const MachineOperand& operand3)
{
	auto inst = context->newMachineInst();
	addDebugInfo(inst);
	inst->opcode = (int)opcode;
	inst->operands.push_back(operand1);
	pushValueOperand(inst, operand2, dataSizeType);
	inst->operands.push_back(operand3);
	bb->code.push_back(inst);
}

void MachineInstSelectionAArch64::emitInst(MachineInstOpcodeAArch64 opcode, const MachineOperand& operand1, const MachineOperand& operand2)
{
	auto inst = context->newMachineInst();
	addDebugInfo(inst);
	inst->opcode = (int)opcode;
	inst->operands.push_back(operand1);
	inst->operands.push_back(operand2);
	bb->code.push_back(inst);
}

void MachineInstSelectionAArch64::emitInst(MachineInstOpcodeAArch64 opcode, const MachineOperand& operand1, IRValue* operand2, int dataSizeType)
{
	auto inst = context->newMachineInst();
	addDebugInfo(inst);
	inst->opcode = (int)opcode;
	inst->operands.push_back(operand1);
	pushValueOperand(inst, operand2, dataSizeType);
	bb->code.push_back(inst);
}

void MachineInstSelectionAArch64::emitInst(MachineInstOpcodeAArch64 opcode, IRValue* operand1, int dataSizeType, const MachineOperand& operand2)
{
	auto inst = context->newMachineInst();
	addDebugInfo(inst);
	inst->opcode = (int)opcode;
	pushValueOperand(inst, operand1, dataSizeType);
	inst->operands.push_back(operand2);
	bb->code.push_back(inst);
}

void MachineInstSelectionAArch64::emitInst(MachineInstOpcodeAArch64 opcode, const MachineOperand& operand)
{
	auto inst = context->newMachineInst();
	addDebugInfo(inst);
	inst->opcode = (int)opcode;
	inst->operands.push_back(operand);
	bb->code.push_back(inst);
}

void MachineInstSelectionAArch64::emitInst(MachineInstOpcodeAArch64 opcode, IRValue* operand, int dataSizeType)
{
	auto inst = context->newMachineInst();
	addDebugInfo(inst);
	inst->opcode = (int)opcode;
	pushValueOperand(inst, operand, dataSizeType);
	bb->code.push_back(inst);
}

void MachineInstSelectionAArch64::emitInst(MachineInstOpcodeAArch64 opcode, IRBasicBlock* target)
{
	auto inst = context->newMachineInst();
	addDebugInfo(inst);
	inst->opcode = (int)opcode;
	pushBBOperand(inst, target);
	bb->code.push_back(inst);
}

void MachineInstSelectionAArch64::emitInst(MachineInstOpcodeAArch64 opcode, MachineBasicBlock* target)
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

void MachineInstSelectionAArch64::pushValueOperand(MachineInst* inst, IRValue* operand, int dataSizeType)
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

void MachineInstSelectionAArch64::pushBBOperand(MachineInst* inst, IRBasicBlock* bb)
{
	MachineOperand operand;
	operand.type = MachineOperandType::basicblock;
	operand.bb = bbMap[bb];
	inst->operands.push_back(operand);
}

MachineOperand MachineInstSelectionAArch64::newReg(IRValue* node)
{
	MachineOperand operand;
	operand.type = MachineOperandType::reg;
	operand.registerIndex = createVirtReg(getDataSizeType(node->type) < 2 ? MachineRegClass::xmm : MachineRegClass::gp);
	instRegister[node] = operand;
	return operand;
}

MachineOperand MachineInstSelectionAArch64::newPhysReg(RegisterNameAArch64 name)
{
	MachineOperand operand;
	operand.type = MachineOperandType::reg;
	operand.registerIndex = (int)name;
	return operand;
}

MachineOperand MachineInstSelectionAArch64::newTempReg(MachineRegClass cls)
{
	MachineOperand operand;
	operand.type = MachineOperandType::reg;
	operand.registerIndex = createVirtReg(cls);
	return operand;
}

int MachineInstSelectionAArch64::createVirtReg(MachineRegClass cls)
{
	int registerIndex = (int)mfunc->registers.size();
	mfunc->registers.push_back(cls);
	return registerIndex;
}

MachineOperand MachineInstSelectionAArch64::newConstant(uint64_t address)
{
	return newConstant(&address, sizeof(uint64_t));
}

MachineOperand MachineInstSelectionAArch64::newConstant(float value)
{
	return newConstant(&value, sizeof(float));
}

MachineOperand MachineInstSelectionAArch64::newConstant(double value)
{
	return newConstant(&value, sizeof(double));
}

MachineOperand MachineInstSelectionAArch64::newConstant(const void* data, int size)
{
	int index = (int)mfunc->constants.size();
	mfunc->constants.push_back({ data, size });

	MachineOperand operand;
	operand.type = MachineOperandType::constant;
	operand.constantIndex = index;
	return operand;
}

MachineOperand MachineInstSelectionAArch64::newImm(uint64_t imm)
{
	MachineOperand operand;
	operand.type = MachineOperandType::imm;
	operand.immvalue = imm;
	return operand;
}

uint64_t MachineInstSelectionAArch64::getConstantValueInt(IRValue* value)
{
	if (isConstantInt(value))
		return static_cast<IRConstantInt*>(value)->value;
	else
		return 0;
}

float MachineInstSelectionAArch64::getConstantValueFloat(IRValue* value)
{
	if (isConstantFP(value))
		return static_cast<float>(static_cast<IRConstantFP*>(value)->value);
	else
		return 0.0f;
}

double MachineInstSelectionAArch64::getConstantValueDouble(IRValue* value)
{
	if (isConstantFP(value))
		return static_cast<IRConstantFP*>(value)->value;
	else
		return 0.0;
}

MachineFunction* MachineInstSelectionAArch64::dumpinstructions(IRFunction* sfunc)
{
	return nullptr;
}
