#ifndef DOSUPERPIXEL
#define DOSUPERPIXEL

#include<vector>
#include"preEnforceConnectivity.h"
#include<algorithm>
#include"EnforceConnectivity.h"
#include"point.h"
using namespace std;


//Perform weighted kmeans iteratively in the ten dimensional feature space.

void DoSuperpixel(
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
		point* seedArray,
		int seedNum,
		int nRows,
		int nCols,
		int StepX,
		int StepY,
		int iterationNum,
		int thresholdCoef
                  );

#endif
