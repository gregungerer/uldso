/* SPDX-License-Identifier: GPL-2.0 */
/*
 * m68k.S -- start up code for no-MMU ELF link loader
 *
 * (C) Copyright 2022, Greg Ungerer <gerg@kernel.org>
 */

.text

.global _start
.global linker

_start:
	movel	%d5, %sp@-		/* 'dynaddr' argument */
	movel	%d3, %sp@-		/* 'mappadr' for executable */
	bsr	linker			/* relocate it */
	addql	#8, %sp

	movel	#0, %a1			/* set _init/_fini parameter NULL */
	movel	%d0, %a0		/* returned with 'entry' address */
	jmp	(%a0)			/* start the program proper */

