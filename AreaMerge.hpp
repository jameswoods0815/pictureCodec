//
//  AreaMerge.hpp
//  pictureCodec
//
//  Created by jameswoods on 11/6/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#ifndef AreaMerge_hpp
#define AreaMerge_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>

class areaMerge {
    
private:
    std::vector<std::vector<float>> myf;
    bool judgeFeature(int f1,int f2);
    
public:
    
    cv::Mat getNeighbor(cv::Mat label, cv::Mat src);
    void  SegObj(cv::Mat label, cv::Mat src);

    
    
};




#endif /* AreaMerge_hpp */
