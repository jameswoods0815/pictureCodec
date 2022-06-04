#ifndef _Enforce___
#define _Enforce___

#include<iostream>
#include<queue>
#include<vector>
#include<algorithm>
#include<float.h>
using namespace std;




void EnforceConnectivity(
		float** L1,
		float** L2,
		float** a1,
		float** a2,
		float** b1,
		float** b2,
		float** x1,
		float** x2,
		float** y1,
		float** y2,
		double** W,
		unsigned short int* label,
		int threshold,
		int nRows,
		int nCols
                         );


#endif