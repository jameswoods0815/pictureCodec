//
//  encodeShape.hpp
//  pictureCodec
//
//  Created by jameswoods on 10/21/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#ifndef encodeShape_hpp
#define encodeShape_hpp

#include <stdio.h>
#include <vector>
#include <opencv2/opencv.hpp>

class EncodeShape
{
    
private:
    // var
    std::vector<std::vector<int>>  location;
    std::vector<cv::Mat> object;
    int numOfObj;
    const int minObjSize=1; //if  x mode minObjsize, log, get the scale size;
    
    std::vector<int> levelOfObj; // get every obj pyr level
    
    std::vector<bool> allDetail;
    std::vector<bool> allError;
    
    std::vector<std::vector<int>> neighbor;
    std::vector<int> codeStateOfObject;
    
    //func
    
    //get location and object;
    void getNeighbor(cv::Mat label);
    
    void getCodeState(void);
    
    void SegObj(cv::Mat label);
    void encodeObj(cv::Mat Obj, std::vector<bool> & error, std::vector<bool> & detail );
    
    void encodeAsubObj(cv ::Mat obj, std::vector<bool> &error, std::vector <bool> & detail);
    
    void getNowEdge(cv:: Mat Obj, cv:: Mat & edge);
    void getUpEdge (cv:: Mat edge, cv::Mat & upEdge);
    
    void getLevelOfObj(void);
    
    void generatePyr(cv::Mat Obj, std::vector< cv:: Mat > & pyr, int level);
    
    void getDownMat(cv::Mat Obj, cv::Mat  & downObj);
    void getUpMat(cv::Mat downObj, cv::Mat  & upObj );
    
    void seeDelete( cv::Mat label);
    
public:
    int  encodeShape(cv::Mat shape, std::vector<uchar> & out);
    int decodeShape(std::vector<uchar> out, cv::Mat & shape );
    
    
    
};

#endif /* encodeShape_hpp */
