//
//  analysisParameter.cpp
//  pictureCodec
//
//  Created by jameswoods on 11/25/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#include "analysisParameter.hpp"

#include<stdlib.h>
#include <math.h>
#include "predictObj.hpp"
#include<opencv2/opencv.hpp>

float getDiff(double *src, double * predict, bool * mask, int length)
{
    float diff=0;
    for(int i=0;i<length;i++)
    {
        if(mask[i])
        {
            diff+=abs(src[i]-predict[i]);
        }
    }
    return  diff;
}
int  getZeroNum(int *src, bool *mask,int length )
{
    int count=0;
    for(int i=0;i<length;i++)
    {
        if(!src[i]&&mask[i])
        {
            count++;
        }
    }
    return  count;
}

int getsegMode(double * src, bool * mask, int stride)
{
    double mean[4]={0};
    double std[4]={0};
    double cnt[4]={0};
    int count=0;
    for(int i=0;i<stride;i+=stride)
        for (int j=0;j<stride;j+=stride)
        {
            
            for(int p=0;p<stride;p++)
                for (int q=0;q<stride;q++)
                {
                    if(mask[(i+p)*stride+j+q])
                    {
                        mean[count]+=src[(i+p)*stride+j+q];
                        cnt[count]++;
                    }
                }
            count++;
        }
    
    for(int i=0;i<4;i++)
    {
        mean[i]/=cnt[i];
    }
    
    count=0;
    for(int i=0;i<stride;i+=stride)
        for (int j=0;j<stride;j+=stride)
        {
            
            for(int p=0;p<stride;p++)
                for (int q=0;q<stride;q++)
                {
                    if(mask[(i+p)*stride+j+q])
                    {
                        std[count]+=(src[(i+p)*stride+j+q]-mean[count])*(src[(i+p)*stride+j+q]-mean[count]);
                    }
                }
            count++;
        }
    
    for(int i=0;i<4;i++)
    {
        std[i]=sqrt(std[i]/(cnt[i]-1));
    }
    
    double mergeMean=0,mergeStd=0;
    for(int i=0;i<4;i++)
    {
        mergeMean+=mean[i]*cnt[i];
    }
    mergeMean/=(cnt[0]+cnt[1]+cnt[2]+cnt[3]);
    
    for(int i=0;i<4;i++)
    {
        mergeStd+=(std[i]*(cnt[i]-1)+(cnt[i]-1)*(mergeMean-mean[i])*(mergeMean-mean[i]));
    }
    mergeStd=mergeStd/(cnt[0]+cnt[1]+cnt[2]+cnt[3]-1);
    
    // get the socre and judge the score;
    float score[4];
    for (int i=0;i<4;i++)
    {
        score[i]=(mean[i]*mergeMean+std[i]*mergeStd)/sqrt(mean[i]*mean[i]+std[i]*std[i]);
    }
    for (int i=1;i<4;i++)
    {
        score[i]=score[i]/sqrt(mergeStd*mergeStd+mergeMean*mergeMean);
    }
    
    float allScore=0;
    for (int i=0;i<4;i++)
    {
        allScore+=score[i];
    }
    
    allScore/=4;
    if(allScore>0.99)
        return 0;
    else
        return 1;
}

void getPridict(uint8_t * predict,uint8_t *left, uint8_t *up, int stride,int mode )
{
    if(mode==0)
    {
        pred_dc(predict, up+1, left+1, stride, log2(stride), 0);

    }
    else if (mode ==1)
    {
        pred_planar(predict, up+1, left+1, stride, log2(stride));

    }
    else{
         pre_angular(predict, up+1, left+1, stride, stride, mode);
    }
}


double getDiffUint(double *src, uint8_t * predict, bool * mask, int length)
{
    double diff=0;
    for(int i=0;i<length;i++)
    {
        if(mask[i])
        {
            diff+=fabs(src[i]-predict[i]);
        }
    }
    return  diff;
}

void showSrcAndPre(double *src, uint8_t * predict ,int stride )
{
    cv::Mat imgsrc(stride,stride,CV_8UC1);
    cv::Mat imgpre(stride,stride,CV_8UC1);
    for(int i=0;i<stride;i++)
        for (int j=0;j<stride;j++)
        {
            imgsrc.at<uchar>(i,j)=src[i*stride+j];
            imgpre.at<uchar>(i,j)=predict[i*stride+j];
        }
    
    cv::imshow("src", imgsrc);
    cv::imshow("predict", imgpre);
    cvWaitKey(1);
    
}

int getBestMode(double *src,uint8_t *predict, bool *mask, uint8_t *Left,uint8_t *up, int stride, std::vector<int> preMode)
{
    uint8_t temp[stride*stride];
    double diff=1e100;
    double tempdiff;
    int mode=-1;
    
    if(preMode.empty())
    {
        for(int i=0;i<35;i++)
        {
            getPridict(temp, Left, up, stride, i);
            tempdiff=getDiffUint(src, temp, mask, stride*stride);
            if(tempdiff<diff)
            {
                mode=i;
                diff=tempdiff;
                memcpy(predict, temp, stride*stride*sizeof(uint8_t));
            }
        }
//        showSrcAndPre(src, predict, stride);

    }
    else {
        for(int i=0;i<preMode.size();i++)
        {
            getPridict(temp, Left, up, stride, preMode[i]);
            tempdiff=getDiffUint(src, temp, mask, stride*stride);
            if(tempdiff<diff)
            {
                mode=i;
                diff=tempdiff;
                memcpy(predict, temp, stride*stride*sizeof(uint8_t));
            }
        }

    }
    
    return mode;
}



