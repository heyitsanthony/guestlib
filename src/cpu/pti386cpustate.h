#ifndef PTI386CPUSTATE_H
#define PTI386CPUSTATE_H

#include "ptcpustate.h"

class PTI386CPUState : public PTCPUState
{
public:
	PTI386CPUState(pid_t in_pid);
	~PTI386CPUState();

	void ignoreSysCall(void) override { assert(false && "STUB"); }
	uint64_t dispatchSysCall(const SyscallParams& sp, int& wss) override {
		assert (false && "STUB");
	}
	void loadRegs(void) override;
	guest_ptr undoBreakpoint(void) override;
	long setBreakpoint(guest_ptr addr) override;

	guest_ptr getPC(void) const override { assert (false && "STUB"); }
	guest_ptr getStackPtr(void) const override;
	uintptr_t getSysCallResult() const override { assert (false && "STUB"); }

	void setStackPtr(guest_ptr) override { assert (false && "STUB"); }
	void setPC(guest_ptr) override { assert (false && "STUB"); }

	bool isSyscallOp(guest_ptr addr, long v) const override {
		assert(false && "STUB");
		return false;
	}

	void print(std::ostream& os, const void*) const override {
		assert(false && "STUB");
		abort();
	}

	// used by ptimgarch
	struct user_regs_struct& getRegs(void) const;
	struct user_fpregs_struct& getFPRegs(void) const;
	void setRegs(const user_regs_struct& regs);

private:
	void reloadRegs(void) const;
};

#endif
