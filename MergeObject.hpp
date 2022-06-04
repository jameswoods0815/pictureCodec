//
//  MergeObject.hpp
//  pictureCodec
//
//  Created by jameswoods on 10/7/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#ifndef MergeObject_hpp
#define MergeObject_hpp

#include <stdio.h>
#include<opencv2/opencv.hpp>

//conditon  only for Y,not UV

typedef struct feature {
    double var;
    double mean;
    double num;
} feature;

class  MergeObj{
public:
   void Merge(double var, cv::Mat label, cv::Mat src);
    bool tryCondition(std::vector<unsigned char> in1,  double var,double & varnow);
    bool varmerge(feature f1, feature f2, feature &output);
    
    void getFeature(std::vector<unsigned char> in, feature &f);
    
};




#endif /* MergeObject_hpp */
