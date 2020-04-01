
#include "RegisterAllocator.h"

void RegisterAllocator::run(IRContext* context, MachineFunction* func)
{
	RegisterAllocator ra(context, func);
	ra.run();
}

void RegisterAllocator::run()
{
	createRegisterInfo();

	//static const RegisterName unixregvars[] = { RegisterName::rdi, RegisterName::rsi, RegisterName::rdx, RegisterName::rcx, RegisterName::r8, RegisterName::r9 };
	static const RegisterName win64regvars[] = { RegisterName::rcx, RegisterName::rdx, RegisterName::r8, RegisterName::r9 };

	int argoffset = -8 * 8; // 8 pushed registers

	IRFunctionType* functype = dynamic_cast<IRFunctionType*>(func->type);
	for (int i = 0; i < (int)functype->args.size(); i++)
	{
		argoffset -= 8;

		int vregindex = (int)RegisterName::vregstart + i;
		reginfo[vregindex].stackoffset = argoffset;

		if (i < 4)
		{
			int pregIndex;
			if (reginfo[vregindex].cls == MachineRegClass::gp)
				pregIndex = (int)win64regvars[i];
			else
				pregIndex = (int)RegisterName::xmm0 + (int)i;

			reginfo[vregindex].spilled = false;
			reginfo[vregindex].physreg = pregIndex;
			reginfo[pregIndex].vreg = (int)i;

			auto& mru = mruPhys[(int)reginfo[pregIndex].cls];
			mru.erase(reginfo[pregIndex].mruIt);
			reginfo[pregIndex].mruIt = mru.insert(mru.end(), pregIndex);
		}
	}

	for (size_t i = 0; i < func->basicBlocks.size(); i++)
	{
		MachineBasicBlock* bb = func->basicBlocks[i];
		for (MachineInst* inst : bb->code)
		{
			for (MachineOperand& operand : inst->operands)
			{
				if (operand.type == MachineOperandType::reg)
				{
					if (operand.registerIndex < (int)RegisterName::vregstart)
					{
						usePhysRegister(operand);
					}
					else
					{
						useVirtRegister(operand);
					}
				}
			}

			if (inst->opcode == MachineInstOpcode::jz || inst->opcode == MachineInstOpcode::jmp)
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

			emittedInstructions.push_back(inst);
		}

		bb->code.swap(emittedInstructions);
		emittedInstructions.clear();
	}

	func->stackSize = nextStackOffset;

	// To do: only save the registers we used
	//std::vector<RegisterName> savedRegsUnix = { RegisterName::rbx, RegisterName::rbp, RegisterName::r12, RegisterName::r13, RegisterName::r14, RegisterName::r15 };
	std::vector<RegisterName> savedRegsWin64 = { RegisterName::rbx, RegisterName::rbp, RegisterName::rdi, RegisterName::rsi, RegisterName::r12, RegisterName::r13, RegisterName::r14, RegisterName::r15 };
	std::vector<RegisterName> savedXmmRegs = { RegisterName::xmm6, RegisterName::xmm7, RegisterName::xmm8, RegisterName::xmm9, RegisterName::xmm10, RegisterName::xmm11, RegisterName::xmm12, RegisterName::xmm13, RegisterName::xmm14, RegisterName::xmm15 };
	func->stackSize += (int)savedXmmRegs.size() * 8;
	emitProlog(savedRegsWin64, savedXmmRegs, func->stackSize);
	emitEpilog(savedRegsWin64, savedXmmRegs, func->stackSize);
}

void RegisterAllocator::emitProlog(const std::vector<RegisterName>& savedRegs, const std::vector<RegisterName>& savedXmmRegs, int stackSize)
{
	for (RegisterName physReg : savedRegs)
	{
		MachineOperand src;
		src.type = MachineOperandType::reg;
		src.registerIndex = (int)physReg;

		auto inst = context->newMachineInst();
		inst->opcode = MachineInstOpcode::push;
		inst->operands.push_back(src);
		inst->unwindHint = MachineUnwindHint::PushNonvolatile;
		func->prolog->code.push_back(inst);
	}

	// sub rsp, stacksize
	{
		MachineOperand dst;
		dst.type = MachineOperandType::reg;
		dst.registerIndex = (int)RegisterName::rsp;

		MachineOperand src;
		src.type = MachineOperandType::imm;
		src.immvalue = stackSize;

		auto inst = context->newMachineInst();
		inst->opcode = MachineInstOpcode::sub64;
		inst->operands.push_back(dst);
		inst->operands.push_back(src);
		inst->unwindHint = MachineUnwindHint::StackAdjustment;
		func->prolog->code.push_back(inst);
	}

	int i = 1;
	for (RegisterName physReg : savedXmmRegs)
	{
		MachineOperand dst;
		dst.type = MachineOperandType::stack;
		dst.stackOffset = nextStackOffset + i * 8;

		MachineOperand src;
		src.type = MachineOperandType::reg;
		src.registerIndex = (int)physReg;

		auto inst = context->newMachineInst();
		inst->opcode = MachineInstOpcode::storesd;
		inst->operands.push_back(dst);
		inst->operands.push_back(src);
		inst->unwindHint = MachineUnwindHint::RegisterStackLocation;
		func->prolog->code.push_back(inst);
		i++;
	}

	{
		MachineOperand dst;
		dst.type = MachineOperandType::basicblock;
		dst.bb = func->basicBlocks.front();

		auto inst = context->newMachineInst();
		inst->opcode = MachineInstOpcode::jmp;
		inst->operands.push_back(dst);
		func->prolog->code.push_back(inst);
	}
}

void RegisterAllocator::emitEpilog(const std::vector<RegisterName>& savedRegs, const std::vector<RegisterName>& savedXmmRegs, int stackSize)
{
	int i = 1;
	for (RegisterName physReg : savedXmmRegs)
	{
		MachineOperand dst;
		dst.type = MachineOperandType::reg;
		dst.registerIndex = (int)physReg;

		MachineOperand src;
		src.type = MachineOperandType::stack;
		src.stackOffset = nextStackOffset + i * 8;

		auto inst = context->newMachineInst();
		inst->opcode = MachineInstOpcode::loadsd;
		inst->operands.push_back(dst);
		inst->operands.push_back(src);
		func->epilog->code.push_back(inst);
		i++;
	}

	// add rsp, stacksize
	{
		MachineOperand dst;
		dst.type = MachineOperandType::reg;
		dst.registerIndex = (int)RegisterName::rsp;

		MachineOperand src;
		src.type = MachineOperandType::imm;
		src.immvalue = stackSize;

		auto inst = context->newMachineInst();
		inst->opcode = MachineInstOpcode::add64;
		inst->operands.push_back(dst);
		inst->operands.push_back(src);
		func->epilog->code.push_back(inst);
	}

	for (auto it = savedRegs.rbegin(); it != savedRegs.rend(); ++it)
	{
		RegisterName physReg = *it;

		MachineOperand dst;
		dst.type = MachineOperandType::reg;
		dst.registerIndex = (int)physReg;

		auto inst = context->newMachineInst();
		inst->opcode = MachineInstOpcode::pop;
		inst->operands.push_back(dst);
		func->epilog->code.push_back(inst);
	}

	{
		auto inst = context->newMachineInst();
		inst->opcode = MachineInstOpcode::ret;
		func->epilog->code.push_back(inst);
	}
}

void RegisterAllocator::createRegisterInfo()
{
	reginfo.resize(func->registers.size());

	for (size_t i = 0; i < func->registers.size(); i++)
		reginfo[i].cls = func->registers[i].cls;

	for (int i = 0; i < (int)RegisterName::vregstart; i++)
	{
		if (reginfo[i].cls != MachineRegClass::reserved)
		{
			auto& mru = mruPhys[(int)reginfo[i].cls];
			reginfo[i].mruIt = mru.insert(mru.end(), i);
		}
	}
}

void RegisterAllocator::setAllToStack()
{
	for (RARegisterInfo &entry : reginfo)
	{
		entry.spilled = true;
	}
}

void RegisterAllocator::assignAllToStack()
{
	for (size_t i = (int)RegisterName::vregstart; i < reginfo.size(); i++)
	{
		int vregIndex = (int)i;
		assignVirt2StackSlot(vregIndex);
	}
}

void RegisterAllocator::usePhysRegister(MachineOperand& operand)
{
	int pregIndex = operand.registerIndex;
	auto& physreg = reginfo[pregIndex];
	auto& mru = mruPhys[(int)physreg.cls];

	if (physreg.vreg != -1)
	{
		assignVirt2StackSlot(physreg.vreg);
	}

	// Move register to the back of the list of registers to pick from
	mru.erase(physreg.mruIt);
	physreg.mruIt = mru.insert(mru.end(), pregIndex);
}

void RegisterAllocator::useVirtRegister(MachineOperand& operand)
{
	int vregIndex = operand.registerIndex;
	auto& vreg = reginfo[vregIndex];
	auto& mru = mruPhys[(int)vreg.cls];
	if (vreg.spilled)
	{
		int pregIndex = mru.front();

		auto& physreg = reginfo[pregIndex];
		if (physreg.vreg != -1)
			assignVirt2StackSlot(physreg.vreg);

		mru.pop_front();
		physreg.mruIt = mru.insert(mru.end(), pregIndex);

		assignVirt2Phys(vregIndex, pregIndex);
		operand.registerIndex = pregIndex;
	}
	else
	{
		// Move register to the back of the list of registers to pick from
		int pregIndex = vreg.physreg;
		auto& physreg = reginfo[pregIndex];
		mru.erase(physreg.mruIt);
		physreg.mruIt = mru.insert(mru.end(), pregIndex);
		operand.registerIndex = pregIndex;
	}
}

void RegisterAllocator::killVirtRegister(int vregIndex)
{
	auto& vreg = reginfo[vregIndex];
	auto& mru = mruPhys[(int)vreg.cls];
	if (!vreg.spilled)
	{
		int pregIndex = vreg.physreg;
		auto& physreg = reginfo[pregIndex];

		vreg.spilled = true;
		vreg.physreg = -1;
		physreg.vreg = -1;

		mru.erase(physreg.mruIt);
		physreg.mruIt = mru.insert(mru.begin(), pregIndex);
	}
}

void RegisterAllocator::assignVirt2Phys(int vregIndex, int pregIndex)
{
	auto& vreg = reginfo[vregIndex];
	auto& physreg = reginfo[pregIndex];

	if (physreg.vreg != -1 && physreg.vreg != vregIndex)
	{
		assignVirt2StackSlot(vregIndex);
	}

	if (vreg.spilled && vreg.stackoffset == -1)
	{
		vreg.physreg = pregIndex;
		vreg.spilled = false;
	}
	else if (vreg.spilled)
	{
		// emit move from stack:

		MachineOperand dest;
		dest.type = MachineOperandType::reg;
		dest.registerIndex = pregIndex;

		MachineOperand src;
		src.type = MachineOperandType::stack;
		src.stackOffset = vreg.stackoffset;

		auto inst = context->newMachineInst();
		inst->opcode = vreg.cls == MachineRegClass::gp ? MachineInstOpcode::load64 : MachineInstOpcode::loadsd;
		inst->operands.push_back(dest);
		inst->operands.push_back(src);
		inst->comment = "load vreg " + std::to_string(vregIndex);
		emittedInstructions.push_back(inst);

		physreg.vreg = vregIndex;
		vreg.physreg = pregIndex;
		vreg.spilled = false;
	}
	else if (vreg.physreg != pregIndex)
	{
		// emit move from other register:

		MachineOperand dest;
		dest.type = MachineOperandType::reg;
		dest.registerIndex = pregIndex;

		MachineOperand src;
		src.type = MachineOperandType::reg;
		src.registerIndex = vreg.physreg;

		auto inst = context->newMachineInst();
		inst->opcode = vreg.cls == MachineRegClass::gp ? MachineInstOpcode::mov64 : MachineInstOpcode::movsd;
		inst->operands.push_back(dest);
		inst->operands.push_back(src);
		inst->comment = "move vreg " + std::to_string(vregIndex);
		emittedInstructions.push_back(inst);

		reginfo[vreg.physreg].vreg = -1;
		physreg.vreg = vregIndex;
		vreg.physreg = pregIndex;
	}
}

void RegisterAllocator::assignVirt2StackSlot(int vregIndex)
{
	auto& vreg = reginfo[vregIndex];
	if (vreg.spilled)
		return;

	auto& physreg = reginfo[vreg.physreg];

	if (vreg.stackoffset == -1)
	{
		nextStackOffset += 8;
		vreg.stackoffset = nextStackOffset;
	}

	// emit move from register to stack:

	MachineOperand dest;
	dest.type = MachineOperandType::stack;
	dest.stackOffset = vreg.stackoffset;

	MachineOperand src;
	src.type = MachineOperandType::reg;
	src.registerIndex = vreg.physreg;

	auto inst = context->newMachineInst();
	inst->opcode = vreg.cls == MachineRegClass::gp ? MachineInstOpcode::store64 : MachineInstOpcode::storesd;
	inst->operands.push_back(dest);
	inst->operands.push_back(src);
	inst->comment = "save vreg " + std::to_string(vregIndex);
	emittedInstructions.push_back(inst);

	physreg.vreg = -1;
	vreg.spilled = true;
}
