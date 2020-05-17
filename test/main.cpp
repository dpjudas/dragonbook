
#include "ir/IR.h"
#include "jit/JITRuntime.h"
#include <iostream>
#include <exception>

void floattest()
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

	JITRuntime jit;
	jit.add(&context);

	std::cout << context.getFunctionAssembly(func) << std::endl;

	float (*mainptr)(float, float, float, float, float, float) = reinterpret_cast<float(*)(float, float, float, float, float, float)>(jit.getPointerToFunction(func->name));
	float r = mainptr(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);
	std::cout << "main returned: " << r << std::endl;
}

void inttest()
{
	IRContext context;
	IRFunctionType* functype = context.getFunctionType(context.getInt32Ty(), { context.getInt32Ty(), context.getInt32Ty() });
	IRFunction* func = context.createFunction(functype, "main");

	IRBuilder builder;
	builder.SetInsertPoint(func->createBasicBlock("entry"));
	builder.CreateRet(builder.CreateUDiv(func->args[0], func->args[1]));

	JITRuntime jit;
	jit.add(&context);

	std::cout << context.getFunctionAssembly(func) << std::endl;

	int (*mainptr)(int, int) = reinterpret_cast<int(*)(int, int)>(jit.getPointerToFunction(func->name));
	int r = mainptr(-32, 2);
	std::cout << "should return: " << (((uint32_t)(int32_t)-32) / 2) << std::endl;
	std::cout << "main returned: " << r << std::endl;
}

void sexttest()
{
	IRContext context;
	IRType* int64Ty = context.getInt64Ty();
	IRType* int32Ty = context.getInt32Ty();
	IRType* int16Ty = context.getInt16Ty();
	IRType* int8Ty = context.getInt8Ty();

	std::vector<IRFunction*> funcs;
	funcs.push_back(context.createFunction(context.getFunctionType(int64Ty, { int8Ty }), "int8_to_64"));
	funcs.push_back(context.createFunction(context.getFunctionType(int32Ty, { int8Ty }), "int8_to_32"));
	funcs.push_back(context.createFunction(context.getFunctionType(int16Ty, { int8Ty }), "int8_to_16"));
	funcs.push_back(context.createFunction(context.getFunctionType(int8Ty, { int8Ty }), "int8_to_8"));
	funcs.push_back(context.createFunction(context.getFunctionType(int64Ty, { int16Ty }), "int16_to_64"));
	funcs.push_back(context.createFunction(context.getFunctionType(int32Ty, { int16Ty }), "int16_to_32"));
	funcs.push_back(context.createFunction(context.getFunctionType(int16Ty, { int16Ty }), "int16_to_16"));
	funcs.push_back(context.createFunction(context.getFunctionType(int64Ty, { int32Ty }), "int32_to_64"));
	funcs.push_back(context.createFunction(context.getFunctionType(int32Ty, { int32Ty }), "int32_to_32"));
	funcs.push_back(context.createFunction(context.getFunctionType(int64Ty, { int64Ty }), "int64_to_64"));

	for (IRFunction* func : funcs)
	{
		IRBuilder builder;
		builder.SetInsertPoint(func->createBasicBlock("entry"));
		builder.CreateRet(builder.CreateSExt(func->args[0], static_cast<IRFunctionType*>(func->type)->returnType));
	}

	JITRuntime jit;
	jit.add(&context);

	int64_t(*int8_to_64)(int8_t) = reinterpret_cast<int64_t(*)(int8_t)>(jit.getPointerToFunction("int8_to_64"));
	int32_t(*int8_to_32)(int8_t) = reinterpret_cast<int32_t(*)(int8_t)>(jit.getPointerToFunction("int8_to_32"));
	int16_t(*int8_to_16)(int8_t) = reinterpret_cast<int16_t(*)(int8_t)>(jit.getPointerToFunction("int8_to_16"));
	int8_t(*int8_to_8)(int8_t) = reinterpret_cast<int8_t(*)(int8_t)>(jit.getPointerToFunction("int8_to_8"));
	int64_t(*int16_to_64)(int16_t) = reinterpret_cast<int64_t(*)(int16_t)>(jit.getPointerToFunction("int16_to_64"));
	int32_t(*int16_to_32)(int16_t) = reinterpret_cast<int32_t(*)(int16_t)>(jit.getPointerToFunction("int16_to_32"));
	int16_t(*int16_to_16)(int16_t) = reinterpret_cast<int16_t(*)(int16_t)>(jit.getPointerToFunction("int16_to_16"));
	int64_t(*int32_to_64)(int32_t) = reinterpret_cast<int64_t(*)(int32_t)>(jit.getPointerToFunction("int32_to_64"));
	int32_t(*int32_to_32)(int32_t) = reinterpret_cast<int32_t(*)(int32_t)>(jit.getPointerToFunction("int32_to_32"));
	int64_t(*int64_to_64)(int64_t) = reinterpret_cast<int64_t(*)(int64_t)>(jit.getPointerToFunction("int64_to_64"));

	int8_to_32(-100);

	std::cout << int8_to_64(-100) << ", " << int8_to_64(100) << std::endl;
	std::cout << int8_to_32(-100) << ", " << int8_to_32(100) << std::endl;
	std::cout << int8_to_16(-100) << ", " << int8_to_16(100) << std::endl;
	std::cout << (int)int8_to_8(-100) << ", " << (int)int8_to_8(100) << std::endl;
	std::cout << std::endl;
	std::cout << int16_to_64(-2000) << ", " << int16_to_64(2000) << std::endl;
	std::cout << int16_to_32(-2000) << ", " << int16_to_32(2000) << std::endl;
	std::cout << int16_to_16(-2000) << ", " << int16_to_16(2000) << std::endl;
	std::cout << std::endl;
	std::cout << int32_to_64(-200000) << ", " << int32_to_64(200000) << std::endl;
	std::cout << int32_to_32(-200000) << ", " << int32_to_32(200000) << std::endl;
	std::cout << std::endl;
	std::cout << int64_to_64(-234567) << ", " << int64_to_64(234567) << std::endl;
}

int main(int argc, char** argv)
{
	try
	{
		//floattest();
		//inttest();
		sexttest();

		return 0;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return 1;
	}

	return 0;
}
