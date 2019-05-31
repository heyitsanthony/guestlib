#include <vector>
#include <string.h>
#include <stdio.h>
#include <sstream>

#include "syscall/syscalls.h"
#include "guestcpustate.h"

std::map<Arch::Arch, make_guestcpustate_t> GuestCPUState::makers;

GuestCPUState::GuestCPUState(const guest_ctx_field* f)
: fields(f)
, state_data(NULL)
, state_byte_c(0)
, xlate(std::make_unique<SyscallXlate>())
{
	unsigned total_elems = 0;

	assert (f != nullptr && "need the field labels");

	/* compute the register name/address offsets mappings */
	for (unsigned i = 0; f[i].f_len != 0; i++) {
		unsigned cur_byte_off = f[i].f_offset;
		if (off2ElemMap.count(cur_byte_off)) {
			auto it =  off2ElemMap.find(cur_byte_off);
			std::cerr << "CONFLICT: " << f[it->second].f_name
					<< " vs " << 	f[i].f_name << '\n';
			abort();
		}
		for (unsigned int c = 0; c < f[i].f_count; c++) {
			off2ElemMap[cur_byte_off] = total_elems;
			if (f[i].f_count > 1) {
				std::stringstream	ss;
				ss << f[i].f_name << "[" << c << "]";
				off2RegMap[cur_byte_off] = ss.str();
			} else {
				off2RegMap[cur_byte_off] = f[i].f_name;
			}
			reg2OffMap[f[i].f_name] = cur_byte_off;
			cur_byte_off += f[i].f_len;
		}
		total_elems++;

		if (cur_byte_off > state_byte_c)
			state_byte_c = cur_byte_off;
	}
}

GuestCPUState::~GuestCPUState()
{
}

uint8_t* GuestCPUState::copyStateData(void) const
{
	uint8_t	*ret;

	assert (state_data != NULL && "NO STATE DATA TO COPY??");
	ret = new uint8_t[getStateSize()];
	memcpy(ret, state_data, getStateSize());
	return ret;
}

unsigned GuestCPUState::name2Off(const char* name) const
{
	reg2byte_map_t::const_iterator	it(reg2OffMap.find(name));
	assert (it != reg2OffMap.end());
	return it->second;
}

void GuestCPUState::setReg(const char* name, unsigned bits, uint64_t v, int off)
{
	assert (bits == 32 || bits == 64);
	auto it = reg2OffMap.find(name);
	if (it == reg2OffMap.end()) {
		std::cerr << "WHAT: " << name << '\n';
		assert ("REG NOT FOUND??" && it != reg2OffMap.end());
	}

	unsigned roff = it->second;
	if (bits == 32)
		((uint32_t*)(state_data+roff))[off] = v;
	else
		((uint64_t*)(state_data+roff))[off] = v;
}

uint64_t GuestCPUState::getReg(const char* name, unsigned bits, int off)
const
{
	assert (bits == 32 || bits == 64);
	auto it = reg2OffMap.find(name);
	if (it == reg2OffMap.end()) {
		std::cerr << "WHAT: " << name << '\n';
		assert ("REG NOT FOUND??" && it != reg2OffMap.end());
	}

	unsigned roff = it->second;
	if (bits == 32)
		return ((uint32_t*)(state_data+roff))[off];

	return ((uint64_t*)(state_data+roff))[off];
}

static const char hex[] = "0123456789abcdef";
void GuestCPUState::print(std::ostream& os, const void* regctx) const
{
	for (const auto& off2elem : off2ElemMap) {
		auto byte_off = off2elem.first;
		auto elem_idx = off2elem.second;
		os << off2RegMap.find(byte_off)->second << ": ";
		// XXX reason about endianess; use LE for now
		for (int i = fields[elem_idx].f_len - 1; i >= 0; i--) {
			uint8_t	v = ((uint8_t*)regctx)[byte_off + i];
			os << hex[(v & 0xf0) >> 4] << hex[v & 0xf];
		}
		os << '\n';
	}
}


bool GuestCPUState::load(const char* fname)
{
	unsigned int	len = getStateSize();
	uint8_t		*data = (uint8_t*)getStateData();
	size_t		br;
	FILE		*f;

	f = fopen(fname, "rb");
	assert (f != NULL && "Could not load register file");
	if (f == NULL) return false;

	br = fread(data, len, 1, f);
	fclose(f);

	assert (br == 1 && "Error reading all register bytes");
	if (br != 1) return false;

	return true;
}

bool GuestCPUState::save(const char* fname)
{
	unsigned int	len = getStateSize();
	const uint8_t	*data = (const uint8_t*)getStateData();
	size_t		br;
	FILE		*f;

	f = fopen(fname, "w");
	if (!f) return false;

	assert (f != NULL);
	br = fwrite(data, len, 1, f);
	assert (br == 1);
	fclose(f);
	if (br != 1) return false;

	return true;
}

uint8_t* GuestCPUState::copyOutStateData(void)
{
	uint8_t	*old_dat = state_data;
	state_data = copyStateData();
	return old_dat;
}

unsigned int GuestCPUState::byteOffset2ElemIdx(unsigned int off) const
{
	auto it = off2ElemMap.find(off);
	if (it == off2ElemMap.end()) {
		unsigned int	c = 0;
		std::cerr << "[GuestCPUState] unknown field at " << off << '\n';
		// dumpIRSBs();
		for (int i = 0; fields[i].f_len; i++) {
			fprintf(stderr, "%s@%d\n", fields[i].f_name, c);
			c += fields[i].f_len * fields[i].f_count;
		}
		assert (0 == 1 && "Could not resolve byte offset");
	}
	return (*it).second;
}

void GuestCPUState::registerCPU(Arch::Arch a, make_guestcpustate_t f)
{
	assert (makers.count(a) == 0);
	makers[a] = f;
}

GuestCPUState* GuestCPUState::create(Arch::Arch a)
{
	assert(makers.count(a) && "unsupported guest architecture");
	return makers[a]();
}

void GuestCPUState::noteRegion(const char* name, guest_ptr addr)
{
	std::cerr	<< "[GuestCPUState] unexpected region '" << name
			<< "' at " << (void*)addr.o << '\n';
}

const char* GuestCPUState::off2Name(unsigned int off) const
{
	const auto it = off2RegMap.find(off);
	if (it == off2RegMap.end()) {
		return nullptr;
	}
	return it->second.c_str();
}
