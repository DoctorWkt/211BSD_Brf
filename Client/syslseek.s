
/ 1 "<stdin>"

/ 1 "SYS.h"

/ 1 "/usr/include/syscall.h" 3

/ 9 "SYS.h"


.comm	_errno,2
/ 43 "SYS.h"
.globl	x_norm, x_error
/ 1 "<stdin>"

.globl _syslseek; _syslseek: .data; 1: _syslseek+1; .text; .globl mcount; mov $1b, r0; jsr pc,mcount;;; sys 19.; bcc 1f; mov r0,_errno; mov $-1,r1; sxt r0; 1: rts pc;
