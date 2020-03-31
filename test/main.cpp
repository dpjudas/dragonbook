
#include "ir/IR.h"
#include <iostream>
#include <exception>

int main(int argc, char** argv)
{
	try
	{
		IRContext context;

		IRFunctionType* functype = context.getFunctionType(
			context.getFloatTy(),
			{
				context.getFloatTy(),
				context.getFloatTy(),
				context.getFloatTy(),
				context.getFloatTy(),
				context.getFloatTy(),
				context.getFloatTy()
			});

		IRFunctionType* functype2 = context.getFunctionType(context.getFloatTy(), {});

		IRFunction* func = context.createFunction(functype, "main");
		IRFunction* func2 = context.createFunction(functype2, "func2");

		IRBuilder builder;

		builder.SetInsertPoint(func->createBasicBlock("entry"));
		builder.CreateRet(builder.CreateFAdd(builder.CreateFAdd(func->args[0], func->args[4]), builder.CreateCall(func2, {})));

		builder.SetInsertPoint(func2->createBasicBlock("entry"));
		builder.CreateRet(context.getConstantFloat(context.getFloatTy(), 42.0f));

		context.codegen();

		std::cout << context.getFunctionAssembly(func) << std::endl;

		float (*mainptr)(float, float, float, float, float, float) = reinterpret_cast<float(*)(float, float, float, float, float, float)>(context.getPointerToFunction(func));
		float r = mainptr(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);
		std::cout << "main returned: " << r << std::endl;

		return 0;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return 1;
	}

	return 0;
}
