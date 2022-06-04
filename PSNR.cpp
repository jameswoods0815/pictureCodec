//
//  PSNR.cpp
//  pictureCodec
//
//  Created by jameswoods on 11/22/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#include "PSNR.hpp"

double calcPSNR(const cv::Mat &I1, const cv::Mat &I2)
{
    const double eps = 1e-10;
    cv::Mat s1 = cv::abs(I1 - I2);
    
    s1.convertTo(s1, CV_32F);						// cannot make a square on 8 bits
    s1 = s1.mul(s1);							// |I1 - I2|^2
    
    const cv::Scalar s = sum(s1);						// sum elements per channel
    const double sse = s.val[0] + s.val[1] + s.val[2];			// sum channels
    
    if (sse <= eps) {							// for small values return zero
        return 0.0;
    }
    
    const double mse  = sse / (double)(I1.rows*I1.cols);
    const double psnr = 10.0 * log10((255.0 * 255.0) / mse);
    
    return psnr;
}
