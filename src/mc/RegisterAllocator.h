#pragma once

#include "MachineInst.h"
#include <list>

enum class RegType
{
	gp,
	xmm,
	reserved
};

struct VirtualRegister
{
	RegType type = RegType::gp;
	int stackoffset = -1;
	int physreg = -1;
	bool spilled = true;
};

struct PhysicalRegister
{
	RegType type;
	int vreg = -1;

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

	void createPhysRegisters();

	void usePhysRegister(MachineOperand& operand);

	void useVirtRegister(MachineOperand& operand);
	void killVirtRegister(int vregIndex);

	void assignAllToStack();
	void setAllToStack();

	void assignVirt2Phys(int vregIndex, int pregIndex);
	void assignVirt2StackSlot(int vregIndex);

	IRContext* context;
	MachineFunction* func;

	std::vector<VirtualRegister> vregs;
	std::vector<PhysicalRegister> physregs;
	std::list<int> mruPhys[2];

	int nextStackOffset = 0;

	std::vector<MachineInst*> emittedInstructions;
};
