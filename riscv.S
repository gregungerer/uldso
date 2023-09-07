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
	mv	a0, sp			/* start of stack */

	call	linker			/* relocate it */

	mv	ra, a0			/* program entry address */
	li	a0, 0			/* NULL _atexit func ptr */
	ret				/* start the program proper */

