//
//  main.cpp
//  pictureCodec
//
//  Created by jameswoods on 10/6/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include "LSC.h"
#include <time.h>
#include "runCodec.hpp"
#include "Arithmetic.hpp"
#include"common.hpp"
#include "encodeshape.hpp"
#include "AreaMerge.hpp"
#include"ViewPic.hpp"
#include "MergeObject.hpp"
#include "predictObj.hpp"
#include "LabelInsert.hpp"
#include"columbiaCode.hpp"
#include "GetContext.hpp"
#include "codecAblock.hpp"
#include "Codec2_0.hpp"
#include <stdlib.h>
using namespace std;
int main(int argc, const char * argv[]) {
    int  x=argc;
    char buf[200];
    if(x>1)
    {
        strcpy(buf, argv[1]);
    }
    cv::Mat src;
    if(x>1)
    {
        src=cv::imread(buf);
    }
    else{
        src=cv::imread("/Users/jameswoods/Documents/MATLAB/test/2.JPG");
    }
    
    if (src.empty())
    {
        cout<<" picture is null, return";
        return 0;
    }
    
    cv::imshow("window", src);
    cv::waitKey(1);
    
    int QP;
    if(x>2)
    {
        char buf1[100];
        strcpy(buf1, argv[2]);
        QP= atoi(buf1);
    }
    else {
        QP=500;
    }
    
   std::cout<<"QP is  " <<QP<<std::endl;
   runPic pp;
   std::vector<std::vector<ljj::Vec3d>> dataA =pp.codecPic(src,QP);
    cvWaitKey(0);
    return 0;
}
