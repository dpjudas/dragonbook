
#pragma once

#include "IRInstVisitor.h"
#include <vector>

class IRFunction;
class IRInst;

class IRStackToRegisterPass : IRInstVisitor
{
public:
	static void run(IRFunction* func);

private:
	IRStackToRegisterPass(IRFunction* func);

	void inst(IRInstLoad* node) override;
	void inst(IRInstStore* node) override;
	void inst(IRInstAdd* node) override;
	void inst(IRInstSub* node) override;
	void inst(IRInstFAdd* node) override;
	void inst(IRInstFSub* node) override;
	void inst(IRInstNot* node) override;
	void inst(IRInstNeg* node) override;
	void inst(IRInstFNeg* node) override;
	void inst(IRInstMul* node) override;
	void inst(IRInstFMul* node) override;
	void inst(IRInstSDiv* node) override;
	void inst(IRInstUDiv* node) override;
	void inst(IRInstFDiv* node) override;
	void inst(IRInstSRem* node) override;
	void inst(IRInstURem* node) override;
	void inst(IRInstShl* node) override;
	void inst(IRInstLShr* node) override;
	void inst(IRInstAShr* node) override;
	void inst(IRInstICmpSLT* node) override;
	void inst(IRInstICmpULT* node) override;
	void inst(IRInstFCmpULT* node) override;
	void inst(IRInstICmpSGT* node) override;
	void inst(IRInstICmpUGT* node) override;
	void inst(IRInstFCmpUGT* node) override;
	void inst(IRInstICmpSLE* node) override;
	void inst(IRInstICmpULE* node) override;
	void inst(IRInstFCmpULE* node) override;
	void inst(IRInstICmpSGE* node) override;
	void inst(IRInstICmpUGE* node) override;
	void inst(IRInstFCmpUGE* node) override;
	void inst(IRInstICmpEQ* node) override;
	void inst(IRInstFCmpUEQ* node) override;
	void inst(IRInstICmpNE* node) override;
	void inst(IRInstFCmpUNE* node) override;
	void inst(IRInstAnd* node) override;
	void inst(IRInstOr* node) override;
	void inst(IRInstXor* node) override;
	void inst(IRInstTrunc* node) override;
	void inst(IRInstZExt* node) override;
	void inst(IRInstSExt* node) override;
	void inst(IRInstFPTrunc* node) override;
	void inst(IRInstFPExt* node) override;
	void inst(IRInstFPToUI* node) override;
	void inst(IRInstFPToSI* node) override;
	void inst(IRInstUIToFP* node) override;
	void inst(IRInstSIToFP* node) override;
	void inst(IRInstBitCast* node) override;
	void inst(IRInstCall* node) override;
	void inst(IRInstGEP* node) override;
	void inst(IRInstBr* node) override;
	void inst(IRInstCondBr* node) override;
	void inst(IRInstRet* node) override;
	void inst(IRInstRetVoid* node) override;
	void inst(IRInstAlloca* node) override;

	IRFunction* func = nullptr;
	std::vector<IRInst*> code;
};
