#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include "dragonbook/IR.h"

class MachineBasicBlock;

enum class MachineOperandType
{
	reg, constant, frameOffset, spillOffset, stackOffset, imm, basicblock, func, global
};

class MachineOperand
{
public:
	MachineOperandType type;
	union
	{
		int registerIndex;
		int constantIndex;
		int frameOffset;
		int spillOffset;
		int stackOffset;
		uint64_t immvalue;
		MachineBasicBlock* bb;
		IRFunction* func;
		IRGlobalVariable* global;
	};
};

enum class MachineUnwindHint
{
	None,
	PushNonvolatile,
	StackAdjustment,
	RegisterStackLocation,
	SaveFrameRegister
};

class MachineInst
{
public:
	int opcode = 0;
	std::vector<MachineOperand> operands;

	int unwindOffset = -1;
	MachineUnwindHint unwindHint = MachineUnwindHint::None;

	std::string comment;
	int fileIndex = -1;
	int lineNumber = -1;
};

class MachineBasicBlock
{
public:
	std::vector<MachineInst*> code;
};

class MachineConstant
{
public:
	MachineConstant(const void* value, int size) : size(size)
	{
		if (size > maxsize)
			throw std::runtime_error("Constant data type too large");
		memcpy(data, value, size);
		memset(data + size, 0, maxsize - size);
	}

	enum { maxsize = 16 };
	uint8_t data[maxsize];
	int size;
};

enum class MachineRegClass
{
	gp,
	xmm,
	reserved
};

class MachineRegister
{
public:
	MachineRegister() = default;
	MachineRegister(MachineRegClass cls) : cls(cls) { }

	MachineRegClass cls = MachineRegClass::reserved;
};

class MachineStackAlloc
{
public:
	MachineStackAlloc() = default;
	MachineStackAlloc(int registerIndex, size_t size, std::string name) : registerIndex(registerIndex), size(size), name(std::move(name)) { }

	int registerIndex = 0;
	size_t size = 0;
	std::string name;
};

class MachineFunction
{
public:
	MachineFunction() = default;
	MachineFunction(std::string name) : name(std::move(name)) { }

	std::string name;
	IRFunctionType* type = nullptr;
	MachineBasicBlock* prolog = nullptr;
	MachineBasicBlock* epilog = nullptr;
	std::vector<MachineBasicBlock*> basicBlocks;
	std::vector<MachineConstant> constants;
	std::vector<MachineRegister> registers;
	std::vector<MachineStackAlloc> stackvars;
	int maxCallArgsSize = 0;
	int frameBaseOffset = 0;
	int spillBaseOffset = 0;
	bool dynamicStackAllocations = false;
	std::vector<std::string> fileInfo;
};
