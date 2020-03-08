
#include "Precomp.h"
#include "IRContext.h"
#include "IRType.h"
#include "IRFunction.h"
#include "MachineCode.h"

IRContext::IRContext()
{
	voidType = new(gc()) IRVoidType();
	int1Type = new(gc()) IRInt1Type();
	int8Type = new(gc()) IRInt8Type();
	int16Type = new(gc()) IRInt16Type();
	int32Type = new(gc()) IRInt32Type();
	int64Type = new(gc()) IRInt64Type();
	floatType = new(gc()) IRFloatType();
	doubleType = new(gc()) IRDoubleType();
}

IRContext::~IRContext()
{
}

void IRContext::codegen()
{
	jit.compile(functions, globalVars, globalMappings);
}

void *IRContext::getPointerToFunction(IRFunction *func)
{
	return jit.getPointerToFunction(func);
}

void* IRContext::getPointerToGlobal(IRGlobalVariable* variable)
{
	return jit.getPointerToGlobal(variable);
}

IRFunction *IRContext::getFunction(const std::string &name)
{
	auto it = functions.find(name);
	return (it != functions.end()) ? it->second : nullptr;
}

IRGlobalVariable *IRContext::getNamedGlobal(const std::string &name)
{
	auto it = globalVars.find(name);
	return (it != globalVars.end()) ? it->second : nullptr;
}

void IRContext::addGlobalMapping(IRValue *value, void *nativeFunc)
{
	globalMappings[value] = nativeFunc;
}

IRFunctionType *IRContext::getFunctionType(IRType *returnType, std::vector<IRType *> args)
{
	for (IRFunctionType *type : functionTypes)
	{
		if (type->returnType == returnType && type->args.size() == args.size())
		{
			bool found = true;
			for (size_t i = 0; i < args.size(); i++)
			{
				if (args[i] != type->args[i])
				{
					found = false;
					break;
				}
			}
			if (found)
				return type;
		}
	}

	IRFunctionType *type = new(gc()) IRFunctionType(returnType, args);
	functionTypes.push_back(type);
	return type;
}

IRFunction *IRContext::createFunction(IRFunctionType *type, const std::string &name)
{
	IRFunction *func = new(gc()) IRFunction(this, type, name);
	functions[name] = func;
	return func;
}

IRGlobalVariable *IRContext::createGlobalVariable(IRType *type, IRConstant *value, const std::string &name)
{
	IRGlobalVariable *var = new(gc()) IRGlobalVariable(type->getPointerTo(this), value, name);
	globalVars[name] = var;
	return var;
}

IRStructType *IRContext::createStructType(const std::string &name)
{
	return new(gc()) IRStructType(name);
}

IRConstantStruct *IRContext::getConstantStruct(IRStructType *type, const std::vector<IRConstant *> &values)
{
	for (IRConstantStruct *value : constantStructs)
	{
		if (value->type == type && value->values.size() == values.size())
		{
			bool found = true;
			for (size_t i = 0; i < values.size(); i++)
			{
				if (values[i] != value->values[i])
				{
					found = false;
					break;
				}
			}
			if (found)
				return value;
		}
	}

	IRConstantStruct *value = new(gc()) IRConstantStruct(type, values);
	constantStructs.push_back(value);
	return value;
}

IRConstantFP *IRContext::getConstantFloat(IRType *type, double value)
{
	auto it = floatConstants.find({ type, value });
	if (it != floatConstants.end())
		return it->second;

	IRConstantFP *c = new(gc()) IRConstantFP(type, value);
	floatConstants[{ type, value }] = c;
	return c;
}

IRConstantInt *IRContext::getConstantInt(IRType *type, uint64_t value)
{
	auto it = intConstants.find({ type, value });
	if (it != intConstants.end())
		return it->second;

	IRConstantInt *c = new(gc()) IRConstantInt(type, value);
	intConstants[{ type, value }] = c;
	return c;
}

IRConstantInt *IRContext::getConstantInt(int32_t value)
{
	return getConstantInt(getInt32Ty(), value);
}

IRConstantInt *IRContext::getConstantIntTrue()
{
	return getConstantInt(getInt1Ty(), 1);
}

IRConstantInt *IRContext::getConstantIntFalse()
{
	return getConstantInt(getInt1Ty(), 0);
}

IRPointerType *IRContext::getVoidPtrTy()
{
	return getVoidTy()->getPointerTo(this);
}

IRPointerType *IRContext::getInt1PtrTy()
{
	return getInt1Ty()->getPointerTo(this);
}

IRPointerType *IRContext::getInt8PtrTy()
{
	return getInt8Ty()->getPointerTo(this);
}

IRPointerType *IRContext::getInt16PtrTy()
{
	return getInt16Ty()->getPointerTo(this);
}

IRPointerType *IRContext::getInt32PtrTy()
{
	return getInt32Ty()->getPointerTo(this);
}

IRPointerType *IRContext::getInt64PtrTy()
{
	return getInt64Ty()->getPointerTo(this);
}

IRPointerType *IRContext::getFloatPtrTy()
{
	return getFloatTy()->getPointerTo(this);
}

IRPointerType *IRContext::getDoublePtrTy()
{
	return getDoubleTy()->getPointerTo(this);
}
