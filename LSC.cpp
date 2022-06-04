//
//  LSC.cpp
//  pictureCodec
//
//  Created by jameswoods on 11/9/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#include <stdio.h>

#include"Initialize.h"
#include"Seeds.h"
#include"DoSuperpixel.h"
#include"point.h"
#include"myrgb2lab.h"
#include "countSuperpixel.h"

#include <opencv2/opencv.hpp>
#include "LSC.h"
#include "AreaMerge.hpp"
using namespace std;


cv::Mat segMat( cv::Mat src,int superpixelnum,double ratio)
{
    int rows=src.rows;
    int cols=src.cols;
    
    cv::Mat ret(rows,cols,CV_16UC1);
    
    unsigned  char *R=new unsigned char[rows*cols];
    unsigned  char *G=new unsigned char[rows*cols];
    unsigned  char *B=new unsigned char [rows*cols];
    
    uint16_t * label= new uint16_t [rows*cols];
    //    cv::cvtColor(src, src, CV_BGR2Lab);
    
    for (int i=0;i<rows;i++)
        for (int j=0;j<cols;j++)
        {
            B[i*cols+j]=src.at<uchar>(i,3*j);
            G[i*cols+j]=src.at<uchar>(i,3*j+1);
            R[i*cols+j]=src.at<uchar>(i,3*j+2);
        }
    
    
    LSC(R, G, B, rows, cols, superpixelnum, ratio, label);
    
    
    for (int i=0;i<rows;i++)
        for (int j=0;j<cols;j++)
        {
            ret.at<unsigned short> (i,j)=label[i*cols+j];
        }
    
    if (1)
    {
    areaMerge area;
    cv:: Mat gray;
    cv::cvtColor(src, gray, CV_BGR2GRAY);
    area.SegObj(ret, src);
    ret= area.getNeighbor(ret,gray );
    }
    return ret;
}



//LSC superpixel segmentation algorithm

void LSC(unsigned char* R,unsigned char* G,unsigned char* B,int nRows,int nCols,int superpixelnum,double ratio,unsigned short* label)
{
    //Setting Parameter
    float colorCoefficient=20;
    float distCoefficient=colorCoefficient*ratio;
    int seedNum=superpixelnum;
    int iterationNum=20;
    int thresholdCoef=5;
    
    unsigned char *L, *a, *b;
    L=new unsigned char[nRows*nCols];
    a=new unsigned char[nRows*nCols];
    b=new unsigned char[nRows*nCols];
    
    myrgb2lab(R,G,B,L,a,b,nRows,nCols);
    
    
    //Produce Seeds
    int ColNum,RowNum,StepY,StepX;
    ColNum=sqrt(float(seedNum*nCols/nRows));
    RowNum=seedNum/ColNum;
    StepX=nRows/RowNum;
    StepY=nCols/ColNum;
    point *seedArray=new point[seedNum];
    int newSeedNum=Seeds(nRows,nCols,RowNum,ColNum,StepX,StepY,seedNum,seedArray);
    
    
    //Initialization
    float **L1,**L2,**a1,**a2,**b1,**b2,**x1,**x2,**y1,**y2;
    double **W;
    L1=new float*[nRows];
    L2=new float*[nRows];
    a1=new float*[nRows];
    a2=new float*[nRows];
    b1=new float*[nRows];
    b2=new float*[nRows];
    x1=new float*[nRows];
    x2=new float*[nRows];
    y1=new float*[nRows];
    y2=new float*[nRows];
    W=new double*[nRows];
    for(int i=0;i<nRows;i++)
    {
        L1[i]=new float[nCols];
        L2[i]=new float[nCols];
        a1[i]=new float[nCols];
        a2[i]=new float[nCols];
        b1[i]=new float[nCols];
        b2[i]=new float[nCols];
        x1[i]=new float[nCols];
        x2[i]=new float[nCols];
        y1[i]=new float[nCols];
        y2[i]=new float[nCols];
        W[i]=new double[nCols];
    }
    Initialize(L,a,b,L1,L2,a1,a2,b1,b2,x1,x2,y1,y2,W,nRows,nCols,StepX,StepY,colorCoefficient,distCoefficient);
    delete [] L;
    delete [] a;
    delete [] b;
    
    
    //Produce Superpixel
    DoSuperpixel(L1,L2,a1,a2,b1,b2,x1,x2,y1,y2,W,label,seedArray,newSeedNum,nRows,nCols,StepX,StepY,iterationNum,thresholdCoef);
    delete []seedArray;
    
    int NUMBER=countSuperpixel(label,nRows,nCols);
    
//    cout<<"nummerb of  block="<<NUMBER<<" and the block pixel is "<<(nRows*nCols)/NUMBER<< endl;
    
    //Clear Memory
    for(int i=0;i<nRows;i++)
    {
        delete [] L1[i];
        delete [] L2[i];
        delete [] a1[i];
        delete [] a2[i];
        delete [] b1[i];
        delete [] b2[i];
        delete [] x1[i];
        delete [] x2[i];
        delete [] y1[i];
        delete [] y2[i];
        delete [] W[i];
        
    }
    delete []L1;
    delete []L2;
    delete []a1;
    delete []a2;
    delete []b1;
    delete []b2;
    delete []x1;
    delete []x2;
    delete []y1;
    delete []y2;
    delete []W;
}

