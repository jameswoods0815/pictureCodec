//
//  analysisParameter.hpp
//  pictureCodec
//
//  Created by jameswoods on 11/25/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#ifndef analysisParameter_hpp
#define analysisParameter_hpp

#include <stdio.h>
#include <vector>
float getDiff(double *src, double * predict, bool * mask, int length);
int  getZeroNum(int *src, bool *mask,int length );
int getsegMode(double * src, bool * mask, int stride);
int getBestMode(double *src,uint8_t *predict, bool *mask, uint8_t *Left,uint8_t *up, int stride, std::vector<int> preMode);

#endif /* analysisParameter_hpp */
