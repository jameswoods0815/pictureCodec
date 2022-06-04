//
//  runCodec.hpp
//  pictureCodec
//
//  Created by jameswoods on 11/9/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#ifndef runCodec_hpp
#define runCodec_hpp
#include <vector>
#include"codec.h"
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <fstream>
#include "s_dct.h"
#include "s_idct.h"

class runPic {
private:
    
    std::vector<std::vector<ljj::Vec3d>> data;
    cv::Mat restore;
    std::vector<ljj::Shape> shapes;
    cv::Mat source;
    
    ljj::Mat_64FC3  codecAshape(ljj::Shape, ljj::Mat_64FC3);
    
    void showPredChannel(uint8_t *Y ,uint8_t *Cb, uint8_t *Cr,int stride);
    ljj::Mat_64FC3    showImageChannel(cv::Mat Y,cv::Mat Cb,cv::Mat Cr);
    void showPredChannelDouble(double *Y ,double *Cb, double *Cr,int stride);
    
    std::vector<std::vector<int>> bouData; //bouData
    std::vector<int>bouLength; //bouData length
    std::vector<int>objectbouLength; //objectLength ;calculate by shape, Every shape can get every bou Lenght
    std::vector<int>objectinnerLength;
    std::vector<std::vector<int>>innerData; //innerData;
    std::vector<int>innerLength;// inner length; calculate by shape
    
    int QuantInit;
    
    DCTcontext context;
    IDCTcontext icontext;
    
public:
    std::vector<std::vector<ljj::Vec3d>> codecPic(cv::Mat src,int Quant);
    cv::Mat decodePic( std::vector<std::vector<ljj::Vec3d>> data);
};


#endif /* runCodec_hpp */
