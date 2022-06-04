//
//  SegMaskToBlock.hpp
//  pictureCodec
//
//  Created by jameswoods on 11/10/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#ifndef SegMaskToBlock_hpp
#define SegMaskToBlock_hpp

#include <stdio.h>
#include <vector>
#include<opencv2/opencv.hpp>
#include<iostream>

class maskSeg {
private:
    const int boundStride=32;
    const int innerstride=128;
    
    std::vector<std::vector<int>> segbou;
    std::vector<std::vector<int>>seginner;
    cv::Mat extendMat;
    
    void viewSeg(cv::Mat src);
public:
    void seg(cv::Mat src);
    std::vector<std::vector<int>>  getBodIndex()
    {
        return segbou;
    }
    
        std::vector<std::vector<int>> getInnerIndex()
    {
        return seginner;
    }
    int getBouStride(void)
    {
        return boundStride;
    }
    
    int getInnerStride(void)
    {
        return innerstride;
    }
    
    cv::Mat getExtendMat()
    {
        return extendMat;
    }
    
};


#endif /* SegMaskToBlock_hpp */
