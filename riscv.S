/* SPDX-License-Identifier: GPL-2.0 */
/*
 * riscv.S -- start up code for no-MMU ELF link loader
 *
 * (C) Copyright 2022, Greg Ungerer <gerg@kernel.org>
 */

.text

.global _start
.global linker

_start:
	mv	s1, a0			/* save the _atexit func for later */
	mv	a0, a1			/* 'mapaddr' for execitable */
	mv	a1, a3			/* 'dynaddr' argument */

	call	linker			/* relocate it */

	mv	ra, a0			/* program entry address */
	mv	a0, s1			/* restore _atexit func ptr */
	ret				/* start the program proper */

