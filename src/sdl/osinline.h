#ifndef _OSINLINE_H_
#define _OSINLINE_H_


#ifdef USE_FLOAT_VEC
#define vec_mult(PRM1,PRM2) (int)(((float)(PRM1))*((float)(PRM2)))
#define vec_div(PRM1,PRM2) (int)(((float)(PRM1))/((float)(PRM2)))
#endif


#endif
