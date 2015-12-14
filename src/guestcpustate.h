#ifndef GUESTCPUSTATE_H
#define GUESTCPUSTATE_H

#include <iostream>
#include <stdint.h>
#include "syscall/syscallparams.h"
#include <sys/user.h>
#include <assert.h>
#include <map>
#include <unordered_map>
#include <memory>
#include <functional>

#include "arch.h"
#include "guestptr.h"

struct guest_ctx_field
{
	const char*	f_name;
	unsigned int	f_len; // in bytes
	unsigned int	f_count;
	unsigned int	f_offset;
	bool		f_export;
};

#define CASE_OFF2NAME_4(s, m)	\
	case offsetof(s, m):	\
	case 1+offsetof(s, m):	\
	case 2+offsetof(s, m):	\
	case 3+offsetof(s, m):

#define CASE_OFF2NAME_8(s, m)	\
	CASE_OFF2NAME_4(s,m)	\
	case 4+offsetof(s, m):	\
	case 5+offsetof(s, m):	\
	case 6+offsetof(s, m):	\
	case 7+offsetof(s, m):

class GuestCPUState;
class SyscallXlate;

typedef std::function<GuestCPUState*(void)> make_guestcpustate_t;

class GuestCPUState
{
public:
	// byte offset -> field element idx
	// must be sorted for iterator
	typedef std::map<unsigned int, unsigned int> byte2elem_map_t;
	typedef std::unordered_map<std::string, unsigned int> reg2byte_map_t;
	typedef std::unordered_map<unsigned int, std::string> byte2reg_map_t;

	GuestCPUState(const guest_ctx_field* f);
	virtual ~GuestCPUState();

	static void registerCPU(Arch::Arch, make_guestcpustate_t);
	static GuestCPUState *create(Arch::Arch);

	void* getStateData(void) { return state_data; }
	const void* getStateData(void) const { return state_data; }
	unsigned int getStateSize(void) const { return state_byte_c+1; }
	uint8_t* copyStateData(void) const;
	virtual uint8_t* copyOutStateData(void);

	virtual void setStackPtr(guest_ptr) = 0;
	virtual guest_ptr getStackPtr(void) const = 0;
	virtual void setPC(guest_ptr) = 0;
	virtual guest_ptr getPC(void) const = 0;

	// offsets
	const char* off2Name(unsigned int off) const;
	unsigned name2Off(const char* name) const;
	unsigned byteOffset2ElemIdx(unsigned int off) const;
	virtual unsigned int getFuncArgOff(unsigned int arg_num) const
	{ assert (0 == 1 && "STUB"); return 0; }
	virtual unsigned int getRetOff(void) const
	{ assert (0 == 1 && "STUB"); return 0; }
	virtual unsigned int getStackRegOff(void) const
	{ assert (0 == 1 && "STUB"); return 0; }
	virtual unsigned int getPCOff(void) const
	{ assert (0 == 1 && "STUB"); return 0; }

	virtual void resetSyscall(void)
	{ assert (0 == 1 && "STUB"); }

	void print(std::ostream& os) const;
	virtual void print(std::ostream& os, const void* regctx) const = 0;

	virtual bool load(const char* fname);
	virtual bool save(const char* fname);

	/* returns byte offset into raw cpu data that maps to
	 * corresponding gdb register file offset; returns -1
	 * if exceeds bounds */
	virtual int cpu2gdb(int gdb_off) const
	{ assert (0 ==1 && "STUB"); return -1; }

	uint64_t getReg(const char* name, unsigned bits, int off=0) const;
	void setReg(const char* name, unsigned bits, uint64_t v, int off=0);

	virtual void noteRegion(const char* name, guest_ptr);

	SyscallXlate& getXlate(void) { return *xlate; }

	const guest_ctx_field& getField(unsigned i) const { return fields[i]; }
	auto begin() const { return off2ElemMap.begin(); }
	auto end() const { return off2ElemMap.end(); }
protected:
	byte2elem_map_t	off2ElemMap;
	reg2byte_map_t	reg2OffMap;
	byte2reg_map_t	off2RegMap;

	const guest_ctx_field	*fields;
	uint8_t		*state_data;
	unsigned int	state_byte_c;
	std::unique_ptr<SyscallXlate>	xlate;

private:
	static std::map<Arch::Arch, make_guestcpustate_t> makers;
};

#endif
