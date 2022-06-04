//
//  common.hpp
//  pictureCodec
//
//  Created by jameswoods on 10/15/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#ifndef common_hpp
#define common_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <vector>


void getMap(cv::Mat label, std::vector<std::vector<int>> &location,std::vector<cv::Mat> &object );
void getScalar(std::vector<cv::Mat> object, std::vector<bool> &data);
void ReSampleDown(cv::Mat data,cv::Mat &result,cv:: Mat &recoverMat,cv:: Mat & error);



#endif /* common_hpp */
