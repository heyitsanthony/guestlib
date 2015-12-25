#ifndef PTI386CPUSTATE_H
#define PTI386CPUSTATE_H

#include "ptcpustate.h"

struct x86_user_fpxregs
{
  uint16_t cwd;
  uint16_t swd;
  uint16_t twd;
  uint16_t fop;
  uint32_t fip;
  uint32_t fcs;
  uint32_t foo;
  uint32_t fos;
  uint32_t mxcsr;
  uint32_t reserved;
  uint32_t st_space[32];   /* 8*16 bytes for each FP-reg = 128 bytes */
  uint32_t xmm_space[32];  /* 8*16 bytes for each XMM-reg = 128 bytes */
  uint32_t padding[56];
};

/* HAHA TOO BAD I CAN'T REUSE A HEADER. */
struct x86_user_regs
{
  uint32_t ebx;
  uint32_t ecx;
  uint32_t edx;
  uint32_t esi;
  uint32_t edi;
  uint32_t ebp;
  uint32_t eax;
  uint32_t xds;
  uint32_t xes;
  uint32_t xfs;
  uint32_t xgs;
  uint32_t orig_eax;
  uint32_t eip;
  uint32_t xcs;
  uint32_t eflags;
  uint32_t esp;
  uint32_t xss;
};

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

	// used by ptimgarch
	struct x86_user_regs& getRegs(void) const;
	struct x86_user_fpxregs& getFPRegs(void) const;
	void setRegs(const struct x86_user_regs& regs);

private:
	void reloadRegs(void) const;
};

#endif
