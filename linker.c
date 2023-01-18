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
 * so this make it pretty simple. We define these numbers here to avoid
 * any dependency on C-library headers. The kernel uapi headers do not
 * define these numbers (well, at least for some architectures).
 */
#ifdef arm
#define R_RELATIVE	23 /*R_ARM_RELATIVE*/
#endif

#ifdef m68k
#define R_RELATIVE	22 /*R_68K_RELATIVE*/
#endif

#ifdef riscv
#define R_RELATIVE	3 /*R_RISCV_RELATIVE*/
#endif

static void relocate(unsigned long loadaddr, struct elf_rela *rela)
{
	unsigned long *paddr;

	switch (rela->r_info) {
	case R_RELATIVE:
		if (rela->r_addend) {
			paddr = (unsigned long *) (loadaddr + rela->r_offset);
			*paddr = loadaddr + rela->r_addend;
		}
		break;
	}
}

unsigned long linker(struct elf_fdpic_loadmap *map, elf_dynamic *dynp)
{
	struct elf_fdpic_loadseg *seg;
	struct elf_rela *rela;
	struct elf_hdr *elfp;
	unsigned long loadaddr;
	int i, rela_cnt;

	seg = map->segs;
	loadaddr = seg->addr;

	/* Find the .rela.dyn section start and size */
	for (i = 0; i < 32; i++, dynp++) {
		if (dynp->d_tag == DT_RELA)
			rela = (struct elf_rela *) (loadaddr + dynp->d_un.d_ptr);
		if (dynp->d_tag == DT_RELASZ)
			rela_cnt = dynp->d_un.d_val / sizeof(struct elf_rela);
		if (dynp->d_tag == DT_NULL)
			break;
	}

	/* Process the relocations */
	for (i = 0; i < rela_cnt; i++, rela++)
		relocate(loadaddr, rela);

	elfp = (struct elf_hdr *) loadaddr;
	return loadaddr + elfp->e_entry;
}

