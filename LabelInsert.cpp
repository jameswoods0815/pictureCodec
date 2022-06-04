//
//  LabelInsert.cpp
//  pictureCodec
//
//  Created by jameswoods on 11/10/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#include "LabelInsert.hpp"

cv:: Mat insertLable( cv::Mat src, int targetRows, int targetCols)
{
    cv:: Mat ret(targetRows,targetCols, CV_16UC1);
    
    int rows=src.rows;
    int cols=src.cols;
    
    int stride= targetRows/rows+((targetRows%rows)>0?1:0);
    
    for (int i=0;i<targetRows;i++)
        for (int j=0;j<targetCols;j++)
        {
            ret.at<unsigned short>(i,j)=src.at<unsigned short>(i/stride,j/stride);
        }
    return ret;
}