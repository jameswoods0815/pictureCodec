#ifndef ENFORCECONNECTIVITY
#define ENFORCECONNECTIVITY

#include<queue>
#include<vector>
using namespace std;

//Enforce Connectivity by merging very small superpixels with their neighbors

void preEnforceConnectivity(unsigned short int* label, int nRows,int nCols);

#endif