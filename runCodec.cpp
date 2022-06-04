//
//  runCodec.cpp
//  pictureCodec
//
//  Created by jameswoods on 11/9/16.
//  Copyright © 2016 jameswoods. All rights reserved.
//

#include "runCodec.hpp"
//#include <Eigen/Dense>
#include"seg.h"
#include "viewPic.hpp"
#include "LSC.h"
#include"SegMaskToBlock.hpp"
#include"codecAshape.hpp"
#include"predictObj.hpp"
#include "GetContext.hpp"
#include "quantization.hpp"
#include"printData.hpp"
#include"columbiaCode.hpp"
#include"Arithmetic.hpp"
#include"PSNR.hpp"
#include"codecAblock.hpp"
#include "LabelInsert.hpp"
#include"Codec1_0.hpp"
#include"Codec2_0.hpp"
#include"analysisParameter.hpp"




ljj::Mat_8UC3 cvMat2EigenMat( cv:: Mat src)
{
    int rows=src.rows;
    int cols=src.cols;
    ljj::Mat_8UC3 ret(rows,cols);
    
    for (int i=0;i<rows;i++)
        for (int j=0;j<cols;j++)
        {
            ret(i,j)(0)=src.at<uchar>(i,3*j);
            ret(i,j)(1)=src.at<uchar>(i,3*j+1);
            ret(i,j)(2)=src.at<uchar>(i,3*j+2);
        }
    
    return  ret;
}

ljj::Mat_16UC1 cvMat2EigenMatShort(cv::Mat src)
{
    int rows=src.rows;
    int cols=src.cols;
    ljj::Mat_16UC1 ret(rows,cols);
    
    for (int i=0;i<rows;i++)
        for (int j=0;j<cols;j++)
        {
            ret(i,j)=src.at<unsigned short>(i,j);
        }
    
    return ret;
}

cv::Mat EigenMat2cvMatshort(ljj::Mat_16UC1 src)
{
    int rows=src.rows();
    int cols=src.cols();
    cv::Mat ret(rows,cols,CV_16UC1);
    for (int i=0;i<rows; i++)
        for (int j=0;j<cols; j++)
        {
            ret.at<unsigned short>(i,j)=src(i,j);
        }
    return ret;
    
}


cv::Mat EigenMat2cvMat(ljj::Mat_8UC3 src)
{
    int rows=src.rows();
    int cols=src.cols();
    cv::Mat ret(rows,cols,CV_8UC3);
    
    for (int i=0;i<rows;i++)
        for (int j=0;j<cols;j++)
        {
            ret.at<uchar>(i,3*j)=src(i,j)(0);
            ret.at<uchar>(i,3*j+1)=src(i,j)(1);
            ret.at<uchar>(i,3*j+2)=src(i,j)(2);
        }
    
    return ret;
}

std::vector<ljj::Shape>  mySplitShape( cv::Mat label)
{
    std::vector<ljj::Shape> ret;
    
    
    double minIdx, maxIdx;
    cv::minMaxIdx(label, &minIdx, &maxIdx);
    
   int  numOfObj=maxIdx;
    
    std::vector<vector<int>> location;
    std::vector<int> temp;
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
        
        ljj::Mat_bool temp=ljj::Mat_bool::Zero(height, width);
        for (int m=y;m<y+height;m++)
            for (int n=x;n<x+width;n++)
            {
                if((label.at<unsigned short>(m,n)-1)==i)
                {
                    temp(m-y,n-x)=1;
                }
            }
        
        ljj::Shape tempShape;
        tempShape.leftTop.x=x;
        tempShape.leftTop.y=y;
        tempShape.mask=temp;
        ret.push_back(tempShape);
    }
    
    return ret;
}


cv::Mat Eigen8U2Mat8U(ljj::Mat_8UC1 src)
{
    int rows=src.rows();
    int cols=src.cols();
    cv::Mat ret(rows,cols,CV_8UC1);
    for (int i=0;i<rows;i++)
        for (int j=0;j<cols;j++)
        {
            ret.at<uchar>(i,j)=src(i,j);
        }
    return ret;
}

cv::Mat Eigenbool2Mat8U(ljj::Mat_bool src)
{
    int rows=src.rows();
    int cols=src.cols();
    cv::Mat ret(rows,cols,CV_8UC1);
    for (int i=0;i<rows;i++)
        for (int j=0;j<cols;j++)
        {
            ret.at<uchar>(i,j)=src(i,j);
        }
    return ret;
}


ljj::Mat_8UC1 Mat2Eigen8u(cv::Mat src)
{
    int rows=src.rows;
    int cols=src.cols;
    ljj::Mat_8UC1 ret(rows,cols);
    for (int i=0;i<rows;i++)
        for (int j=0;j<cols;j++)
        {
            ret(i,j)=src.at<uchar>(i,j);
        }
    return ret;
}

ljj::Mat_bool Mat2Eigenbool(cv::Mat src)
{
    int rows=src.rows;
    int cols=src.cols;
    ljj::Mat_bool ret(rows,cols);
    for (int i=0;i<rows;i++)
        for (int j=0;j<cols;j++)
        {
            ret(i,j)=src.at<uchar>(i,j);
        }
    return ret;
}

ljj::Mat_64FC3 MergeImage(std::vector<ljj::Shape> shape, std::vector<ljj::Mat_64FC3> texture)
{
    // first get rows and cols of the source image;
    int rows=-1;
    int cols=-1;
    for (int i=0;i<shape.size();i++)
    {
        int x=shape[i].leftTop.x+shape[i].mask.cols();
        int y=shape[i].leftTop.y+shape[i].mask.rows();
        
        if(x>cols)
        {
            cols=x;
        }
        if(y>rows)
        {
            rows=y;
        }

    }
    
    ljj::Mat_64FC3 ret(rows,cols);
    for (int i=0;i<rows;i++)
        for (int j=0;j<cols;j++)
        {
            ret(i,j)(0)=0;
            ret(i,j)(1)=0;
            ret(i,j)(2)=0;
        }
    
    for (int i=0;i<shape.size();i++)
    {
        for (int y=0;y<shape[i].mask.rows();y++)
        {
            for(int x=0;x<shape[i].mask.cols();x++)
            {
                ret(y+shape[i].leftTop.y,x+shape[i].leftTop.x)(0)+=texture[i](y,x)(0);
                ret(y+shape[i].leftTop.y,x+shape[i].leftTop.x)(1)+=texture[i](y,x)(1);
                ret(y+shape[i].leftTop.y,x+shape[i].leftTop.x)(2)+=texture[i](y,x)(2);

            }
        }
    }
    
    return  ret;
}


ljj::Mat_16UC1 MergeShape(std::vector<ljj::Shape> shape)
{
    // first get rows and cols of the source image;
    int rows=-1;
    int cols=-1;
    for (int i=0;i<shape.size();i++)
    {
        int x=shape[i].leftTop.x+shape[i].mask.cols();
        int y=shape[i].leftTop.y+shape[i].mask.rows();
        
        if(x>cols)
        {
            cols=x;
        }
        if(y>rows)
        {
            rows=y;
        }
        
    }
    
    ljj::Mat_16UC1 ret=ljj::Mat_16UC1::Zero(rows, cols);
    
    for (int i=0;i<shape.size();i++)
    {
        
        for (int y=0;y<shape[i].mask.rows();y++)
        {
            for(int x=0;x<shape[i].mask.cols();x++)
            {
                if(shape[i].mask(y,x)>0)
                {
                ret(y+shape[i].leftTop.y,x+shape[i].leftTop.x)=i+1;
                }
            }
        }
    }
    
    return  ret;
}



std::vector<std::vector<ljj::Vec3d>> runPic:: codecPic(cv::Mat src,int Quant)
{
#pragma -mark Init DCT and IDCT context

    char *str = "/Users/jameswoods/Documents/project/pictureCodec/DCT/table.bin";
    InitDctContext(str, &context);
    
    
   
    char *istr = "/Users/jameswoods/Documents/project/pictureCodec/DCT/itable.bin";
    InitIDctContext(istr, &icontext);
    
    
    QuantInit=Quant;
    
    const ljj::Mat_8UC3 img=cvMat2EigenMat(src);
    
    source=src.clone();
    
    ljj:: Mat_16UC1 label= ljj::Mat_16UC1::Zero(src.rows, src.cols);
   ljj:: Mat_16UC1 labelSub= ljj::Mat_16UC1::Zero(src.rows/4, src.cols/4);
#pragma -mark lsc codec;
    	const size_t superpixel_num = 200;
    	const double ratio = 0.2; //0.075;
    cv::Mat segtemp;
    cv::Mat segUp;
    
    {
        double start = clock();
    
//            ljj::segM(img, superpixel_num, ratio,label);
         cv::Mat srctmp;
         cv::resize(src, srctmp, cv::Size(src.cols/4,src.rows/4));
        
            segtemp=segMat(srctmp, superpixel_num, ratio);
            segUp=insertLable(segtemp, src.rows, src.cols);
            label=cvMat2EigenMatShort(segUp);
           labelSub=cvMat2EigenMatShort(segtemp);
    
    		double end = clock();
//         segtemp=EigenMat2cvMatshort(label);
    
    		std::cout << "LSC takes time: " << (end - start) / 1000.0 << "ms" << std::endl;
        viewImage(segUp);

    }
    //convert img to ycbcr;
    const ljj::Mat_64FC3 img_ycbcr=ljj::rgb2ycbcr(img);
    
    
   double start = clock();

    std::vector<ljj::Shape> shapes1;
//    shapes = ljj::splitShapeArray(label);
    shapes=mySplitShape(segUp);
    shapes1=mySplitShape(segtemp);
    
    {
        const std::vector<uint16_t> necessary = ljj::calcNecessaryShape(labelSub);
//        const std::vector<ljj::Shape> shapes = ljj::splitShapeArray(label);
        
        int count=0;

// shape write;
#pragma -mark shape codec
        FILE *f;
        f= fopen("/Users/jameswoods/Documents/project/pictureCodec/data/shape.dat", "w");
        for (const uint16_t id : necessary) {
            
            const std::vector<uint8_t> encoded_data =ljj:: encode_mask(shapes1[id - 1].mask);
            count+=encoded_data.size();
            const ljj::Mat_bool shape = ljj::decode_mask(shapes1[id - 1].mask.rows(), shapes1[id - 1].mask.cols(), encoded_data);
            
            assert(shape == shapes1[id - 1].mask);
            
            for (int i=0;i<encoded_data.size();i++)
            {
                fwrite(&encoded_data[i], 1, 1, f);
            }
        }
         fclose(f);
        std::cout<<"the num of  shape byte=" <<count<<std::endl;
    }
    
    
    std::vector<ljj::Mat_64FC3> texture;
    if(1)
    {
        
        double start = clock();
        
        for (int i=0;i<shapes.size();i++)
        {
        ljj::Mat_64FC3 temp= codecAshape(shapes[i],img_ycbcr);
            texture.push_back(temp);
        }
        
        double end = clock();
        std::cout << "encode  and decode takes time: " << (end - start) / 1000.0 << "ms" << std::endl;
        
        cv::Mat resultImg;
        
        ljj::Mat_64FC3 finalImage=MergeImage(shapes, texture);
        ljj::Mat_8UC3 decodeImg=ljj::ycbcr2rgb(finalImage);
        resultImg=EigenMat2cvMat(decodeImg);
        
        cv::imshow("FinalImage", resultImg);
        cvWaitKey(1);
        
        cv::imwrite("/Users/jameswoods/Documents/project/pictureCodec/data/decode.png", resultImg);
        
        double psnr=calcPSNR(src, resultImg);
        std::cout<<"psnr= "<<psnr<<std::endl;
    
    

    blockCoeff  CoeffblockI1,CoeffblockI2,CoeffblockI3;
    blockCoeff  CoeffblockB1,CoeffblockB2,CoeffblockB3;

    {
        vector<int> innerY(innerData.size()),innerCb(innerData.size()),innerCr(innerData.size());
        vector<int> bouY(bouData.size()),bouCb(bouData.size()),bouCr(bouData.size());
        for(int i=0;i<innerData.size();i++)
        {
            innerY[i]=innerData[i][0];
            innerCb[i]=innerData[i][1];
            innerCr[i]=innerData[i][2];
        }
        
        for(int i=0;i<bouData.size();i++)
        {
            bouY[i]=bouData[i][0];
            bouCb[i]=bouData[i][1];
            bouCr[i]=bouData[i][2];

        }
        
        encodeCoeff(innerY,  CoeffblockI1, innerLength);
        encodeCoeff(innerCb,  CoeffblockI2, innerLength);
        encodeCoeff(innerCr,  CoeffblockI3, innerLength);

        encodeCoeff(bouY, CoeffblockB1, bouLength);
        encodeCoeff(bouCb, CoeffblockB2, bouLength);
        encodeCoeff(bouCr, CoeffblockB3, bouLength);

        
        std::vector<blockCoeff> s2(6);
        s2[0]=CoeffblockI1;
        s2[1]=CoeffblockI2;
        s2[2]=CoeffblockI3;
        s2[3]=CoeffblockB1;
        s2[4]=CoeffblockB2;
        s2[5]=CoeffblockB3;
        
        CodeDemo_2_0_TS(s2);
        
    }
    
    
#pragma -mark write file

}
    
    std::cout<<"codec and decode OK"<<endl;
    
    return data;
}

#pragma  -mark decodePic
cv::Mat runPic:: decodePic( std::vector<std::vector<ljj::Vec3d>> data)
{
    
//    CodecDemo(0.5, data, data);
    
    
    
    double start = clock();
    std::vector<ljj::Mat_64FC3> texture;

    		for (size_t i = 0; i < shapes.size(); i++) {
    			const auto &shape = shapes[i];
                const ljj::Mat_64FC3 deserialized = ljj::zigzagDeserialize(shape, data[i]);
                const ljj::Mat_64FC3 resolvedTexture = ljj::solveTextureYcbcr(shape, deserialized, &icontext);
                texture.push_back(resolvedTexture);
    		}
    double end = clock();
    
    std::cout << "decode takes time: " << (end - start) / 1000.0 << "ms" << std::endl;
    
    ljj::Mat_64FC3 finalImage=MergeImage(shapes, texture);
    ljj::Mat_8UC3 decodeImg=ljj::ycbcr2rgb(finalImage);
    restore=EigenMat2cvMat(decodeImg);
    
    cv::imshow("windowsss", restore);
    
    cvWaitKey(10);
    
    double psnr=calcPSNR(source, restore);
    std::cout<<"psnr11= "<<psnr<<std::endl;

    return restore;
}

// input shape in the image, coordinate at the shape
// pridict 3 channels
#pragma -mark start coding a shape ***************************
ljj::Mat_64FC3   runPic:: codecAshape(ljj::Shape shapeOri, ljj::Mat_64FC3 srcData)
{
   
    int bouLengthCount=0;
    int innerLengthCount=0;
    
    // get seri;
    cv::Mat tempShape=Eigenbool2Mat8U(shapeOri.mask);
    
    maskSeg shapeseg;
    shapeseg.seg(tempShape);
    std::vector<vector<int>> boundary=shapeseg.getBodIndex();
    std::vector<vector<int>> innerSrc=shapeseg.getInnerIndex();
    
    int bouStride=shapeseg.getBouStride();
    int innerStride=shapeseg.getInnerStride();
    
    std::vector<vector<int>>inner;
    vector<int> temp(2);
    for(int i=0;i<innerSrc.size();i++)
    {
        temp[0]=innerSrc[i][0];
        temp[1]=innerSrc[i][1];
        inner.push_back(temp);
        
        temp[0]=innerSrc[i][0]+innerStride/2;
        inner.push_back(temp);
        
        temp[0]=innerSrc[i][0];
        temp[1]=innerSrc[i][1]+innerStride/2;
        inner.push_back(temp);
        
        temp[0]=innerSrc[i][0]+innerStride/2;
        temp[1]=innerSrc[i][1]+innerStride/2;
        inner.push_back(temp);

    }
    innerStride=innerStride/2;
    
    
    
    cv::Mat shapeMat=shapeseg.getExtendMat();
    
    ljj::Shape shape;
    shape.leftTop=shapeOri.leftTop;
    shape.mask=Mat2Eigenbool(shapeMat);
    
    
    //get shape img;
    int rows=shape.mask.rows();
    int cols=shape.mask.cols();
    //img extend
    
    ljj::Mat_64FC3 img(rows,cols);
    ljj::Mat_64FC3 codecImage(rows,cols);
    
    for (int p=0;p<shapeOri.mask.rows();p++)
        for (int q=0;q<shapeOri.mask.cols();q++)
        {
            img(p,q)(0)=srcData(p+shape.leftTop.y,q+shape.leftTop.x)(0);
            img(p,q)(1)=srcData(p+shape.leftTop.y,q+shape.leftTop.x)(1);
            img(p,q)(2)=srcData(p+shape.leftTop.y,q+shape.leftTop.x)(2);
        }
    
    codecObjectContext obj;
    // init codecObject context;
//    cv::Mat init(rows,cols,CV_64FC1,cv::Scalar(0));
    cv::Mat init1(rows,cols,CV_8UC1,cv::Scalar(0));
    obj.contextMask=init1.clone();
    obj.contextY=init1.clone();
    obj.contextCb=init1.clone();
    obj.contextCr=init1.clone();
    
    ljj:: Mat_64FC3 restoreImage(rows,cols);
    
    for (int i=0;i<rows;i++)
        for (int j=0;j<cols;j++)
        {
            restoreImage(i,j)(0)=0;
            restoreImage(i,j)(1)=0;
            restoreImage(i,j)(2)=0;
        }
    
    
    obj.size=0;
    
    cv::Mat tempcontexY(rows,cols,CV_8UC1);

    //mask all zeros;
    for(int i=0;i<rows;i++)
        for(int j=0;j<cols;j++)
        {
            obj.contextMask.at<uchar>(i,j)=0;
        }
  
#pragma -mark QP init
    float QPY=QuantInit*0.1;
    float QPCb=QuantInit*0.1;
    float QPCr=QuantInit*0.1;
    int count=0;
    
    //code boundary  //shape not extend so ,the bugs happen;
    for (int i=0;i<boundary.size();i++)
    {
        //get tempShape;
        ljj::Shape tempShape;
        tempShape.leftTop.x=boundary[i][1]; //img position x
        tempShape.leftTop.y=boundary[i][0]; //img position y
        tempShape.mask=ljj::Mat_bool(bouStride,bouStride);
        // get now shape;
        for (int p=0;p<bouStride;p++)
            for (int q=0;q<bouStride;q++)
            {
                tempShape.mask(p,q)=shape.mask(p+boundary[i][0],q+boundary[i][1]);
            }
        //get context for pridict
        
        uint8_t left[2*bouStride+1],up[2*bouStride+1];
        uint8_t leftCb[2*bouStride+1],upCb[2*bouStride+1];
        uint8_t leftCr[2*bouStride+1],upCr[2*bouStride+1];
        getContext(obj.contextMask, obj.contextY, up, left, boundary[i][1], boundary[i][0], bouStride, bouStride);
        getContext(obj.contextMask, obj.contextCb, upCb, leftCb, boundary[i][1], boundary[i][0], bouStride, bouStride);
        getContext(obj.contextMask, obj.contextCr, upCr, leftCr, boundary[i][1], boundary[i][0], bouStride, bouStride);

    
        //pridict mode; first only use the dc mode; here can be replaced by the optimal mode;
        uint8_t src[bouStride*bouStride];
        uint8_t srcCb[bouStride*bouStride];
        uint8_t srcCr[bouStride*bouStride];
        
        //get the best predict mode
        {
            double sourceChannelL[bouStride*bouStride];
            double sourceChannelCb[bouStride*bouStride];
            double sourceChannelCr[bouStride*bouStride];

            bool maskShape[bouStride*bouStride];
            for (int p=0;p<bouStride;p++)
                for (int q=0;q<bouStride;q++)
                {
                    sourceChannelL[p*bouStride+q]=img(p+boundary[i][0],q+boundary[i][1])(0);
                    sourceChannelCb[p*bouStride+q]=img(p+boundary[i][0],q+boundary[i][1])(1);
                    sourceChannelCr[p*bouStride+q]=img(p+boundary[i][0],q+boundary[i][1])(2);

                }
            for(int p=0;p<bouStride;p++)
                for(int q=0;q<bouStride;q++)
                {
                    maskShape[p*bouStride+q]=tempShape.mask(p,q);
                }
#pragma -mark get best bou mode;
       //L pridict
        std::vector<int> preMode;
            
            int mode;
            
            

           mode=getBestMode(sourceChannelL, src, maskShape, left, up, bouStride, preMode);
            
            preMode.push_back(mode);
            preMode.push_back(0);
            preMode.push_back(1);
            preMode.push_back(10);
            preMode.push_back(26);
            
          mode= getBestMode(sourceChannelCb, srcCb, maskShape, leftCb, upCb, bouStride, preMode);
            preMode.clear();
            preMode.push_back(mode);
            getBestMode(sourceChannelCr, srcCr, maskShape, leftCr, upCr, bouStride, preMode);
            
        }
        //get CodecImage, insert the diff into the image
        for (int p=0;p<bouStride;p++)
            for (int q=0;q<bouStride;q++)
            {
                codecImage(p+boundary[i][0],q+boundary[i][1])(0)=img(p+boundary[i][0],q+boundary[i][1])(0)-src[p*bouStride+q];
                codecImage(p+boundary[i][0],q+boundary[i][1])(1)=img(p+boundary[i][0],q+boundary[i][1])(1)-srcCb[p*bouStride+q];
                codecImage(p+boundary[i][0],q+boundary[i][1])(2)=img(p+boundary[i][0],q+boundary[i][1])(2)-srcCr[p*bouStride+q];
            }
        
        
        // get dctMat_64F
        ljj::Mat_64FC3 dctMat= ljj::makeShapeTextureYcbcr(codecImage, tempShape, &context);
        
        std::vector<ljj::Vec3d> serilize = zigzagSerialize(tempShape, dctMat);

        //Quatilize  Vec3d
        std::vector<std::vector <int> > out(serilize.size() ,std::vector<int>(3,0));
#pragma -mark quantization edge
        quant(serilize, out, QPY, QPCb, QPCr);
        
        bouData.insert(bouData.end(), out.begin(),out.end());
        bouLength.push_back(out.size());
        bouLengthCount+=out.size();
        
        //Dequalize the Vec3d
        std::vector<ljj::Vec3d> deqp;
//        dequant(out, deqp , QPY, QPCb, QPCr);
        
        for(int i=0;i<serilize.size();i++)
        {
            serilize[i](0)=out[i][0];
            serilize[i][1]=out[i][1];
            serilize[i][2]=out[i][2];
        }

        
        //decode the image diff
        ljj::Mat_64FC3 iZigZagMat=ljj::zigzagDeserialize(tempShape, serilize);

//        ljj::Mat_64FC3 iZigZagMat=ljj::zigzagDeserialize(tempShape, serilize);
        ljj:: Mat_64FC3 idctMat=ljj::solveTextureYcbcr(tempShape, iZigZagMat, &icontext);
        deQuMat(idctMat, QPY, QPCb, QPCr);

        
        //refresh  mask;
        
        for(int p=0;p<bouStride;p++)
            for (int q=0;q<bouStride;q++)
            {
                obj.contextMask.at<uchar>(p+boundary[i][0],q+boundary[i][1])=shape.mask(p+boundary[i][0],q+boundary[i][1]);
            }
        // refresh decode Image
        for (int p=0;p<bouStride;p++)
            for (int q=0;q<bouStride;q++)
            {
                if(tempShape.mask(p,q))
                {
                    obj.contextY.at<uchar>(p+boundary[i][0],q+boundary[i][1])=idctMat(p,q)(0)+src[p*bouStride+q];                    obj.contextCb.at<uchar>(p+boundary[i][0],q+boundary[i][1])=idctMat(p,q)(1)+srcCb[p*bouStride+q];
                    obj.contextCr.at<uchar>(p+boundary[i][0],q+boundary[i][1])=idctMat(p,q)(2)+srcCr[p*bouStride+q];
                    restoreImage(p+boundary[i][0],q+boundary[i][1])(0)=idctMat(p,q)(0)+src[p*bouStride+q];
                    restoreImage(p+boundary[i][0],q+boundary[i][1])(1)=idctMat(p,q)(1)+srcCb[p*bouStride+q];
                    restoreImage(p+boundary[i][0],q+boundary[i][1])(2)=idctMat(p,q)(2)+srcCr[p*bouStride+q];
                    
                }
                
                
            }
        
        
//        showImageChannel(obj.contextY, obj.contextCb, obj.contextCr);

        
    }
/*********************************
 *push object bou length
 *********************************/
 
    objectbouLength.push_back(bouLengthCount);
//
//    //code inner;
    
/**************************************************************************************************
 *inner codec
 *jameswoods
 *encode and decode
 **************************************************************************************************/

    for (int i=0;i<inner.size();i++)
    {
        //get tempShape;
        ljj::Shape tempShape;
        tempShape.leftTop.x=inner[i][1]; //img position x
        tempShape.leftTop.y=inner[i][0]; //img position y
        tempShape.mask=ljj::Mat_bool(innerStride,innerStride);
        // get now shape;
        for (int p=0;p<innerStride;p++)
            for (int q=0;q<innerStride;q++)
            {
                tempShape.mask(p,q)=shape.mask(p+inner[i][0],q+inner[i][1]);
            }
        //get context for pridict
        
        uint8_t left[2*innerStride+1],up[2*innerStride+1];
        uint8_t leftCb[2*innerStride+1],upCb[2*innerStride+1];
        uint8_t leftCr[2*innerStride+1],upCr[2*innerStride+1];

        
        getContext(obj.contextMask, obj.contextY, up, left, inner[i][1], inner[i][0], innerStride, innerStride);
        getContext(obj.contextMask, obj.contextCb, upCb, leftCb, inner[i][1], inner[i][0], innerStride, innerStride);
        getContext(obj.contextMask, obj.contextCr, upCr, leftCr, inner[i][1], inner[i][0], innerStride, innerStride);
        
        
        //pridict mode; first only use the dc mode; here can be replaced by the optimal mode;
        uint8_t src[innerStride*innerStride];
        uint8_t srcCb[innerStride*innerStride];
        uint8_t srcCr[innerStride*innerStride];
        
        {
            double sourceChannelL[innerStride*innerStride];
            double sourceChannelCb[innerStride*innerStride];
            double sourceChannelCr[innerStride*innerStride];
            
            bool maskShape[innerStride*innerStride];
            for (int p=0;p<innerStride;p++)
                for (int q=0;q<innerStride;q++)
                {
                    sourceChannelL[p*innerStride+q]=img(p+inner[i][0],q+inner[i][1])(0);
                    sourceChannelCb[p*innerStride+q]=img(p+inner[i][0],q+inner[i][1])(1);
                    sourceChannelCr[p*innerStride+q]=img(p+inner[i][0],q+inner[i][1])(2);
                    
                }
            for(int p=0;p<innerStride;p++)
                for(int q=0;q<innerStride;q++)
                {
                    maskShape[p*innerStride+q]=tempShape.mask(p,q);
                }
#pragma -mark get best inner mode;
            //L pridict
            std::vector<int> preMode;
            
            int mode;

           mode= getBestMode(sourceChannelL, src, maskShape, left, up, innerStride, preMode);
            preMode.push_back(mode);
            preMode.push_back(0);
            preMode.push_back(1);
            preMode.push_back(10);
            preMode.push_back(26);

            
           mode= getBestMode(sourceChannelCb, srcCb, maskShape, leftCb, upCb, innerStride, preMode);
//            cout<<"Mode= "<<mode<<endl;
            preMode.clear();
            preMode.push_back(mode);
            getBestMode(sourceChannelCr, srcCr, maskShape, leftCr, upCr, innerStride, preMode);
//            showPredChannel(src, srcCb, srcCr, innerStride);
//            showPredChannelDouble(sourceChannelL,sourceChannelCb,sourceChannelCr,innerStride);
            
        }

        //get CodecImage, insert the diff into the image
        for (int p=0;p<innerStride;p++)
            for (int q=0;q<innerStride;q++)
            {
                codecImage(p+inner[i][0],q+inner[i][1])(0)=img(p+inner[i][0],q+inner[i][1])(0)-src[p*innerStride+q];
                codecImage(p+inner[i][0],q+inner[i][1])(1)=img(p+inner[i][0],q+inner[i][1])(1)-srcCb[p*innerStride+q];
                codecImage(p+inner[i][0],q+inner[i][1])(2)=img(p+inner[i][0],q+inner[i][1])(2)-srcCr[p*innerStride+q];
            }
        
        
        // get dctMat_64F
        ljj::Mat_64FC3 dctMat= ljj::makeShapeTextureYcbcr(codecImage, tempShape,&context);
        std::vector<ljj::Vec3d> serilize = zigzagSerialize(tempShape, dctMat);
        
        //Quatilize  Vec3d
       std::vector<std::vector <int> > out(serilize.size() ,std::vector<int>(3,0));
        quant(serilize, out, QPY, QPCb, QPCr);
#pragma -mark print Data to file inner
        
        innerData.insert(innerData.end(), out.begin(),out.end());
        innerLength.push_back(out.size());
//        printQuant(out, count);
        count++;
        
        innerLengthCount+=out.size();
        //Dequalize the Vec3d
        std::vector<ljj::Vec3d> deqp;
//        dequant(out, deqp , QPY, QPCb, QPCr);
        
        
        //decode the image diff
        for(int i=0;i<serilize.size();i++)
        {
            serilize[i](0)=out[i][0];
            serilize[i][1]=out[i][1];
            serilize[i][2]=out[i][2];
        }
        
        ljj::Mat_64FC3 iZigZagMat=ljj::zigzagDeserialize(tempShape, serilize);

//        ljj::Mat_64FC3 iZigZagMat=ljj::zigzagDeserialize(tempShape, serilize);
        ljj:: Mat_64FC3 idctMat=ljj::solveTextureYcbcr(tempShape, iZigZagMat, &icontext);
        deQuMat(idctMat, QPY, QPCb, QPCr);
        
        //refresh  mask;
        for(int p=0;p<innerStride;p++)
            for (int q=0;q<innerStride;q++)
            {
                obj.contextMask.at<uchar>(p+inner[i][0],q+inner[i][1])=shape.mask(p+inner[i][0],q+inner[i][1]);
            }
        
        
        // refresh decode Image
        for (int p=0;p<innerStride;p++)
            for (int q=0;q<innerStride;q++)
            {
                if(tempShape.mask(p,q))
                {
                    obj.contextY.at<uchar>(p+inner[i][0],q+inner[i][1])=idctMat(p,q)(0)+src[p*innerStride+q];
                    obj.contextCb.at<uchar>(p+inner[i][0],q+inner[i][1])=idctMat(p,q)(1)+srcCb[p*innerStride+q];
                    obj.contextCr.at<uchar>(p+inner[i][0],q+inner[i][1])=idctMat(p,q)(2)+srcCr[p*innerStride+q];
                    
                    restoreImage(p+inner[i][0],q+inner[i][1])(0)=idctMat(p,q)(0)+src[p*innerStride+q];
                    restoreImage(p+inner[i][0],q+inner[i][1])(1)=idctMat(p,q)(1)+srcCb[p*innerStride+q];
                    restoreImage(p+inner[i][0],q+inner[i][1])(2)=idctMat(p,q)(2)+srcCr[p*innerStride+q];

                    
                }

            }
        

//        showImageChannel(obj.contextY, obj.contextCb, obj.contextCr);
        
    }
    
//    ljj::Mat_64FC3 ret= showImageChannel(obj.contextY, obj.contextCb, obj.contextCr);
    

/*****************************************************************************
 *push the inner size
 *****************************************************************************/

    objectinnerLength.push_back(innerLengthCount);
    
    return restoreImage;
    
}
#pragma  -mark image show fucntion
/*****************************************************************************
 *INPUT: Y, CB,CR ,UINT8 CHANNEL
 *SHOW THE IMAGE;
 *JAMESWOODS
 *****************************************************************************/
void runPic:: showPredChannel(uint8_t *Y ,uint8_t *Cb, uint8_t *Cr,int stride)
{
    ljj::Mat_64FC3 temp(stride,stride);
    for(int i=0;i<stride;i++)
        for (int j=0;j<stride;j++)
        {
            temp(i,j)(0)=Y[i*stride+j];
            temp(i,j)(1)=Cb[i*stride+j];
            temp(i,j)(2)=Cr[i*stride+j];
        }
    
    ljj::Mat_8UC3 temp1=ljj::ycbcr2rgb(temp);
    cv::Mat ret=EigenMat2cvMat(temp1);
    imshow("ret image", ret);
    cvWaitKey(1);
    
}

void runPic:: showPredChannelDouble(double *Y ,double *Cb, double *Cr,int stride)
{
    ljj::Mat_64FC3 temp(stride,stride);
    for(int i=0;i<stride;i++)
        for (int j=0;j<stride;j++)
        {
            temp(i,j)(0)=Y[i*stride+j];
            temp(i,j)(1)=Cb[i*stride+j];
            temp(i,j)(2)=Cr[i*stride+j];
        }
    
    ljj::Mat_8UC3 temp1=ljj::ycbcr2rgb(temp);
    cv::Mat ret=EigenMat2cvMat(temp1);
    imshow("src image", ret);
    cvWaitKey(1);
    
}

/*****************************************************************************
 *INPUT: Y, CB,CR ,UINT8 CHANNEL MAT
 *SHOW THE IMAGE;
 *JAMESWOODS
 *****************************************************************************/

ljj::Mat_64FC3  runPic::showImageChannel(cv::Mat Y,cv::Mat Cb,cv::Mat Cr)
{
    ljj::Mat_64FC3 temp(Y.rows,Y.cols);
    for (int i=0;i<Y.rows;i++)
        for(int j=0;j<Y.cols;j++)
        {
            temp(i,j)(0)=Y.at<uchar>(i,j);
            temp(i,j)(1)=Cb.at<uchar>(i,j);
            temp(i,j)(2)=Cr.at<uchar>(i,j);
        }
    
    ljj::Mat_8UC3 ret=ljj::ycbcr2rgb(temp);
    
    cv::Mat result=EigenMat2cvMat(ret);
    imshow("image shape", result);
    cvWaitKey(1);
    return temp;
    
}
