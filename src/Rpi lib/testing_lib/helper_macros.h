//https://github.com/dddrrreee/cs140e-20win/blob/master/libpi/libc/helper-macros.h
#ifndef __RPI_MACROS_H__
#define __RPI_MACROS_H__

// powers of 2
#define is_aligned(x, a)        (((x) & ((typeof(x))(a) - 1)) == 0)
#define is_aligned_ptr(x, a)        is_aligned((uintptr_t)x,a)

#define pi_roundup(x,n) (((x)+((n)-1))&(~((n)-1)))

#define NULL ((void*)0)


#endif