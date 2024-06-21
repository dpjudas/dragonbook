#pragma once

#include "MachineInstX64.h"
#include <list>
#include <set>

class RARegisterLiveReferenceX64
{
public:
	RARegisterLiveReferenceX64(MachineBasicBlock* bb) : bb(bb) { }

	MachineBasicBlock* bb = nullptr;
	int refcount = 1;
	std::unique_ptr<RARegisterLiveReferenceX64> next;
};

struct RARegisterInfoX64
{
	static MachineOperand nullStackLocation() { MachineOperand op; op.type = MachineOperandType::spillOffset; op.spillOffset = -1; return op; }

	MachineRegClass cls = MachineRegClass::reserved;
	int vreg = -1;
	int physreg = -1;
	MachineOperand stacklocation = nullStackLocation();
	bool modified = false;
	bool stackvar = false;
	const std::string* name = nullptr;

	std::unique_ptr<RARegisterLiveReferenceX64> liveReferences;
};

struct RARegisterClassX64
{
	std::list<int> mru;
	std::list<int>::iterator mruIt[(int)RegisterNameX64::vregstart];
};

class RegisterAllocatorX64
{
public:
	static void run(IRContext* context, MachineFunction* func);

private:
	RegisterAllocatorX64(IRContext* context, MachineFunction* func) : context(context), func(func) { }
	void run();

	void allocStackVars();

	void setupArgsWin64();
	void setupArgsUnix64();

	void emitProlog(const std::vector<RegisterNameX64>& savedRegs, const std::vector<RegisterNameX64>& savedXmmRegs, int stackAdjustment, bool dsa);
	void emitEpilog(const std::vector<RegisterNameX64>& savedRegs, const std::vector<RegisterNameX64>& savedXmmRegs, int stackAdjustment, bool dsa);

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

	std::vector<RegisterNameX64> volatileRegs;
	std::set<RegisterNameX64> usedRegs;

	IRContext* context;
	MachineFunction* func;

	std::vector<RARegisterInfoX64> reginfo;
	RARegisterClassX64 regclass[2];

	int nextSpillOffset = 0;
	std::vector<int> freeSpillOffsets;

	std::vector<MachineInst*> emittedInstructions;
};
