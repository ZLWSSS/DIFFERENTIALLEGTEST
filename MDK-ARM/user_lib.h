#ifndef USER_LIBS_H
#define USER_LIBS_H
#include "struct_typedef.h"
int32_t float_to_uint(float x, float x_min, float x_max, int bits);
fp32 uint_to_float(int x_int, float x_min, float x_max, int bits);
float fmaxf(float x, float y);
float fminf(float x, float y);
#endif
