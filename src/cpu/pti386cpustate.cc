#include <sys/ptrace.h>
#include <asm/ptrace-abi.h>
#include <linux/elf.h>
#include <sys/uio.h>
#include <string.h>
#include <cstddef>
#include "cpu/pti386cpustate.h"

struct pt_regs
{
	struct x86_user_regs	regs;
	struct x86_user_fpxregs	fpregs;
};

#define pt_sz(x)	sizeof((((struct pt_regs*)0)->x))
#define pt_field_ent(x)		{#x, pt_sz(x), 1, offsetof(struct pt_regs, x), true}
#define pt_field_ent_s(y,x)	{y, pt_sz(x), 1, offsetof(struct pt_regs, x), true}

static struct guest_ctx_field pti386_fields[] = {
	pt_field_ent_s("EBX", regs.ebx),
	pt_field_ent_s("ECX", regs.ecx),
	pt_field_ent_s("EDX", regs.edx),
	pt_field_ent_s("ESI", regs.esi),
	pt_field_ent_s("EDI", regs.edi),
	pt_field_ent_s("EBP", regs.ebp),
	pt_field_ent_s("EAX", regs.eax),
	pt_field_ent_s("DS", regs.xds),
	pt_field_ent_s("ES", regs.xes),
	pt_field_ent_s("FS", regs.xfs),
	pt_field_ent_s("GS", regs.xgs),
	pt_field_ent_s("orig_EAX", regs.orig_eax),
	pt_field_ent_s("EIP", regs.eip),
	pt_field_ent_s("CS", regs.xcs),
	pt_field_ent_s("EFLAGS", regs.eflags),
	pt_field_ent_s("ESP", regs.esp),
	pt_field_ent_s("SS", regs.xss),

	pt_field_ent(fpregs.cwd),
	pt_field_ent(fpregs.swd),
	pt_field_ent(fpregs.twd),
	pt_field_ent(fpregs.fop),
	pt_field_ent(fpregs.fip),
	pt_field_ent(fpregs.fcs),
	pt_field_ent(fpregs.foo),
	pt_field_ent(fpregs.fos),
	pt_field_ent(fpregs.mxcsr),
	pt_field_ent(fpregs.reserved),
	{ "st_space", 4, 32, offsetof(pt_regs, fpregs.st_space), true },
	{ "xmm_space", 4, 32, offsetof(pt_regs, fpregs.xmm_space), true },
	{ "padding", 4, 56, offsetof(pt_regs, fpregs.padding), true },
	{0},
};

#define GET_PTREGS()	((struct pt_regs*)state_data)

PTI386CPUState::PTI386CPUState(pid_t in_pid)
	: PTCPUState(pti386_fields, in_pid)
{
	state_byte_c = sizeof(struct pt_regs);
	state_data = new uint8_t[state_byte_c+1];
	memset(state_data, 0, state_byte_c+1);
}

PTI386CPUState::~PTI386CPUState()
{
	delete [] state_data;
}

guest_ptr PTI386CPUState::undoBreakpoint()
{
	auto		pr = GET_PTREGS();
	struct iovec	iov;
	int		err;

	/* should be halted on our trapcode. need to set eip prior to
	 * trapcode addr */
	iov.iov_base = &pr->regs;
	iov.iov_len = sizeof(pr->regs);
	err = ptrace((__ptrace_request)PTRACE_GETREGSET, pid, NT_PRSTATUS, &iov);
	assert (err != -1);

	pr->regs.eip--; /* backtrack before int3 opcode */
	err = ptrace((__ptrace_request)PTRACE_SETREGSET, pid, NT_PRSTATUS, &iov);

	/* run again w/out reseting BP and you'll end up back here.. */
	return guest_ptr(getRegs().eip);
}

long int PTI386CPUState::setBreakpoint(guest_ptr addr)
{
	uint64_t		old_v, new_v;
	int			err;

	old_v = ptrace(PTRACE_PEEKTEXT, pid, addr.o, NULL);
	new_v = old_v & ~0xff;
	new_v |= 0xcc;

	err = ptrace(PTRACE_POKETEXT, pid, addr.o, new_v);
	assert (err != -1 && "Failed to set breakpoint");

	return old_v;
}

guest_ptr PTI386CPUState::getStackPtr() const { return guest_ptr(getRegs().esp); }

void PTI386CPUState::loadRegs(void) { reloadRegs(); }

void PTI386CPUState::reloadRegs(void) const
{
	int		err;
	struct iovec 	iov;
	auto		pr = GET_PTREGS();

	iov.iov_base = &pr->regs;
	iov.iov_len = sizeof(pr->regs);
	err = ptrace((__ptrace_request)PTRACE_GETREGSET, pid, NT_PRSTATUS, &iov);
	assert(err != -1);
	assert(pr->regs.eip != ~0 && "bogus EIP?");

	iov.iov_base = &pr->fpregs;
	iov.iov_len = sizeof(pr->fpregs);
	err = ptrace((__ptrace_request)PTRACE_GETREGSET, pid, NT_PRFPREG, &pr->fpregs);
	assert(err != -1);

	recent_shadow = true;
}

struct x86_user_regs& PTI386CPUState::getRegs(void) const {
	if (!recent_shadow) reloadRegs();
	return GET_PTREGS()->regs;
}
struct x86_user_fpxregs& PTI386CPUState::getFPRegs(void) const {
	if (!recent_shadow) reloadRegs();
	return GET_PTREGS()->fpregs;
}
void PTI386CPUState::setRegs(const x86_user_regs& regs) {
	GET_PTREGS()->regs = regs;
}

