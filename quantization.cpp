//
//  quantization.cpp
//  pictureCodec
//
//  Created by jameswoods on 11/20/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#include "quantization.hpp"


float num2step(float num)
{
    return num;
}


void quant (std::vector<ljj::Vec3d> src, std::vector<std::vector<int>> & out, float QPY,float QPCb,float QPCr  )
{
    float qpy=num2step(QPY);
    float qpcb=num2step(QPCb);
    float qpcr=num2step(QPCr);
    
    std:: vector<int> temp(3);
    
    for (int i=0;i<src.size();i++)
    {
        temp[0]=std::round(src[i][0]/qpy);
        temp[1]=std::round(src[i][1]/qpcb);
        temp[2]=std::round(src[i][2]/qpcr);
//        out.push_back(temp);
        out[i]=temp;
    }
    
}
void dequant(std::vector<std::vector<int>> src, std::vector<ljj::Vec3d> & out,float QPY, float QPCb,float QPCr)
{
    float qpy=num2step(QPY);
    float qpcb=num2step(QPCb);
    float qpcr=num2step(QPCr);
    
    ljj:: Vec3d temp;
    out.reserve(src.size());
    for (int i=0;i<src.size();i++)
    {
        temp[0]=src[i][0]*qpy;
        temp[1]=src[i][1]*qpcb;
        temp[2]=src[i][2]*qpcr;
        out[i]=temp;
    }
}


void deQuMat(ljj::Mat_64FC3 &src, float QPY,float QPCb, float QPCr)
{
    float qpy=num2step(QPY);
    float qpcb=num2step(QPCb);
    float qpcr=num2step(QPCr);

    
    for(int i=0;i<src.rows();i++)
        for(int j=0;j<src.cols();j++)
        {
            src(i,j)(0)*=qpy;
            src(i,j)(1)*=qpcb;
            src(i,j)(2)*=qpcr;
        }
}
