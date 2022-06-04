//
//  SegMaskToBlock.cpp
//  pictureCodec
//
//  Created by jameswoods on 11/10/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#include "SegMaskToBlock.hpp"

#include"ViewPic.hpp"

void maskSeg:: viewSeg(cv::Mat src)
{
    
}

// insert the map to the stride


cv::Mat  fillGap(cv::Mat src, int stride)
{
    int rows=src.rows;
    int cols=src.cols;
    int countx=rows%stride;
    int county=cols%stride;
    
    int fixRows=rows+(stride-countx)%stride;
    int fixCols=cols+(stride-county)%stride;
    
    cv::Mat ret(fixRows,fixCols,CV_8UC1,cv::Scalar(0));
    {
        for (int i=0;i<rows;i++)
            for (int j=0;j<cols;j++)
            {
                ret.at<uchar>(i,j)=src.at< uchar>(i,j);
            }
    }
    
    return  ret;
    
}

cv::Mat getBound(cv::Mat src)
{
    cv:: Mat ret(src.rows,src.cols,CV_8UC1,cv::Scalar(0));
    
    //0 cols and last cols;
    for (int i=0;i<src.rows;i++)
    {
        if(src.at<uchar>(i,0)>0)
            ret.at<uchar>(i,0)=1;
        if(src.at<uchar>(i,src.cols-1)>0)
            ret.at<uchar>(i,src.cols-1)=1;
    }
    
    for (int i=0;i<src.cols;i++)
    {
        if(src.at<uchar>(0,i)>0)
            ret.at<uchar>(0,i)=1;
        if(src.at<uchar>(src.rows-1,i)>0)
            ret.at<uchar>(src.rows-1)=1;
    }
    
    for (int i=1;i<src.rows-1;i++)
        for (int j=1;j<src.cols-1;j++)
        {
            if(src.at<uchar>(i,j)>0)
            {
                for (int x=i-1;x<i+2;x++)
                    for (int y=j-1;y<j+2;y++)
                    {
                        if (src.at<uchar>(x,y)==0)
                        {
                            ret.at<uchar>(i,j)=1;
                            break;
                        }
                    }
            }
            
        }
    
    return ret;
}

void maskSeg:: seg(cv::Mat src)
{
    assert(src.channels()==1);
    
    //bou extend 32*32
    cv:: Mat mask=fillGap(src, innerstride);
    
    extendMat=mask.clone();
    
//    imshow("mask", mask*50);
//    cvWaitKey(1);
    
    cv::Mat edge=getBound(mask);
    
    cv::Mat bouMatrix(mask.rows,mask.cols,CV_16UC1,cv::Scalar(1)); // >2 bou, 1 inner , 2, background
    
    cv::Mat innerIndexMat(mask.rows,mask.cols,CV_16UC1,cv::Scalar(1));
    cv::Mat innerMatrix=mask.clone();
    
    //get bouIndex;
    int count=1;
    for (int i=0;i<edge.rows;i+=boundStride)
        for (int j=0;j<edge.cols;j+=boundStride)
        {
            bool flag=false;
            for(int x=i;x<i+boundStride;x++)
                for(int y=j;y<j+boundStride;y++)
                {
                    if(edge.at<uchar>(x,y)>0)
                    {
                        flag=true;
                        
                    }
                }
            if (flag)
            {
                count++;
                std::vector<int> temp;
                temp.push_back(i);
                temp.push_back(j);
                segbou.push_back(temp);
            }
            // bouMarix flash
            if (flag)
            {
                for (int x=i;x<i+boundStride;x++)
                    for(int y=j;y<j+boundStride;y++)
                    {
                        if(mask.at<uchar>(x,y))
                        {
                            bouMatrix.at<unsigned short>(x,y)=count;
                        }
                    }
            }
            // delete the innerMatrix
            if (flag)
            {
                for (int x=i;x<i+boundStride;x++)
                    for(int y=j;y<j+boundStride;y++)
                    {
                        if(mask.at<uchar>(x,y))
                        {
                            innerMatrix.at<uchar>(x,y)=0;
                        }
                    }
            }

        }
    
//    viewImage(bouMatrix);
    //get the inside split;
    
    int tempIndex=1;
    for (int i=0;i<innerMatrix.rows;i+=innerstride)
        for (int j=0;j<innerMatrix.cols;j+=innerstride)
        {
            bool flag=false;
            for(int x=i;x<i+innerstride;x++)
                for (int y=j;y<j+innerstride;y++)
                {
                    if(innerMatrix.at<uchar>(x,y))
                    {
                        flag=true;
                        break;
                    }
                }
            if (flag)
            {
                tempIndex++;
                for(int x=i;x<i+innerstride;x++)
                    for (int y=j;y<j+innerstride;y++)
                    {
                        if(innerMatrix.at<uchar>(x,y))
                        {
                            innerIndexMat.at<unsigned short>(x,y)=tempIndex;
                        }
                    }
                
                std::vector<int> temp;
                temp.push_back(i);
                temp.push_back(j);
                seginner.push_back(temp);

            }
        }
    
//    viewImage(innerIndexMat);
//    std::cout<<"OK";
    
    //test if megere is ok
    if(0)
    {
    
    cv::Mat merge(mask.rows,mask.cols,CV_8UC1,cv::Scalar(0));
    
    for (int i=0;i<mask.rows;i++)
        for (int j=0;j<mask.cols;j++)
        {
            if (innerIndexMat.at<unsigned short>(i,j)>1|| bouMatrix.at<unsigned short>(i,j)>1)
            {
                merge.at<uchar>(i,j)=1;
            }
        }
    cv::Mat diff(mask.rows,mask.cols,CV_8UC1,cv::Scalar(0));
    
    for (int i=0;i<mask.rows;i++)
        for (int j=0;j<mask.cols;j++)
        {
            if (mask.at<uchar>(i,j)!=merge.at<uchar>(i,j))
            {
                diff.at<uchar>(i,j)=255;
                std:: cout<<"wrong";
            }
        }
    
    cv::imshow("diff",diff);
    cvWaitKey(500);
//    std::cout<<"OK";
    }

}




