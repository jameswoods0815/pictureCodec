//
//  MergeObject.cpp
//  pictureCodec
//
//  Created by jameswoods on 10/7/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#include "MergeObject.hpp"
using namespace cv;
using namespace std;
#include <vector>
#include <queue>
#include <iostream>


class SuperPix{
public:
    
    int label;
    vector<int> x;
    vector<int> y;
    vector<unsigned char> lum;
    vector<int > neighbor;
    feature f;
    double var;
    int status;
    int size;
    
};

//label unsinged short
//src uchar

void MergeObj::Merge(double var, cv::Mat label, cv::Mat src)
{
    vector<SuperPix> cannidate;
    Mat mask=Mat(label.rows,label.cols,CV_8UC1,Scalar(0));
    
    int rows= label.rows;
    int cols = label.cols;
    
    queue<unsigned short> xloc;
    queue<unsigned short> yloc;
    
    
    
    vector <int> Size;
    int L;
    
    for(int i=0;i<rows;i++)
        for (int j=0;j<cols;j++)
        {
            SuperPix sup;
            
            if(mask.at<uchar>(i,j)==0)
            {
                int count=1;
                mask.at<uchar>(i,j)=1;
                xloc.push(i);
                yloc.push(j);
                L=label.at<unsigned short>(i,j);
                sup.label=L;
              //  cout<<L<<endl;
                
                while (!xloc.empty()) {
                    
                    
                    int x=xloc.front();
                    xloc.pop();
                    int y=yloc.front();
                    yloc.pop();
                    mask.at<uchar>(x,y)=1;
                    
                    sup.x.push_back(x);
                    sup.y.push_back(y);
                    sup.lum.push_back(src.at<uchar>(x,3*y));
                    
                    
                    int minX=(x-1<=0)?0:x-1;
                    int maxX=(x+1>=rows-1)?rows-1:x+1;
                    int minY=(y-1<=0)?0:y-1;
                    int maxY=(y+1>=cols-1)?cols-1:y+1;
                    
                    for (int m=minX;m<=maxX;m++)
                        for(int n=minY;n<=maxY;n++)
                        {
                           if(mask.at<uchar>(m,n)==0&&label.at<unsigned short>(m,n)==L)
                           {
                               count++;
                               mask.at<uchar>(m,n)=1;
                               xloc.push(m);
                               yloc.push(n);
                           }
                           else if (label.at<unsigned short>(m,n)!=L){
                               int newlabel=label.at<unsigned short>(m,n);
                               vector<int>::iterator pointer=std::find(sup.neighbor.begin(), sup.neighbor.end(), newlabel);
                               if(pointer==sup.neighbor.end())
                               {
                                   sup.neighbor.push_back(newlabel);
                               }
                           }
                        }
                    
                    
                }
                Size.push_back(count);
                sup.size=count;
                
                getFeature(sup.lum, sup.f);
                cannidate.push_back(sup);
                
            }
        }
  
    vector<SuperPix> mergeCan;
    for (int i=0;i<cannidate.size();i++)
    {
        if(cannidate[i].f.var<var)
        {
            mergeCan.push_back(cannidate[i]);
        }
    }
    
    vector<SuperPix>::iterator pointer;
    pointer=mergeCan.begin();
    
    while( pointer!=mergeCan.end())
    {
        double varnow=1e100;
        
        int labelp=-1;
        feature ff;
        
        for (int i=0;i<(*pointer).neighbor.size();i++)
        {
            int neilabel=(*pointer).neighbor[i];
            feature f1=cannidate[neilabel-1].f;
            feature out;
            varmerge((*pointer).f, f1 , out);
            
            if(out.var<varnow)
            {
                labelp=neilabel;
                ff=out;
            }
            
        }
        
        if (ff.var<var)
        {
            //merge
            for (int m=0;m<(*pointer).x.size();m++)
            {
                int x= (*pointer).x[m];
                int y=(*pointer).y[m];
                label.at<unsigned short>(x,y)=labelp;
            }
            // refresh parameter
            
            
            
            
        }
        
        else {
            mergeCan.erase(pointer);
        }
        
        pointer=mergeCan.begin();
    }
    
    
    
    
    
}



bool MergeObj::tryCondition(std::vector<unsigned char> in1,  double var,double &varnow)
{
    vector<unsigned char> mergevec=in1;
    
    
    double sum=0;
    for (int i=0;i<mergevec.size();i++)
    {
        sum+=mergevec[i];
    }
    
    double mean=sum/mergevec.size();
    
    double square=0.0;
    for (int i=0;i<mergevec.size();i++)
    {
        square+=(mergevec[i]-mean)*(mergevec[i]-mean);
    }
    
    double varMerge= square/(mergevec.size()-1);
    varnow=varMerge;
    
    if(varMerge>var)
    {
        return false;
    }
    else{
        return true;
    }
}


bool MergeObj:: varmerge(feature f1, feature f2, feature &output)
{
    output.mean=(f1.num*f1.mean+f2.num*f2.mean)/(f1.num+f1.mean);
    output.num=f1.num+f2.num;
    double dt1=output.mean-f1.mean;
    double dt2=output.mean-f2.mean;
    output.var=((f1.num-1)*f1.var+(f2.num-1)*f2.var+f1.num*dt1*dt1+f2.num*dt2*dt2)/(f1.num+f2.num-1);
    
    return true;
}

void MergeObj:: getFeature(std::vector<unsigned char> in, feature &f)
{
    
    double sum=0;
    for (int i=0;i<in.size();i++)
    {
        sum+=in[i];
    }
    
    double mean=sum/in.size();
    
    double square=0.0;
    for (int i=0;i<in.size();i++)
    {
        square+=(in[i]-mean)*(in[i]-mean);
    }
    
    double varMerge= square/(in.size()-1);
    
    f.num=in.size();
    f.var=varMerge;
    f.mean=mean;
}


