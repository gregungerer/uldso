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
	mov	r0, r7			/* 'mapaddr' for executable */
	mov	r1, r9			/* 'dynaddr' argument */

	bl	linker			/* relocate it */

	mov	r1, r0			/* program entry address */
	ldr	r0, =0			/* restore _atexit func ptr */

#ifdef ELF_ARGC_ARGV
	/*
	 * Convert the stack layout of argc/argv values from traditional MMU
	 * kernel (which ELF-FDPIC loader uses - even on no-MMU) to FLAT
	 * format no-MMU layout (which is what an un-modified uClibc-ng will
	 * expect).
	 */
	ldr	r4, [sp], #4		/* pop argc */
	add	r5, r4, #1
	add	r5, sp, r5, lsl #2	/* calculate env ptr */
	str	r5, [sp, #-4]!		/* push env ptr */
	add	r5, sp, #4		/* calculate argv ptr */
	str	r5, [sp, #-4]!		/* push argv ptr */
	str	r4, [sp, #-4]!		/* push argc */
#endif

	mov	pc, r1			/* start the program proper */
