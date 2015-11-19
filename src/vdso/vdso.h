#ifndef GUEST_VDSO_H
#define GUEST_VDSO_H

struct vdso_ent
{
	void		*ve_f;
	unsigned long	ve_sz;
	const char	*ve_name;
};

extern vdso_ent vdso_tab[];

#endif
