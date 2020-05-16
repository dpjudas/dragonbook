#pragma once

#include <map>
#include <vector>
#include <string>

class IRContext;
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

	void add(IRContext* context);

	void* getPointerToFunction(const std::string& func);
	void* getPointerToGlobal(const std::string& variable);

private:
	void initGlobal(int offset, IRConstant* value);

	void add(MachineCodeHolder* codeholder);
	void* allocJitMemory(size_t size);
	void* virtualAlloc(size_t size);
	void virtualFree(void* ptr);

	std::map<std::string, void*> functionTable;
	std::map<std::string, size_t> globalTable;
	std::vector<uint8_t> globals;

	std::vector<uint8_t*> blocks;
	std::vector<uint8_t*> frames;
	size_t blockPos = 0;
	size_t blockSize = 0;
};
