#pragma once

#include "IRValue.h"
#include "IRType.h"
#include "IRInstVisitor.h"
#include <stdexcept>

class IRBasicBlock;

class IRInst : public IRValue
{
public:
	using IRValue::IRValue;

	virtual void visit(IRInstVisitor *visitor) = 0;
};

class IRInstUnary : public IRInst
{
public:
	IRInstUnary(IRValue *operand) : IRInst(operand->type), operand(operand) { }

	IRValue *operand;
};

class IRInstBinary : public IRInst
{
public:
	IRInstBinary(IRValue *operand1, IRValue *operand2) : IRInst(operand1->type), operand1(operand1), operand2(operand2) { }

	IRValue *operand1;
	IRValue *operand2;
};

class IRInstLoad : public IRInstUnary { public: IRInstLoad(IRValue *operand) : IRInstUnary(operand) { type = operand->type->getPointerElementType(); } void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstStore : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };

class IRInstAdd : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstSub : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstFAdd : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstFSub : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };

class IRInstNot : public IRInstUnary { public: using IRInstUnary::IRInstUnary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstNeg : public IRInstUnary { public: using IRInstUnary::IRInstUnary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstFNeg : public IRInstUnary { public: using IRInstUnary::IRInstUnary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };

class IRInstMul : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstFMul : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };

class IRInstSDiv : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstUDiv : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstFDiv : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };

class IRInstSRem : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstURem : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };

class IRInstShl : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstLShr : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor* visitor) { visitor->inst(this); } };
class IRInstAShr : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };

class IRInstICmpSLT : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstICmpULT : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstFCmpULT : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };

class IRInstICmpSGT : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstICmpUGT : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstFCmpUGT : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };

class IRInstICmpSLE : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstICmpULE : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstFCmpULE : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };

class IRInstICmpSGE : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstICmpUGE : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstFCmpUGE : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };

class IRInstICmpEQ : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstFCmpUEQ : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };

class IRInstICmpNE : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstFCmpUNE : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };

class IRInstAnd : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstOr : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstXor : public IRInstBinary { public: using IRInstBinary::IRInstBinary; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };

class IRInstCast : public IRInst
{
public:
	IRInstCast(IRValue *value, IRType *type) : IRInst(type), value(value) { }

	IRValue *value;
};

class IRInstTrunc : public IRInstCast { public: using IRInstCast::IRInstCast; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstZExt : public IRInstCast { public: using IRInstCast::IRInstCast; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstSExt : public IRInstCast { public: using IRInstCast::IRInstCast; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstFPTrunc : public IRInstCast { public: using IRInstCast::IRInstCast; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstFPExt : public IRInstCast { public: using IRInstCast::IRInstCast; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstFPToUI : public IRInstCast { public: using IRInstCast::IRInstCast; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstFPToSI : public IRInstCast { public: using IRInstCast::IRInstCast; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstUIToFP : public IRInstCast { public: using IRInstCast::IRInstCast; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstSIToFP : public IRInstCast { public: using IRInstCast::IRInstCast; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };
class IRInstBitCast : public IRInstCast { public: using IRInstCast::IRInstCast; void visit(IRInstVisitor *visitor) { visitor->inst(this); } };

class IRInstCall : public IRInst
{
public:
	IRInstCall(IRValue *func, const std::vector<IRValue *> &args) : IRInst(nullptr), func(func), args(args)
	{
		IRFunctionType *functype;
		if (dynamic_cast<IRPointerType*>(func->type))
		{
			functype = dynamic_cast<IRFunctionType*>(func->type->getPointerElementType());
		}
		else
		{
			functype = dynamic_cast<IRFunctionType*>(func->type);
		}
		type = functype->returnType;
	}

	void visit(IRInstVisitor *visitor) { visitor->inst(this); }

	IRValue *func;
	std::vector<IRValue *> args;
};

class IRInstGEP : public IRInst
{
public:
	IRInstGEP(IRContext *context, IRValue *ptr, const std::vector<IRValue *> &indices) : IRInst(nullptr), ptr(ptr), indices(indices)
	{
		IRType *curType = ptr->type;
		for (IRValue *index : indices)
		{
			if (dynamic_cast<IRPointerType*>(curType))
			{
				curType = curType->getPointerElementType();
			}
			else
			{
				IRStructType *stype = dynamic_cast<IRStructType*>(curType);
				if (stype)
				{
					IRConstantInt *cindex = dynamic_cast<IRConstantInt*>(index);
					if (!cindex)
						throw std::runtime_error("Indexing into a structure must use a constant integer");
					curType = stype->elements[cindex->value];
				}
				else
				{
					throw std::runtime_error("Invalid arguments to GEP instruction");
				}
			}
		}
		type = curType->getPointerTo(context);
	}

	void visit(IRInstVisitor *visitor) { visitor->inst(this); }

	IRValue *ptr;
	std::vector<IRValue *> indices;
};

class IRInstBasicBlockEnd : public IRInst
{
public:
	IRInstBasicBlockEnd() : IRInst(nullptr) { }
};

class IRInstBr : public IRInstBasicBlockEnd
{
public:
	IRInstBr(IRBasicBlock *bb) : bb(bb) { }

	void visit(IRInstVisitor *visitor) { visitor->inst(this); }

	IRBasicBlock *bb;
};

class IRInstCondBr : public IRInstBasicBlockEnd
{
public:
	IRInstCondBr(IRValue *condition, IRBasicBlock *bb1, IRBasicBlock *bb2) : condition(condition), bb1(bb1), bb2(bb2) { }

	void visit(IRInstVisitor *visitor) { visitor->inst(this); }

	IRValue *condition;
	IRBasicBlock *bb1;
	IRBasicBlock *bb2;
};

class IRInstRet : public IRInstBasicBlockEnd
{
public:
	IRInstRet(IRValue *operand) : operand(operand) { }

	void visit(IRInstVisitor *visitor) { visitor->inst(this); }

	IRValue *operand;
};

class IRInstRetVoid : public IRInstBasicBlockEnd
{
public:
	void visit(IRInstVisitor *visitor) { visitor->inst(this); }
};

class IRInstAlloca : public IRInst
{
public:
	IRInstAlloca(IRContext *context, IRType *type, IRValue *arraySize, const std::string &name) : IRInst(type->getPointerTo(context)), arraySize(arraySize), name(name) { }

	void visit(IRInstVisitor *visitor) { visitor->inst(this); }

	IRValue *arraySize;
	std::string name;
};
