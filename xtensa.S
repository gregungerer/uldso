/* SPDX-License-Identifier: GPL-2.0 */
/*
 * xtensa.S -- start up code for no-MMU ELF link loader
 *
 * (C) Copyright 2022-2023, Greg Ungerer <gerg@kernel.org>
 */

.text

.global _start
.global linker

_start:
	mov	a10, a4			/* 'mapaddr' for executable */
	mov	a11, a6			/* 'dynaddr' argument */
	call8	linker			/* relocate it */
	jx	a10			/* start the program proper */

