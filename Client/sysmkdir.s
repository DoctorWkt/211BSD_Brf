
/ 1 "<stdin>"

/ 1 "SYS.h"

/ 1 "/usr/include/syscall.h" 3

/ 9 "SYS.h"


.comm	_errno,2
/ 43 "SYS.h"
.globl	x_norm, x_error
/ 1 "<stdin>"

.globl _sysmkdir; _sysmkdir: .data; 1: _sysmkdir+1; .text; .globl mcount; mov $1b, r0; jsr pc,mcount;;; sys 136.; jmp x_norm
