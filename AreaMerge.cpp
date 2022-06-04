//
//  AreaMerge.cpp
//  pictureCodec
//
//  Created by jameswoods on 11/6/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#include "AreaMerge.hpp"
#include "queue"


using namespace std;
using namespace cv;



typedef struct feature {
    double var;
    double mean;
    double num;
} feature;


bool  varmerge(feature f1, feature f2, feature &output)
{
    output.mean=(f1.num*f1.mean+f2.num*f2.mean)/(f1.num+f1.mean);
    output.num=f1.num+f2.num;
    double dt1=output.mean-f1.mean;
    double dt2=output.mean-f2.mean;
    output.var=((f1.num-1)*f1.var+(f2.num-1)*f2.var+f1.num*dt1*dt1+f2.num*dt2*dt2)/(f1.num+f2.num-1);
    
    return true;
}



Mat areaMerge:: getNeighbor(cv::Mat label, Mat src)
{
    double minIdx, maxIdx;
    minMaxIdx(label, &minIdx, &maxIdx);
    int  numOfObj=maxIdx;
    
    int rows=label.rows;
    int cols=label.cols;
    
    std::vector<std::vector<int>> neighbor;
    
    std::vector<std::vector<std::vector<cv::Point2i>>> location;

    
    for (int i=0;i<numOfObj;i++)
    {
        vector<int> temp;
        temp.push_back(-1);
        neighbor.push_back(temp);
        
    }
    
    
    for (int i=1;i<rows-1;i++)
        for (int j=1;j<cols-1;j++)
        {
            
            int nowLabel=label.at<unsigned short>(i,j);
            
            for(int p=i-1;p<i+2;p++)
                for(int q=j-1;q<j+2;q++)
                {
                    if(label.at<unsigned short>(p,q)!=nowLabel)
                    {
                        vector<int> ::iterator ptr;
                        ptr=std::find(neighbor[nowLabel-1].begin(), neighbor[nowLabel-1].end(), label.at<unsigned short>(p,q));
                        
                        if (ptr==neighbor[nowLabel-1].end()) {
                            neighbor[nowLabel-1].push_back(label.at<unsigned short>(p,q));
                            
                        }
                        
                    }
                    
                }
        }
    
    //get neighhor location  init
    
    for (int i=0; i<numOfObj;i++)
    {
        vector<vector<cv::Point2i>> temp2;

        for (int j=1;j<neighbor[i].size();j++)
        {
            Point2i temp(0,0);
            vector<cv::Point2i> temp1;
            temp1.push_back(temp);
            temp2.push_back(temp1);
           
        }
        
      location.push_back(temp2);
    }
    
    // push all the data
    for (int i=1;i<rows-1;i++)
        for (int j=1;j<cols-1;j++)
        {
            
            int nowLabel=label.at<unsigned short>(i,j);
            
            bool flag=false;
            int L;
            
            for(int p=i-1;p<i+2;p++)
                for(int q=j-1;q<j+2;q++)
                {
                    if(label.at<unsigned short>(p,q)!=nowLabel) {
                    
                        L=label.at<unsigned short>(p,q);
                        flag= true;
                        break;
                    }
                }
             if (flag)
             {
                 int ptr;
                 for (ptr=1;ptr<neighbor[nowLabel-1].size();ptr++)
                 {
                     if (L==neighbor[nowLabel-1][ptr])
                     {
                         break;
                     }
                 }
                 location[nowLabel-1][ptr-1].push_back(Point2i(i,j));
                 
             }
            
            
        }


    
    //test ok
//    Mat label1(rows,cols,CV_8UC1,Scalar(0));
//    for (int i=0;i<location.size();i++)
//        for (int j=0;j<location[i].size();j++)
//            for (int p=1;p<location[i][j].size();p++)
//            {
//                label1.at<uchar>(location[i][j][p].x,location[i][j][p].y)=255;
//            }
//    imshow("noweee", label1);
//    cvWaitKey();
    
//check boundary
    //  2 condition : check bou>th1, check ave<th2
    
    
    int th1=1;
    float th2=30; // may be merge
    float th3=0.4;  // merge right now
    
//    float th4=40;
    // abovious no edge
    
    std::vector<std::vector<int>> neighborMerge=neighbor; // if =-2 merge
    
    for (int i=0;i<location.size();i++)
                for (int j=0;j<location[i].size();j++)
                {
                    if(location[i][j].size()>th1)
                    {
                        float edge=0;
                        
                        for (int p=1;p<location[i][j].size();p++)
                        {
                            int x=location[i][j][p].x;
                            int y=location[i][j][p].y;
                            
                            for (int m=x-1;m<x+2;m++)
                                for (int n=y-1;n<y+2;n++)
                                {
                                    edge+=abs(src.at<uchar>(m,n)-1.0*src.at<uchar>(x,y));
                                }
                            
                        }
                           edge=edge/8/(location[i][j].size()-1);
                        
                        if(edge<th2)
                        {
                            neighborMerge[i][j+1]=-3;
                        }
                        if(edge<th3)
                        {
                           neighborMerge[i][j+1]=-2;
                        }
    
                    
                }
              }

//   cout <<"OK";
    
    
    //second merge
    for (int i=0;i<neighborMerge.size();i++)
    {
        for(int j=1;j<neighborMerge[i].size();j++)
        {
            if (neighborMerge[i][j]==-3)
            {
                int index1=i;
                int index2=neighbor[i][j]-1;
                
                std::vector<float> f1,f2;
                f1=myf[index1];
                f2=myf[index2];
                
                //  first megre can not merge
                if(fabs(f1[1]-f2[1])>30||fabs(f1[2]-f2[2]>30||fabs(f1[3]-f2[3])>30))
                {
                    neighborMerge[i][j]=neighbor[i][j];
                }
                
                
                if (!judgeFeature(index1, index2))
                {
                    neighborMerge[i][j]=neighbor[i][j];

                }
                else {
                    neighborMerge[i][j]=-2;
                }
                
                
                //second merge with varience
                
                
                
                
            }
        }
    }
    
    
    //second level :
    
    

 //test all right now
    
//    for ( int set=179;1;set)
//
//    
////    for ( int set=1;set<numOfObj;set++)
//    {
//        Mat meResult=label.clone();
//        Mat temppp(rows,cols,CV_8UC1,Scalar(0));
//
//    
//    for (int i=0;i<rows;i++)
//        for (int j=0;j<cols; j++)
//        {
//            for (int p=1;p<neighborMerge[set].size();p++)
//            {
//
//                 if(label.at<unsigned short>(i,j)==set)
//                {
//                    temppp.at<uchar>(i,j)=255;
//                }
//                
//                if ((label.at<unsigned short>(i,j)==neighbor[set-1][p]&&neighborMerge[set-1][p]==-2))
//                {
//                    temppp.at<uchar>(i,j)=128;
//
//                }
//                
//                
//            }
//        }
//    
//    
//    imshow("me", temppp);
//    cvWaitKey(100);
//    }
    
    
    vector<int> mergeList;
    vector<bool> status;
    for(int i=0;i<numOfObj;i++)
    {
        mergeList.push_back(i+1);
        status.push_back(false);
    }
    
//    for (int i=0;i<neighborMerge.size();i++)
//        for(int j=1;j<neighborMerge[i].size();j++)
//        {
//            if(neighborMerge[i][j]==-2)
//            {
//                mergeList[neighbor[i][j]-1]=mergeList[i];
//            }
//        }
    
    
    for (int i=0;i<neighborMerge.size();i++)
    {
        neighborMerge[i][0]=i;
    }
    
    
    
    for (int i=0;i<neighborMerge.size();i++)
    {
        if (!status[i])
        {
            queue<vector<int>> index;
            index.push(neighborMerge[i]);
            
            queue<vector<int>> indexNei;
            indexNei.push(neighbor[i]);
            
            
            status[i]=true;
            while (!index.empty())
            {
                for(int x=1;x<index.front().size();x++)
                {
                    if (index.front()[x]==-2&&status[indexNei.front()[x]-1]==false)
                    {
                        mergeList[indexNei.front()[x]-1]=mergeList[i];
                        index.push(neighborMerge[indexNei.front()[x]-1]);
                        indexNei.push(neighbor[indexNei.front()[x]-1]);
                    }
                    
                }
                status[index.front()[0]]=true;
                index.pop();
                indexNei.pop();
                
            
            }
        }
        
    }
    
//    cout<<"OK";
    
    vector<int> newLabel;
    vector<int> oldLabel;
    int count=0;
    for (int i=0;i<mergeList.size();i++)
    {
        vector<int>::iterator ptr;
        
        ptr=std::find(oldLabel.begin(), oldLabel.end(), mergeList[i]);
        
        if(ptr==oldLabel.end())
        {
            count++;
            oldLabel.push_back(mergeList[i]);
            newLabel.push_back(count);
        }
        
        
    }
   
    
    
    
    
    vector<int> newList;
    for (int i=0;i<mergeList.size();i++)
    {
        for (int j=0;j<oldLabel.size();j++)
        {
            if (mergeList[i]==oldLabel[j])
            {
                newList.push_back(j+1);
                break;
            }
        }

        
    }
//    cout<<"OK";
//
//    
    Mat meResult=label.clone();
    Mat temppp(rows,cols,CV_8UC1,Scalar(0));
    
    for (int i=0;i<rows;i++)
        for (int j=0;j<cols; j++)
        {
            
//            if(newList[label.at<unsigned short>(i,j)-1]==11)
//            {
                meResult.at<unsigned short >(i,j)=newList[label.at<unsigned short>(i,j)-1];
                temppp.at<uchar>(i,j)=meResult.at<unsigned short >(i,j);
//            }
        }
    
    
//    imshow("me", temppp*5);
    
    return meResult;
//    cvWaitKey();
//    cout<<"OK";

    
}


void areaMerge:: SegObj(cv::Mat label, cv::Mat src)
{
    double minIdx, maxIdx;
    minMaxIdx(label, &minIdx, &maxIdx);
    
   int  numOfObj=maxIdx;
    
    std::vector<cv::Mat> object;
    std::vector<std::vector<int>>  location;


    
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
        vector<vector<int>> pixel;
        
        for (int m=y;m<y+height;m++)
            for (int n=x;n<x+width;n++)
            {
                if((label.at<unsigned short>(m,n)-1)==i)
                {
                    vector<int> tt;
                    tt.push_back(src.at<uchar>(m,3*n));
                    tt.push_back(src.at<uchar>(m,3*n+1));
                    tt.push_back(src.at<uchar>(m,3*n+2));
                    pixel.push_back(tt);
                }
            }
        float r=0,g=0,b=0,vr=0,vg=0,vb=0;
        
        for (int xx=0;xx<pixel.size();xx++)
        {
            r+=pixel[xx][0];
            g+=pixel[xx][1];
            b+=pixel[xx][2];
        }
        
        r=r/pixel.size();
        g=g/pixel.size();
        b=b/pixel.size();
        
        for (int xx=0;xx<pixel.size();xx++)
        {
            vr+=(pixel[xx][0]-r)*(pixel[xx][0]-r);
            vg+=(pixel[xx][1]-g)*(pixel[xx][1]-g);
            vb+=(pixel[xx][2]-b)*(pixel[xx][2]-b);
        }
        
        vr=vr/(pixel.size()-1);
        vg=vg/(pixel.size()-1);
        vb=vb/(pixel.size()-1);
        
        
        vector<float> data;
        data.push_back(pixel.size());
        data.push_back(r);
        data.push_back(g);
        data.push_back(b);
        data.push_back(vr);
        data.push_back(vg);
        data.push_back(vb);
        myf.push_back(data);
        
    }
}



bool  areaMerge:: judgeFeature(int f1,int f2)
{
    std::vector<float> x1,x2;
    x1=myf[f1];
    x2=myf[f2];
    
    float score1=0,score=0,var1=0,var2=0;
    
    for (int i=1;i<7;i++)
    {
        score1+=x1[i]*x2[i];
        var1+=x1[i]*x1[i];
        var2+=x2[i]*x2[i];
    }
    
    score=score1/(sqrt(var1)*sqrt(var2));
    
    float sum=fabs(x1[1]-x2[1])+fabs(x1[2]-x2[2])+fabs(x1[3]-x2[3]);
    
    // if similar, merge
    if (score>0.97&&sum<90)
    {
        return true;
    }
   
    //varience is small , than merge and score is little small
    feature inr1={x1[4],x1[1],x1[0]};
    feature inr2={x2[4],x2[1],x2[0]};
    feature ing1={x1[5],x1[2],x1[0]};
    feature ing2={x2[5],x2[2],x2[0]};
    feature inb1={x1[6],x2[3],x1[0]};
    feature inb2={x2[6],x2[3],x2[0]};
    feature out1,out2,out3;
    varmerge(inr1, inr2, out1);
    varmerge(ing1, ing2, out2);
    varmerge(inb1, inb2, out3);
    
    if(out1.var<80&&out2.var<80&&out3.var<80&& score>0.96&&sum<60)
    {
        return true;
    }
    
    
//    if(x1[4]<30&&x2[4]<30&&x1[5]<30&&x2[5]<30&& score>0.94)
//    {
//        return true;
//    }
    
    return false;
    
}










