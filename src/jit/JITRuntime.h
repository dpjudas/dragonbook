#pragma once

class IRFunction;
class IRValue;
class IRConstant;
class IRGlobalVariable;
class MachineCodeHolder;

class JITRuntime
{
public:
	JITRuntime();
	~JITRuntime();

	void compile(const std::map<std::string, IRFunction*> &functions, const std::map<std::string, IRGlobalVariable*> &variables, const std::map<IRValue*, void*> &globalMappings);

	void* getPointerToFunction(IRFunction* func);
	void* getPointerToGlobal(IRGlobalVariable* variable);

private:
	void initGlobal(int offset, IRConstant* value);

	void* add(MachineCodeHolder* codeholder);
	void* allocJitMemory(size_t size);
	void* virtualAlloc(size_t size);
	void virtualFree(void* ptr);

	std::map<IRFunction*, void*> functionTable;
	uint8_t* globals = nullptr;

	std::vector<uint8_t*> blocks;
	std::vector<uint8_t*> frames;
	size_t blockPos = 0;
	size_t blockSize = 0;
};
