//
//  ViewPic.cpp
//  pictureCodec
//
//  Created by jameswoods on 11/7/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#include "ViewPic.hpp"
using namespace std;
using namespace cv;
#include<queue>

cv::Mat viewImage(cv::Mat src)
{
    Mat temp(src.rows,src.cols,CV_8UC3,Scalar(0,0,0));
    Mat mask(src.rows,src.cols,CV_8UC1, Scalar(0));
    
    int rows=src.rows;
    int cols=src.cols;
    
    queue <unsigned short> xloc;
    queue<unsigned short> yloc;
    
    int L;
    
    cv::RNG rng=cv::theRNG();

    for (int i=0;i<rows;i++)
        for (int j=0;j<cols;j++)
        {
            int r=rng(256);
            int g=rng(256);
            int b=rng(256);
            if (mask.at<uchar>(i,j)==0)
            {
                mask.at<uchar>(i,j)=1;
                xloc.push(i);
                yloc.push(j);
                L=src.at<unsigned short>(i,j);
                
                while (!xloc.empty()) {
                    int x=xloc.front();
                    xloc.pop();
                    int y=yloc.front();
                    yloc.pop();
                    mask.at<uchar>(x,y)=1;
                    temp.at<uchar>(x,3*y)=r;
                    temp.at<uchar>(x,3*y+1)=g;
                    temp.at<uchar>(x,3*y+2)=b;
                    
                    int minX=(x-1<=0)?0:x-1;
                    int maxX=(x+1>=rows-1)?rows-1:x+1;
                    int minY=(y-1<=0)?0:y-1;
                    int maxY=(y+1>=cols-1)?cols-1:y+1;
                    
                    for (int m=minX;m<=maxX;m++)
                        for(int n=minY;n<=maxY;n++)
                        {
                            if(mask.at<uchar>(m,n)==0&&src.at<unsigned short>(m,n)==L)
                            {
                                mask.at<uchar>(m,n)=1;
                                xloc.push(m);
                                yloc.push(n);
                                temp.at<uchar>(x,3*y)=r;
                                temp.at<uchar>(x,3*y+1)=g;
                                temp.at<uchar>(x,3*y+2)=b;
                            }
                        }
                    
                }
                
                
            }
        }
    
    
    imshow("windowRGB", temp);
    cvWaitKey(1);
    return temp;
    
}
