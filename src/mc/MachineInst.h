#pragma once

#include <cstdint>
#include <vector>
#include "ir/IR.h"

class MachineBasicBlock;

enum class MachineInstOpcode
{
	nop,
	lea,
	loadss, loadsd, load64, load32, load16, load8, // mov reg,ptr
	storess, storesd, store64, store32, store16, store8, // mov ptr,reg
	movss, movsd, mov64, mov32, mov16, mov8,
	addss, addsd, add64, add32, add16, add8,
	subss, subsd, sub64, sub32, sub16, sub8,
	not64, not32, not16, not8,
	neg64, neg32, neg16, neg8,
	shl64, shl32, shl16, shl8,
	sar64, sar32, sar16, sar8,
	and64, and32, and16, and8,
	or64, or32, or16, or8,
	xorpd, xorps, xor64, xor32, xor16, xor8,
	mulss, mulsd, imul64, imul32, imul16, imul8,
	divss, divsd, idiv64, idiv32, idiv16, idiv8,
	mul64, mul32, mul16, mul8,
	div64, div32, div16, div8,
	ucomisd, ucomiss, cmp64, cmp32, cmp16, cmp8,
	setl, setb, seta, setg, setle, setbe, setae, setge, sete, setne,
	cvtsd2ss, cvtss2sd,
	cvttsd2si, cvttss2si,
	cvtsi2sd, cvtsi2ss,
	jmp, jz,
	call, alloca_, ret, push, pop
};

enum class MachineOperandType
{
	reg, constant, stack, imm, basicblock, func, global
};

class MachineOperand
{
public:
	MachineOperandType type;
	union
	{
		int registerIndex;
		int constantIndex;
		int stackOffset;
		uint64_t immvalue;
		MachineBasicBlock* bb;
		IRFunction* func;
		IRGlobalVariable* global;
	};
};

enum class MachineUnwindHint
{
	None,
	PushNonvolatile,
	StackAdjustment,
	RegisterStackLocation
};

class MachineInst
{
public:
	MachineInstOpcode opcode;
	std::vector<MachineOperand> operands;

	int unwindOffset = -1;
	MachineUnwindHint unwindHint = MachineUnwindHint::None;
};

class MachineBasicBlock
{
public:
	std::vector<MachineInst*> code;
};

class MachineConstant
{
public:
	MachineConstant(const void* value, int size) : size(size)
	{
		if (size > maxsize)
			throw std::runtime_error("Constant data type too large");
		memcpy(data, value, size);
		memset(data + size, 0, maxsize - size);
	}

	enum { maxsize = 16 };
	uint8_t data[maxsize];
	int size;
};

class MachineFunction
{
public:
	IRFunctionType* type = nullptr;
	MachineBasicBlock* prolog = nullptr;
	MachineBasicBlock* epilog = nullptr;
	std::vector<MachineBasicBlock*> basicBlocks;
	std::vector<MachineConstant> constants;
	int stackSize = 0;
};

enum class RegisterName : int
{
	rax,
	rcx,
	rdx,
	rbx,
	rsp,
	rbp,
	rsi,
	rdi,
	r8,
	r9,
	r10,
	r11,
	r12,
	r13,
	r14,
	r15,
	xmm0,
	xmm1,
	xmm2,
	xmm3,
	xmm4,
	xmm5,
	xmm6,
	xmm7,
	xmm8,
	xmm9,
	xmm10,
	xmm11,
	xmm12,
	xmm13,
	xmm14,
	xmm15,
	vregstart = 1024
};

class MachineInstSelection : IRInstVisitor
{
public:
	static MachineFunction *codegen(IRFunction* function);

private:
	MachineInstSelection(IRFunction* sfunc) : context(sfunc->context), sfunc(sfunc) { }

	void inst(IRInstLoad* node) override;
	void inst(IRInstStore* node) override;
	void inst(IRInstAdd* node) override;
	void inst(IRInstSub* node) override;
	void inst(IRInstFAdd* node) override;
	void inst(IRInstFSub* node) override;
	void inst(IRInstNot* node) override;
	void inst(IRInstNeg* node) override;
	void inst(IRInstFNeg* node) override;
	void inst(IRInstMul* node) override;
	void inst(IRInstFMul* node) override;
	void inst(IRInstSDiv* node) override;
	void inst(IRInstUDiv* node) override;
	void inst(IRInstFDiv* node) override;
	void inst(IRInstSRem* node) override;
	void inst(IRInstURem* node) override;
	void inst(IRInstFRem* node) override;
	void inst(IRInstShl* node) override;
	void inst(IRInstAShr* node) override;
	void inst(IRInstICmpSLT* node) override;
	void inst(IRInstICmpULT* node) override;
	void inst(IRInstFCmpULT* node) override;
	void inst(IRInstICmpSGT* node) override;
	void inst(IRInstICmpUGT* node) override;
	void inst(IRInstFCmpUGT* node) override;
	void inst(IRInstICmpSLE* node) override;
	void inst(IRInstICmpULE* node) override;
	void inst(IRInstFCmpULE* node) override;
	void inst(IRInstICmpSGE* node) override;
	void inst(IRInstICmpUGE* node) override;
	void inst(IRInstFCmpUGE* node) override;
	void inst(IRInstICmpEQ* node) override;
	void inst(IRInstFCmpUEQ* node) override;
	void inst(IRInstICmpNE* node) override;
	void inst(IRInstFCmpUNE* node) override;
	void inst(IRInstAnd* node) override;
	void inst(IRInstOr* node) override;
	void inst(IRInstXor* node) override;
	void inst(IRInstTrunc* node) override;
	void inst(IRInstZExt* node) override;
	void inst(IRInstSExt* node) override;
	void inst(IRInstFPTrunc* node) override;
	void inst(IRInstFPExt* node) override;
	void inst(IRInstFPToUI* node) override;
	void inst(IRInstFPToSI* node) override;
	void inst(IRInstUIToFP* node) override;
	void inst(IRInstSIToFP* node) override;
	void inst(IRInstBitCast* node) override;
	void inst(IRInstCall* node) override;
	void inst(IRInstGEP* node) override;
	void inst(IRInstMemSet* node) override;
	void inst(IRInstBr* node) override;
	void inst(IRInstCondBr* node) override;
	void inst(IRInstRet* node) override;
	void inst(IRInstRetVoid* node) override;
	void inst(IRInstAlloca* node) override;

	bool isConstant(IRValue* value) const { return dynamic_cast<IRConstant*>(value); }
	bool isConstantInt(IRValue* value) const { return dynamic_cast<IRConstantInt*>(value); }
	bool isConstantFP(IRValue* value) const { return dynamic_cast<IRConstantFP*>(value); }
	bool isFunctionArg(IRValue* value) const { return dynamic_cast<IRFunctionArg*>(value); }
	bool isGlobalVariable(IRValue* value) const { return dynamic_cast<IRGlobalVariable*>(value); }

	bool isVoid(IRType* type) const { return dynamic_cast<IRVoidType*>(type); }
	bool isInt1(IRType* type) const { return dynamic_cast<IRInt1Type*>(type); }
	bool isInt8(IRType* type) const { return dynamic_cast<IRInt8Type*>(type); }
	bool isInt16(IRType* type) const { return dynamic_cast<IRInt16Type*>(type); }
	bool isInt32(IRType* type) const { return dynamic_cast<IRInt32Type*>(type); }
	bool isInt64(IRType* type) const { return dynamic_cast<IRInt64Type*>(type); }
	bool isFloat(IRType* type) const { return dynamic_cast<IRFloatType*>(type); }
	bool isDouble(IRType* type) const { return dynamic_cast<IRDoubleType*>(type); }
	bool isPointer(IRType* type) const { return dynamic_cast<IRPointerType*>(type); }
	bool isFunction(IRType* type) const { return dynamic_cast<IRFunctionType*>(type); }
	bool isStruct(IRType* type) const { return dynamic_cast<IRStructType*>(type); }

	void simpleCompareInst(IRInstBinary* node, MachineInstOpcode opSet);
	void simpleBinaryInst(IRInstBinary* node, const MachineInstOpcode* binaryOps);
	void pushValueOperand(MachineInst* inst, IRValue* operand, int dataSizeType);
	void pushBBOperand(MachineInst* inst, IRBasicBlock* bb);

	int getDataSizeType(IRType* type);

	uint64_t getConstantValueInt(IRValue* value);
	float getConstantValueFloat(IRValue* value);
	double getConstantValueDouble(IRValue* value);

	MachineOperand newReg(IRValue* node);
	MachineOperand newConstant(uint64_t address);
	MachineOperand newConstant(float value);
	MachineOperand newConstant(double value);
	MachineOperand newConstant(const void *data, int size);
	MachineOperand newImm(uint64_t imm);
	MachineOperand newPhysReg(RegisterName name);
	MachineOperand newTempReg();

	IRContext* context;
	IRFunction* sfunc;
	MachineFunction* mfunc = nullptr;
	MachineBasicBlock* bb = nullptr;
	std::map<IRBasicBlock*, MachineBasicBlock*> bbMap;

	std::map<IRValue*, MachineOperand> instRegister;
	int nextRegIndex = (int)RegisterName::vregstart;
};
