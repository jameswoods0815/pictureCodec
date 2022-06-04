//
//  Codec1_0.cpp
//  pictureCodec
//
//  Created by jameswoods on 11/25/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#include "Codec1_0.hpp"
#include"columbiaCode.hpp"
#include "Arithmetic.hpp"
#include <fstream>
#include<stdlib.h>

void writeTS(std::vector<std::vector<ljj::Vec3d>> data)
{
    std::vector<int> firstnum(data.size()*3);
    for(int i=0;i<data.size();i++)
    {
        firstnum[3*i]=data[i][0][0];
        firstnum[3*i+1]=data[i][0][1];
        firstnum[3*i+2]=data[i][0][2];
    }
    
    std::vector<int> lastPosition(data.size()*3);
    for(int i=0;i<data.size();i++)
        for(int channel=0;channel<3;channel++)
        {
            for(int j=data[i].size()-1;j>=0;j--)
            {
                if (data[i][j][channel])
                {
                    lastPosition[i*3+channel]=j;
                    break;
                }
            }
        }
    
    std::vector<int> num_abs;
     std::vector<int> num_positon;
    for(int i=0;i<data.size();i++)
        for(int channel=0;channel<3;channel++)
            for(int j=1;j<=lastPosition[i*3+channel];j++)
            {
                bool flag=false;
                if(data[i][j][channel])
                {
                    
                    if (flag==true)
                    {
                       num_positon.push_back(j-num_positon.at(num_positon.size()-1));

                    }
                    else{
                        num_positon.push_back(j);
                    }
                    num_abs.push_back(data[i][j][channel]);

                    flag=true;
                    
                }
            }
    
    std::vector<bool>sign(num_abs.size());
    for(int i=0;i<num_abs.size();i++)
    {
        if(num_abs[i]<0)
        {
            sign[i]=1;
            num_abs[i]=-num_abs[i];
        }
        else {
            sign[i]=0;
        }
    }
    
    std::vector<bool> postion;
    std::vector<bool> abs_num;
    std::vector<bool> first_num;
    std::vector<bool> last_postion;
    
    postion=encodeColumbiaSeri(num_positon, 0);
    abs_num=encodeColumbiaSeri(num_abs, 0);
    first_num=encodeColumbiaSeri(firstnum, 0);
    last_postion=encodeColumbiaSeri(lastPosition, 0);
    
    std::vector<uint8_t>  code1,code2,code3,code4,code5;
    entropyCodec::encode(postion, code1);
    entropyCodec::encode(abs_num, code2);
    entropyCodec::encode(first_num, code3);
    entropyCodec::encode(last_postion, code4);
    entropyCodec::encode(sign, code5);
    
    FILE * f;
    char *buf="/Users/jameswoods/Documents/project/pictureCodec/data/all.dat";
    f= fopen(buf, "w");

    for (int i=0;i<code1.size();i++)
    {
        fwrite(&code1[i], 1, 1, f);
    }

    for (int i=0;i<code2.size();i++)
    {
        fwrite(&code2[i], 1, 1, f);
    }
    for (int i=0;i<code3.size();i++)
    {
        fwrite(&code3[i], 1, 1, f);
    }
    for (int i=0;i<code4.size();i++)
    {
        fwrite(&code4[i], 1, 1, f);
    }
    for (int i=0;i<code5.size();i++)
    {
        fwrite(&code5[i], 1, 1, f);
    }
    
    fclose(f);
    
    
}

void CodecDemo(double QP,  std::vector<std::vector<ljj::Vec3d>> data,std::vector<std::vector<ljj::Vec3d>> &dataOut)
{
    std::vector<double> qp(3,QP);
    dataOut=data;
    for(int i=0;i<data.size();i++)
      for(int j=0;j<data[i].size();j++)
        {
            for(int p=0;p<3;p++)
            {
                dataOut[i][j][p]=round(data[i][j][p]/qp[p]);
            }
        }
    
    writeTS(dataOut);
    
    
    for(int i=0;i<data.size();i++)
        for(int j=0;j<data[i].size();j++)
        {
            for(int p=0;p<3;p++)
            {
                dataOut[i][j][p]*=qp[p];
            }
        }
}
