#ifndef ELFDEBUG_H
#define ELFDEBUG_H

class Symbol;
class Symbols;
class GuestMem;

/* stupid little class to slurp up symbols */
/* we need to find out who debugging information really well / robustly
 * and integrate it with us */
class ElfDebug
{
public:
	static Symbols* getSyms(const char* elf_path, 
		uintptr_t base = 0);

	static Symbols* getSyms(const void* base);

	static Symbols* getLinkageSyms(
		const GuestMem* m, const char* elf_path);

private:
	ElfDebug(const char* path);
	ElfDebug(const void* base);
	virtual ~ElfDebug(void);
	template <
		typename Elf_Ehdr, typename Elf_Shdr,
		typename Elf_Sym>
		void setupTables(void);

	std::unique_ptr<Symbol>	nextSym(void);
	std::unique_ptr<Symbol>	nextSym32(void);
	std::unique_ptr<Symbol>	nextSym64(void);

	std::unique_ptr<Symbol>	nextLinkageSym(const GuestMem* m);
	std::unique_ptr<Symbol> nextLinkageSym32(const GuestMem* m);
	std::unique_ptr<Symbol> nextLinkageSym64(const GuestMem* m);
	bool isExec(void) const { return is_exec; }

	static Symbols* getSymsAll(ElfDebug& ed, uintptr_t base);

	bool	is_valid;
	bool	is_exec;

	int		fd;
	char		*img;
	unsigned int	img_byte_c;

	void		*symtab;
	unsigned int	next_sym_idx;
	unsigned int	sym_count;
	const char	*strtab;
	bool		is_reloc;

	void		*rela_tab;
	unsigned int	next_rela_idx;
	unsigned int	rela_count;
	void		*dynsymtab;
	unsigned int	dynsym_count;
	const char	*dynstrtab;

	Arch::Arch	elf_arch;
};

#endif
