
#ifndef __OSINLINE__
#define __OSINLINE__

/* What goes herein depends heavily on the OS. */

#define vec_mult _vec_mult
INLINE int _vec_mult(int x, int y)
{
    int res_hi, res_lo;

    __asm__ __volatile__
    ("smull\t%1,%0,%2,%3"
    : "=r"(res_hi), "=r"(res_lo)
    : "r"(x), "r"(y)
    );

    return res_hi;
}

#endif /* __OSINLINE__ */
