/* guest image for a ptrace generated image */
#ifndef GUESTSTATEPTIMG_H
#define GUESTSTATEPTIMG_H

#include <map>
#include <set>
#include <stdlib.h>
#include "Sugar.h"
#include "guest.h"
#include "procmap.h"

class Symbols;
class PTImgArch;
class PTShadow;

#if defined(__amd64__)
#define SETUP_ARCH_PT	\
	do {	\
	pt_shadow = PTShadow::create(arch, this, pid);			\
	if (!pt_shadow)							\
		pt_arch = (arch != Arch::I386)				\
			? (PTImgArch*)(new PTImgAMD64(this, pid))	\
			: (PTImgArch*)(new PTImgI386(this, pid));	\
	else								\
		pt_arch = dynamic_cast<PTImgArch*>(pt_shadow);		\
	} while (0);
#elif defined(__arm__)
#define SETUP_ARCH_PT					\
	do {						\
	pt_shadow = PTShadow::create(arch, this, pid);	\
	if (!pt_shadow)					\
		pt_arch = new PTImgARM(this, pid);	\
	else						\
		pt_arch = dynamic_cast<PTImgArch*>(pt_shadow);	\
	} while (0);
#else
#define SETUP_ARCH_PT	0; assert (0 == 1 && "UNKNOWN PTRACE HOST ARCHITECTURE! AIEE");
#endif

class GuestPTImg : public Guest
{
public:
	virtual ~GuestPTImg(void);
	guest_ptr getEntryPoint(void) const override { return entry_pt; }

	static pid_t createChild(
		int argc, char* const argv[], char *const envp[]);

	template <class T>
	static T* create(pid_t new_pid, char* const argv[])
	{
		GuestPTImg		*pt_img;
		T			*pt_t;
		bool			ok;
		int			sys_nr;
		const char		*bp;

		bp = (getenv("GUEST_REAL_BINPATH"))
			? getenv("GUEST_REAL_BINPATH")
			: argv[0];
		pt_t = new T(bp);
		pt_img = pt_t;
		
		sys_nr = (getenv("GUEST_WAIT_SYSNR") != NULL)
			? atoi(getenv("GUEST_WAIT_SYSNR"))
			: -1;

		ok = (sys_nr == -1)
			? pt_img->slurpChild(new_pid, argv)
			: pt_img->slurpChildOnSyscall(new_pid, argv, sys_nr);

		if (!ok) {
			delete pt_img;
			return NULL;
		}

		pt_img->handleChild(new_pid);
		return pt_t;
	}

	template <class T>
	static T* create(int argc, char* const argv[], char* const envp[])
	{
		pid_t	new_pid = createChild(argc, argv, envp);
		if (new_pid <= 0) return nullptr;
		return create<T>(new_pid, argv);
	}

	template <class T>
	static T* createAttached(int pid, char* const argv[])
	{
		GuestPTImg		*pt_img;
		T			*pt_t;
		pid_t			slurped_pid;

		pt_t = new T(argv[0], false /* ignore binary */);
		pt_img = pt_t;

		slurped_pid = pt_img->createSlurpedAttach(pid);
		if (slurped_pid <= 0) {
			delete pt_img;
			return NULL;
		}

		pt_img->handleChild(slurped_pid);
		return pt_t;
	}

	/* NOTE: destroys the guest 'gs' on success */
	template <class T>
	static T* create(Guest* gs)
	{
		GuestPTImg		*pt_img;
		T			*pt_t;
		pid_t			slurped_pid;

		pt_t = new T(gs->getBinaryPath(), false /* ignore binary */);
		pt_img = pt_t;
		slurped_pid = pt_img->createFromGuest(gs);
		if (slurped_pid <= 0) {
			delete pt_img;
			return NULL;
		}

		pt_img->handleChild(slurped_pid);
		return pt_t;
	}


	Arch::Arch getArch() const override { return arch; }
	std::vector<guest_ptr> getArgvPtrs(void) const override
	{ return argv_ptrs; }

	guest_ptr getArgcPtr(void) const override { return argc_ptr; }


	void setBreakpoint(guest_ptr addr);
	void resetBreakpoint(guest_ptr addr);
	guest_ptr undoBreakpoint();


	static void stackTrace(
		std::ostream& os, const char* binname, pid_t pid,
		guest_ptr range_begin = guest_ptr(0), 
		guest_ptr range_end = guest_ptr(0));

	static void dumpSelfMap(void);

	PTImgArch* getPTArch(void) const { return pt_arch; }
	PTShadow* getPTShadow(void) const { return pt_shadow; }

	void slurpRegisters(pid_t pid);

protected:
	GuestPTImg(const char* binpath, bool use_entry=true);
	virtual void handleChild(pid_t pid);

	virtual void slurpBrains(pid_t pid);
	virtual pid_t createSlurpedAttach(int pid);
	void attachSyscall(int pid);
	void fixupRegsPreSyscall(int pid);
	void slurpThreads(void);

	std::unique_ptr<Symbols> loadSymbols() const override;
	std::unique_ptr<Symbols> loadDynSymbols() const override;

	PTImgArch		*pt_arch;
	PTShadow		*pt_shadow;
	Arch::Arch		arch;
	ptr_list_t<ProcMap>	mappings;
	guest_ptr		entry_pt;

private:
	bool slurpChild(pid_t pid, char *const argv[]);
	bool slurpChildOnSyscall(
		pid_t pid, char *const argv[], unsigned sys_nr);

	pid_t createFromGuest(Guest* gs);


	void waitForEntry(int pid);
	void slurpArgPtrs(char *const argv[]);
	static void forcePreloads(
		Symbols			&symbols,
		std::set<std::string>	&mmap_fnames,
		const ptr_list_t<ProcMap>& mappings);

	std::map<guest_ptr, uint64_t>	breakpoints;
	std::vector<guest_ptr>		argv_ptrs;
	guest_ptr			argc_ptr;
};

#endif
