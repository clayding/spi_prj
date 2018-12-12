/*
 * Copyright (c) 2008 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __DEBUG_H
#define __DEBUG_H

#include <stdio.h>


#if defined(__cplusplus)
extern "C" {
#endif

#if defined(DEBUG)
#define DEBUGLEVEL DEBUG
#else
#define DEBUGLEVEL 1
#endif

/* debug levels */
#define CRITICAL 0
#define ALWAYS   0
#define INFO     1
#define WARN     2
#define ERR      3
#define SPEW     4


#if DEBUGLEVEL >= ERR
/* Show all kind of errors e.g. when passing invalid payload length */
#define ERROR(...)      do{printf("Error:");printf(__VA_ARGS__);}while(0)
#else
#define ERROR(...)
#endif

#if DEBUGLEVEL >= WARN
/* Show warnings e.g. for FIFO errors */
#define WARNING(...)    do{printf("Warning:");printf(__VA_ARGS__);}while(0)
#else
#define WARNING(...)
#endif

#if DEBUGLEVEL >= INFO
/* Show warnings e.g. for FIFO errors */
#define INFOR(...)    do{printf(__VA_ARGS__);}while(0)
#else
#define INFOR(...)
#endif

void dump_debug_log(char *name, void* buf, int len);

#if defined(__cplusplus)

#endif

#endif