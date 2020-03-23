
#include "ir/IR.h"
#include <iostream>
#include <exception>

int main(int argc, char** argv)
{
	try
	{
		IRContext context;
		IRBuilder builder;

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

		IRFunction* func = context.createFunction(functype, "main");

		IRBasicBlock* entry = func->createBasicBlock("entry");
		builder.SetInsertPoint(entry);
		builder.CreateRet(builder.CreateFAdd(func->args[0], func->args[4]));

		context.codegen();

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
