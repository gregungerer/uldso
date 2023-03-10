/* SPDX-License-Identifier: GPL-2.0 */
/*
 * arm.S -- start up code for no-MMU ELF link loader
 *
 * (C) Copyright 2022-2023, Greg Ungerer <gerg@kernel.org>
 */

.text

.global _start
.global linker

_start:
	mov	r4, r0			/* save the _atexit func for later */
	mov	r0, r7			/* 'mapaddr' for executable */
	mov	r1, r9			/* 'dynaddr' argument */

	bl	linker			/* relocate it */

	mov	r1, r0			/* program entry address */
	mov	r0, r4			/* restore _atexit func ptr */
	mov	pc, r1			/* start the program proper */
