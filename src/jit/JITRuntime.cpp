
#include "Precomp.h"
#include "JITRuntime.h"
#include "MachineCode.h"
#include "MachineInst.h"
#include "UnwindInfoWindows.h"
#include "UnwindInfoUnix.h"
#include <memory>
#include "Scripting/Runtime/GC.h"

#ifndef WIN32
#include <sys/mman.h>

extern "C"
{
	void __register_frame(const void*);
	void __deregister_frame(const void*);
}
#endif

JITRuntime::JITRuntime()
{
}

JITRuntime::~JITRuntime()
{
	if (globals)
		gcReleaseRoot(globals);

#ifdef _WIN64
	for (auto p : frames)
	{
		RtlDeleteFunctionTable((PRUNTIME_FUNCTION)p);
	}
#elif !defined(WIN32)
	for (auto p : frames)
	{
		__deregister_frame(p);
	}
#endif

	for (uint8_t* block : blocks)
	{
		virtualFree(block);
	}
}

void JITRuntime::compile(const std::map<std::string, IRFunction*>& functions, const std::map<std::string, IRGlobalVariable*>& variables, const std::map<IRValue*, void*>& globalMappings)
{
	int globalsSize = 0;
	for (const auto& it : variables)
	{
		IRGlobalVariable* var = it.second;

		int size = (var->type->getPointerElementType()->getTypeAllocSize() + 7) / 8 * 8;
		var->globalsOffset = globalsSize;
		globalsSize += size;
	}

	globals = (uint8_t*)gcAlloc(globalsSize);
	gcCreateRoot(globals);
	memset(globals, 0, globalsSize);

	MachineCodeHolder codeholder;

	for (const auto& it : functions)
	{
		IRFunction* func = it.second;
		codeholder.addFunction(func);
	}

	for (const auto& it : globalMappings)
	{
		IRFunction* func = dynamic_cast<IRFunction*>(it.first);
		if (func)
		{
			codeholder.addFunction(func, it.second);
		}
	}

	add(&codeholder);

	for (const auto& it : variables)
	{
		IRGlobalVariable* var = it.second;
		if (var->initialValue)
		{
			initGlobal(var->globalsOffset, var->initialValue);
		}
	}
}

void JITRuntime::initGlobal(int offset, IRConstant* value)
{
	IRConstantStruct* initStruct = dynamic_cast<IRConstantStruct*>(value);
	IRConstantInt* initInt = dynamic_cast<IRConstantInt*>(value);
	IRConstantFP* initFloat = dynamic_cast<IRConstantFP*>(value);
	IRFunction* initFunc = dynamic_cast<IRFunction*>(value);

	if (initStruct)
	{
		for (IRConstant* svalue : initStruct->values)
		{
			initGlobal(offset, svalue);
			int size = (svalue->type->getTypeAllocSize() + 7) / 8 * 8;
			offset += size;
		}
	}
	else if (initInt)
	{
		if (dynamic_cast<IRInt32Type*>(initInt->type) || dynamic_cast<IRInt1Type*>(initInt->type))
		{
			*reinterpret_cast<uint32_t*>(globals + offset) = (uint32_t)initInt->value;
		}
		else if (dynamic_cast<IRInt64Type*>(initInt->type))
		{
			*reinterpret_cast<uint64_t*>(globals + offset) = (uint64_t)initInt->value;
		}
		else if (dynamic_cast<IRInt16Type*>(initInt->type))
		{
			*reinterpret_cast<uint16_t*>(globals + offset) = (uint16_t)initInt->value;
		}
		else if (dynamic_cast<IRInt8Type*>(initInt->type))
		{
			*reinterpret_cast<uint8_t*>(globals + offset) = (uint8_t)initInt->value;
		}
		else
		{
			throw std::runtime_error("Unknown IRConstantInt type");
		}
	}
	else if (initFloat)
	{
		if (dynamic_cast<IRFloatType*>(initFloat->type))
		{
			*reinterpret_cast<float*>(globals + offset) = (float)initFloat->value;
		}
		else if (dynamic_cast<IRDoubleType*>(initFloat->type))
		{
			*reinterpret_cast<double*>(globals + offset) = initFloat->value;
		}
		else
		{
			throw std::runtime_error("Unknown IRConstantFP type");
		}
	}
	else if (initFunc)
	{
		*reinterpret_cast<void**>(globals + offset) = getPointerToFunction(initFunc);
	}
	else
	{
		throw std::runtime_error("Unknown IRConstant type");
	}
}

void* JITRuntime::getPointerToFunction(IRFunction* func)
{
	auto it = functionTable.find(func);
	if (it != functionTable.end())
		return it->second;
	else
		return nullptr;
}

void* JITRuntime::getPointerToGlobal(IRGlobalVariable* var)
{
	return globals + var->globalsOffset;
}

#ifdef WIN32

void* JITRuntime::add(MachineCodeHolder* codeholder)
{
	size_t codeSize = codeholder->codeSize();
	codeSize = (codeSize + 15) / 16 * 16;

#ifdef _WIN64
	std::vector<uint16_t> unwindInfo = UnwindInfoWindows::create(codeholder->getFunc());
	size_t unwindInfoSize = unwindInfo.size() * sizeof(uint16_t);
	size_t functionTableSize = sizeof(RUNTIME_FUNCTION);
#else
	size_t unwindInfoSize = 0;
	size_t functionTableSize = 0;
#endif

	uint8_t* p = (uint8_t*)allocJitMemory(codeSize + unwindInfoSize + functionTableSize);
	size_t relocSize = codeholder->relocate(p);

	size_t unwindStart = relocSize;
	unwindStart = (unwindStart + 15) / 16 * 16;
	blockPos -= codeSize - unwindStart;

#ifdef _WIN64
	uint8_t* baseaddr = blocks.back();
	uint8_t* startaddr = p;
	uint8_t* endaddr = p + relocSize;
	uint8_t* unwindptr = p + unwindStart;
	memcpy(unwindptr, unwindInfo.data(), unwindInfoSize);

	RUNTIME_FUNCTION* table = (RUNTIME_FUNCTION*)(unwindptr + unwindInfoSize);
	table[0].BeginAddress = (DWORD)(ptrdiff_t)(startaddr - baseaddr);
	table[0].EndAddress = (DWORD)(ptrdiff_t)(endaddr - baseaddr);
#ifndef __MINGW64__
	table[0].UnwindInfoAddress = (DWORD)(ptrdiff_t)(unwindptr - baseaddr);
#else
	table[0].UnwindData = (DWORD)(ptrdiff_t)(unwindptr - baseaddr);
#endif
	BOOLEAN result = RtlAddFunctionTable(table, 1, (DWORD64)baseaddr);
	frames.push_back((uint8_t*)table);
	if (result == 0)
		throw std::runtime_error("RtlAddFunctionTable failed");
#endif

	return p;
}

void* JITRuntime::virtualAlloc(size_t size)
{
	return VirtualAllocEx(GetCurrentProcess(), nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}

void JITRuntime::virtualFree(void* ptr)
{
	VirtualFreeEx(GetCurrentProcess(), ptr, 0, MEM_RELEASE);
}

#else

void* JITRuntime::add(MachineCodeHolder* codeholder)
{
	size_t codeSize = codeholder->codeSize();
	if (codeSize == 0)
		return nullptr;

	unsigned int fdeFunctionStart = 0;
	std::vector<uint8_t> unwindInfo = UnwindInfoUnix::create(codeholder->getFunc(), fdeFunctionStart);
	size_t unwindInfoSize = unwindInfo.size();

	codeSize = (codeSize + 15) / 16 * 16;

	uint8_t* p = (uint8_t*)allocJitMemory(codeSize + unwindInfoSize);
	size_t relocSize = codeholder->relocate(p);

	size_t unwindStart = relocSize;
	unwindStart = (unwindStart + 15) / 16 * 16;
	blockPos -= codeSize - unwindStart;

	uint8_t* baseaddr = blocks.back();
	uint8_t* startaddr = p;
	uint8_t* endaddr = p + relocSize;
	uint8_t* unwindptr = p + unwindStart;
	memcpy(unwindptr, &unwindInfo[0], unwindInfoSize);

	if (unwindInfo.size() > 0)
	{
		uint64_t* unwindfuncaddr = (uint64_t*)(unwindptr + fdeFunctionStart);
		unwindfuncaddr[0] = (ptrdiff_t)startaddr;
		unwindfuncaddr[1] = (ptrdiff_t)(endaddr - startaddr);

#ifdef __APPLE__
		// On macOS __register_frame takes a single FDE as an argument
		uint8_t* entry = unwindptr;
		while (true)
		{
			uint32_t length = *((uint32_t*)entry);
			if (length == 0)
				break;

			if (length == 0xffffffff)
			{
				uint64_t length64 = *((uint64_t*)(entry + 4));
				if (length64 == 0)
					break;

				uint64_t offset = *((uint64_t*)(entry + 12));
				if (offset != 0)
				{
					__register_frame(entry);
					frames.push_back(entry);
				}
				entry += length64 + 12;
			}
			else
			{
				uint32_t offset = *((uint32_t*)(entry + 4));
				if (offset != 0)
				{
					__register_frame(entry);
					frames.push_back(entry);
				}
				entry += length + 4;
			}
		}
#else
		// On Linux it takes a pointer to the entire .eh_frame
		__register_frame(unwindptr);
		frames.push_back(unwindptr);
#endif
	}

	return p;
}

void* JITRuntime::virtualAlloc(size_t size)
{
	void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (ptr == MAP_FAILED) return nullptr;
	return ptr;
}

void JITRuntime::virtualFree(void* ptr)
{
	munmap(ptr, blockSize);
}

#endif

void* JITRuntime::allocJitMemory(size_t size)
{
	if (blockPos + size <= blockSize)
	{
		uint8_t* p = blocks[blocks.size() - 1];
		p += blockPos;
		blockPos += size;
		return p;
	}
	else
	{
		size_t allocSize = 1024 * 1024;
		void* base = virtualAlloc(allocSize);
		if (!base)
			throw std::runtime_error("VirtualAllocEx failed");

		blocks.push_back((uint8_t*)base);
		blockSize = allocSize;
		blockPos = size;
		return base;
	}
}
