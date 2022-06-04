//
//  CodecAShape.hpp
//  pictureCodec
//
//  Created by jameswoods on 11/16/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#ifndef CodecAShape_hpp
#define CodecAShape_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>

typedef struct{
    
    std::vector<bool> Y;
    std::vector<bool> Cb;
    std::vector<bool> Cr;
    
    std::vector<int> enY;
    std::vector<int> enCb;
    std::vector<int> enCr;
    
    std::vector<bool> flagY;
    std::vector<bool> flagCb;
    std::vector<bool> flagCr;
    
    std::vector<int> mode;
    std::vector<int> splitmode;
    
    int size;
    std::vector<uint8_t> code;
    
    cv::Mat contextY;
    cv::Mat contextCb;
    cv::Mat contextCr;
    
    cv::Mat contextMask;
    
}codecObjectContext;




#endif /* CodecAShape_hpp */
