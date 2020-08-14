/*
 *  math-dlx.h:  Misc. declarations useful with dlx simulators.
 *  Copyright (C) 1994,1995 Peter Bergner, Aaron Sawdey
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Email: bergner@mountains.ee.umn.edu or sawdey@mountains.ee.umn.edu
 *
 *  Snail Mail:
 *
 *  Department of Electrical Engineering
 *  University of Minnesota
 *  4-174 EE/CSci Building
 *  200 Union Street S.E.
 *  Minneapolis, MN 55455
 *
 */

#define array
#define func_decl
#define func_def
#define FCAST
#define BOGUS_DIM
#define rpcc()	1
#define fmove(cond,src1,src2,dest)      (dest) = (cond) ? (src1) : (src2)
#define imove(cond,src1,src2,dest)      (dest) = (cond) ? (src1) : (src2)

#define fabs(x)                         (((x) < 0.0) ? -(x) : (x))
#define iabs(x)                         (((x) < 0) ? -(x) : (x))

/* Function declarations for common math functions
*/
extern double cos(double);
extern double acos(double);
extern double sin(double);
extern double asin(double);
extern double tan(double);
extern double atan(double);
extern double log10(double);
extern double log(double);
extern double exp(double);
extern double sqrt(double);
extern double pow(double,double);

extern long random(void);
extern void srandom(int);
extern int  fork(void);
extern void join(void);

extern void bcopy(void *,void *,int);

/* 
 * From /usr/include/sys/fcntlcom.h. These are
 * needed to use the open trap in the simulators.
 */
#define O_ACCMODE       (O_RDONLY|O_WRONLY|O_RDWR)
#define O_RDONLY        0
#define O_WRONLY        1
#define O_RDWR          2
#define O_APPEND        0x0008
#define O_CREAT         0x0200
#define O_TRUNC         0x0400
#define O_EXCL          0x0800
#define O_NONBLOCK      0x4000
#define O_NOCTTY        0x8000
extern int open();
extern int close();
extern int read();
extern int write();


