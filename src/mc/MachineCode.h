#pragma once

#include "IR.h"
#include "MachineInst.h"

class FuncSignatureContainer;

class MachineCodeHolder
{
public:
	void addFunction(IRFunction* func);
	void addFunction(IRFunction* func, const void* external);

	size_t codeSize() const { return code.size(); }
	size_t relocate(void* dest) const;

private:
	std::vector<uint8_t> code;

	struct FunctionEntry
	{
		size_t pos = 0;
		const void* external = nullptr;
	};
	std::map<IRFunction*, FunctionEntry> functionTable;
	
	struct BBRelocateEntry
	{
		BBRelocateEntry() = default;
		BBRelocateEntry(size_t pos, MachineBasicBlock* bb) : pos(pos), bb(bb) { }

		size_t pos = 0;
		MachineBasicBlock* bb = nullptr;
	};

	struct CallRelocateEntry
	{
		CallRelocateEntry() = default;
		CallRelocateEntry(size_t pos, IRFunction* func) : pos(pos), func(func) { }

		size_t pos = 0;
		IRFunction* func = nullptr;
	};

	std::vector<CallRelocateEntry> callRelocateInfo;
	std::vector<BBRelocateEntry> bbRelocateInfo;
	std::map<MachineBasicBlock*, size_t> bbOffsets;

	friend class MachineCodeWriter;
};

class MachineCodeWriter
{
public:
	MachineCodeWriter(MachineCodeHolder *codeholder, MachineFunction* sfunc) : codeholder(codeholder), sfunc(sfunc) { }
	void codegen();

	MachineFunction* getFunc() const { return sfunc; }

private:
	void basicblock(MachineBasicBlock* bb);
	void opcode(MachineInst* inst);

	void nop(MachineInst* inst);
	void lea(MachineInst* inst);
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
	void mov16(MachineInst* inst);
	void mov8(MachineInst* inst);
	void addss(MachineInst* inst);
	void addsd(MachineInst* inst);
	void add64(MachineInst* inst);
	void add32(MachineInst* inst);
	void add16(MachineInst* inst);
	void add8(MachineInst* inst);
	void subss(MachineInst* inst);
	void subsd(MachineInst* inst);
	void sub64(MachineInst* inst);
	void sub32(MachineInst* inst);
	void sub16(MachineInst* inst);
	void sub8(MachineInst* inst);
	void not64(MachineInst* inst);
	void not32(MachineInst* inst);
	void not16(MachineInst* inst);
	void not8(MachineInst* inst);
	void neg64(MachineInst* inst);
	void neg32(MachineInst* inst);
	void neg16(MachineInst* inst);
	void neg8(MachineInst* inst);
	void shl64(MachineInst* inst);
	void shl32(MachineInst* inst);
	void shl16(MachineInst* inst);
	void shl8(MachineInst* inst);
	void sar64(MachineInst* inst);
	void sar32(MachineInst* inst);
	void sar16(MachineInst* inst);
	void sar8(MachineInst* inst);
	void and64(MachineInst* inst);
	void and32(MachineInst* inst);
	void and16(MachineInst* inst);
	void and8(MachineInst* inst);
	void or64(MachineInst* inst);
	void or32(MachineInst* inst);
	void or16(MachineInst* inst);
	void or8(MachineInst* inst);
	void xorpd(MachineInst* inst);
	void xorps(MachineInst* inst);
	void xor64(MachineInst* inst);
	void xor32(MachineInst* inst);
	void xor16(MachineInst* inst);
	void xor8(MachineInst* inst);
	void mulss(MachineInst* inst);
	void mulsd(MachineInst* inst);
	void imul64(MachineInst* inst);
	void imul32(MachineInst* inst);
	void imul16(MachineInst* inst);
	void imul8(MachineInst* inst);
	void divss(MachineInst* inst);
	void divsd(MachineInst* inst);
	void idiv64(MachineInst* inst);
	void idiv32(MachineInst* inst);
	void idiv16(MachineInst* inst);
	void idiv8(MachineInst* inst);
	void mul64(MachineInst* inst);
	void mul32(MachineInst* inst);
	void mul16(MachineInst* inst);
	void mul8(MachineInst* inst);
	void div64(MachineInst* inst);
	void div32(MachineInst* inst);
	void div16(MachineInst* inst);
	void div8(MachineInst* inst);
	void ucomisd(MachineInst* inst);
	void ucomiss(MachineInst* inst);
	void cmp64(MachineInst* inst);
	void cmp32(MachineInst* inst);
	void cmp16(MachineInst* inst);
	void cmp8(MachineInst* inst);
	void setl(MachineInst* inst);
	void setb(MachineInst* inst);
	void seta(MachineInst* inst);
	void setg(MachineInst* inst);
	void setle(MachineInst* inst);
	void setbe(MachineInst* inst);
	void setae(MachineInst* inst);
	void setge(MachineInst* inst);
	void sete(MachineInst* inst);
	void setne(MachineInst* inst);
	void cvtsd2ss(MachineInst* inst);
	void cvtss2sd(MachineInst* inst);
	void cvttsd2si(MachineInst* inst);
	void cvttss2si(MachineInst* inst);
	void cvtsi2sd(MachineInst* inst);
	void cvtsi2ss(MachineInst* inst);
	void jmp(MachineInst* inst);
	void jz(MachineInst* inst);
	void call(MachineInst* inst);
	void alloca_(MachineInst* inst);
	void ret(MachineInst* inst);
	void push(MachineInst* inst);
	void pop(MachineInst* inst);

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

	struct OpFlags
	{
		enum
		{
			Rex = 1,
			RexW = 2,
			SizeOverride = 4, // 0x66
			NP = 8 // No additional prefixes allowed
		};
	};

	void emitInstZO(int flags, int opcode);
	void emitInstO(int flags, int opcode, MachineInst* inst);
	void emitInstOI(int flags, int opcode, int immsize, MachineInst* inst);
	void emitInstMI(int flags, int opcode, int immsize, MachineInst* inst);
	void emitInstMI(int flags, int opcode, int modopcode, int immsize, MachineInst* inst);
	void emitInstMR(int flags, int opcode, MachineInst* inst);
	void emitInstM(int flags, int opcode, MachineInst* inst);
	void emitInstM(int flags, int opcode, int modopcode, MachineInst* inst);
	void emitInstM(int flags, std::initializer_list<int> opcode, MachineInst* inst);
	void emitInstM(int flags, std::initializer_list<int> opcode, int modopcode, MachineInst* inst);
	void emitInstMC(int flags, int opcode, MachineInst* inst);
	void emitInstMC(int flags, int opcode, int modopcode, MachineInst* inst);
	void emitInstRM(int flags, int opcode, MachineInst* inst);
	void emitInstRM(int flags, std::initializer_list<int> opcode, MachineInst* inst);
	void emitInstRMI(int flags, int opcode, int immsize, MachineInst* inst);
	void emitInstSSE_RM(int flags, std::initializer_list<int> opcode, MachineInst* inst);
	void emitInstSSE_MR(int flags, std::initializer_list<int> opcode, MachineInst* inst);

	void writeOpcode(int flags, std::initializer_list<int> opcode, int rexR, int rexX, int rexB);
	void writeModRM(int mod, int modreg, int rm);
	void writeSIB(int scale, int index, int base);
	void writeImm(int immsize, uint64_t value);

	static int getPhysReg(const MachineOperand& operand);

	MachineCodeHolder* codeholder;
	MachineFunction* sfunc;
};
