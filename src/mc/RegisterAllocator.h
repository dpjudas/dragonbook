#pragma once

#include "MachineInst.h"
#include <list>

class RARegisterLiveReference
{
public:
	RARegisterLiveReference(MachineBasicBlock* bb) : bb(bb) { }

	MachineBasicBlock* bb = nullptr;
	int refcount = 1;
	std::unique_ptr<RARegisterLiveReference> next;
};

struct RARegisterInfo
{
	MachineRegClass cls = MachineRegClass::reserved;
	int vreg = -1;
	int physreg = -1;
	int stackoffset = -1;
	bool spilled = true;

	std::unique_ptr<RARegisterLiveReference> liveReferences;
};

struct RARegisterClass
{
	std::list<int> mru;
	std::list<int>::iterator mruIt[(int)RegisterName::vregstart];
};

class RegisterAllocator
{
public:
	static void run(IRContext* context, MachineFunction* func);

private:
	RegisterAllocator(IRContext* context, MachineFunction* func) : context(context), func(func) { }
	void run();

	void emitProlog(const std::vector<RegisterName>& savedRegs, const std::vector<RegisterName>& savedXmmRegs, int stackSize, bool dsa);
	void emitEpilog(const std::vector<RegisterName>& savedRegs, const std::vector<RegisterName>& savedXmmRegs, int stackSize, bool dsa);

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

	void setAsMostRecentlyUsed(int pregIndex);
	void setAsLeastRecentlyUsed(int pregIndex);
	int getLeastRecentlyUsed(MachineRegClass cls);

	void runLiveAnalysis();
	void addLiveReference(size_t vregIndex, MachineBasicBlock* bb);

	IRContext* context;
	MachineFunction* func;

	std::vector<RARegisterInfo> reginfo;
	RARegisterClass regclass[2];

	int nextStackOffset = 0;
	std::vector<int> freeStackOffsets;

	std::vector<MachineInst*> emittedInstructions;
};
