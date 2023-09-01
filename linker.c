/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linker.c -- no-MMU ELF link loader
 *
 * (C) Copyright 2022-2023, Greg Ungerer <gerg@kernel.org>
 */

/*
 * Abstract away the bit size structure differences. So use the appropriate
 * 32-bit or 64-bit structure for word size of the system we are running on.
 */
#include <linux/elf.h>

#if __SIZEOF_POINTER__ == 8
#define elf_hdr			elf64_hdr
#define elf_phdr		elf64_phdr
#define elf_shdr		elf64_shdr
#define elf_rel			elf64_rel
#define elf_rela		elf64_rela
#define elf_dynamic		Elf64_Dyn
#define Elf_Addr		Elf64_Addr
#define Elf_Half		Elf64_Half
#define Elf_Word		Elf64_Word
#define Elf_Sword		Elf64_Sxword
#define elf_fdpic_loadmap	elf64_fdpic_loadmap
#define elf_fdpic_loadseg	elf64_fdpic_loadseg
#else
#define elf_hdr			elf32_hdr
#define elf_phdr		elf32_phdr
#define elf_shdr		elf32_shdr
#define elf_rel			elf32_rel
#define elf_rela		elf32_rela
#define elf_dynamic		Elf32_Dyn
#define Elf_Addr		Elf32_Addr
#define Elf_Half		Elf32_Half
#define Elf_Word		Elf32_Word
#define Elf_Sword		Elf32_Sword
#define elf_fdpic_loadseg	elf32_fdpic_loadseg
#define elf_fdpic_loadmap	elf32_fdpic_loadmap
#endif

#include <linux/elf-fdpic.h>

/*
 * Abstract away definition differences for the relocation types.
 * We only need to support a small set of common relocation actions,
 * so this makes it pretty simple. We define these numbers here to avoid
 * any dependency on C-library headers. The kernel uapi headers do not
 * define these numbers (well, at least for some architectures).
 */
#ifdef arm
#define RELOCATE
#define R_RELATIVE	23 /*R_ARM_RELATIVE*/
#endif

#ifdef m68k
#define RELOCATEAA
#define R_RELATIVE	22 /*R_68K_RELATIVE*/
#endif

#ifdef riscv
#define RELOCATEAA
#define R_RELATIVE	3 /*R_RISCV_RELATIVE*/
#endif

#ifdef xtensa
#define RELOCATEA
#define R_RELATIVE	5 /*R_XTENSA_RELATIVE*/
#endif

/*
 * Abstract away the differences in dealing with "rel" sections or "rela"
 * sections. The relocation calculation is slightly different, not to
 * mention that the data structures involved are ever so similar but
 * still slightly different. In practice each architecture supports one
 * type only - not all types.
 */
#ifdef RELOCATE
#define DT_RELOCATION	DT_REL
#define DT_RELOCATIONSZ	DT_RELSZ
#define elf_relocation	elf_rel

static inline void relative(unsigned long loadaddr, struct elf_rel *rel)
{
	unsigned long *paddr;
	paddr = (unsigned long *) (loadaddr + rel->r_offset);
	*paddr += loadaddr;
}
#endif /* RELOCATE */

#ifdef RELOCATEA
#define DT_RELOCATION	DT_RELA
#define DT_RELOCATIONSZ	DT_RELASZ
#define elf_relocation	elf_rela

static inline void relative(unsigned long loadaddr, struct elf_rela *rela)
{
	unsigned long *paddr;
	paddr = (unsigned long *) (loadaddr + rela->r_offset);
	*paddr += loadaddr;
}
#endif /* RELOCATEA */

#ifdef RELOCATEAA
#define DT_RELOCATION	DT_RELA
#define DT_RELOCATIONSZ	DT_RELASZ
#define elf_relocation	elf_rela

static inline void relative(unsigned long loadaddr, struct elf_rela *rela)
{
	unsigned long *paddr;
	if (rela->r_addend) {
		paddr = (unsigned long *) (loadaddr + rela->r_offset);
		*paddr = loadaddr + rela->r_addend;
	}
}
#endif /* RELOCATEAA */

/*
 * The PIE style constant displacement binaries appear to end up very
 * simple.
 */
static void relocate(unsigned long loadaddr, struct elf_relocation *rel)
{
	unsigned long *paddr;

	switch (rel->r_info) {
	case R_RELATIVE:
		relative(loadaddr, rel);
		break;
	}
}

unsigned long linker(struct elf_fdpic_loadmap *map, elf_dynamic *dynp)
{
	struct elf_fdpic_loadseg *seg;
	struct elf_relocation *rel;
	struct elf_hdr *elfp;
	unsigned long loadaddr;
	int i, size;

	seg = map->segs;
	loadaddr = seg->addr;

	/* Find the .rela.dyn section start and size */
	for (i = 0; i < 64; i++, dynp++) {
		if (dynp->d_tag == DT_RELOCATION)
			rel = (struct elf_relocation *) (loadaddr + dynp->d_un.d_ptr);
		if (dynp->d_tag == DT_RELOCATIONSZ)
			size = dynp->d_un.d_val / sizeof(struct elf_relocation);
		if (dynp->d_tag == DT_NULL)
			break;
	}

	/* Process the relocations */
	for (i = 0; i < size; i++, rel++)
		relocate(loadaddr, rel);

	elfp = (struct elf_hdr *) loadaddr;
	return loadaddr + elfp->e_entry;
}

