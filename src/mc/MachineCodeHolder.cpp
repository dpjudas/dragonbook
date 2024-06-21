
#include "MachineCodeHolder.h"
#include "MachineCodeWriterX64.h"
#include "MachineInstSelectionX64.h"
#include "RegisterAllocatorX64.h"
#include "UnwindInfoWindowsX64.h"
#include "UnwindInfoUnixX64.h"

void MachineCodeHolder::addFunction(IRFunction* func)
{
	if (!func->basicBlocks.empty())
	{
		FunctionEntry entry;
		entry.func = func;
		entry.beginAddress = code.size();

#if 0 // for checking if the machine code actually matches with a disassembler
		MachineFunction* mcfunc = MachineInstSelectionX64::dumpinstructions(func);
#else
		MachineFunction* mcfunc = MachineInstSelectionX64::codegen(func);
		RegisterAllocatorX64::run(func->context, mcfunc);
#endif

		MachineCodeWriterX64 mcwriter(this, mcfunc);
		mcwriter.codegen();

		entry.endAddress = code.size();

#ifdef WIN32
		std::vector<uint16_t> data = UnwindInfoWindowsX64::create(mcfunc);
		entry.beginUnwindData = unwindData.size();
		unwindData.resize(unwindData.size() + data.size() * sizeof(uint16_t));
		memcpy(unwindData.data() + entry.beginUnwindData, data.data(), data.size() * sizeof(uint16_t));
		entry.endUnwindData = unwindData.size();
#else
		std::vector<uint8_t> data = UnwindInfoUnixX64::create(mcfunc, entry.unixUnwindFunctionStart);
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

void MachineCodeHolder::relocate(void* codeDest, void* dataDest, void* unwindDest) const
{
	uint8_t* codeDest8 = (uint8_t*)codeDest;
	uint8_t* dataDest8 = (uint8_t*)dataDest;
	uint8_t* unwindDest8 = (uint8_t*)unwindDest;

	memcpy(codeDest8, code.data(), code.size());
	memcpy(dataDest8, data.data(), data.size());
	memcpy(unwindDest8, unwindData.data(), unwindData.size());

#ifndef WIN32
	// Patch absolute address and size for each FDE entry in the .eh_frame
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

	for (const auto& entry : bbRelocateInfo)
	{
		int32_t value = (int32_t)(bbOffsets.find(entry.bb)->second - entry.pos - 4);
		memcpy(codeDest8 + entry.pos, &value, sizeof(int32_t));
	}

	for (const auto& entry : callRelocateInfo)
	{
		const auto& funcEntry = functionTable[funcToTableIndex.find(entry.func)->second];

		uint64_t value;
		if (!funcEntry.external)
		{
			value = (ptrdiff_t)codeDest8 + funcEntry.beginAddress;
		}
		else
		{
			value = (ptrdiff_t)funcEntry.external;
		}

		memcpy(codeDest8 + entry.pos, &value, sizeof(uint64_t));
	}

	for (const auto& entry : dataRelocateInfo)
	{
		int32_t ripoffset = (int32_t)(ptrdiff_t)((dataDest8 + entry.datapos) - (codeDest8 + entry.codepos + 4));
		memcpy(codeDest8 + entry.codepos, &ripoffset, sizeof(int32_t));
	}
}
