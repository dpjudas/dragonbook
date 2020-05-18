
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

class InstructionTest
{
public:
	InstructionTest(std::string name, std::function<void(IRContext*)> createCallback, std::function<bool(JITRuntime*)> runCallback) : name(name), createCallback(createCallback), runCallback(runCallback) { }

	void CreateFunc(IRContext* context) { createCallback(context);}
	bool RunTest(JITRuntime* runtime) { return runCallback(runtime); }
	
	std::string name;
	
private:
	std::function<void(IRContext*)> createCallback;
	std::function<bool(JITRuntime*)> runCallback;
};

class InstructionTester
{
public:
	void Test(std::string name, std::function<void(IRContext*)> createCallback, std::function<bool(JITRuntime*)> runCallback)
	{
		std::unique_ptr<InstructionTest> t(new InstructionTest(name, createCallback, runCallback));
		tests.push_back(std::move(t));
	}
	
	template<typename T>
	void Binary(std::string name, std::function<IRValue*(IRBuilder*, IRValue*, IRValue*)> codegen, T a, T b, T result)
	{
		auto create = [=](IRContext* context)
		{
			IRType* type = GetIRType<T>(context);
			IRFunction* func = context->createFunction(context->getFunctionType(type, { type, type }), name);
			
			IRBuilder builder;
			builder.SetInsertPoint(func->createBasicBlock("entry"));
			builder.CreateRet(codegen(&builder, func->args[0], func->args[1]));
		};

		auto run = [=](JITRuntime* jit)
		{
			auto ptr = reinterpret_cast<T(*)(T,T)>(jit->getPointerToFunction(name));
			return ptr(a, b) == result;
		};
		
		Test(name, create, run);
	}
	
	void Run()
	{
		IRContext context;
		for (auto& test : tests)
		{
			test->CreateFunc(&context);
		}
		
		JITRuntime jit;
		jit.add(&context);
		
		for (auto& test : tests)
		{
			std::cout << "Testing " << test->name.c_str() << "...";
			std::cout.flush();
			bool result = test->RunTest(&jit);
			std::cout << (result ? " OK" : " FAILED") << std::endl;
		}
	}

private:
	template<typename T> IRType* GetIRType(IRContext* ircontext) { return ircontext->getInt8PtrTy(); }
	template<> IRType* GetIRType<void>(IRContext* ircontext) { return ircontext->getVoidTy(); }
	template<> IRType* GetIRType<char>(IRContext* ircontext) { return ircontext->getInt8Ty(); }
	template<> IRType* GetIRType<unsigned char>(IRContext* ircontext) { return ircontext->getInt8Ty(); }
	template<> IRType* GetIRType<short>(IRContext* ircontext) { return ircontext->getInt16Ty(); }
	template<> IRType* GetIRType<unsigned short>(IRContext* ircontext) { return ircontext->getInt16Ty(); }
	template<> IRType* GetIRType<int>(IRContext* ircontext) { return ircontext->getInt32Ty(); }
	template<> IRType* GetIRType<unsigned int>(IRContext* ircontext) { return ircontext->getInt32Ty(); }
	template<> IRType* GetIRType<long long>(IRContext* ircontext) { return ircontext->getInt64Ty(); }
	template<> IRType* GetIRType<unsigned long long>(IRContext* ircontext) { return ircontext->getInt64Ty(); }
	template<> IRType* GetIRType<float>(IRContext* ircontext) { return ircontext->getFloatTy(); }
	template<> IRType* GetIRType<double>(IRContext* ircontext) { return ircontext->getDoubleTy(); }
	
	std::vector<std::unique_ptr<InstructionTest>> tests;
};

void insttest()
{
	InstructionTester tester;

	tester.Binary<int8_t>("add_int8", [](auto cc, auto a, auto b) { return cc->CreateAdd(a, b); }, 5, 10, 5 + 10);
	tester.Binary<int8_t>("sub_int8", [](auto cc, auto a, auto b) { return cc->CreateSub(a, b); }, 5, 10, 5 - 10);
	tester.Binary<int8_t>("mul_int8", [](auto cc, auto a, auto b) { return cc->CreateMul(a, b); }, 5, 10, 5 * 10);
	tester.Binary<int8_t>("sdiv_int8", [](auto cc, auto a, auto b) { return cc->CreateSDiv(a, b); }, -50, 10, (-50) / 10);
	tester.Binary<int8_t>("udiv_int8", [](auto cc, auto a, auto b) { return cc->CreateUDiv(a, b); }, 50, 10, 50 / 10);
	tester.Binary<int8_t>("srem_int8", [](auto cc, auto a, auto b) { return cc->CreateSRem(a, b); }, -50, 3, (-50) % 3);
	tester.Binary<int8_t>("urem_int8", [](auto cc, auto a, auto b) { return cc->CreateURem(a, b); }, 50, 3, 50 % 3);
	tester.Binary<int8_t>("and_int8", [](auto cc, auto a, auto b) { return cc->CreateAnd(a, b); }, 50, 10, 50 & 10);
	tester.Binary<int8_t>("or_int8", [](auto cc, auto a, auto b) { return cc->CreateOr(a, b); }, 50, 10, 50 | 10);
	tester.Binary<int8_t>("xor_int8", [](auto cc, auto a, auto b) { return cc->CreateXor(a, b); }, 50, 10, 50 ^ 10);
	tester.Binary<int8_t>("shl_int8", [](auto cc, auto a, auto b) { return cc->CreateShl(a, b); }, 5, 2, 5 << 2);
	tester.Binary<int8_t>("lshr_int8", [](auto cc, auto a, auto b) { return cc->CreateLShr(a, b); }, -50, 2, static_cast<uint8_t>(-50) >> 2);
	tester.Binary<int8_t>("ashr_int8", [](auto cc, auto a, auto b) { return cc->CreateAShr(a, b); }, -50, 2, (-50) >> 2);
	tester.Binary<int16_t>("add_int16", [](auto cc, auto a, auto b) { return cc->CreateAdd(a, b); }, 5, 10, 5 + 10);
	tester.Binary<int16_t>("sub_int16", [](auto cc, auto a, auto b) { return cc->CreateSub(a, b); }, 5, 10, 5 - 10);
	tester.Binary<int16_t>("sdiv_int16", [](auto cc, auto a, auto b) { return cc->CreateSDiv(a, b); }, -50, 10, (-50) / 10);
	tester.Binary<int16_t>("udiv_int16", [](auto cc, auto a, auto b) { return cc->CreateUDiv(a, b); }, 50, 10, 50 / 10);
	tester.Binary<int16_t>("srem_int16", [](auto cc, auto a, auto b) { return cc->CreateSRem(a, b); }, -50, 3, (-50) % 3);
	tester.Binary<int16_t>("urem_int16", [](auto cc, auto a, auto b) { return cc->CreateURem(a, b); }, 50, 3, 50 % 3);
	tester.Binary<int16_t>("and_int16", [](auto cc, auto a, auto b) { return cc->CreateAnd(a, b); }, 50, 10, 50 & 10);
	tester.Binary<int16_t>("or_int16", [](auto cc, auto a, auto b) { return cc->CreateOr(a, b); }, 50, 10, 50 | 10);
	tester.Binary<int16_t>("xor_int16", [](auto cc, auto a, auto b) { return cc->CreateXor(a, b); }, 50, 10, 50 ^ 10);
	tester.Binary<int16_t>("shl_int16", [](auto cc, auto a, auto b) { return cc->CreateShl(a, b); }, 5, 2, 5 << 2);
	tester.Binary<int16_t>("lshr_int16", [](auto cc, auto a, auto b) { return cc->CreateLShr(a, b); }, -50, 2, static_cast<uint16_t>(-50) >> 2);
	tester.Binary<int16_t>("ashr_int16", [](auto cc, auto a, auto b) { return cc->CreateAShr(a, b); }, -50, 2, (-50) >> 2);
	tester.Binary<int32_t>("add_int32", [](auto cc, auto a, auto b) { return cc->CreateAdd(a, b); }, 5, 10, 5 + 10);
	tester.Binary<int32_t>("sub_int32", [](auto cc, auto a, auto b) { return cc->CreateSub(a, b); }, 5, 10, 5 - 10);
	tester.Binary<int32_t>("sdiv_int32", [](auto cc, auto a, auto b) { return cc->CreateSDiv(a, b); }, -50, 10, (-50) / 10);
	tester.Binary<int32_t>("udiv_int32", [](auto cc, auto a, auto b) { return cc->CreateUDiv(a, b); }, 50, 10, 50 / 10);
	tester.Binary<int32_t>("srem_int32", [](auto cc, auto a, auto b) { return cc->CreateSRem(a, b); }, -50, 3, (-50) % 3);
	tester.Binary<int32_t>("urem_int32", [](auto cc, auto a, auto b) { return cc->CreateURem(a, b); }, 50, 3, 50 % 3);
	tester.Binary<int32_t>("and_int32", [](auto cc, auto a, auto b) { return cc->CreateAnd(a, b); }, 50, 10, 50 & 10);
	tester.Binary<int32_t>("or_int32", [](auto cc, auto a, auto b) { return cc->CreateOr(a, b); }, 50, 10, 50 | 10);
	tester.Binary<int32_t>("xor_int32", [](auto cc, auto a, auto b) { return cc->CreateXor(a, b); }, 50, 10, 50 ^ 10);
	tester.Binary<int32_t>("shl_int32", [](auto cc, auto a, auto b) { return cc->CreateShl(a, b); }, 5, 2, 5 << 2);
	tester.Binary<int32_t>("lshr_int32", [](auto cc, auto a, auto b) { return cc->CreateLShr(a, b); }, -50, 2, static_cast<uint32_t>(-50) >> 2);
	tester.Binary<int32_t>("ashr_int32", [](auto cc, auto a, auto b) { return cc->CreateAShr(a, b); }, -50, 2, (-50) >> 2);
	tester.Binary<int64_t>("add_int64", [](auto cc, auto a, auto b) { return cc->CreateAdd(a, b); }, 5, 10, 5 + 10);
	tester.Binary<int64_t>("sub_int64", [](auto cc, auto a, auto b) { return cc->CreateSub(a, b); }, 5, 10, 5 - 10);
	tester.Binary<int64_t>("sdiv_int64", [](auto cc, auto a, auto b) { return cc->CreateSDiv(a, b); }, -50, 10, (-50) / 10);
	tester.Binary<int64_t>("udiv_int64", [](auto cc, auto a, auto b) { return cc->CreateUDiv(a, b); }, 50, 10, 50 / 10);
	tester.Binary<int64_t>("srem_int64", [](auto cc, auto a, auto b) { return cc->CreateSRem(a, b); }, -50, 3, (-50) % 3);
	tester.Binary<int64_t>("urem_int64", [](auto cc, auto a, auto b) { return cc->CreateURem(a, b); }, 50, 3, 50 % 3);
	tester.Binary<int64_t>("and_int64", [](auto cc, auto a, auto b) { return cc->CreateAnd(a, b); }, 50, 10, 50 & 10);
	tester.Binary<int64_t>("or_int64", [](auto cc, auto a, auto b) { return cc->CreateOr(a, b); }, 50, 10, 50 | 10);
	tester.Binary<int64_t>("xor_int64", [](auto cc, auto a, auto b) { return cc->CreateXor(a, b); }, 50, 10, 50 ^ 10);
	tester.Binary<int64_t>("shl_int64", [](auto cc, auto a, auto b) { return cc->CreateShl(a, b); }, 5, 2, 5 << 2);
	tester.Binary<int64_t>("lshr_int64", [](auto cc, auto a, auto b) { return cc->CreateLShr(a, b); }, -50, 2, static_cast<uint64_t>(-50) >> 2);
	tester.Binary<int64_t>("ashr_int64", [](auto cc, auto a, auto b) { return cc->CreateAShr(a, b); }, -50, 2, (-50) >> 2);
	tester.Binary<float>("fadd_float", [](auto cc, auto a, auto b) { return cc->CreateFAdd(a, b); }, 5.0f, 10.0f, 5.0f + 10.0f);
	tester.Binary<float>("fsub_float", [](auto cc, auto a, auto b) { return cc->CreateFSub(a, b); }, 5.0f, 10.0f, 5.0f - 10.0f);
	tester.Binary<float>("fmul_float", [](auto cc, auto a, auto b) { return cc->CreateFMul(a, b); }, 5.0f, 10.0f, 5.0f * 10.0f);
	tester.Binary<float>("fdiv_float", [](auto cc, auto a, auto b) { return cc->CreateFDiv(a, b); }, 50.0f, 10.0f, 55.0f / 10.0f);
	tester.Binary<double>("fadd_double", [](auto cc, auto a, auto b) { return cc->CreateFAdd(a, b); }, 5.0, 10.0, 5.0 + 10.0);
	tester.Binary<double>("fsub_double", [](auto cc, auto a, auto b) { return cc->CreateFSub(a, b); }, 5.0, 10.0, 5.0 - 10.0);
	tester.Binary<double>("fmul_double", [](auto cc, auto a, auto b) { return cc->CreateFMul(a, b); }, 5.0, 10.0, 5.0 * 10.0);
	tester.Binary<double>("fdiv_double", [](auto cc, auto a, auto b) { return cc->CreateFDiv(a, b); }, 50.0, 10.0, 55.0 / 10.0);

	tester.Run();
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
		//sexttest();
		insttest();

		return 0;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return 1;
	}

	return 0;
}
