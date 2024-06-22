#pragma once

#include "dragonbook/IR.h"
#include "MachineInstAArch64.h"

class MachineCodeHolder;

class MachineCodeWriterAArch64
{
public:
	MachineCodeWriterAArch64(MachineCodeHolder *codeholder, MachineFunction* sfunc) : codeholder(codeholder), sfunc(sfunc) { }
	void codegen();

	MachineFunction* getFunc() const { return sfunc; }

private:
	void basicblock(MachineBasicBlock* bb);
	void opcode(MachineInst* inst);

	void nop(MachineInst* inst);
	void loadss(MachineInst* inst);
	void loadsd(MachineInst* inst);
	void load64(MachineInst* inst);
	void load32(MachineInst* inst);
	void load16(MachineInst* inst);
	void load8(MachineInst* inst);
	void storess(MachineInst* inst);
	void storesd(MachineInst* inst);
	void store64(MachineInst* inst);
	void store32(MachineInst* inst);
	void store16(MachineInst* inst);
	void store8(MachineInst* inst);
	void movss(MachineInst* inst);
	void movsd(MachineInst* inst);
	void mov64(MachineInst* inst);
	void mov32(MachineInst* inst);
	void movsx8_32(MachineInst* inst);
	void movsx8_64(MachineInst* inst);
	void movsx16_32(MachineInst* inst);
	void movsx16_64(MachineInst* inst);
	void movsx32_64(MachineInst* inst);
	void movzx8_32(MachineInst* inst);
	void movzx8_64(MachineInst* inst);
	void movzx16_32(MachineInst* inst);
	void movzx16_64(MachineInst* inst);
	void addss(MachineInst* inst);
	void addsd(MachineInst* inst);
	void add64(MachineInst* inst);
	void add32(MachineInst* inst);
	void subss(MachineInst* inst);
	void subsd(MachineInst* inst);
	void sub64(MachineInst* inst);
	void sub32(MachineInst* inst);
	void not64(MachineInst* inst);
	void not32(MachineInst* inst);
	void neg64(MachineInst* inst);
	void neg32(MachineInst* inst);
	void shl64(MachineInst* inst);
	void shl32(MachineInst* inst);
	void shr64(MachineInst* inst);
	void shr32(MachineInst* inst);
	void sar64(MachineInst* inst);
	void sar32(MachineInst* inst);
	void and64(MachineInst* inst);
	void and32(MachineInst* inst);
	void or64(MachineInst* inst);
	void or32(MachineInst* inst);
	void negss(MachineInst* inst);
	void negsd(MachineInst* inst);
	void xor64(MachineInst* inst);
	void xor32(MachineInst* inst);
	void mulss(MachineInst* inst);
	void mulsd(MachineInst* inst);
	void mul64(MachineInst* inst);
	void mul32(MachineInst* inst);
	void divss(MachineInst* inst);
	void divsd(MachineInst* inst);
	void sdiv64(MachineInst* inst);
	void sdiv32(MachineInst* inst);
	void udiv64(MachineInst* inst);
	void udiv32(MachineInst* inst);

	void writeOpcode(uint32_t opcode, MachineInst* debugInfo);

	std::pair<RegisterNameAArch64, uint32_t> getImmediateOffset(const MachineOperand& operand);

	static uint32_t getPhysReg(const MachineOperand& operand);
	static uint32_t getPhysReg(RegisterNameAArch64 name);

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

	MachineCodeHolder* codeholder;
	MachineFunction* sfunc;
	size_t funcBeginAddress = 0;
};
