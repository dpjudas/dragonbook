
#include "RegisterAllocator.h"

void RegisterAllocator::run(IRContext* context, MachineFunction* func)
{
	RegisterAllocator ra(context, func);
	ra.run();
}

void RegisterAllocator::run()
{
	createPhysRegisters();

	//static const RegisterName unixregvars[] = { RegisterName::rdi, RegisterName::rsi, RegisterName::rdx, RegisterName::rcx, RegisterName::r8, RegisterName::r9 };
	static const RegisterName win64regvars[] = { RegisterName::rcx, RegisterName::rdx, RegisterName::r8, RegisterName::r9 };

	int argoffset = -8 * 8; // 8 pushed registers

	IRFunctionType* functype = dynamic_cast<IRFunctionType*>(func->type);
	for (size_t i = 0; i < functype->args.size(); i++)
	{
		IRType* type = functype->args[i];

		argoffset -= 8;

		VirtualRegister vreg;
		vreg.type = (isFloat(type) || isDouble(type)) ? RegType::xmm : RegType::gp;
		vreg.stackoffset = argoffset;

		if (i < 4)
		{
			vreg.spilled = false;

			int pregIndex;
			if (vreg.type == RegType::gp)
				pregIndex = (int)win64regvars[i];
			else
				pregIndex = (int)RegisterName::xmm0 + (int)i;
			vreg.physreg = pregIndex;

			auto& physreg = physregs[pregIndex];
			physreg.vreg = (int)i;

			auto& mru = mruPhys[(int)physreg.type];
			mru.erase(physreg.mruIt);
			physreg.mruIt = mru.insert(mru.end(), pregIndex);
		}

		vregs.push_back(vreg);
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

void RegisterAllocator::createPhysRegisters()
{
	physregs.clear();

	PhysicalRegister reserved;
	reserved.type = RegType::reserved;

	PhysicalRegister gp;
	gp.type = RegType::gp;
	physregs.push_back(gp); // rax
	physregs.push_back(gp); // rcx
	physregs.push_back(gp); // rdx
	physregs.push_back(gp); // rbx
	physregs.push_back(reserved); // rsp
	physregs.push_back(gp); // rbp
	physregs.push_back(gp); // rsi
	physregs.push_back(gp); // rdi
	physregs.push_back(gp); // r8
	physregs.push_back(gp); // r9
	physregs.push_back(gp); // r10
	physregs.push_back(gp); // r11
	physregs.push_back(gp); // r12
	physregs.push_back(gp); // r13
	physregs.push_back(gp); // r14
	physregs.push_back(gp); // r15

	size_t numGP = physregs.size();

	PhysicalRegister xmm;
	xmm.type = RegType::xmm;
	physregs.push_back(xmm); // xmm0
	physregs.push_back(xmm); // xmm1
	physregs.push_back(xmm); // xmm2
	physregs.push_back(xmm); // xmm3
	physregs.push_back(xmm); // xmm4
	physregs.push_back(xmm); // xmm5
	physregs.push_back(xmm); // xmm6
	physregs.push_back(xmm); // xmm7
	physregs.push_back(xmm); // xmm8
	physregs.push_back(xmm); // xmm9
	physregs.push_back(xmm); // xmm10
	physregs.push_back(xmm); // xmm11
	physregs.push_back(xmm); // xmm12
	physregs.push_back(xmm); // xmm13
	physregs.push_back(xmm); // xmm14
	physregs.push_back(xmm); // xmm15

	for (int i = 0; i < physregs.size(); i++)
	{
		if (physregs[i].type != RegType::reserved)
		{
			auto& mru = mruPhys[(int)physregs[i].type];
			physregs[i].mruIt = mru.insert(mru.end(), i);
		}
	}
}

void RegisterAllocator::setAllToStack()
{
	for (size_t i = 0; i < vregs.size(); i++)
	{
		vregs[i].spilled = true;
	}
}

void RegisterAllocator::assignAllToStack()
{
	for (size_t i = 0; i < vregs.size(); i++)
	{
		int vregIndex = (int)i;
		assignVirt2StackSlot(vregIndex);
	}
}

void RegisterAllocator::usePhysRegister(MachineOperand& operand)
{
	int pregIndex = operand.registerIndex;
	auto& physreg = physregs[pregIndex];
	auto& mru = mruPhys[(int)physreg.type];

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
	int vregIndex = operand.registerIndex - (int)RegisterName::vregstart;
	if (vregs.size() <= (size_t)vregIndex)
		vregs.resize((size_t)vregIndex + 1);
	auto& vreg = vregs[vregIndex];
	auto& mru = mruPhys[(int)vreg.type];
	if (vreg.spilled)
	{
		int pregIndex = mru.front();

		auto& physreg = physregs[pregIndex];
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
		auto& physreg = physregs[pregIndex];
		mru.erase(physreg.mruIt);
		physreg.mruIt = mru.insert(mru.end(), pregIndex);
		operand.registerIndex = pregIndex;
	}
}

void RegisterAllocator::killVirtRegister(int vregIndex)
{
	auto& vreg = vregs[vregIndex];
	auto& mru = mruPhys[(int)vreg.type];
	if (!vreg.spilled)
	{
		int pregIndex = vreg.physreg;
		auto& physreg = physregs[pregIndex];

		vreg.spilled = true;
		vreg.physreg = -1;
		physreg.vreg = -1;

		mru.erase(physreg.mruIt);
		physreg.mruIt = mru.insert(mru.begin(), pregIndex);
	}
}

void RegisterAllocator::assignVirt2Phys(int vregIndex, int pregIndex)
{
	auto& vreg = vregs[vregIndex];
	auto& physreg = physregs[pregIndex];

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
		inst->opcode = vreg.type == RegType::gp ? MachineInstOpcode::load64 : MachineInstOpcode::loadsd;
		inst->operands.push_back(dest);
		inst->operands.push_back(src);
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
		inst->opcode = vreg.type == RegType::gp ? MachineInstOpcode::mov64 : MachineInstOpcode::movsd;
		inst->operands.push_back(dest);
		inst->operands.push_back(src);
		emittedInstructions.push_back(inst);

		physregs[vreg.physreg].vreg = -1;
		physreg.vreg = vregIndex;
		vreg.physreg = pregIndex;
	}
}

void RegisterAllocator::assignVirt2StackSlot(int vregIndex)
{
	auto& vreg = vregs[vregIndex];
	if (vreg.spilled)
		return;

	auto& physreg = physregs[vreg.physreg];

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
	inst->opcode = vreg.type == RegType::gp ? MachineInstOpcode::store64 : MachineInstOpcode::storesd;
	inst->operands.push_back(dest);
	inst->operands.push_back(src);
	emittedInstructions.push_back(inst);

	physreg.vreg = -1;
	vreg.spilled = true;
}
