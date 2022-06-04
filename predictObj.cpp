//
//  predictObj.cpp
//  pictureCodec
//
//  Created by jameswoods on 11/8/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#include "predictObj.hpp"
#include <vector>
#include <fstream>
#include <iostream>
#include <map>
#include <set>

#include<opencv2/opencv.hpp>

using namespace std;

#define POS(x, y) src[(x) + stride * (y)]

//planar mode
void pred_planar(uint8_t *src,  uint8_t *top, uint8_t *left, int stride,int log2size)
{
    int size = 1 << log2size;
    for (int y = 0; y < size; y++)
        for (int x = 0; x < size; x++)
        {
            src[stride*y+x] = ((size - 1 - x) * left[y] + (x + 1) * top[size]  +
                         (size - 1 - y) * top[x]  + (y + 1) * left[size] + size) >> (log2size + 1);
        }
}


// if light is small, dc  need to do some fix;
void pred_dc (uint8_t *src, uint8_t *top, uint8_t *left, int stride, int log2size,int flag)
{
    int size=1<<log2size;
    int dc=size;
    for (int i=0;i<size;i++)
    {
        dc+=(left[i]+top[i]);
    }
    
    dc>>=(log2size+1);
    for (int i=0;i<size;i++)
        for(int j=0;j<size;j++)
        {
            src[i*stride+j]=dc;
        }
    
    if (flag)
    {
        src[0]=(left[0]+2*dc+top[0]+2)>>2;
        for(int i=1;i<size;i++)
        {
            src[i]=(top[i]+3*dc+2)>>2;
        }
        
        for(int i=1;i<size;i++)
        {
            src[stride*i]=(left[i]+3*dc+2)>>2;
        }
    }
}
 void pre_angular(uint8_t * src, uint8_t *top,uint8_t *left, int size, int stride, int mode)
{
    static const int8_t intra_pred_angle[] = {
        32,  26,  21,  17, 13,  9,  5, 2, 0, -2, -5, -9, -13, -17, -21, -26, -32,
        -26, -21, -17, -13, -9, -5, -2, 0, 2,  5,  9, 13,  17,  21,  26,  32
    };
    static const int16_t inv_angle[] = {
        -4096, -1638, -910, -630, -482, -390, -315, -256, -315, -390, -482,
        -630, -910, -1638, -4096
    };
    
    int angle=intra_pred_angle[mode-2];
    uint8_t ref_array[2000];
    uint8_t * ref_tmp=ref_array+size;
    uint8_t *ref;
    int last=(size*angle)>>5;
    
    if(mode>=18)
    {
        ref=top-1;
        if(angle<0&&last<-1) {
            for(int i=0;i<size;i++)
            {
                ref_tmp[i]=top[i-1]; //problem
            }
            
            for(int i=last;i<=-1;i++)
            {
                ref_tmp[i]=left[-1+((i*inv_angle[mode-11]+128)>>8)]; // may have some problem
            }
            ref=ref_tmp;
            
        }
        
        for(int y=0;y<size;y++)
        {
            int idx=((y+1)*angle)>>5;
            int ratio=((y+1)*angle)&31;
            for(int x=0;x<size;x++)
            {
                src[y*size+x]=((32-ratio)*ref[x+idx+1]+ratio*ref[x+idx+2]+16)>>5;
            }
        }
    }
    else {
        ref=left-1;
        if(angle<0&&last<-1) {
            for(int x=0;x<size;x++)
            {
                ref_tmp[x]=left[x-1]; //problem
            }
            
            for(int x=last;x<=-1;x++)
            {
                ref_tmp[x]=top[-1+((x*inv_angle[mode-11]+128)>>8)];
            }
            ref=ref_tmp;
        }
        
        for(int x=0;x<size;x++)
        {
            int idx=((x+1)*angle)>>5;
            int ratio=((x+1)*angle)&31;
            for(int y=0;y<size;y++)
            {
                src[y*stride+x]=((32-ratio)*ref[y+idx+1]+ratio*ref[y+idx+2]+16)>>5;
            }
            
        }
    }
    
    
}

void testFunctionPred(void)
{
    uint8_t  data1[16]={1,1,1,1,10,10,10,10,20,20,20,20,30,30,30,30};
    uint8_t left1[9]={1,1,10,20,30,40,50,60,70};
    uint8_t up1[9]={1,1,1,1,1,1,1,1,1};
    uint8_t *left=left1+1;
    uint8_t *up=up1+1;
    
    uint8_t srcc[16];
    pred_planar(srcc, up, left, 4, 2);
    
    pred_dc(srcc, up, left, 4, 2, 0);
    
    pre_angular(srcc, up, left, 4, 4, 26);
    
    
}

cv::Mat u2Mat(uint8_t *src, int num)
{
    cv::Mat img(num,num, CV_8UC1);
    
    for(int i=0;i<num;i++)
        for (int j=0; j<num; j++) {
            img.at<uchar>(i,j)=src[i*num+j];
        }
    return img;
}

void testImagePred(void)
{
    cv::Mat src;
    src=cv::imread("/Users/jameswoods/Documents/MATLAB/test/2.JPG");
    
    cv::resize(src, src, cv::Size(src.cols/2,src.rows/2));
    
    cv::imshow("window", src);
    cvWaitKey(1);
    
    int x=20;
    int y=20;
    
   int  num=128;
    
    uint8_t top[num*2+1];
    uint8_t  left[num*2+1];
    for(int i=x;i<x+num*2+1;i++)
    {
        top[i-x]=src.at<uchar>(y,3*i);
    }
    
    for(int i=y;i<y+num*2+1;i++)
    {
        left[i-y]=src.at<uchar>(i,3*x);
    }
    
    cv::Mat img(num,num,CV_8UC1);
    for(int i=x+1;i<x+num+1;i++)
     for(int j=y+1;j<y+num+1;j++)
     {
        img.at<uchar>(i-x-1,j-y-1)=src.at<uchar>(i,3*j);
     }
    
    imshow("img",img);

    cvWaitKey(1);
    
    //plannar
    uint8_t ss[num*num];
    pred_planar(ss, top+1, left+1, num, log2(num));
    cv::Mat predP=u2Mat(ss, num);
    imshow("prr", predP);
    cvWaitKey(1);

    //DC
    uint8_t ss1[num*num];
    pred_dc(ss1, top+1, left+1, num, log2(num), 0);
    cv::Mat predDc=u2Mat(ss1, num);
    imshow("prDC", predDc);
    cvWaitKey(1);

    //angle
    uint8_t ss2[num*num];
    
    for(int i=2;i<35;i++)
    {
        pre_angular(ss2, top+1, left+1, num, num, i);
        cv::Mat predAn=u2Mat(ss2, num);
        imshow("prAngle", predAn);
        cvWaitKey(300);
    }
    
   

    

//    cout<<"OK";
    
    
    
}
