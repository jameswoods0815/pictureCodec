//
//  GetContext.cpp
//  pictureCodec
//
//  Created by jameswoods on 11/14/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#include "GetContext.hpp"


// mask is a codition ,that gives the status of the restore pixel;
void getContext(cv::Mat mask, cv::Mat restore,uint8_t * top, uint8_t  * left, int x, int y, int width,int height)
{
    bool checktop1=true,checktop2=true,checkleft1=true,checkleft2=true,checkedge=true;
    
    bool parttop1=false,parttop2=false,partleft1=false,partleft2=false;
    
    //partmode unused;
    
    //checktop1 &checktop2
    if(y>0)
    {
        for(int i =x;i<x+width;i++)
        {
            if(!mask.at<uchar>(y-1,i))
            {
                checktop1=false;
                break;
            }
            
        }
        //check parttop1
        if(!checktop1)
        {
            for(int i =x;i<x+width;i++)
            {
                if(mask.at<uchar>(y-1,i))
                {
                    parttop1=true;
                    break;
                }
                
            }
        }
        
        
        for(int i =x+width;i<x+width+width;i++)
        {
            if(i>mask.cols-1)
            {
                checktop2=false;
                break;
            }
            
            if(!mask.at<uchar>(y-1,i))
            {
                checktop2=false;
                break;
            }
            
        }
        
        //top2 flag
        if(!checktop2)
        {
            for(int i =x+width;i<x+width+width;i++)
            {
                
                
                if(i<mask.cols&&mask.at<uchar>(y-1,i))
                {
                    parttop2=true;
                    break;
                    
                }
                
            }

        }
    }
    else {
        checktop1=false;
        checktop2=false;
    }
    
    //check edge
    if(x==0||y==0)
    {
        checkedge=false;
        
    }
    else {
        if(!mask.at<uchar>(y-1,x-1))
        {
            checkedge=false;
        }
    }
    
    //check left
    
    if (x>0)
    {
        for(int i=y;i<y+height;i++)
        {
            if(!mask.at<uchar>(i,x-1))
            {
                checkleft1=false;
                break;
            }
        }
        //partleft1
        if(!checkleft1)
        {
            for(int i=y;i<y+height;i++)
            {
                if(mask.at<uchar>(i,x-1))
                {
                    partleft1=true;
                    break;
                }
            }

        }
        //checkleft2
        for (int i=y+height;i<y+height+height;i++)
        {
            if(i>mask.rows-1)
            {
                checkleft2=false;
                break;
            }
            
            if(!mask.at<uchar>(i,x-1))
            {
                checkleft2=false;
                break;
            }
        }
        //leftpart2
        if(!checkleft2)
        {
            for (int i=y+height;i<y+height+height;i++)
            {
                
                if(i<mask.rows&&mask.at<uchar>(i,x-1))
                {
                    partleft2=true;
                    break;
                }
            }
 
        }
        
    }
    else {
        checkleft1=false;
        checkleft2=false;
    }
    
  // checkedge
  if(checkedge)
  {
      left[0]=restore.at<uchar>(y-1,x-1);
      top[0]=restore.at<uchar>(y-1,x-1);
  }
  else {
      left[0]=128;
      top[0]=128;
  }
   
  //checkleft1
  if (checkleft1)
  {
      for(int i=0;i<height;i++)
      {
          left[i+1]=restore.at<uchar>(i+y,x-1);
      }
  }
  else {
      
      if (partleft1)
      {
          float sum=0;
          int count=0;
          for (int i=0;i<height;i++)
          {
              if(mask.at<uchar>(i+y,x-1))
              {
                  count++;
                  sum+=restore.at<uchar>(i+y,x-1);
                  left[i+1]=restore.at<uchar>(i+y,x-1);
              }
          }
          sum=sum/count;
          for (int i=0;i<height;i++)
          {
              if(!mask.at<uchar>(i+y,x-1))
              {
                  left[i+1]=sum;
              }
          }
          
      }
      else{
          for(int i=0;i<height;i++)
          {
              left[i+1]=left[0];
          }
      }
      
      
          }
  //check left2
 if(checkleft2)
 {
     for(int i=0;i<height;i++)
     {
         left[i+1+height]=restore.at<uchar>(i+y+height,x-1);
     }

 }
 else {
     
     if (partleft2)
     {
         float sum=0;
         int count=0;
         for (int i=0;i<height;i++)
         {
             if(mask.at<uchar>(i+y+height,x-1)&&(i+y+height<mask.rows))
             {
                 count++;
                 sum+=restore.at<uchar>(i+y+height,x-1);
                 left[i+1+height]=restore.at<uchar>(i+y+height,x-1);
             }
         }
         sum=sum/count;
         for (int i=0;i<height;i++)
         {
             if(!mask.at<uchar>(i+y+height,x-1)||i+y+height>=mask.rows)
             {
                 left[i+1+height]=sum;
             }
         }

     }
     else{
         for(int i=0;i<height;i++)
         {
             left[i+1+height]=left[height];
         }
 
     }
     
 }

 //check top1
    
  if(checktop1)
  {
      for(int i=0;i<width;i++)
      {
          top[i+1]=restore.at<uchar>(y-1,i+x);
      }
  }
  else {
      
      if (parttop1)
      {
          float sum=0;
          int count=0;
          for (int i=0;i<width;i++)
          {
              if(mask.at<uchar>(y-1,x+i))
              {
                  count++;
                  sum+=restore.at<uchar>(y-1,x+i);
                  top[i+1]=restore.at<uchar>(y-1,x+i);
              }
          }
          sum=sum/count;
          for (int i=0;i<width;i++)
          {
              if(!mask.at<uchar>(y-1,x+i))
              {
                  top[i+1]=sum;
              }
          }
          
      }
      else{
          for(int i=0;i<width;i++)
          {
              top[i+1]=top[0];
          }
      }

  }
//check top2
    if(checktop2)
    {
        for(int i=0;i<width;i++)
        {
            top[i+1+width]=restore.at<uchar>(y-1, i+x+width);
        }
    }
    else{
    
        if (parttop2)
        {
            float sum=0;
            int count=0;
            for (int i=0;i<width;i++)
            {
                if(mask.at<uchar>(y-1,x+i+width)&&(i+x+width<mask.cols))
                {
                    count++;
                    sum+=restore.at<uchar>(y-1,i+x+width);
                    top[i+1+width]=restore.at<uchar>(y-1,x+i+width);
                }
            }
            sum=sum/count;
            for (int i=0;i<width;i++)
            {
                if(!mask.at<uchar>(y-1,x+i+width)&&i+x+width<mask.cols)
                {
                    top[i+1+width]=sum;
                }
                if(x+i+width>=mask.cols)
                {
                    top[i+1+width]=sum;

                }
                
            }
            
        }
        else{
            for(int i=0;i<width;i++)
            {
                top[i+1+width]=top[width];
            }
            
        }
    }
  
 //modify chekedge
  if (!checkedge)
  {
      top[0]=(top[1]+left[1])>>1;
      left[0]=(top[1]+left[1])>>1;
  }
    
    
}

void testContext(void)
{
    int num=4;
    cv::Mat src(9,9,CV_8UC1, cv::Scalar(0));
    cv::Mat mask(8,8,CV_8UC1,cv::Scalar(1));
    for(int i=0;i<src.cols;i++)
    {
        src.at<uchar>(0,i)=i+1;
    }
    
    for(int i=0;i<src.rows;i++)
    {
        src.at<uchar>(i,0)=i+10;
    }
    
    uint8_t top[num*2+1];
    uint8_t left[num*2+1];
    int x=1;
    int y=1;
    int width=num;
    int height=num;
    
//    mask.at<uchar>(0,0)=0;
//    mask.at<uchar>(0,1)=0;
//     mask.at<uchar>(0,3)=0;
//     mask.at<uchar>(5,0)=0;
//         mask.at<uchar>(3,0)=0;
    
    getContext(mask, src, top, left, x, y, width, height);
    
    std::cout<<"top"<<std::endl;
    for(int i=0;i<num*2+1;i++)
        std::cout<<int(top[i])<<"  ";
    
    
    std::cout<<std::endl<<"left"<<std::endl;
    
    for(int i=0;i<num*2+1;i++)
        std::cout<<int(left[i])<<"  ";
    
//    std::cout<<"OK";

        
    
}
