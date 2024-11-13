#pragma once

#include "MachineInstAArch64.h"
#include <list>
#include <set>

class RARegisterLiveReferenceAArch64
{
public:
	RARegisterLiveReferenceAArch64(MachineBasicBlock* bb) : bb(bb) { }

	MachineBasicBlock* bb = nullptr;
	int refcount = 1;
	std::unique_ptr<RARegisterLiveReferenceAArch64> next;
};

struct RARegisterInfoAArch64
{
	static MachineOperand nullStackLocation() { MachineOperand op; op.type = MachineOperandType::spillOffset; op.spillOffset = -1; return op; }

	MachineRegClass cls = MachineRegClass::reserved;
	int vreg = -1;
	int physreg = -1;
	MachineOperand stacklocation = nullStackLocation();
	bool modified = false;
	bool stackvar = false;
	const std::string* name = nullptr;

	std::unique_ptr<RARegisterLiveReferenceAArch64> liveReferences;
};

struct RARegisterClassAArch64
{
	std::list<int> mru;
	std::list<int>::iterator mruIt[(int)RegisterNameAArch64::vregstart];
};

class RegisterAllocatorAArch64
{
public:
	static void run(IRContext* context, MachineFunction* func);

private:
	RegisterAllocatorAArch64(IRContext* context, MachineFunction* func) : context(context), func(func) { }
	void run();

	void allocStackVars();

	void setupArgs();

	void emitProlog(const std::vector<RegisterNameAArch64>& savedRegs, const std::vector<RegisterNameAArch64>& savedXmmRegs, int stackAdjustment, bool dsa);
	void emitEpilog(const std::vector<RegisterNameAArch64>& savedRegs, const std::vector<RegisterNameAArch64>& savedXmmRegs, int stackAdjustment, bool dsa);

	bool isFloat(IRType* type) const { return dynamic_cast<IRFloatType*>(type); }
	bool isDouble(IRType* type) const { return dynamic_cast<IRDoubleType*>(type); }

	void createRegisterInfo();

	void useRegister(MachineOperand& operand);
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

	void updateModifiedStatus(MachineInst* inst);

	std::string getVRegName(size_t vregIndex);

	static int alignUp(int size, int alignment) { return (size + alignment - 1) / alignment * alignment; }

	std::vector<RegisterNameAArch64> volatileRegs;
	std::set<RegisterNameAArch64> usedRegs;

	IRContext* context;
	MachineFunction* func;

	std::vector<RARegisterInfoAArch64> reginfo;
	RARegisterClassAArch64 regclass[2];

	int nextSpillOffset = 0;
	std::vector<int> freeSpillOffsets;

	std::vector<MachineInst*> emittedInstructions;
};
