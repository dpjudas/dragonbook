
#include "Precomp.h"
#include "IRFunction.h"
#include "IRContext.h"
#include "IRInst.h"
#include "IRBasicBlock.h"
#include "IRType.h"

IRFunction::IRFunction(IRContext *context, IRFunctionType *type, const std::string &name) : IRConstant(type), context(context), name(name)
{
	int index = 0;
	args.reserve(type->args.size());
	for (IRType *argType : type->args)
	{
		args.push_back(new(context->gc()) IRFunctionArg(argType, index++));
	}
}

IRInstAlloca *IRFunction::createAlloca(IRType *type, IRValue *arraySize, const std::string &name)
{
	if (!arraySize)
		arraySize = context->getConstantInt(1);
	auto inst = new(context->gc()) IRInstAlloca(context, type, arraySize, name);
	stackVars.push_back(inst);
	return inst;
}

IRBasicBlock *IRFunction::createBasicBlock(const std::string &comment)
{
	auto bb = new(context->gc()) IRBasicBlock(this, comment);
	basicBlocks.push_back(bb);
	return bb;
}
