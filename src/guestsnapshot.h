#ifndef GUESTSNAPSHOT_H
#define GUESTSNAPSHOT_H

#include <list>
#include <set>
#include "guest.h"

class GuestSnapshot : public Guest
{
public:
	static GuestSnapshot* create(const char* dirname);
	static char* readMemory(const char* dirname, guest_ptr p, unsigned int len);
	virtual ~GuestSnapshot(void);
	static void save(const Guest*, const char* dirname);
	static void saveDiff(
		const Guest*,
		const char* dirname,
		const char* last_dirname,
		const std::set<guest_ptr>& changed_maps);

	guest_ptr getEntryPoint(void) const override { return entry_pt; }
	Arch::Arch getArch(void) const override { return arch; }

	std::vector<guest_ptr> getArgvPtrs(void) const override
	{ return argv_ptrs; }

	guest_ptr getArgcPtr(void) const override { return argc_ptr; }

	const std::string& getPath(void) const { return srcdir; }

	bool getPlatform(
		const char* plat_key, void* buf = NULL, unsigned len = 0) const;
protected:
	GuestSnapshot(const char* dirname);

	std::unique_ptr<Symbols> loadSymbols(const char* name) const;
	std::unique_ptr<Symbols> loadSymbols(void) const override;
	std::unique_ptr<Symbols> loadDynSymbols(void) const override;

private:
	static void saveSymbols(
		const Symbols& g_syms,
		const char* dirpath,
		const char* name);

	void loadMappings(void);
	void loadThreads(void);

	bool			is_valid;
	guest_ptr		entry_pt;
	Arch::Arch		arch;
	std::list<int>		fd_list;
	std::vector<guest_ptr>	argv_ptrs;
	guest_ptr		argc_ptr;

	std::string		srcdir;
};

#endif
