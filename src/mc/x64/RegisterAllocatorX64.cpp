
#include "RegisterAllocatorX64.h"

void RegisterAllocatorX64::run(IRContext* context, MachineFunction* func)
{
	RegisterAllocatorX64 ra(context, func);
	ra.run();
}

void RegisterAllocatorX64::run()
{
	createRegisterInfo();
	runLiveAnalysis();

#ifdef WIN32
	setupArgsWin64();
#else
	setupArgsUnix64();
#endif

	allocStackVars();

	for (size_t i = 0; i < func->basicBlocks.size(); i++)
	{
		MachineBasicBlock* bb = func->basicBlocks[i];
		for (MachineInst* inst : bb->code)
		{
			for (MachineOperand& operand : inst->operands)
			{
				if (operand.type == MachineOperandType::reg)
				{
					useRegister(operand);
				}
			}

			//std::string comment;
			for (MachineOperand& operand : inst->operands)
			{
				if (operand.type == MachineOperandType::reg)
				{
					int pregIndex = operand.registerIndex;
					int vregIndex = reginfo[pregIndex].vreg;
					if (vregIndex != -1)
					{
						// If vreg is only used by one BB we can safely kill it once we get to the last inst using it
						auto& liveref = reginfo[vregIndex].liveReferences;
						if (liveref && liveref->bb == bb && !liveref->next)
						{
							liveref->refcount--;
							if (liveref->refcount == 0)
							{
								killVirtRegister(vregIndex);

								/*if (comment.empty())
									comment = "kill vreg ";
								else
									comment += ", ";
								comment += getVRegName(vregIndex);*/
							}
						}
					}
				}
			}
			//if (!comment.empty())
			//	inst->comment = comment;

			if ((inst->opcode >= (int)MachineInstOpcodeX64::idiv64 && inst->opcode <= (int)MachineInstOpcodeX64::idiv8) ||
				(inst->opcode >= (int)MachineInstOpcodeX64::div64 && inst->opcode <= (int)MachineInstOpcodeX64::div8))
			{
				usedRegs.insert(RegisterNameX64::rax);
				usedRegs.insert(RegisterNameX64::rdx);
			}

			updateModifiedStatus(inst);

			if (inst->opcode == (int)MachineInstOpcodeX64::call)
			{
				for (RegisterNameX64 regName : volatileRegs)
				{
					int pregIndex = (int)regName;
					if (reginfo[pregIndex].vreg != -1)
					{
						assignVirt2StackSlot(reginfo[pregIndex].vreg);
						setAsLeastRecentlyUsed(pregIndex);
					}
				}

				// To avoid mov rax,rax or mov xmm0,xmm0
				setAsMostRecentlyUsed((int)RegisterNameX64::rax);
				setAsMostRecentlyUsed((int)RegisterNameX64::xmm0);
			}

			if (inst->opcode == (int)MachineInstOpcodeX64::je || inst->opcode == (int)MachineInstOpcodeX64::jne)
			{
				assignAllToStack();
			}
			else if (inst->opcode == (int)MachineInstOpcodeX64::jmp)
			{
				if (inst->operands[0].bb == func->epilog)
				{
					setAllToStack();
				}
				else
				{
					assignAllToStack();
				}
			}

			if (inst->opcode == (int)MachineInstOpcodeX64::jmp && i + 1 < func->basicBlocks.size() && func->basicBlocks[i + 1] == inst->operands[0].bb)
				continue;

			emittedInstructions.push_back(inst);
		}

		bb->code.swap(emittedInstructions);
		emittedInstructions.clear();
	}

	for (RegisterNameX64 reg : volatileRegs)
		usedRegs.erase(reg);

	usedRegs.erase(RegisterNameX64::rsp);
	if (func->dynamicStackAllocations)
		usedRegs.insert(RegisterNameX64::rbp);

	std::vector<RegisterNameX64> savedRegs, savedXmmRegs;
	for (RegisterNameX64 reg : usedRegs)
	{
		if (reg < RegisterNameX64::xmm0)
		{
			savedRegs.push_back(reg);
		}
		else
		{
			savedXmmRegs.push_back(reg);
		}
	}

#if 0 // Force save all non-volatile registers (for debugging)
	savedRegs =
	{
		RegisterNameX64::rbx,
		RegisterNameX64::rsp, RegisterNameX64::rbp, RegisterNameX64::rsi, RegisterNameX64::rdi,
		RegisterNameX64::r12, RegisterNameX64::r13, RegisterNameX64::r14, RegisterNameX64::r15
	};

	savedXmmRegs =
	{
		RegisterNameX64::xmm8, RegisterNameX64::xmm9, RegisterNameX64::xmm10, RegisterNameX64::xmm11,
		RegisterNameX64::xmm12, RegisterNameX64::xmm13, RegisterNameX64::xmm14, RegisterNameX64::xmm15
	};
#endif

	// [funcargs] [retaddr] <framebase> [save/spill area] <spillbase> [dsa/alloca area] [callargs] <rsp>

	int callReturnAddr = 8;
	int pushStackAdjustment = (int)savedRegs.size() * 8;
	int spillAreaSize = (int)savedXmmRegs.size() * 16 + nextSpillOffset;
	int callargsSize = (func->maxCallArgsSize + 15) / 16 * 16;

	if ((savedRegs.size() & 1) == 0) // xmm must be 16-byte aligned
		spillAreaSize += 8;

	// rsp is 16-byte aligned + 8 bytes (for the call return address) when entering the prolog.
	// Alloca needs to be 16-byte aligned, so the sum of the return address, the pushed registers and spill area needs to end at a 16-byte boundary
	int allocaStart = (callReturnAddr + pushStackAdjustment + spillAreaSize + 15) / 16 * 16;
	spillAreaSize = allocaStart - pushStackAdjustment - callReturnAddr;

	int frameSize = pushStackAdjustment + spillAreaSize + callargsSize;
	int frameStackAdjustment = frameSize - pushStackAdjustment;

	func->frameBaseOffset = frameSize;
	func->spillBaseOffset = callargsSize;

	emitProlog(savedRegs, savedXmmRegs, frameStackAdjustment, func->dynamicStackAllocations);
	emitEpilog(savedRegs, savedXmmRegs, frameStackAdjustment, func->dynamicStackAllocations);
}

void RegisterAllocatorX64::updateModifiedStatus(MachineInst* inst)
{
	if (inst->operands.empty() || inst->operands[0].type != MachineOperandType::reg) return;

	MachineInstOpcodeX64 opcode = (MachineInstOpcodeX64)inst->opcode;
	if (opcode == MachineInstOpcodeX64::nop ||
		(opcode >= MachineInstOpcodeX64::storess && opcode <= MachineInstOpcodeX64::store8) ||
		(opcode >= MachineInstOpcodeX64::idiv64 && opcode <= MachineInstOpcodeX64::idiv8) ||
		(opcode >= MachineInstOpcodeX64::div64 && opcode <= MachineInstOpcodeX64::div8) ||
		(opcode >= MachineInstOpcodeX64::ucomisd && opcode <= MachineInstOpcodeX64::cmp8) ||
		(opcode >= MachineInstOpcodeX64::jmp && opcode <= MachineInstOpcodeX64::pop))
	{
		return;
	}

	int pregIndex = inst->operands[0].registerIndex;
	int vregIndex = reginfo[pregIndex].vreg;
	if (vregIndex != -1)
		reginfo[vregIndex].modified = true;
}

void RegisterAllocatorX64::emitProlog(const std::vector<RegisterNameX64>& savedRegs, const std::vector<RegisterNameX64>& savedXmmRegs, int stackAdjustment, bool dsa)
{
	for (RegisterNameX64 physReg : savedRegs)
	{
		MachineOperand src;
		src.type = MachineOperandType::reg;
		src.registerIndex = (int)physReg;

		auto inst = context->newMachineInst();
		inst->opcode = (int)MachineInstOpcodeX64::push;
		inst->operands.push_back(src);
		inst->unwindHint = MachineUnwindHint::PushNonvolatile;
		func->prolog->code.push_back(inst);
	}

	// sub rsp, stacksize
	if (stackAdjustment > 0)
	{
		MachineOperand dst;
		dst.type = MachineOperandType::reg;
		dst.registerIndex = (int)RegisterNameX64::rsp;

		MachineOperand src;
		src.type = MachineOperandType::imm;
		src.immvalue = stackAdjustment;

		auto inst = context->newMachineInst();
		inst->opcode = (int)MachineInstOpcodeX64::sub64;
		inst->operands.push_back(dst);
		inst->operands.push_back(src);
		inst->unwindHint = MachineUnwindHint::StackAdjustment;
		func->prolog->code.push_back(inst);
	}

	int xmmStartOffset = -(int)savedRegs.size() * 8;
	if ((savedRegs.size() & 1) == 0) // xmm must be 16-byte aligned
		xmmStartOffset -= 8;

	int i = 1;
	for (RegisterNameX64 physReg : savedXmmRegs)
	{
		MachineOperand dst;
		dst.type = MachineOperandType::frameOffset;
		dst.frameOffset = xmmStartOffset - i * 16;

		MachineOperand src;
		src.type = MachineOperandType::reg;
		src.registerIndex = (int)physReg;

		auto inst = context->newMachineInst();
		inst->opcode = (int)MachineInstOpcodeX64::movdqa;
		inst->operands.push_back(dst);
		inst->operands.push_back(src);
		inst->unwindHint = MachineUnwindHint::RegisterStackLocation;
		func->prolog->code.push_back(inst);
		i++;
	}

	if (dsa)
	{
		MachineOperand dst;
		dst.type = MachineOperandType::reg;
		dst.registerIndex = (int)RegisterNameX64::rbp;

		MachineOperand src;
		src.type = MachineOperandType::reg;
		src.registerIndex = (int)RegisterNameX64::rsp;

		auto inst = context->newMachineInst();
		inst->opcode = (int)MachineInstOpcodeX64::mov64;
		inst->operands.push_back(dst);
		inst->operands.push_back(src);
		inst->unwindHint = MachineUnwindHint::SaveFrameRegister;
		func->prolog->code.push_back(inst);
	}
}

void RegisterAllocatorX64::emitEpilog(const std::vector<RegisterNameX64>& savedRegs, const std::vector<RegisterNameX64>& savedXmmRegs, int stackAdjustment, bool dsa)
{
	if (dsa)
	{
		MachineOperand dst;
		dst.type = MachineOperandType::reg;
		dst.registerIndex = (int)RegisterNameX64::rsp;

		MachineOperand src;
		src.type = MachineOperandType::reg;
		src.registerIndex = (int)RegisterNameX64::rbp;

		auto inst = context->newMachineInst();
		inst->opcode = (int)MachineInstOpcodeX64::mov64;
		inst->operands.push_back(dst);
		inst->operands.push_back(src);
		func->epilog->code.push_back(inst);
	}

	int xmmStartOffset = -(int)savedRegs.size() * 8;
	if ((savedRegs.size() & 1) == 0) // xmm must be 16-byte aligned
		xmmStartOffset -= 8;

	int i = 1;
	for (RegisterNameX64 physReg : savedXmmRegs)
	{
		MachineOperand dst;
		dst.type = MachineOperandType::reg;
		dst.registerIndex = (int)physReg;

		MachineOperand src;
		src.type = MachineOperandType::frameOffset;
		src.frameOffset = xmmStartOffset - i * 16;

		auto inst = context->newMachineInst();
		inst->opcode = (int)MachineInstOpcodeX64::movdqa;
		inst->operands.push_back(dst);
		inst->operands.push_back(src);
		func->epilog->code.push_back(inst);
		i++;
	}

	// add rsp, stacksize
	if (stackAdjustment > 0)
	{
		MachineOperand dst;
		dst.type = MachineOperandType::reg;
		dst.registerIndex = (int)RegisterNameX64::rsp;

		MachineOperand src;
		src.type = MachineOperandType::imm;
		src.immvalue = stackAdjustment;

		auto inst = context->newMachineInst();
		inst->opcode = (int)MachineInstOpcodeX64::add64;
		inst->operands.push_back(dst);
		inst->operands.push_back(src);
		func->epilog->code.push_back(inst);
	}

	for (auto it = savedRegs.rbegin(); it != savedRegs.rend(); ++it)
	{
		RegisterNameX64 physReg = *it;

		MachineOperand dst;
		dst.type = MachineOperandType::reg;
		dst.registerIndex = (int)physReg;

		auto inst = context->newMachineInst();
		inst->opcode = (int)MachineInstOpcodeX64::pop;
		inst->operands.push_back(dst);
		func->epilog->code.push_back(inst);
	}

	{
		auto inst = context->newMachineInst();
		inst->opcode = (int)MachineInstOpcodeX64::ret;
		func->epilog->code.push_back(inst);
	}
}

void RegisterAllocatorX64::createRegisterInfo()
{
	reginfo.resize(func->registers.size());
	for (size_t i = 0; i < func->registers.size(); i++)
	{
		reginfo[i].cls = func->registers[i].cls;
	}

	for (int i = 0; i < (int)RegisterNameX64::vregstart; i++)
	{
		if (reginfo[i].cls != MachineRegClass::reserved)
		{
			RARegisterClassX64& cls = regclass[(int)reginfo[i].cls];
			cls.mruIt[i] = cls.mru.insert(cls.mru.end(), i);
		}
	}
}

void RegisterAllocatorX64::allocStackVars()
{
	for (const MachineStackAlloc& stackvar : func->stackvars)
	{
		auto& vreg = reginfo[stackvar.registerIndex];
		vreg.stacklocation.spillOffset = nextSpillOffset;
		vreg.stackvar = true;
		vreg.name = &stackvar.name;
		nextSpillOffset += (int)stackvar.size;
	}
}

void RegisterAllocatorX64::setAllToStack()
{
	for (RARegisterInfoX64 &entry : reginfo)
	{
		if (entry.physreg != -1)
		{
			reginfo[entry.physreg].vreg = -1;
			entry.physreg = -1;
		}
	}
}

void RegisterAllocatorX64::assignAllToStack()
{
	for (size_t i = (int)RegisterNameX64::vregstart; i < reginfo.size(); i++)
	{
		int vregIndex = (int)i;
		assignVirt2StackSlot(vregIndex);
	}
}

void RegisterAllocatorX64::useRegister(MachineOperand& operand)
{
	if (operand.registerIndex < (int)RegisterNameX64::vregstart)
	{
		int pregIndex = operand.registerIndex;

		auto& physreg = reginfo[pregIndex];
		if (physreg.vreg != -1)
		{
			assignVirt2StackSlot(physreg.vreg);
		}

		setAsMostRecentlyUsed(pregIndex);
	}
	else
	{
		int vregIndex = operand.registerIndex;
		auto& vreg = reginfo[vregIndex];
		if (vreg.physreg == -1)
		{
			int pregIndex = getLeastRecentlyUsed(vreg.cls);
			assignVirt2Phys(vregIndex, pregIndex);
		}

		setAsMostRecentlyUsed(vreg.physreg);
		operand.registerIndex = vreg.physreg;
	}
}

void RegisterAllocatorX64::killVirtRegister(int vregIndex)
{
	auto& vreg = reginfo[vregIndex];
	if (vreg.physreg != -1)
	{
		int pregIndex = vreg.physreg;
		auto& physreg = reginfo[pregIndex];

		physreg.vreg = -1;
		vreg.physreg = -1;

		setAsLeastRecentlyUsed(pregIndex);
	}

	if (!vreg.stackvar && vreg.stacklocation.type == MachineOperandType::spillOffset && vreg.stacklocation.spillOffset != -1)
		freeSpillOffsets.push_back(vreg.stacklocation.spillOffset);
}

void RegisterAllocatorX64::assignVirt2Phys(int vregIndex, int pregIndex)
{
	auto& vreg = reginfo[vregIndex];
	auto& physreg = reginfo[pregIndex];

	if (physreg.vreg == vregIndex) // Already assigned to physreg
		return;

	if (physreg.vreg != -1) // Already in use. Make it available.
		assignVirt2StackSlot(physreg.vreg);

	usedRegs.insert((RegisterNameX64)pregIndex);

	if (vreg.physreg == -1 && vreg.stacklocation.spillOffset == -1) // first time vreg is used
	{
		vreg.physreg = pregIndex;
		physreg.vreg = vregIndex;
	}
	else if (vreg.stackvar) // ptr to stack location
	{
		MachineOperand dest;
		dest.type = MachineOperandType::reg;
		dest.registerIndex = pregIndex;

		MachineOperand src = vreg.stacklocation;

		auto inst = context->newMachineInst();
		inst->opcode = (int)MachineInstOpcodeX64::lea;
		inst->operands.push_back(dest);
		inst->operands.push_back(src);
		inst->comment = "ptr " + getVRegName(vregIndex);
		emittedInstructions.push_back(inst);

		physreg.vreg = vregIndex;
		vreg.physreg = pregIndex;
	}
	else if (vreg.physreg == -1) // move from stack
	{
		MachineOperand dest;
		dest.type = MachineOperandType::reg;
		dest.registerIndex = pregIndex;

		MachineOperand src = vreg.stacklocation;

		auto inst = context->newMachineInst();
		inst->opcode = (int)(vreg.cls == MachineRegClass::gp ? MachineInstOpcodeX64::load64 : MachineInstOpcodeX64::loadsd);
		inst->operands.push_back(dest);
		inst->operands.push_back(src);
		inst->comment = "load " + getVRegName(vregIndex);
		emittedInstructions.push_back(inst);

		physreg.vreg = vregIndex;
		vreg.physreg = pregIndex;
	}
	else // move from other register
	{
		MachineOperand dest;
		dest.type = MachineOperandType::reg;
		dest.registerIndex = pregIndex;

		MachineOperand src;
		src.type = MachineOperandType::reg;
		src.registerIndex = vreg.physreg;

		auto inst = context->newMachineInst();
		inst->opcode = (int)(vreg.cls == MachineRegClass::gp ? MachineInstOpcodeX64::mov64 : MachineInstOpcodeX64::movsd);
		inst->operands.push_back(dest);
		inst->operands.push_back(src);
		inst->comment = "move " + getVRegName(vregIndex);
		emittedInstructions.push_back(inst);

		reginfo[vreg.physreg].vreg = -1;
		physreg.vreg = vregIndex;
		vreg.physreg = pregIndex;
	}
}

void RegisterAllocatorX64::assignVirt2StackSlot(int vregIndex)
{
	auto& vreg = reginfo[vregIndex];
	if (vreg.physreg == -1)
		return;

	auto& physreg = reginfo[vreg.physreg];

	if (vreg.modified)
	{
		if (vreg.stacklocation.spillOffset == -1)
		{
			if (freeSpillOffsets.empty())
			{
				vreg.stacklocation.spillOffset = nextSpillOffset;
				nextSpillOffset += 8;
			}
			else
			{
				vreg.stacklocation.spillOffset = freeSpillOffsets.back();
				freeSpillOffsets.pop_back();
			}
		}

		// emit move from register to stack:

		MachineOperand dest = vreg.stacklocation;

		MachineOperand src;
		src.type = MachineOperandType::reg;
		src.registerIndex = vreg.physreg;

		auto inst = context->newMachineInst();
		inst->opcode = (int)(vreg.cls == MachineRegClass::gp ? MachineInstOpcodeX64::store64 : MachineInstOpcodeX64::storesd);
		inst->operands.push_back(dest);
		inst->operands.push_back(src);
		inst->comment = "save " + getVRegName(vregIndex);
		emittedInstructions.push_back(inst);

		vreg.modified = false;
	}

	physreg.vreg = -1;
	vreg.physreg = -1;
}

void RegisterAllocatorX64::setAsMostRecentlyUsed(int pregIndex)
{
	if (reginfo[pregIndex].cls != MachineRegClass::reserved)
	{
		RARegisterClassX64& cls = regclass[(int)reginfo[pregIndex].cls];
		cls.mru.erase(cls.mruIt[pregIndex]);
		cls.mruIt[pregIndex] = cls.mru.insert(cls.mru.end(), pregIndex);
	}
}

void RegisterAllocatorX64::setAsLeastRecentlyUsed(int pregIndex)
{
	if (reginfo[pregIndex].cls != MachineRegClass::reserved)
	{
		RARegisterClassX64& cls = regclass[(int)reginfo[pregIndex].cls];
		cls.mru.erase(cls.mruIt[pregIndex]);
		cls.mruIt[pregIndex] = cls.mru.insert(cls.mru.begin(), pregIndex);
	}
}

int RegisterAllocatorX64::getLeastRecentlyUsed(MachineRegClass cls)
{
	return regclass[(int)cls].mru.front();
}

void RegisterAllocatorX64::runLiveAnalysis()
{
	for (MachineBasicBlock* bb : func->basicBlocks)
	{
		for (MachineInst* inst : bb->code)
		{
			for (const MachineOperand& operand : inst->operands)
			{
				if (operand.type == MachineOperandType::reg)
				{
					addLiveReference(operand.registerIndex, bb);
				}
			}
		}
	}
}

void RegisterAllocatorX64::addLiveReference(size_t vregIndex, MachineBasicBlock* bb)
{
	RARegisterInfoX64& vreg = reginfo[vregIndex];
	if (vreg.liveReferences)
	{
		RARegisterLiveReferenceX64* ref = vreg.liveReferences.get();
		while (ref->bb != bb && ref->next)
			ref = ref->next.get();

		if (ref->bb == bb)
		{
			ref->refcount++;
		}
		else
		{
			ref->next = std::make_unique<RARegisterLiveReferenceX64>(bb);
		}
	}
	else
	{
		vreg.liveReferences = std::make_unique<RARegisterLiveReferenceX64>(bb);
	}
}

void RegisterAllocatorX64::setupArgsWin64()
{
	static const RegisterNameX64 registerArgs[] = { RegisterNameX64::rcx, RegisterNameX64::rdx, RegisterNameX64::r8, RegisterNameX64::r9 };
	const int numRegisterArgs = 4;

	MachineOperand stacklocation;
	stacklocation.type = MachineOperandType::frameOffset;
	stacklocation.frameOffset = 8;

	IRFunctionType* functype = dynamic_cast<IRFunctionType*>(func->type);
	for (int i = 0; i < (int)functype->args.size(); i++)
	{
		int vregindex = (int)RegisterNameX64::vregstart + i;

		reginfo[vregindex].stacklocation = stacklocation;
		stacklocation.frameOffset += 8;

		if (i < numRegisterArgs)
		{
			int pregIndex;
			if (reginfo[vregindex].cls == MachineRegClass::gp)
				pregIndex = (int)registerArgs[i];
			else
				pregIndex = (int)RegisterNameX64::xmm0 + (int)i;

			reginfo[vregindex].modified = true;
			reginfo[vregindex].physreg = pregIndex;
			reginfo[pregIndex].vreg = vregindex;

			setAsMostRecentlyUsed(pregIndex);
		}
	}

	volatileRegs =
	{
		RegisterNameX64::rax, RegisterNameX64::rcx, RegisterNameX64::rdx, RegisterNameX64::r8, RegisterNameX64::r9, RegisterNameX64::r10, RegisterNameX64::r11,
		RegisterNameX64::xmm0, RegisterNameX64::xmm1, RegisterNameX64::xmm2, RegisterNameX64::xmm3, RegisterNameX64::xmm4, RegisterNameX64::xmm5
	};
}

void RegisterAllocatorX64::setupArgsUnix64()
{
	static const RegisterNameX64 registerArgs[] = { RegisterNameX64::rdi, RegisterNameX64::rsi, RegisterNameX64::rdx, RegisterNameX64::rcx, RegisterNameX64::r8, RegisterNameX64::r9 };
	const int numRegisterArgs = 6;
	const int numXmmRegisterArgs = 8;
	int nextRegisterArg = 0;
	int nextXmmRegisterArg = 0;

	MachineOperand stacklocation;
	stacklocation.type = MachineOperandType::frameOffset;
	stacklocation.frameOffset = 8;

	IRFunctionType* functype = dynamic_cast<IRFunctionType*>(func->type);
	for (int i = 0; i < (int)functype->args.size(); i++)
	{
		int vregindex = (int)RegisterNameX64::vregstart + i;

		if (reginfo[vregindex].cls == MachineRegClass::gp)
		{
			if (nextRegisterArg < numRegisterArgs)
			{
				int pregIndex = (int)registerArgs[nextRegisterArg++];
				reginfo[vregindex].physreg = pregIndex;
				reginfo[pregIndex].vreg = vregindex;
				setAsMostRecentlyUsed(pregIndex);
			}
			else
			{
				reginfo[vregindex].stacklocation = stacklocation;
				stacklocation.frameOffset += 8;
			}
		}
		else
		{
			if (nextXmmRegisterArg < numXmmRegisterArgs)
			{
				int pregIndex = (int)RegisterNameX64::xmm0 + (int)(nextXmmRegisterArg++);
				reginfo[vregindex].physreg = pregIndex;
				reginfo[pregIndex].vreg = vregindex;
				setAsMostRecentlyUsed(pregIndex);
			}
			else
			{
				reginfo[vregindex].stacklocation = stacklocation;
				stacklocation.frameOffset += 8;
			}
		}
	}

	volatileRegs =
	{
		RegisterNameX64::rax, RegisterNameX64::rcx, RegisterNameX64::rdx, RegisterNameX64::rdi, RegisterNameX64::rsi,
		RegisterNameX64::xmm0, RegisterNameX64::xmm1, RegisterNameX64::xmm2, RegisterNameX64::xmm3, RegisterNameX64::xmm4, RegisterNameX64::xmm5, RegisterNameX64::xmm6, RegisterNameX64::xmm7,
		RegisterNameX64::xmm8, RegisterNameX64::xmm9, RegisterNameX64::xmm10, RegisterNameX64::xmm11, RegisterNameX64::xmm12, RegisterNameX64::xmm13, RegisterNameX64::xmm14, RegisterNameX64::xmm15
	};
}

std::string RegisterAllocatorX64::getVRegName(size_t vregIndex)
{
	if (reginfo[vregIndex].name)
		return *reginfo[vregIndex].name;
	else
		return "vreg" + std::to_string(vregIndex);
}
