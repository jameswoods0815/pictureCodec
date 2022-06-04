//
//  common.cpp
//  pictureCodec
//
//  Created by jameswoods on 10/15/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#include "common.hpp"
using namespace cv;
using namespace std;

void getMap(cv::Mat label, std::vector<std::vector<int>> &location,std::vector<cv::Mat> &object )
{
    
    double minIdx, maxIdx;
    minMaxIdx(label, &minIdx, &maxIdx);
    
    vector<int> temp;
    temp.push_back(label.rows+1);
    temp.push_back(label.cols+1);
    temp.push_back(-1);
    temp.push_back(-1);
    
    for(int i=0;i<maxIdx;i++)
    {
        location.push_back(temp);
    }
    
    //get the size
    
    for(int i=0;i<label.rows;i++)
        for(int j=0;j<label.cols;j++)
        {
            int L=label.at<unsigned short>(i,j)-1;
            
            if (i<location[L][0])
            {
                location[L][0]=i;
            }
            
            if(j<location[L][1])
            {
                location[L][1]=j;
            }
            
            if(i>location[L][2])
            {
                location[L][2]=i;
            }
            
            if(j>location[L][3])
            {
                location[L][3]=j;
            }
            
        }
    
    //get the Mat
    for (int i=0;i<location.size();i++)
    {
        int height=location[i][2]-location[i][0]+1;
        int width=location[i][3]-location[i][1]+1;
        
        int y=location[i][0];
        int x=location[i][1];
        
        Mat temp=Mat(height,width,CV_8UC1,Scalar(0));
        for (int m=y;m<y+height;m++)
            for (int n=x;n<x+width;n++)
            {
                if((label.at<unsigned short>(m,n)-1)==i)
                {
                    temp.at<uchar>(m-y,n-x)=1;
                }
            }
        object.push_back(temp);
        
//        imshow("Mat", object[i]*255);
//        cvWaitKey(1);
        
        
    }
    
    
    
}

#define file 1
void getScalar(std::vector<cv::Mat> object, std::vector<bool> &data)
{
    for (int i=0;i<object.size();i++)
    {
        for (int m=0;m<object[i].rows;m++)
            for (int n=0;n<object[i].cols;n++)
            {
                data.push_back(object[i].at<uchar>(m,n));
            }
    }
    
    if(file)
    {
        std::vector<uchar> DAT;
        for (int i=0;i<data.size()/8+1;i++)
        {
            uchar temp=0;
            for (int j=0;j<8;j++)
            {
                temp=temp|data[i*8+j];
                temp<<=1;
            }
            DAT.push_back(temp);
        }
        
        FILE * f;
        f= fopen("/Users/jameswoods/Documents/MATLAB/1.dat", "w");
        for (int i=0;i<DAT.size();i++)
        {
            fwrite(&DAT[i], 1, 1, f);
        }
        fclose(f);
        
        
        
    }
    
}

void ReSampleDown(Mat data,Mat &result, Mat &recoverMat, Mat & error)
{
    result=Mat (data.rows/2,data.cols/2, CV_8UC1);
    
    recoverMat=Mat(data.rows,data.cols,CV_8UC1);
    
    error=Mat(data.rows,data.cols,CV_8UC1);
    
    for (int i=0;i<data.rows/2;i++)
        for (int j=0;j<data.cols/2;j++)
        {
            result.at<uchar>(i,j)=data.at<uchar>(i*2,j*2);
        }
    
    
    for (int i=0;i<data.rows;i++)
        for (int j=0;j<data.cols;j++)
        {
            recoverMat.at<uchar>(i,j)=result.at<uchar>(i/2,j/2);
        }
    
  
    
    
    Mat edgeDown=Mat(result.rows,result.cols,CV_8UC1);
    int x1,x2,x3,x4;
    
    for (int i=0;i<edgeDown.rows;i++)
        for (int j=0;j<edgeDown.cols;j++)
        {
            
            if(i>1)
            {
                x1=result.at<uchar>(i,j)-result.at<uchar>(i-1,j);
            }
            else
            {
                x1=0;
            }
            
            if(i+1<edgeDown.rows)
            {
                x2=result.at<uchar>(i,j)-result.at<uchar>(i+1,j);
            }
            else
            {
                x2=0;
            }

            if (j>1)
            {
                x3=result.at<uchar>(i,j)-result.at<uchar>(i,j-1);
            }
            else
            {
                x3=0;
            }

            if(j+1<edgeDown.cols)
            {
                x4=result.at<uchar>(i,j)-result.at<uchar>(i,j+1);
            }
            else
            {
                x4=0;
            }
            
            if(x1||x2||x3||x4)
            {
                edgeDown.at<uchar>(i,j)=1;
            }
            else {
                edgeDown.at<uchar>(i,j)=0;

            }
            
            if(j==0||i==0||i==edgeDown.rows-1||j==edgeDown.cols-1)
            {
                if (result.at<uchar>(i,j))
                {
                    edgeDown.at<uchar>(i,j)=1;
                }
            }
            
            

        }
    
    
    //big edge probility;
    
    int count1=0;
    
    Mat mapEdge(data.rows,data.cols,CV_8UC1,Scalar(0));
    for (int i=0;i<data.rows/2;i++)
        for(int j=0;j<data.cols/2;j++)
        {
            if(edgeDown.at<uchar>(i,j)!=0)
            {
                if(2*i-1>=0)
                {
                    mapEdge.at<uchar>(2*i-1,2*j)=1;
                    count1++;
                    
                    if(2*j-1>0)
                    {
                        mapEdge.at<uchar>(2*i-1,2*j-1)=1;
                        count1++;
   
                    }
                    
                    if(2*j+1<data.rows)
                    {
                        mapEdge.at<uchar>(2*i-1,2*j+1)=1;
                        count1++;
                        
                    }

                }
                
                if(2*i+1<data.rows)
                {
                    mapEdge.at<uchar>(2*i+1,2*j)=1;
                    count1++;
                    
                    if(2*j-1>0)
                    {
                        mapEdge.at<uchar>(2*i+1,2*j-1)=1;
                        count1++;
                        
                    }
                    
                    if(2*j+1<data.rows)
                    {
                        mapEdge.at<uchar>(2*i+1,2*j+1)=1;
                        count1++;
                        
                    }
                    

                }
                
                if(2*j-1>=0)
                {
                    mapEdge.at<uchar>(2*i,2*j-1)=1;
                    count1++;
                }
                
                if(2*j+1<edgeDown.cols)
                {
                    mapEdge.at<uchar>(2*i,2*j+1)=1;
                    count1++;

                }
                
                
            }
        }
    
    
    int count=0;
    
    for (int i=0;i<data.rows;i++)
        for (int j=0;j<data.cols;j++)
        {
            
            
            if(data.at<uchar>(i,j)!=recoverMat.at<uchar>(i,j))//&&mapEdge.at<uchar>(i,j)!=1)
            {
                error.at<uchar>(i,j)=1;
                count++;
            }
            else{
                error.at<uchar>(i,j)=0;
            }
        }
    
    
    std::vector<bool> errorEdge,errorElse;
    errorEdge.clear();
    errorElse.clear();
    
    
    
    
    
    for (int i=0; i<error.rows;i++)
        for (int j=0; j<error.cols;j++)
        {
            
            if (mapEdge.at<uchar>(i,j)) {
                errorEdge.push_back(error.at<uchar>(i,j));
            }
            else{
                errorElse.push_back(error.at<uchar>(i,j));
            }
            
            
        }
    
    
    float sum1=0;
    float sum2=0;
    for (int i=0;i<errorEdge.size();i++)
    {
        sum1+=errorEdge[i];
    }
    
    sum1=sum1*1.0/errorEdge.size();
    
    for (int i=0;i<errorElse.size();i++)
    {
        sum2+=errorElse[i];
    }
    sum2=sum2*1.0/errorElse.size();
    
    

    
    
    
//    imshow("edge", edgeDown*255);
//    
//    
//    imshow("src", data*255);
//    
//    imshow("result", result*255);
//    
//    imshow("reMat", recoverMat*255);
//    
//    imshow("errMat", error*255);
//    
//    imshow("edge1", mapEdge*255);
//    waitKey(1);
//    cout<<"count"<<count*1.0/data.rows/data.cols/0.75<<"counttt"<< count1*1.0/data.rows/data.cols/0.75  << "sum1 "<<sum1<<"  sum2  "<<sum2<<    endl;
//    
}







