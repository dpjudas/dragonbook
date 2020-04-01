#pragma once

#include "MachineInst.h"
#include <list>

struct RARegisterInfo
{
	MachineRegClass cls = MachineRegClass::reserved;
	int vreg = -1;
	int physreg = -1;
	int stackoffset = -1;
	bool spilled = true;
	std::list<int>::iterator mruIt;
};

class RegisterAllocator
{
public:
	static void run(IRContext* context, MachineFunction* func);

private:
	RegisterAllocator(IRContext* context, MachineFunction* func) : context(context), func(func) { }
	void run();

	void emitProlog(const std::vector<RegisterName>& savedRegs, const std::vector<RegisterName>& savedXmmRegs, int stackSize);
	void emitEpilog(const std::vector<RegisterName>& savedRegs, const std::vector<RegisterName>& savedXmmRegs, int stackSize);

	bool isFloat(IRType* type) const { return dynamic_cast<IRFloatType*>(type); }
	bool isDouble(IRType* type) const { return dynamic_cast<IRDoubleType*>(type); }

	void createRegisterInfo();

	void usePhysRegister(MachineOperand& operand);

	void useVirtRegister(MachineOperand& operand);
	void killVirtRegister(int vregIndex);

	void assignAllToStack();
	void setAllToStack();

	void assignVirt2Phys(int vregIndex, int pregIndex);
	void assignVirt2StackSlot(int vregIndex);

	IRContext* context;
	MachineFunction* func;

	std::vector<RARegisterInfo> reginfo;
	std::list<int> mruPhys[2];

	int nextStackOffset = 0;

	std::vector<MachineInst*> emittedInstructions;
};
