
#include "RegisterAllocatorAArch64.h"

void RegisterAllocatorAArch64::run(IRContext* context, MachineFunction* func)
{
	RegisterAllocatorAArch64 ra(context, func);
	ra.run();
}

void RegisterAllocatorAArch64::run()
{
	createRegisterInfo();
	runLiveAnalysis();

	setupArgs();

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

			updateModifiedStatus(inst);

			if (inst->opcode == (int)MachineInstOpcodeAArch64::bl || inst->opcode == (int)MachineInstOpcodeAArch64::blr)
			{
				for (RegisterNameAArch64 regName : volatileRegs)
				{
					int pregIndex = (int)regName;
					if (reginfo[pregIndex].vreg != -1)
					{
						assignVirt2StackSlot(reginfo[pregIndex].vreg);
						setAsLeastRecentlyUsed(pregIndex);
					}
				}

				// To avoid mov x0,x0 or mov xmm0,xmm0
				setAsMostRecentlyUsed((int)RegisterNameAArch64::x0);
				setAsMostRecentlyUsed((int)RegisterNameAArch64::xmm0);
			}

			if (inst->opcode == (int)MachineInstOpcodeAArch64::beq || inst->opcode == (int)MachineInstOpcodeAArch64::bne)
			{
				assignAllToStack();
			}
			else if (inst->opcode == (int)MachineInstOpcodeAArch64::b)
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

			if (inst->opcode == (int)MachineInstOpcodeAArch64::b && i + 1 < func->basicBlocks.size() && func->basicBlocks[i + 1] == inst->operands[0].bb)
				continue;

			emittedInstructions.push_back(inst);
		}

		bb->code.swap(emittedInstructions);
		emittedInstructions.clear();
	}

	for (RegisterNameAArch64 reg : volatileRegs)
		usedRegs.erase(reg);

	if (func->dynamicStackAllocations)
		usedRegs.insert(RegisterNameAArch64::fp);

	std::vector<RegisterNameAArch64> savedRegs, savedXmmRegs;
	for (RegisterNameAArch64 reg : usedRegs)
	{
		if (reg < RegisterNameAArch64::xmm0)
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
		RegisterNameAArch64::x19, RegisterNameAArch64::x20, RegisterNameAArch64::x21, RegisterNameAArch64::x22,
		RegisterNameAArch64::x23, RegisterNameAArch64::x24, RegisterNameAArch64::x25, RegisterNameAArch64::x26,
		RegisterNameAArch64::x27, RegisterNameAArch64::x28, RegisterNameAArch64::x29, RegisterNameAArch64::x30,
	};

	savedXmmRegs =
	{
		RegisterNameAArch64::xmm8, RegisterNameAArch64::xmm9, RegisterNameAArch64::xmm10, RegisterNameAArch64::xmm11,
		RegisterNameAArch64::xmm12, RegisterNameAArch64::xmm13, RegisterNameAArch64::xmm14, RegisterNameAArch64::xmm15
	};
#endif

	// [funcargs] <framebase> [saved regs] [spill area] <spillbase> [dsa/alloca area] [callargs] <sp>

	int savedRegsSize = alignUp((int)savedRegs.size() * 8, 16) + (int)savedXmmRegs.size() * 16;
	int spillAreaSize = alignUp(nextSpillOffset, 16);
	int callargsSize = alignUp(func->maxCallArgsSize, 16);
	int frameSize = savedRegsSize + spillAreaSize + callargsSize;

	func->frameBaseOffset = frameSize;
	func->spillBaseOffset = callargsSize;

	emitProlog(savedRegs, savedXmmRegs, frameSize, func->dynamicStackAllocations);
	emitEpilog(savedRegs, savedXmmRegs, frameSize, func->dynamicStackAllocations);
}

void RegisterAllocatorAArch64::updateModifiedStatus(MachineInst* inst)
{
	if (inst->operands.empty() || inst->operands[0].type != MachineOperandType::reg) return;

	MachineInstOpcodeAArch64 opcode = (MachineInstOpcodeAArch64)inst->opcode;
	if (opcode == MachineInstOpcodeAArch64::nop ||
		(opcode >= MachineInstOpcodeAArch64::storess && opcode <= MachineInstOpcodeAArch64::store8) ||
		(opcode >= MachineInstOpcodeAArch64::fcmpsd && opcode <= MachineInstOpcodeAArch64::cmp64_zx64) ||
		(opcode >= MachineInstOpcodeAArch64::b && opcode <= MachineInstOpcodeAArch64::blr))
	{
		return;
	}

	int pregIndex = inst->operands[0].registerIndex;
	int vregIndex = reginfo[pregIndex].vreg;
	if (vregIndex != -1)
		reginfo[vregIndex].modified = true;
}

void RegisterAllocatorAArch64::emitProlog(const std::vector<RegisterNameAArch64>& savedRegs, const std::vector<RegisterNameAArch64>& savedXmmRegs, int stackAdjustment, bool dsa)
{
	// sub sp, sp, stacksize
	if (stackAdjustment > 0)
	{
		MachineOperand dst;
		dst.type = MachineOperandType::reg;
		dst.registerIndex = (int)RegisterNameAArch64::sp;

		MachineOperand src;
		src.type = MachineOperandType::imm;
		src.immvalue = stackAdjustment;

		auto inst = context->newMachineInst();
		inst->opcode = (int)MachineInstOpcodeAArch64::sub64;
		inst->operands.push_back(dst);
		inst->operands.push_back(dst);
		inst->operands.push_back(src);
		inst->unwindHint = MachineUnwindHint::StackAdjustment;
		func->prolog->code.push_back(inst);
	}

	MachineOperand stacklocation;
	stacklocation.type = MachineOperandType::spillOffset;
	stacklocation.spillOffset = alignUp(nextSpillOffset, 16);

	for (RegisterNameAArch64 physReg : savedRegs)
	{
		MachineOperand src;
		src.type = MachineOperandType::reg;
		src.registerIndex = (int)physReg;

		auto inst = context->newMachineInst();
		inst->opcode = (int)MachineInstOpcodeAArch64::mov64;
		inst->operands.push_back(stacklocation);
		inst->operands.push_back(src);
		inst->unwindHint = MachineUnwindHint::RegisterStackLocation;
		func->prolog->code.push_back(inst);

		stacklocation.spillOffset += 8;
	}
	stacklocation.spillOffset = alignUp(stacklocation.spillOffset, 16);

	for (RegisterNameAArch64 physReg : savedXmmRegs)
	{
		MachineOperand src;
		src.type = MachineOperandType::reg;
		src.registerIndex = (int)physReg;

		auto inst = context->newMachineInst();
		inst->opcode = (int)MachineInstOpcodeAArch64::movsd;
		inst->operands.push_back(stacklocation);
		inst->operands.push_back(src);
		inst->unwindHint = MachineUnwindHint::RegisterStackLocation;
		func->prolog->code.push_back(inst);

		stacklocation.spillOffset += 16;
	}

	if (dsa)
	{
		MachineOperand dst;
		dst.type = MachineOperandType::reg;
		dst.registerIndex = (int)RegisterNameAArch64::fp;

		MachineOperand src;
		src.type = MachineOperandType::reg;
		src.registerIndex = (int)RegisterNameAArch64::sp;

		auto inst = context->newMachineInst();
		inst->opcode = (int)MachineInstOpcodeAArch64::mov64;
		inst->operands.push_back(dst);
		inst->operands.push_back(src);
		inst->unwindHint = MachineUnwindHint::SaveFrameRegister;
		func->prolog->code.push_back(inst);
	}
}

void RegisterAllocatorAArch64::emitEpilog(const std::vector<RegisterNameAArch64>& savedRegs, const std::vector<RegisterNameAArch64>& savedXmmRegs, int stackAdjustment, bool dsa)
{
	if (dsa)
	{
		MachineOperand dst;
		dst.type = MachineOperandType::reg;
		dst.registerIndex = (int)RegisterNameAArch64::sp;

		MachineOperand src;
		src.type = MachineOperandType::reg;
		src.registerIndex = (int)RegisterNameAArch64::fp;

		auto inst = context->newMachineInst();
		inst->opcode = (int)MachineInstOpcodeAArch64::mov64;
		inst->operands.push_back(dst);
		inst->operands.push_back(src);
		func->epilog->code.push_back(inst);
	}

	MachineOperand stacklocation;
	stacklocation.type = MachineOperandType::spillOffset;
	stacklocation.spillOffset = alignUp(nextSpillOffset, 16);

	for (RegisterNameAArch64 physReg : savedRegs)
	{
		MachineOperand dst;
		dst.type = MachineOperandType::reg;
		dst.registerIndex = (int)physReg;

		auto inst = context->newMachineInst();
		inst->opcode = (int)MachineInstOpcodeAArch64::mov64;
		inst->operands.push_back(dst);
		inst->operands.push_back(stacklocation);
		func->epilog->code.push_back(inst);

		stacklocation.spillOffset += 8;
	}
	stacklocation.spillOffset = alignUp(stacklocation.spillOffset, 16);

	for (RegisterNameAArch64 physReg : savedXmmRegs)
	{
		MachineOperand dst;
		dst.type = MachineOperandType::reg;
		dst.registerIndex = (int)physReg;

		auto inst = context->newMachineInst();
		inst->opcode = (int)MachineInstOpcodeAArch64::movsd;
		inst->operands.push_back(dst);
		inst->operands.push_back(stacklocation);
		func->epilog->code.push_back(inst);

		stacklocation.spillOffset += 16;
	}

	// add rsp, stacksize
	if (stackAdjustment > 0)
	{
		MachineOperand dst;
		dst.type = MachineOperandType::reg;
		dst.registerIndex = (int)RegisterNameAArch64::sp;

		MachineOperand src;
		src.type = MachineOperandType::imm;
		src.immvalue = stackAdjustment;

		auto inst = context->newMachineInst();
		inst->opcode = (int)MachineInstOpcodeAArch64::add64;
		inst->operands.push_back(dst);
		inst->operands.push_back(dst);
		inst->operands.push_back(src);
		func->epilog->code.push_back(inst);
	}

	{
		auto inst = context->newMachineInst();
		inst->opcode = (int)MachineInstOpcodeAArch64::ret;
		func->epilog->code.push_back(inst);
	}
}

void RegisterAllocatorAArch64::createRegisterInfo()
{
	reginfo.resize(func->registers.size());
	for (size_t i = 0; i < func->registers.size(); i++)
	{
		reginfo[i].cls = func->registers[i].cls;
	}

	for (int i = 0; i < (int)RegisterNameAArch64::vregstart; i++)
	{
		if (reginfo[i].cls != MachineRegClass::reserved)
		{
			RARegisterClassAArch64& cls = regclass[(int)reginfo[i].cls];
			cls.mruIt[i] = cls.mru.insert(cls.mru.end(), i);
		}
	}
}

void RegisterAllocatorAArch64::allocStackVars()
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

void RegisterAllocatorAArch64::setAllToStack()
{
	for (RARegisterInfoAArch64 &entry : reginfo)
	{
		if (entry.physreg != -1)
		{
			reginfo[entry.physreg].vreg = -1;
			entry.physreg = -1;
		}
	}
}

void RegisterAllocatorAArch64::assignAllToStack()
{
	for (size_t i = (int)RegisterNameAArch64::vregstart; i < reginfo.size(); i++)
	{
		int vregIndex = (int)i;
		assignVirt2StackSlot(vregIndex);
	}
}

void RegisterAllocatorAArch64::useRegister(MachineOperand& operand)
{
	if (operand.registerIndex < (int)RegisterNameAArch64::vregstart)
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

void RegisterAllocatorAArch64::killVirtRegister(int vregIndex)
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

void RegisterAllocatorAArch64::assignVirt2Phys(int vregIndex, int pregIndex)
{
	auto& vreg = reginfo[vregIndex];
	auto& physreg = reginfo[pregIndex];

	if (physreg.vreg == vregIndex) // Already assigned to physreg
		return;

	if (physreg.vreg != -1) // Already in use. Make it available.
		assignVirt2StackSlot(physreg.vreg);

	usedRegs.insert((RegisterNameAArch64)pregIndex);

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
		inst->opcode = (int)MachineInstOpcodeAArch64::add64;
		inst->operands.push_back(dest);
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
		inst->opcode = (int)(vreg.cls == MachineRegClass::gp ? MachineInstOpcodeAArch64::load64 : MachineInstOpcodeAArch64::loadsd);
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
		inst->opcode = (int)(vreg.cls == MachineRegClass::gp ? MachineInstOpcodeAArch64::mov64 : MachineInstOpcodeAArch64::movsd);
		inst->operands.push_back(dest);
		inst->operands.push_back(src);
		inst->comment = "move " + getVRegName(vregIndex);
		emittedInstructions.push_back(inst);

		reginfo[vreg.physreg].vreg = -1;
		physreg.vreg = vregIndex;
		vreg.physreg = pregIndex;
	}
}

void RegisterAllocatorAArch64::assignVirt2StackSlot(int vregIndex)
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
		inst->opcode = (int)(vreg.cls == MachineRegClass::gp ? MachineInstOpcodeAArch64::store64 : MachineInstOpcodeAArch64::storesd);
		inst->operands.push_back(dest);
		inst->operands.push_back(src);
		inst->comment = "save " + getVRegName(vregIndex);
		emittedInstructions.push_back(inst);

		vreg.modified = false;
	}

	physreg.vreg = -1;
	vreg.physreg = -1;
}

void RegisterAllocatorAArch64::setAsMostRecentlyUsed(int pregIndex)
{
	if (reginfo[pregIndex].cls != MachineRegClass::reserved)
	{
		RARegisterClassAArch64& cls = regclass[(int)reginfo[pregIndex].cls];
		cls.mru.erase(cls.mruIt[pregIndex]);
		cls.mruIt[pregIndex] = cls.mru.insert(cls.mru.end(), pregIndex);
	}
}

void RegisterAllocatorAArch64::setAsLeastRecentlyUsed(int pregIndex)
{
	if (reginfo[pregIndex].cls != MachineRegClass::reserved)
	{
		RARegisterClassAArch64& cls = regclass[(int)reginfo[pregIndex].cls];
		cls.mru.erase(cls.mruIt[pregIndex]);
		cls.mruIt[pregIndex] = cls.mru.insert(cls.mru.begin(), pregIndex);
	}
}

int RegisterAllocatorAArch64::getLeastRecentlyUsed(MachineRegClass cls)
{
	return regclass[(int)cls].mru.front();
}

void RegisterAllocatorAArch64::runLiveAnalysis()
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

void RegisterAllocatorAArch64::addLiveReference(size_t vregIndex, MachineBasicBlock* bb)
{
	RARegisterInfoAArch64& vreg = reginfo[vregIndex];
	if (vreg.liveReferences)
	{
		RARegisterLiveReferenceAArch64* ref = vreg.liveReferences.get();
		while (ref->bb != bb && ref->next)
			ref = ref->next.get();

		if (ref->bb == bb)
		{
			ref->refcount++;
		}
		else
		{
			ref->next = std::make_unique<RARegisterLiveReferenceAArch64>(bb);
		}
	}
	else
	{
		vreg.liveReferences = std::make_unique<RARegisterLiveReferenceAArch64>(bb);
	}
}

void RegisterAllocatorAArch64::setupArgs()
{
	const int numRegisterArgs = 8;

	MachineOperand stacklocation;
	stacklocation.type = MachineOperandType::frameOffset;
	stacklocation.frameOffset = 0;

	IRFunctionType* functype = dynamic_cast<IRFunctionType*>(func->type);
	for (int i = 0; i < (int)functype->args.size(); i++)
	{
		int vregindex = (int)RegisterNameAArch64::vregstart + i;

		reginfo[vregindex].stacklocation = stacklocation;
		stacklocation.frameOffset += 8;

		if (i < numRegisterArgs)
		{
			int pregIndex;
			if (reginfo[vregindex].cls == MachineRegClass::gp)
				pregIndex = (int)RegisterNameAArch64::x0 + (int)i;
			else
				pregIndex = (int)RegisterNameAArch64::xmm0 + (int)i;

			reginfo[vregindex].modified = true;
			reginfo[vregindex].physreg = pregIndex;
			reginfo[pregIndex].vreg = vregindex;

			setAsMostRecentlyUsed(pregIndex);
		}
	}

	volatileRegs =
	{
		RegisterNameAArch64::x0, RegisterNameAArch64::x1, RegisterNameAArch64::x2, RegisterNameAArch64::x3,
		RegisterNameAArch64::x4, RegisterNameAArch64::x5, RegisterNameAArch64::x6, RegisterNameAArch64::x7,
		RegisterNameAArch64::x8, RegisterNameAArch64::x9, RegisterNameAArch64::x10, RegisterNameAArch64::x11,
		RegisterNameAArch64::x12, RegisterNameAArch64::x13, RegisterNameAArch64::x14, RegisterNameAArch64::x15,
		RegisterNameAArch64::xmm0, RegisterNameAArch64::xmm1, RegisterNameAArch64::xmm2, RegisterNameAArch64::xmm3,
		RegisterNameAArch64::xmm4, RegisterNameAArch64::xmm5, RegisterNameAArch64::xmm6, RegisterNameAArch64::xmm7,
		RegisterNameAArch64::xmm16, RegisterNameAArch64::xmm17, RegisterNameAArch64::xmm18, RegisterNameAArch64::xmm19,
		RegisterNameAArch64::xmm20, RegisterNameAArch64::xmm21, RegisterNameAArch64::xmm22, RegisterNameAArch64::xmm23,
		RegisterNameAArch64::xmm24, RegisterNameAArch64::xmm25, RegisterNameAArch64::xmm26, RegisterNameAArch64::xmm27,
		RegisterNameAArch64::xmm28, RegisterNameAArch64::xmm29, RegisterNameAArch64::xmm30, RegisterNameAArch64::xmm31,
	};
}

std::string RegisterAllocatorAArch64::getVRegName(size_t vregIndex)
{
	if (reginfo[vregIndex].name)
		return *reginfo[vregIndex].name;
	else
		return "vreg" + std::to_string(vregIndex);
}
