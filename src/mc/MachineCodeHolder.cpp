
#include "MachineCodeHolder.h"
#include "MachineCodeWriter.h"
#include "MachineInstSelection.h"
#include "RegisterAllocator.h"
#include "UnwindInfoWindows.h"
#include "UnwindInfoUnix.h"

void MachineCodeHolder::addFunction(IRFunction* func)
{
	if (!func->basicBlocks.empty())
	{
		FunctionEntry entry;
		entry.func = func;
		entry.beginAddress = code.size();

		MachineFunction* mcfunc = MachineInstSelection::codegen(func);
		RegisterAllocator::run(func->context, mcfunc);
		MachineCodeWriter mcwriter(this, mcfunc);
		mcwriter.codegen();

		entry.endAddress = code.size();

#ifdef WIN32
		std::vector<uint16_t> data = UnwindInfoWindows::create(mcfunc);
		entry.beginUnwindData = unwindData.size();
		unwindData.resize(unwindData.size() + data.size() * sizeof(uint16_t));
		memcpy(unwindData.data() + entry.beginUnwindData, data.data(), data.size() * sizeof(uint16_t));
		entry.endUnwindData = unwindData.size();
#else
		std::vector<uint8_t> data = UnwindInfoUnix::create(mcfunc, entry.unixUnwindFunctionStart);
		entry.beginUnwindData = unwindData.size();
		unwindData.resize(unwindData.size() + data.size());
		memcpy(unwindData.data() + entry.beginUnwindData, data.data(), data.size());
		entry.endUnwindData = unwindData.size();
#endif

		funcToTableIndex[entry.func] = functionTable.size();
		functionTable.push_back(entry);
	}
}

void MachineCodeHolder::addFunction(IRFunction* func, const void* external)
{
	FunctionEntry entry;
	entry.func = func;
	entry.external = external;

	funcToTableIndex[entry.func] = functionTable.size();
	functionTable.push_back(entry);
}

void MachineCodeHolder::relocate(void* codeDest, void* unwindDest) const
{
	memcpy(codeDest, code.data(), code.size());
	memcpy(unwindDest, unwindData.data(), unwindData.size());

#ifndef WIN32
	// Patch absolute address and size for each FDE entry in the .eh_frame
	uint8_t* unwindDest8 = (uint8_t*)unwindDest;
	for (const auto& entry : functionTable)
	{
		if (entry.beginUnwindData != entry.endUnwindData)
		{
			uint64_t* fdeFunc = (uint64_t*)(unwindDest8 + entry.beginUnwindData + entry.unixUnwindFunctionStart);
			fdeFunc[0] = (uint64_t)(ptrdiff_t)((uint8_t*)codeDest + entry.beginAddress);
			fdeFunc[1] = (uint64_t)(ptrdiff_t)(entry.endAddress - entry.beginAddress);
		}
	}
#endif

	uint8_t* d = (uint8_t*)codeDest;

	for (const auto& entry : bbRelocateInfo)
	{
		int32_t value = (int32_t)(bbOffsets.find(entry.bb)->second - entry.pos - 4);
		memcpy(d + entry.pos, &value, sizeof(int32_t));
	}

	for (const auto& entry : callRelocateInfo)
	{
		const auto& funcEntry = functionTable[funcToTableIndex.find(entry.func)->second];

		uint64_t value;
		if (!funcEntry.external)
		{
			value = (ptrdiff_t)d + funcEntry.beginAddress;
		}
		else
		{
			value = (ptrdiff_t)funcEntry.external;
		}

		memcpy(d + entry.pos, &value, sizeof(uint64_t));
	}
}
