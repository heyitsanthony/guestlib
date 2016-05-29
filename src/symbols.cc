#include "Sugar.h"
#include <stdio.h>
#include <assert.h>
#include "symbols.h"

Symbols::~Symbols() {}

Symbols::Symbols(const Symbols& s)
{
	if (&s == this) return;
	addSyms(&s);
}

const Symbol* Symbols::findSym(const std::string& s) const
{
	const auto it = name_map.find(s);
	return (it == name_map.end()) ? nullptr : (*it).second;
}

const Symbol* Symbols::findSym(uint64_t ptr) const
{
	const Symbol			*ret;
	symaddr_map::const_iterator	it;

	if (ptr == 0)
		return NULL;

	it = addr_map.upper_bound((symaddr_t)ptr-1);
	if (it == addr_map.end())
		return NULL;

	ret = it->second.get();
	if (ret->getBaseAddr() == (symaddr_t)ptr)
		return ret;

	--it;
	if (it == addr_map.end())
		return NULL;

	ret = it->second.get();
	assert (ret->getBaseAddr() <= (symaddr_t)ptr && "WTF");

	if ((symaddr_t)ptr > ret->getEndAddr())
		return NULL;

	return ret;
}

bool Symbols::addSym(const std::string& name, symaddr_t addr, unsigned int len)
{
	if (addr_map.count(addr)) return false;
	if (name_map.count(name)) return false;

	auto sym  = std::make_unique<Symbol>(name, addr, len);
	name_map[name] = sym.get();
	addr_map.emplace(addr, std::move(sym));
	return true;
}

void Symbols::addSym(const Symbol* sym)
{ addSym(sym->getName(), sym->getBaseAddr(), sym->getLength()); }

void Symbols::addSyms(const Symbols* syms)
{
	for (const auto &p : syms->name_map)
		addSym(p.second);
}