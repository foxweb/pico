/*
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
*/

/* [[[
# Variadic functions

Say you want to define a variadic function with the following prototype:

    myfunc(int a [, char b [, void *c]])

In other words, you want `b` and `c` to be optional.

Simply, define your function with another name (say `my_func()`) and specify
how it should be called when invoked with 1, 2 or 3 paramenters as shown 
in the example below.

Example:

    #include "utl.h"

    int my_func(int a, char b, void *c);
    
    #define myfunc(...)     vrg(myfunc, __VA_ARGS__)
    #define myfunc1(a)      my_func(a,'\0',NULL)
    #define myfunc2(a,b)    my_func(a,b,NULL)
    #define myfunc3(a,b,c)  my_func(a,b,c)

**
]]] */

#ifndef VRG_VERSION
#define VRG_VERSION 0x0001000C

#define vrg_cnt(vrg1,vrg2,vrg3,vrg4,vrg5,vrg6,vrg7,vrg8,vrgN, ...) vrgN
#define vrg_argn(...)  vrg_cnt(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define vrg_cat0(x,y)  x ## y
#define vrg_cat(x,y)   vrg_cat0(x,y)

#define vrg(vrg_f,...) vrg_cat(vrg_f, vrg_argn(__VA_ARGS__))(__VA_ARGS__)

#endif