//
//  predictObj.hpp
//  pictureCodec
//
//  Created by jameswoods on 11/8/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#ifndef predictObj_hpp
#define predictObj_hpp

#include <stdio.h>
#include<fftw3.h>
#include <vector>
#include <opencv2/opencv.hpp>

void pred_planar(uint8_t *src,  uint8_t *top, uint8_t *left, int stride,int log2size);
void pred_dc (uint8_t *src, uint8_t *top, uint8_t *left, int stride, int log2size,int flag);
void pre_angular(uint8_t * src, uint8_t *top,uint8_t *left, int size, int stride, int mode);
void testFunctionPred(void);
void testImagePred(void);

#endif /* predictObj_hpp */
