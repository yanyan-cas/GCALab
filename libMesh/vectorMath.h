/* GCALab: An analysis tool for Graph Cellular Automata
 * Copyright (C) 2012  David J. Warne
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/* File: vectorMath.h
 *
 * Author: David J. Warne (david.warne@qut.edu.au)
 * 
 * School of Electrical Engineering and Computer Science
 * Faculty of Science and Engineering
 * Queensland University of Technology
 * 
 * Date Created: 10/03/2012
 * Last Modified: 10/03/2012
 *
 * Descritpion: A few prototypes for common vector math functions
 *
 * NOTE: at the moment not much, will be incrementally added two
 * =============================================================================
 */
 
#ifndef __VECTORMATH_H
#define __VECTORMATH_H
/*standard headers*/
#include <malloc.h>
#include <math.h>

/*norm types*/
#define TWO_NORM 2
#define ONE_NORM 1
#define INF_NORM 255
#define EUCLIDEAN TWO_NORM

/*function prototypes*/

/*32 - bit versions*/
float *Add_f(float *x, float *y, int n);
float *Diff_f(float *x, float *y, int n);
float *Midpoint_f(float *x, float *y, int n);
float *Mean_f(int num,float *vList, int n);
float Norm_f(float *x,int n,unsigned char norm);
float NormDiff_f(float *x, float *y,int n,unsigned char norm);
float Dot_f(float *x, float *y, int n);
float *Cross_f(float *x, float *y);
float MagCross_f(float *x, float *y);
float *Normal_f(float *x, float *y,float *z);
float ScalarTriple_f(float *x, float *y,float *z);

/*64 - bit versions*/
double *Add_lf(double *x, double *y, int n);
double *Diff_lf(double *x, double *y, int n);
double *Midpoint_lf(double *x, double *y, int n);
double *Mean_lf(int num,double *vList, int n);
double Norm_lf(double *x,int n,unsigned char norm);
double NormDiff_lf(double *x, double *y,int n,unsigned char norm);
double Dot_lf(double *x, double *y, int n);
double *Cross_lf(double *x, double *y);
double MagCross_lf(double *x, double *y);
double ScalarTriple_lf(double *x, double *y,double *z);
#endif
