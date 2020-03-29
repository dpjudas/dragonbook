#pragma once

#include "ir/IR.h"
#include "MachineInst.h"

class MachineCodeHolder
{
public:
	void addFunction(IRFunction* func);
	void addFunction(IRFunction* func, const void* external);

	size_t codeSize() const { return code.size(); }
	size_t unwindDataSize() const { return unwindData.size(); }

	void relocate(void* codeDest, void* unwindDest) const;

	struct FunctionEntry
	{
		IRFunction* func = nullptr;
		const void* external = nullptr;

		size_t beginAddress = 0;
		size_t endAddress = 0;

		size_t beginUnwindData = 0;
		size_t endUnwindData = 0;
		unsigned int unixUnwindFunctionStart = 0;
	};

	const std::vector<FunctionEntry>& getFunctionTable() const { return functionTable; };

private:
	std::vector<uint8_t> code;
	std::vector<uint8_t> unwindData;
	std::vector<FunctionEntry> functionTable;
	std::map<IRFunction*, size_t> funcToTableIndex;

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
