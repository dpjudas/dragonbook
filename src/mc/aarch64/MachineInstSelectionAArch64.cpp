
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
}

void MachineInstSelectionAArch64::inst(IRInstStore* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstAdd* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstSub* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstFAdd* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstFSub* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstNot* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstNeg* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstFNeg* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstMul* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstFMul* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstSDiv* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstUDiv* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstFDiv* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstSRem* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstURem* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstShl* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstLShr* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstAShr* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstICmpSLT* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstICmpULT* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstFCmpULT* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstICmpSGT* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstICmpUGT* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstFCmpUGT* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstICmpSLE* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstICmpULE* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstFCmpULE* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstICmpSGE* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstICmpUGE* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstFCmpUGE* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstICmpEQ* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstFCmpUEQ* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstICmpNE* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstFCmpUNE* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstAnd* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstOr* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstXor* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstTrunc* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstZExt* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstSExt* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstFPTrunc* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstFPExt* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstFPToUI* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstFPToSI* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstUIToFP* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstSIToFP* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstBitCast* node)
{
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
}

void MachineInstSelectionAArch64::inst(IRInstBr* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstCondBr* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstRet* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstRetVoid* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstAlloca* node)
{
}

void MachineInstSelectionAArch64::inst(IRInstPhi* node)
{
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
