
#include "dragonbook/IRStackToRegisterPass.h"
#include "dragonbook/IRFunction.h"
#include "dragonbook/IRBasicBlock.h"

void IRStackToRegisterPass::run(IRFunction* func)
{
	IRStackToRegisterPass pass(func);

	for (IRBasicBlock* bb : func->basicBlocks)
	{
		pass.code.clear();
		pass.code.reserve(bb->code.size());
		for (IRInst* inst : bb->code)
		{
			inst->visit(&pass);
		}
		bb->code.swap(pass.code);
	}
}

IRStackToRegisterPass::IRStackToRegisterPass(IRFunction* func) : func(func)
{
}

void IRStackToRegisterPass::inst(IRInstLoad* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstStore* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstAdd* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstSub* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstFAdd* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstFSub* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstNot* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstNeg* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstFNeg* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstMul* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstFMul* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstSDiv* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstUDiv* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstFDiv* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstSRem* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstURem* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstShl* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstLShr* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstAShr* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstICmpSLT* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstICmpULT* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstFCmpULT* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstICmpSGT* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstICmpUGT* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstFCmpUGT* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstICmpSLE* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstICmpULE* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstFCmpULE* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstICmpSGE* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstICmpUGE* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstFCmpUGE* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstICmpEQ* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstFCmpUEQ* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstICmpNE* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstFCmpUNE* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstAnd* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstOr* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstXor* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstTrunc* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstZExt* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstSExt* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstFPTrunc* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstFPExt* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstFPToUI* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstFPToSI* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstUIToFP* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstSIToFP* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstBitCast* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstCall* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstGEP* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstBr* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstCondBr* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstRet* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstRetVoid* node)
{
	code.push_back(node);
}

void IRStackToRegisterPass::inst(IRInstAlloca* node)
{
	code.push_back(node);
}
