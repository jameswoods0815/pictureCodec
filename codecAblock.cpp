//
//  codecAblock.cpp
//  pictureCodec
//
//  Created by jameswoods on 11/22/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#include "codecAblock.hpp"
#include<cmath>
#include<stdlib.h>
#include<iostream>

bool judgeSkip(std::vector<int > src)
{
    for(int i=0;i<src.size();i++)
    {
        if(src[i])
        {
            return false;
        }
    }
    return true;
}


void encodeAblock(std::vector<int> src,codeUint &output)
{
    //check if block can skip
    output.skipFlag=true;
    
    for(int i=0;i<src.size();i++)
    {
        if(src[i]!=0)
        {
            output.skipFlag=false;
        }
    }
    
    if(output.skipFlag)
    {
        return;
    }
    else {
        // not skip
        output.firstNum=src[0];
        output.ElseFlag=true;
        for(int i=1;i<src.size();i++)
        {
            if(src[i]!=0)
            {
                output.ElseFlag=false;
            }
        }
        
        if (output.ElseFlag)
        {
            return;
        }
        else {
            for(int i=1;i<src.size();i++)
            {
                if(src[i]!=0)
                {
                    output.coeff.push_back(abs(src[i]));
                    output.coeffFlag.push_back(src[i]>=0?0:1);
                    output.ElseStatus.push_back(1);
                    
                }
                else {
                    output.ElseStatus.push_back(0);
                }
            }
        }
        
    }
}
void decodeAblock(codeUint src  ,std::vector<int> &output ,int size)
{
    // init 16
    if(src.skipFlag)
    {
        output.insert(output.end(), size,0);
    }
    else {
        output.push_back(src.firstNum);
        if(src.ElseFlag)
        {
            output.insert(output.end(), size-1,0);
        }
        else {
            int count=0;
            for (int i=0;i<size-1;i++)
            {
                if(src.ElseStatus[i])
                {
                    output.push_back(src.coeff[count]*(src.coeffFlag[count]>0?-1:1));
                    count++;
                }
                else {
                    output.push_back(0);
                }
            }
        }
    }
    
}

void  testEncodeAndDecode(void)
{
    std::vector<int> src(20,0);
//    src[0]=10;
//    src[4]=3;
//    src[7]=-3;
    
    codeUint output;
    encodeAblock(src, output);
    std:: vector<int> out;
    decodeAblock(output, out,src.size());
    
    
//    std::cout<<"OK";
    
    
}

void encodeVector(std::vector<int> src, TransformCoeff &out, std::vector<int>seg)
{
    int index=0;
    std::vector<int> tempVector;
    for(int i=0;i<seg.size();i++)
    {
        codeUint tmp;
        tempVector.clear();
        tempVector.insert(tempVector.end(), &src[index],&src[index+seg[i]]);
        encodeAblock(tempVector, tmp);
        index+=seg[i];
        
        out.skipFlag.push_back(tmp.skipFlag);
        if(!tmp.skipFlag)
        {
            out.firstNum.push_back(tmp.firstNum);
            out.elseFlag.push_back(tmp.ElseFlag);
            if(!tmp.ElseFlag)
            {
                out.elseStatus.insert(out.elseStatus.end(), tmp.ElseStatus.begin(),tmp.ElseStatus.end());
                out.coeffFlag.insert(out.coeffFlag.end(), tmp.coeffFlag.begin(),tmp.coeffFlag.end());
                out.coeff.insert(out.coeff.end(), tmp.coeff.begin(),tmp.coeff.end());
            }
        }
        
    }
}
void decodeVector(TransformCoeff src, std::vector<int>seg, std::vector<int> &out)
{
    int index=0;
    int skipPtr;
    int elseFlagPtr;
    int coeffPtr;
    
    
    for(int i=0;i<seg.size();i++)
    {
        codeUint temp;
        
    }
}




void encodeAsubBlock(std::vector<int> src,subblockContext &out)
{
    out.IfSkip=true;
    for(int i=0;i<src.size();i++)
    {
        if(src[i])
        {
            out.IfSkip=false;
            break;
        }
    }
    
    if(out.IfSkip)
    {
        return;
    }
    else {
        //push if zero and sign
        
        std::vector<int> temp;
        for(int i=0;i<src.size();i++)
        {
            if(src[i])
            {
                out.IfZero.push_back(0);
                temp.push_back(src[i]);
                
            }
            else{
                out.IfZero.push_back(1);
            }
        }
        
        //push else flag
        for(int i=0;i<temp.size();i++)
        {
            out.Sign.push_back(temp[i]>0?0:1);
        }
        
        //push If greaterThan1;
        
        
        int count=-1;
        for (int i=0;i<temp.size();i++)
        {
            if (abs(temp[i])==1||i>=8)
            {
                out.IfGreaterThan1.push_back(0); //biger than 8 init 0;
            }
            else {
                out.IfGreaterThan1.push_back(1);
                }
            //wrong
            
            if(abs(temp[i])>2&&count==-1)
            {
                out.IfGreaterThan2.push_back(1);
            }
            else {
                out.IfGreaterThan2.push_back(0); //greater than 2 else 0;
            }
            
            if(abs(temp[i])>1&&count==-1)
            {
                count=i;
            }
            
        }
        
        
     //remain:
        //get temp2;
        std::vector<int> temp2(temp.size());
        for(int i=0;i<temp.size();i++)
        {
            temp2[i]=abs(temp[i])-out.Sign[i]-out.IfGreaterThan1[i]-out.IfGreaterThan2[i];
        }
        
        //get Remain
        bool flag=true;
        for(int i=0;i<temp2.size();i++)
        {
            if (i!=count)
            {
                if(out.IfGreaterThan1[i]||i>=8)
                {
                    out.abs_else_num.push_back(temp2[i]);
                }
            }
            else {
               if(out.IfGreaterThan2[i])
                   out.abs_else_num.push_back(temp2[i]);
            }
        }
        
        //push geaterthan1;
        
        if (out.IfGreaterThan1.size()>8)
        {
            out.IfGreaterThan1.erase(&out.IfGreaterThan1[8],out.IfGreaterThan1.end());
        }
        
        if(count==-1)
        {
            out.IfGreaterThan2.clear();
        }
        else {
            bool x=out.IfGreaterThan2[count];
            out.IfGreaterThan2.clear();
            out.IfGreaterThan2.push_back(x);
        }
        
    }
    
}


void testEncodeAsubBlokc()
{
    std::vector<int >x1(13,0);
//    for(int i=0;i<6;i++)
//    {
//        x1[i]=-1;
//    }
    x1[10]=1;
//    x1[11]=2;
    x1[12]=-3;
    subblockContext out;
    
    encodeAsubBlock(x1, out);
//    std::cout<<"OK";
}

#define __length 16

void encodeCoeff(std::vector<int> src, blockCoeff &out, std::vector<int>seg)
{
    int index=0;
    std::vector<int> tempVector;
    for(int i=0;i<seg.size();i++)
    {
        tempVector.clear();
        tempVector.insert(tempVector.end(), &src[index],&src[index+seg[i]]);
        std::reverse(tempVector.begin(), tempVector.end());
        
        bool flag=judgeSkip(tempVector);
        out.TUSkip.push_back(flag);
        if(!flag){
            int lastNum;
            for(int j=0;j<tempVector.size();j++){
                if(tempVector[j]){
                    lastNum=j;
                    break;
                }
            }
            out.lastnumindex.push_back(lastNum);
            // first n num there is a bug last zero num, else no coding;
            for (int p=lastNum;p<tempVector.size();p+=__length)
            {
                std::vector<int> subTempVector;
                if(p+__length>=tempVector.size())
                {
                    for(int k=p;k<tempVector.size();k++)
                    {
                        subTempVector.push_back(tempVector[k]);
                    }
                }else {
                subTempVector.insert(subTempVector.end(), &tempVector[p],&tempVector[p+__length]);
                }
                subblockContext subout;
                encodeAsubBlock(subTempVector, subout);

                // insert to out;
                out.IfSkip.push_back(subout.IfSkip);
                out.IfZero.insert(out.IfZero.end(), subout.IfZero.begin(),subout.IfZero.end());
                out.Sign.insert(out.Sign.end(), subout.Sign.begin(),subout.Sign.end());
                out.IfGreaterThan1.insert(out.IfGreaterThan1.end(), subout.IfGreaterThan1.begin(),subout.IfGreaterThan1.end());
                out.IfGreaterThan2.insert(out.IfGreaterThan2.end(), subout.IfGreaterThan2.begin(),subout.IfGreaterThan2.end());
                out.abs_else_num.insert(out.abs_else_num.end(), subout.abs_else_num.begin(),subout.abs_else_num.end());
                
            }
            
            
        }
    index+=seg[i];
    }
}

