#ifndef LSCSUPERPIXEL
#define LSCSUPERPIXEL

#include"Initialize.h"
#include"Seeds.h"
#include"DoSuperpixel.h"
#include"point.h"
#include"myrgb2lab.h"
#include "countSuperpixel.h"
#include <opencv2/opencv.hpp>
using namespace std;

//LSC superpixel segmentation algorithm

void LSC(unsigned char* R,unsigned char* G,unsigned char* B,int nRows,int nCols,int superpixelnum,double ratio,unsigned short* label);
cv::Mat segMat( cv::Mat src,int superpixelnum,double ratio);

#endif
