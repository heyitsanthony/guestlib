#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <iostream>
#include <list>
#include <vector>

#include "syscallparams.h"
#include "guestmem.h"

#define MAX_SC_TRACE	1000

class Guest;
class GuestCPUState;

#define SYSCALL_HANDLER(arch, call)		\
	bool arch##_##call(			\
		Guest&			g,	\
		SyscallParams&		args,	\
		unsigned long&		sc_ret)

class Syscalls
{
public:
	static std::unique_ptr<Syscalls> create(Guest*);

	virtual ~Syscalls();

	static uintptr_t passthroughSyscall(SyscallParams& args);

	virtual uint64_t apply(SyscallParams& args);
	uint64_t apply(void); /* use guest params */
	std::string getSyscallName(int guest) const;
	int translateSyscall(int guest) const;

	void print(std::ostream& os) const;
	void print(	std::ostream& os,
			const SyscallParams& sp, 
			uintptr_t *result = NULL) const;
	bool isExit(void) const { return exited; }

protected:
	Syscalls(Guest*);

	bool interceptSyscall(
		int sys_nr,
		SyscallParams& args,
		unsigned long& sc_ret);

	Guest				*guest;
	std::list<SyscallParams>	sc_trace;
	uint64_t			sc_seen_c; /* list.size can be O(n) */
	bool				exited;
	GuestCPUState			*cpu_state;
	GuestMem*			mappings;
	const std::string		binary;
	bool				log_syscalls;
	bool				force_xlate_syscalls;
public:
	const static std::string	chroot;
};


class SyscallXlate
{
public:
	SyscallXlate();
	virtual ~SyscallXlate() {}
	/* map a guest syscall number to the host equivalent */
	virtual int translateSyscall(int sys_nr) const { return sys_nr; }
	virtual std::string getSyscallName(int sys_nr) const { return "???"; }
	/* either passthrough or emulate the syscall */
	virtual uintptr_t apply(Guest& g, SyscallParams& args) {
		return Syscalls::passthroughSyscall(args);
	}

protected:
	bool tryPassthrough(Guest& g, SyscallParams& args, uintptr_t& sc_ret);

	static bool force_xlate_syscalls;
};

#endif
