
/ 1 "<stdin>"

/ 1 "SYS.h"

/ 1 "/usr/include/syscall.h" 3

/ 9 "SYS.h"


.comm	_errno,2
/ 43 "SYS.h"
.globl	x_norm, x_error
/ 1 "<stdin>"

.globl _sysaccess; _sysaccess: .data; 1: _sysaccess+1; .text; .globl mcount; mov $1b, r0; jsr pc,mcount;;; sys 33.; jmp x_norm
