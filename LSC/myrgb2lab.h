#ifndef MYRGB2LSB
#define MYRGB2LAB

#include<cmath>

// Change from RGB colour space to LAB colour space

void RGB2XYZ(unsigned char sR,unsigned char sG,unsigned char sB,double&	X,double& Y,double& Z);

void RGB2LAB(const unsigned char& sR, const unsigned char& sG, const unsigned char& sB, unsigned char& lval, unsigned char& aval, unsigned char& bval);

void myrgb2lab(unsigned char* r,unsigned char* g,unsigned char* b,
		unsigned char* L,unsigned char* A,unsigned char* B,
		int nRows,int nCols
               );


#endif