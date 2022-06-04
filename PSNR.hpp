//
//  PSNR.hpp
//  pictureCodec
//
//  Created by jameswoods on 11/22/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#ifndef PSNR_hpp
#define PSNR_hpp

#include <stdio.h>
#include<opencv2/opencv.hpp>
double calcPSNR(const cv::Mat &I1, const cv::Mat &I2);

#endif /* PSNR_hpp */
