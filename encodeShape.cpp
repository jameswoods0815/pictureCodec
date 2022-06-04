//
//  encodeShape.cpp
//  pictureCodec
//
//  Created by jameswoods on 10/21/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

// all mat use uchar , if object, 1; else 0


#include "encodeShape.hpp"
#include "Arithmetic.hpp"

using namespace std;
using namespace cv;


//get the Obj of label

void EncodeShape:: SegObj(cv::Mat label)
{
    double minIdx, maxIdx;
    minMaxIdx(label, &minIdx, &maxIdx);
    
    numOfObj=maxIdx;
    
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
        
        if (codeStateOfObject[i]==2)
        {
           object.push_back(temp);
        }
    
    }
}


// get the pyr level

void EncodeShape::getLevelOfObj(void)
{
    for (int i=0;i<object.size();i++)
    {
        int rows=object[i].rows;
        int cols=object[i].cols;
        
        int ratio=min(rows,cols);
        levelOfObj.push_back(log((ratio/minObjSize))/log(2));
        
    }
    
    
}


// there is a bug, when rows =0; or cols=0;
// so just stop when rows<4;
void EncodeShape:: generatePyr(Mat Obj, vector< Mat> & pyr, int level)
{
    pyr.push_back(Obj);
    Mat temp=Obj;
    for (int i=0;i<level;i++)
    {
        Mat temp1(temp.rows/2,temp.cols/2,CV_8UC1);
        for (int p=0;p<temp.rows/2;p++)
            for (int q=0;q<temp.cols/2;q++)
            {
                temp1.at<uchar>(p,q)=temp.at<uchar>(p*2,q*2);
            }
        pyr.push_back(temp1);
        temp=temp1;
    }
}

void EncodeShape:: encodeObj(cv::Mat Obj, std::vector<bool> & error, std::vector<bool> & detail )
{
  
    
}

void EncodeShape:: encodeAsubObj(cv ::Mat obj, std::vector<bool> &error, std::vector <bool> & detail)
{
    int rows=obj.rows;
    int cols=obj.cols;
    
    Mat downObj(rows/2,cols/2,CV_8UC1);
    Mat downEdge(rows/2,cols/2,CV_8UC1);
    Mat upObj(rows,cols,CV_8UC1);
    Mat upEdge(rows,cols,CV_8UC1);
    
    getDownMat(obj, downObj);
    
//    imshow ("obj",obj*255);
//    
//    imshow("downObj", downObj*255);
//    
//    waitKey(1);

    
    
//    getNowEdge(downObj, downEdge);
//    getUpEdge(downEdge, upEdge);
    getUpMat(downObj, upObj);
    getNowEdge(upObj, upEdge);
    
    
    
    //edge is error,
    // else is detail;
    
    for (int i=0;i<rows;i++)
        for (int j=0;j<cols; j++)
        {
            if(i%2!=0||j%2!=0)
            {
                if (upEdge.at<uchar>(i,j))
                {
                    error.push_back(obj.at<uchar>(i,j)-upObj.at<uchar>(i,j));
                }
                else
                {
                    detail.push_back(obj.at<uchar>(i,j)-upObj.at<uchar>(i,j));
                }
            }
            
        }
    
    if (0)
    {
        vector<bool> e1,e2;
        Mat verror(upEdge.rows,upEdge.cols,CV_8UC1, Scalar(0));
        Mat vdetail=verror.clone();
        Mat allError=verror.clone();
        
        for (int i=0;i<rows;i++)
            for (int j=0;j<cols;j++)
            {
                if (i%2!=0&&j%2!=0)
                {
                    if (upEdge.at<uchar>(i,j))
                    {
                        e1.push_back(obj.at<uchar>(i,j)-upObj.at<uchar>(i,j));
                        verror.at<uchar>(i,j)=bool((obj.at<uchar>(i,j)-upObj.at<uchar>(i,j)));
                        
                    }
                    else
                    {
                        e2.push_back(obj.at<uchar>(i,j)-upObj.at<uchar>(i,j));
                        vdetail.at<uchar>(i,j)=bool((obj.at<uchar>(i,j)-upObj.at<uchar>(i,j)));

                    }
                }
                
                allError.at<uchar>(i,j)=bool(obj.at<uchar>(i,j)-upObj.at<uchar>(i,j));
                
            }
        
        
        float sum1=0;
        float sum2=0;
        
        for (int i=0;i<e1.size();i++)
        {
            sum1+=e1[i];
        }
        sum1/=e1.size();
        
        
        for (int i=0;i<e2.size();i++)
        {
            sum2+=e2[i];
        }
        sum2/=e2.size();
        
        imshow("verror", verror*255);
        imshow("vdetail", vdetail*255);
        imshow ("obj",obj*255);
        
        imshow ("all error", allError*255);
        
        imshow("downObj", downObj*255);
        
        
        Mat DatMix(rows,cols,CV_8UC1,Scalar(0));
        
        for (int i=0;i<rows;i++)
            for (int j=0; j<cols;j++)
            {
                DatMix.at<uchar>(i,j)=128*upEdge.at<uchar>(i,j)+127*verror.at<uchar>(i,j);
            }
        
        imshow("DatMix", DatMix);
        
        
        if (e2.size()>0)
        {
        
//          if( e2.size()>0&& sum2>0)
            {
                cout<<"e1 size=" <<e1.size()<<",  e2 size= "<<e2.size()<<",  ratio1= "<<sum1<<", ratio2 = "<<sum2<<   endl;
            }
        }
        
        waitKey(1);
        
        
    }
    
    
}


// get edge of the obj
void EncodeShape:: getNowEdge(cv:: Mat Obj, cv:: Mat & edge)
{
    edge=Mat(Obj.rows,Obj.cols, CV_8UC1, Scalar(0));
    
    Mat temp=Mat(Obj.rows+2,Obj.cols+2,CV_8UC1, Scalar(0));
    
    for (int i=1;i<temp.rows-1;i++)
        for (int j=1;j<temp.cols-1;j++)
        {
            temp.at<uchar>(i,j)=Obj.at<uchar>(i-1,j-1);
        }
    
    
    // in
    
    for(int i=0;i<Obj.rows;i++)
        for (int j=0;j<Obj.cols;j++)
        {
            
            //method 1
            for (int p=i-1;p<=i+1;p++)
            {
                for (int q=j-1;q<=j+1;q++)
                {
                    if (temp.at<uchar>(p+1,q+1)-temp.at<uchar>(i+1,j+1)&&i!=p&&j!=q)
                    {
                        edge.at<uchar>(i,j)=1;
                        break;
                    }
                }
            }
            
            //method 2
//             int x1=temp.at<uchar>(i+1,j)-temp.at<uchar>(i+1,j+1);
//             int x2=temp.at<uchar>(i+1,j+2)-temp.at<uchar>(i+1,j+1);
//             int x3=temp.at<uchar>(i,j+1)-temp.at<uchar>(i+1,j+1);
//             int x4=temp.at<uchar>(i+2,j)-temp.at<uchar>(i+1,j+1);
//            
//            if(x1||x2||x3||x4)
//            {
//                edge.at<uchar>(i,j)=1;
//            }
            
            
            
        }
    
    //boundry
    if (0)
    {
        imshow("edge1", edge*255);
        cvWaitKey(1);
    }
    
}

//upedge ,get the size of upEdge;

void EncodeShape:: getUpEdge (cv:: Mat edge, cv::Mat & upEdge)
{
    int rows=upEdge.rows;
    int cols=upEdge.cols;
    
    for (int i=0;i<rows;i++)
        for (int j=0;j<cols;j++)
        {
            upEdge.at<uchar>(i,j)=edge.at<uchar>(i/2,j/2);
        }
    
    
    Mat temp=upEdge.clone();
    
    for (int i=1;i<rows-1;i++)
        for (int j=1;j<cols-1;j++)
        {
            if(temp.at<uchar>(i,j))
            {
                for (int p=i-1;p<i+2;p++)
                    for (int q=j-1;q<j+2;q++)
                    {
                        upEdge.at<uchar>(p,q)=1;
                    }
            }
        }
    
//    imshow ("upedge",upEdge*255);
//    waitKey(1);

}


void EncodeShape:: getDownMat(cv::Mat Obj, cv::Mat  & downObj)
{
    int rows=Obj.rows;
    int cols=Obj.cols;
    
    for(int i=0; i< rows/2; i++)
        for (int j=0; j<cols/2; j++)
        {
//            downObj.at<uchar>(i,j)=(Obj.at<uchar>(i*2,j*2)||Obj.at<uchar>(i*2+1,j*2)||Obj.at<uchar>(i*2,j*2+1)||Obj.at<uchar>(i*2+1,j*2+1));
//             downObj.at<uchar>(i,j)=(Obj.at<uchar>(i*2,j*2)&&Obj.at<uchar>(i*2+1,j*2)&&Obj.at<uchar>(i*2,j*2+1)&&Obj.at<uchar>(i*2+1,j*2+1));
            
//            downObj.at<uchar>(i,j)=(Obj.at<uchar>(i*2,j*2)+Obj.at<uchar>(i*2+1,j*2)+Obj.at<uchar>(i*2,j*2+1)+Obj.at<uchar>(i*2+1,j*2+1))>1?1:0;
            downObj.at<uchar>(i,j)=(Obj.at<uchar>(i*2,j*2));
        }
    
}


void EncodeShape:: getUpMat(cv::Mat downObj, cv::Mat  & upObj )
{
    int rows=upObj.rows;
    int cols=upObj.cols;
    
    for (int i=0; i<rows; i++)
        for (int j=0; j<cols; j++)
        {
            upObj.at<uchar>(i,j)=downObj.at<uchar>(i/2,j/2);
        }
}



int EncodeShape:: encodeShape(cv::Mat shape, std::vector<uchar> & out)
{
    
    getNeighbor(shape);
    getCodeState();
    
    seeDelete(shape);
    
    SegObj(shape);
    getLevelOfObj();
    
   
    
    int objNum=object.size();
    
    for (int i=0;i<objNum;i++)
    {
        //get pyr;
        vector<cv::Mat> pyr;
        generatePyr(object[i], pyr, levelOfObj[i]);
        int j;
        for (j=0;j<levelOfObj[i]-1;j++)
        {
            encodeAsubObj(pyr[j], allError, allDetail);
        }
        
        for (int p=0;p<pyr[j].rows;p++)
            for (int q=0;q<pyr[j].cols;q++)
            {
                allError.push_back(pyr[j].at<uchar>(p,q));
            }
        
        
        
        
    }
    
    
    float sum3=0;
    for (int i=0;i<allError.size();i++)
    {
        sum3+=allError[i];
    }
    
    cout<< "rate of 1 error="<<sum3/allError.size() <<"  num= "<<allError.size()<<endl;
    
    
    float sum4=0;
    for (int i=0;i<allDetail.size();i++)
    {
        sum4+=allDetail[i];
    }
    
    cout<< "rate of 1 detail="<<sum4/allDetail.size() <<"  num= "<<allDetail.size()<<endl;

    
//    entropyCodec x1,x2;
    std::vector<uint8_t>  code, code1;
   int num1= entropyCodec:: encode(allError, code);
    int num2= entropyCodec:: encode(allDetail, code1);
    
    out=code;
    for (int i=0;i<code1.size();i++)
    {
        out.push_back(code1[i]);
    }
    
    cout<<"num1  "<<num1<<"  num2  "<<num2<<"numObj  "<<object.size()*6<<"  numAll "<<num1+num2+6*object.size()<<endl;
    
    
    FILE * f;
    f= fopen("/Users/jameswoods/Documents/MATLAB/2.dat", "w");
    for (int i=0;i<out.size();i++)
    {
        fwrite(&out[i], 1, 1, f);
    }
    fclose(f);

    
    
    return 0;
}



int EncodeShape:: decodeShape(std::vector<uchar> out, cv::Mat & shape )
{
    return 0;
}



//label start from 1

void EncodeShape:: getNeighbor(cv::Mat label)
{
    double minIdx, maxIdx;
    minMaxIdx(label, &minIdx, &maxIdx);
    numOfObj=maxIdx;
    
    int rows=label.rows;
    int cols=label.cols;
    
    
    for (int i=0;i<numOfObj;i++)
    {
        vector<int> temp;
        temp.push_back(-1);
        neighbor.push_back(temp);
        codeStateOfObject.push_back(0);
        
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
}


//0 not decision
//1 uncode, just delete
//2 code


void EncodeShape:: getCodeState(void)
{
    //first is ignore;
    
    for(int i=0;i<codeStateOfObject.size();i++)
    {
        
        if (codeStateOfObject[i]==0)
        {
            bool status=true;
            
            // try
            for (int j=1;j<neighbor[i].size();j++)
            {
                if (codeStateOfObject[ neighbor[i][j]-1]==1)
                {
                    status=false;
                    break;
                }
            }
            
            if (status)
            {
                codeStateOfObject[i]=1;
                for (int j=1;j<neighbor[i].size();j++)
                {
                    codeStateOfObject[neighbor[i][j]-1]=2;
                }
            }
            

        }
        
    }
    
}

 void EncodeShape:: seeDelete( cv::Mat label)
{
    int rows=label.rows;
    int cols= label.cols;
    
    Mat fig(rows,cols, CV_8UC1, Scalar(0) );
    
    for (int i=0;i<rows;i++)
        for (int j=0;j<cols; j++)
        {
            int L=label.at<unsigned short>(i,j);
            if (codeStateOfObject[L-1]==1)
            {
                fig.at<uchar>(i,j)=255;
            }
            
        }
    imshow("Delete", fig);
    waitKey(1);
    
}





