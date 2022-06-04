#include <vector>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <fftw3.h>
#include<list>

#include "Arithmetic.hpp"
#include "imgproc.h"
#include "seg.h"
#include "codec.h"


namespace ljj {
    

    
    std::vector<uint16_t> calcNecessaryShape(const Mat_16UC1 &label)
    {
        const size_t rows = label.rows();
        const size_t cols = label.cols();
        
        class Superpixel {
        public:
            bool necessary = true;
            std::vector<std::pair<uint16_t, uint16_t>> loc;
            std::set<uint16_t> neighbors;
        };
        
        std::vector<Superpixel> superpxls;
        std::vector<uint16_t> ret;
        
        for (size_t x = 0; x < cols; x++) {
            for (size_t y = 0; y < rows; y++) {
                
                const uint16_t l = label(y, x);
                
                if (l >= superpxls.size()) {
                    superpxls.resize(l + 1);
                }
                
                superpxls[l].loc.push_back(std::make_pair(x, y));
                
                for (size_t xi = std::max(1ul, x) - 1; xi <= std::min(cols - 1, x + 1); xi++) {
                    for (size_t yi = std::max(1ul, y) - 1; yi <= std::min(rows - 1, y + 1); yi++) {
                        if (l != label(yi, xi)) {
                            superpxls[l].neighbors.insert(label(yi, xi));
                        }
                    }
                }
            }
        }
        
        for (size_t id = 1; id < superpxls.size(); id++) {
            
            bool foundUnnecessaryNeighbor = false;
            
            for (const uint16_t neighbor_label : superpxls[id].neighbors) {
                if (!superpxls[neighbor_label].necessary) {
                    foundUnnecessaryNeighbor = true;
                    break;
                }
            }
            
            if (!foundUnnecessaryNeighbor) {
                superpxls[id].necessary = false;
            } else {
                ret.push_back(id);
            }
        }
        
        return ret;
    }
    
    Mat_16UC1 mergeLabel(const Mat_16UC1 &label, const Mat_64FC3 &ycbcr, const size_t block_num, const double var)
    {
        assert(label.rows() == ycbcr.rows());
        assert(label.cols() == ycbcr.cols());
        
        const size_t rows = ycbcr.rows();
        const size_t cols = ycbcr.cols();
        
        class Superpixel {
        public:
            double sumX = 0.0;
            double sumX2 = 0.0;
            std::vector<std::pair<uint16_t, uint16_t>> loc;
            std::set<uint16_t> neighbors;
        };
        
        std::map<uint16_t, Superpixel> superpxls;				// Var < var
        std::map<uint16_t, Superpixel> results;					// Var >= var
        
        for (size_t x = 0; x < cols; x++) {
            for (size_t y = 0; y < rows; y++) {
                
                const uint16_t l = label(y, x);
                
                superpxls[l].loc.push_back(std::make_pair(x, y));
                superpxls[l].sumX += ycbcr(y, x)(0);
                superpxls[l].sumX2 += ycbcr(y, x)(0) * ycbcr(y, x)(0);
                
                for (size_t xi = std::max(1ul, x) - 1; xi <= std::min(cols - 1, x + 1); xi++) {
                    for (size_t yi = std::max(1ul, y) - 1; yi <= std::min(rows - 1, y + 1); yi++) {
                        if (l != label(yi, xi)) {
                            superpxls[l].neighbors.insert(label(yi, xi));
                        }
                    }
                }
            }
        }
        
        for (auto S = superpxls.begin(); S != superpxls.end(); S++) {
            
            const Superpixel &superpxl = S->second;
            const double var_superpxl = superpxl.sumX2 / superpxl.loc.size() - square(superpxl.sumX / superpxl.loc.size());
            
            if (var_superpxl >= var) {
                
                for (const uint16_t neighbor_label : superpxl.neighbors) {
                    superpxls[neighbor_label].neighbors.erase(S->first);
                }
                
                results[S->first] = std::move(S->second);
                superpxls.erase(S);
            }
        }
        
        while (superpxls.size() > 1) {
            
            auto S = superpxls.begin();
            Superpixel &superpxl = S->second;
            
            while (!superpxl.neighbors.empty()) {
                
                const uint16_t neighbor_label = *superpxl.neighbors.cbegin();
                const Superpixel &neighbor = superpxls[neighbor_label];
                const double new_var = (superpxl.sumX2 + neighbor.sumX2) / (superpxl.loc.size() + neighbor.loc.size()) - square((superpxl.sumX + neighbor.sumX) / (superpxl.loc.size() + neighbor.loc.size()));
                
                superpxl.loc.insert(superpxl.loc.end(), neighbor.loc.cbegin(), neighbor.loc.cend());
                superpxl.sumX += neighbor.sumX;
                superpxl.sumX2 += neighbor.sumX2;
                superpxl.neighbors.insert(neighbor.neighbors.cbegin(), neighbor.neighbors.cend());
                superpxl.neighbors.erase(S->first);
                
                for (const uint16_t l : neighbor.neighbors) {
                    
                    superpxls[l].neighbors.erase(neighbor_label);
                    
                    if (l != S->first) {
                        superpxls[l].neighbors.insert(S->first);
                    }
                }
                
                superpxls.erase(neighbor_label);
                
                if (new_var >= var) {
                    
                    for (const uint16_t neighbor_label : superpxl.neighbors) {
                        superpxls[neighbor_label].neighbors.erase(S->first);
                    }
                    
                    break;
                }
            }
            
            results[S->first] = std::move(S->second);
            superpxls.erase(S);
        }
        
        Mat_16UC1 retMat(rows, cols);
        
        for (const auto S : results) {
            for (const auto &loc : S.second.loc) {
                retMat(loc.second, loc.first) = S.first;
            }
        }
        
        return retMat;
    }
    
   
    Rect calcConvexRect(const Mat_bool &src)
    {
        int left, right, top, bottom;
        
        for (left = 0; left < src.cols(); left++) {
            
            bool foundLeft = false;
            
            for (top = 0; top < src.rows(); top++) {
                if (src(top, left)) {
                    foundLeft = true;
                    break;
                }
            }
            
            if (foundLeft) {
                break;
            }
        }
        
        for (int x = left + 1; x < src.cols(); x++) {
            for (int y = 0; y < top; y++) {
                if (src(y, x)) {
                    top = y;
                    break;
                }
            }
        }
        
        for (right = src.cols() - 1; right >= left; right--) {
            
            bool foundRight = false;
            
            for (bottom = src.rows() - 1; bottom >= 0; bottom--) {
                if (src(bottom, right)) {
                    foundRight = true;
                    break;
                }
            }
            
            if (foundRight) {
                break;
            }
        }
        
        for (int x = right - 1; x >= left; x--) {
            for (int y = src.rows() - 1; y > bottom; y--) {
                if (src(y, x)) {
                    bottom = y;
                    break;
                }
            }
        }
        
        return Rect { size_t(left), size_t(top), size_t(right - left + 1), size_t(bottom - top + 1) };
    }
    
    bool isMaskBorder(const Mat_8UC1 &mask, const size_t x, const size_t y)
    {
        return mask(y, x) > 0 &&
	       (x == 0 || x == mask.cols() - 1 || y == 0 || y == mask.rows() - 1 ||
            mask(std::max(y - 1, size_t(0)), x) == 0 ||
            mask(std::min(y + 1, size_t(mask.rows() - 1)), x) == 0 ||
            mask(y, std::max(x - 1, size_t(0))) == 0 ||
            mask(y, std::min(x + 1, size_t(mask.cols() -1))) == 0);
    }
    
    Shape makeShapeWithMask(const Mat_bool &mask)
    {
        const Rect convexRect = calcConvexRect(mask);
        
        Shape s = { .leftTop = convexRect.tl(), .mask = Mat_bool(convexRect.height, convexRect.width) };
        
        for (size_t x = 0; x < s.mask.cols(); x++) {
            for (size_t y = 0; y < s.mask.rows(); y++) {
                s.mask(y, x) = mask(s.leftTop.y + y, s.leftTop.x + x);
            }
        }
        
        return s;
    }
    
    std::vector<Shape> splitShapeArray(const Mat_16UC1 &src)
    {
        std::vector<Shape> ret;
        
        for (size_t currentId = 1; true; currentId++) {
            
            bool found = false;
            Mat_bool currentShapeMask = Mat_bool::Constant(src.rows(), src.cols(), false);
            
            for (size_t x = 0; x < src.cols(); x++) {
                
                bool foundCol = false;
                
                for (size_t y = 0; y < src.rows(); y++) {
                    if (src(y, x) == currentId) {
                        foundCol = true;
                        currentShapeMask(y, x) = true;
                    }
                }
                
                if (foundCol) {
                    found = true;
                } else if (found) {
                    break;
                }
            }
            
            if (!found) {
                break;
            }
            
            ret.push_back(makeShapeWithMask(currentShapeMask));
        }
        
        return ret;
    }
    
    void dct(std::vector<double> &v)
    {
        if (v.size() == 0) {
            return;
        }
        
        const fftw_plan plan = fftw_plan_r2r_1d(v.size(), v.data(), v.data(), FFTW_REDFT10, FFTW_ESTIMATE);
        
        fftw_execute(plan);
        
        fftw_destroy_plan(plan);
        
        for (double &x : v) {
            x /= 2 * v.size();
        }
    }
    
    void idct(std::vector<double> &v)
    {
        if (v.size() == 0) {
            return;
        }
        
        const fftw_plan plan = fftw_plan_r2r_1d(v.size(), v.data(), v.data(), FFTW_REDFT01, FFTW_ESTIMATE);
        
        fftw_execute(plan);
        
        fftw_destroy_plan(plan);
    }
    
    Mat_64FC3 makeShapeTexture(const Mat_8UC3 &rgb_img, const Shape &shape)
    {
        static const size_t strideY = 4;
        
        const size_t rows = shape.mask.rows();
        const size_t cols = shape.mask.cols();
        
        Mat_64FC3 YCrCbImg = rgb2ycbcr(rgb_img.block(shape.leftTop.y, shape.leftTop.x, rows, cols));
        
        std::vector<Mat_64FC1> YCrCbChannels = split(YCrCbImg);
        
        std::vector<std::vector<size_t>> colsPos(cols);
        std::vector<std::vector<size_t>> rowsPos(rows);
        
        for (size_t x = 0; x < cols; x++) {
            
            size_t row = 0;
            
            for (size_t y = 0; y < rows; y++) {
                if (shape.mask(y, x) != 0) {
                    rowsPos[row].push_back(x);
                    colsPos[x].push_back(y);
                    row++;
                }
            }
        }
        
        for (auto &channel : YCrCbChannels) {
            
            std::vector<std::vector<double>> dct1(cols);
            
            for (size_t x = 0; x < cols; x++) {
                
                std::vector<double> col;
                
                for (const size_t y : colsPos[x]) {
                    col.push_back(channel(y, x));
                }
                
                dct(col);
                
                for (size_t y = 0; y < col.size(); y++) {
                    dct1[x].push_back(col[y]);
                }
            }
            
            Mat_64FC1 dct2 = Mat_64FC1::Zero(rows, cols);
            
            for (size_t yi = 0; yi * strideY < rows; yi++) {
                
                const size_t startY = yi * strideY;
                const size_t endY = std::min(startY + strideY, rows);
                size_t endX = 0;
                
                std::vector<std::vector<double>> rows(endY - startY);
                
                for (size_t y = startY; y < endY; y++) {
                    
                    for (const size_t pos : rowsPos[y]) {
                        rows[y - startY].push_back(dct1[pos][y]);
                    }
                    
                    dct(rows[y - startY]);
                    endX = std::max(endX, rows[y - startY].size());
                }
                
                for (size_t x = 0; x < endX; x++) {
                    for (size_t y = startY; y < endY; y++) {
                        if (x < rows[y - startY].size()) {
                            dct2(y, x) = rows[y - startY][x];
                        }
                    }
                }
            }
            
            channel = dct2;
        }
        
        return merge<3>(YCrCbChannels);
    }
    
    Mat_8UC3 solveTexture(const Shape &shape, const Mat_64FC3 &YCrCbImg)
    {
        static const size_t strideY = 4;
        
        assert(shape.mask.cols() == YCrCbImg.cols());
        assert(shape.mask.rows() == YCrCbImg.rows());
        
        const size_t rows = shape.mask.rows();
        const size_t cols = shape.mask.cols();
        
        std::vector<Mat_64FC1> YCrCbChannels = split(YCrCbImg);
        
        std::vector<std::vector<size_t>> colsPos(cols);
        std::vector<std::vector<size_t>> rowsPos(rows);
        
        for (size_t x = 0; x < cols; x++) {
            
            size_t row = 0;
            
            for (size_t y = 0; y < rows; y++) {
                if (shape.mask(y, x) != 0) {
                    colsPos[x].push_back(y);
                    rowsPos[row].push_back(x);
                    row++;
                }
            }
        }
        
        for (auto &channel : YCrCbChannels) {
            
            std::vector<std::vector<double>> idct1(cols);
            
            for (size_t yi = 0; yi * strideY < rows; yi++) {
                
                const size_t startY = yi * strideY;
                const size_t endY   = std::min(rows, startY + strideY);
                const size_t endX   = std::max_element(&rowsPos[startY], &rowsPos[endY], [] (const auto &v1, const auto &v2) { return v1.size() < v2.size(); })->size();
                
                std::vector<std::vector<double>> rows(endY - startY);
                
                for (size_t x = 0; x < endX; x++) {
                    for (size_t y = startY; y < endY; y++) {
                        if (x < rowsPos[y].size()) {
                            rows[y - startY].push_back(channel(y, x));
                        }
                    }
                }
                
                for (auto &v : rows) {
                    idct(v);
                }
                
                for (size_t y = startY; y < endY; y++) {
                    for (size_t pos = 0; pos < rowsPos[y].size(); pos++) {
                        
                        const size_t x = rowsPos[y][pos];
                        
                        assert(idct1[x].size() == y);
                        
                        idct1[x].push_back(rows[y - startY][pos]);
                    }
                }
            }
            
            Mat_64FC1 originalChannel = Mat_64FC1::Zero(rows, cols);
            
            for (size_t x = 0; x < cols; x++) {
                
                idct(idct1[x]);
                
                for (size_t y = 0; y < colsPos[x].size(); y++) {
                    originalChannel(colsPos[x][y], x) = idct1[x][y];
                }
            }
            
            channel = originalChannel;
        }
        
        const Mat_64FC3 originalYCrCbImg = merge<3>(YCrCbChannels);
        const Mat_8UC3 bgrImg = ycbcr2rgb(originalYCrCbImg);
        
        return bgrImg;
    }
    
    std::vector<Vec3d> zigzagSerialize(const Shape &shape, const Mat_64FC3 &img)
    {
        assert(shape.mask.rows() == img.rows());
        assert(shape.mask.cols() == img.cols());
        
        const size_t rows = shape.mask.rows();
        const size_t cols = shape.mask.cols();
        
        std::vector<size_t> rowCnt(rows, 0);
        
        for (size_t x = 0; x < cols; x++) {
            
            size_t pos = 0;
            
            for (size_t y = 0; y < rows; y++) {
                if (shape.mask(y, x) != 0) {
                    rowCnt[pos]++;
                    pos++;
                }
            }
        }
        
        std::vector<Vec3d> zVec;
        
        for (size_t d = 0; d < rows + cols - 1; d++) {
            if (d % 2 == 0) {
                for (size_t x = std::max(d, rows - 1) - rows + 1; x <= std::min(d, cols - 1); x++) {
                    
                    const size_t y = d - x;
                    
                    if (x < rowCnt[y]) {
                        zVec.push_back(img(y, x));
                    }
                }
            } else {
                for (size_t y = std::max(d, cols - 1) - cols + 1; y <= std::min(d, rows - 1); y++) {
                    
                    const size_t x = d - y;
                    
                    if (x < rowCnt[y]) {
                        zVec.push_back(img(y, x));
                    }
                }
            }
        }
        
        return zVec;
    }
    
    Mat_64FC3 zigzagDeserialize(const Shape &shape, const std::vector<Vec3d> &zVec)
    {
        const size_t rows = shape.mask.rows();
        const size_t cols = shape.mask.cols();
        
        std::vector<size_t> rowCnt(rows, 0);
        
        for (size_t x = 0; x < cols; x++) {
            
            size_t pos = 0;
            
            for (size_t y = 0; y < rows; y++) {
                if (shape.mask(y, x) != 0) {
                    rowCnt[pos]++;
                    pos++;
                }
            }
        }
        
        Mat_64FC3 img = Mat_64FC3::Constant(rows, cols, Vec3d::Zero());
        size_t pos = 0;
        
        for (size_t d = 0; d < rows + cols - 1; d++) {
            if (d % 2 == 0) {
                for (size_t x = std::max(d, rows - 1) - rows + 1; x <= std::min(d, cols - 1); x++) {
                    
                    const size_t y = d - x;
                    
                    if (x < rowCnt[y]) {
                        img(y, x) = zVec[pos];
                        pos++;
                    }
                }
            } else {
                for (size_t y = std::max(d, cols - 1) - cols + 1; y <= std::min(d, rows - 1); y++) {
                    
                    const size_t x = d - y;
                    
                    if (x < rowCnt[y]) {
                        img(y, x) = zVec[pos];
                        pos++;
                    }
                }
            }
        }
        
        return img;
    }
    
    
    Mat_64FC3 makeShapeTextureYcbcr(const Mat_64FC3 &src, const Shape &shape, DCTcontext *context)
    {
        static const size_t strideY = 4;
        
        const size_t rows = shape.mask.rows();
        const size_t cols = shape.mask.cols();
        
        Mat_64FC3 YCrCbImg = src.block(shape.leftTop.y, shape.leftTop.x, rows, cols);
        
        std::vector<Mat_64FC1> YCrCbChannels = split(YCrCbImg);
        
        std::vector<std::vector<size_t>> colsPos(cols);
        std::vector<std::vector<size_t>> rowsPos(rows);
        
        for (size_t x = 0; x < cols; x++) {
            
            size_t row = 0;
            
            for (size_t y = 0; y < rows; y++) {
                if (shape.mask(y, x) != 0) {
                    rowsPos[row].push_back(x);
                    colsPos[x].push_back(y);
                    row++;
                }
            }
        }
        
        for (auto &channel : YCrCbChannels) {
            
            std::vector<std::vector<double>> dct1(cols);
            
            for (size_t x = 0; x < cols; x++) {
                
                std::vector<double> col;
                
                for (const size_t y : colsPos[x]) {
                    col.push_back(channel(y, x));
                }
                
//                dct(col);
                
                std::vector<double> out(col.size());
                DCT(context, col, out);
                col=out;
                
                for (size_t y = 0; y < col.size(); y++) {
                    dct1[x].push_back(col[y]);
                }
            }
            
            Mat_64FC1 dct2 = Mat_64FC1::Zero(rows, cols);
            
            for (size_t yi = 0; yi * strideY < rows; yi++) {
                
                const size_t startY = yi * strideY;
                const size_t endY = std::min(startY + strideY, rows);
                size_t endX = 0;
                
                std::vector<std::vector<double>> rows(endY - startY);
                
                for (size_t y = startY; y < endY; y++) {
                    
                    for (const size_t pos : rowsPos[y]) {
                        rows[y - startY].push_back(dct1[pos][y]);
                    }
                    
//                    dct(rows[y - startY]);
                    std::vector<double> out(rows[y - startY].size());
                    DCT(context, rows[y - startY], out);
                    rows[y-startY]=out;
                    
                    endX = std::max(endX, rows[y - startY].size());
                }
                
                for (size_t x = 0; x < endX; x++) {
                    for (size_t y = startY; y < endY; y++) {
                        if (x < rows[y - startY].size()) {
                            dct2(y, x) = rows[y - startY][x];
                        }
                    }
                }
            }
            
            channel = dct2;
        }
        
        return merge<3>(YCrCbChannels);
    }

    Mat_64FC3 solveTextureYcbcr(const Shape &shape, const Mat_64FC3 &YCrCbImg, IDCTcontext * icontext)
    {
        static const size_t strideY = 4;
        
        assert(shape.mask.cols() == YCrCbImg.cols());
        assert(shape.mask.rows() == YCrCbImg.rows());
        
        const size_t rows = shape.mask.rows();
        const size_t cols = shape.mask.cols();
        
        std::vector<Mat_64FC1> YCrCbChannels = split(YCrCbImg);
        
        std::vector<std::vector<size_t>> colsPos(cols);
        std::vector<std::vector<size_t>> rowsPos(rows);
        
        for (size_t x = 0; x < cols; x++) {
            
            size_t row = 0;
            
            for (size_t y = 0; y < rows; y++) {
                if (shape.mask(y, x) != 0) {
                    colsPos[x].push_back(y);
                    rowsPos[row].push_back(x);
                    row++;
                }
            }
        }
        
        for (auto &channel : YCrCbChannels) {
            
            std::vector<std::vector<double>> idct1(cols);
            
            for (size_t yi = 0; yi * strideY < rows; yi++) {
                
                const size_t startY = yi * strideY;
                const size_t endY   = std::min(rows, startY + strideY);
                const size_t endX   = std::max_element(&rowsPos[startY], &rowsPos[endY], [] (const auto &v1, const auto &v2) { return v1.size() < v2.size(); })->size();
                
                std::vector<std::vector<double>> rows(endY - startY);
                
                for (size_t x = 0; x < endX; x++) {
                    for (size_t y = startY; y < endY; y++) {
                        if (x < rowsPos[y].size()) {
                            rows[y - startY].push_back(channel(y, x));
                        }
                    }
                }
                
                
                
                for (auto &v : rows) {
//                    idct(v);
                    std::vector<int> data(v.size());
                    for(int i=0;i<v.size();i++)
                    {
                        data[i]=v[i];
                    }
                    double out[64];
                    IDCT(icontext, data, out);
                    for(int i=0;i<v.size();i++)
                    {
                        v[i]=out[i];
                    }
                    
                    
                }
                
                for (size_t y = startY; y < endY; y++) {
                    for (size_t pos = 0; pos < rowsPos[y].size(); pos++) {
                        
                        const size_t x = rowsPos[y][pos];
                        
                        assert(idct1[x].size() == y);
                        
                        idct1[x].push_back(rows[y - startY][pos]);
                    }
                }
            }
            
            Mat_64FC1 originalChannel = Mat_64FC1::Zero(rows, cols);
            
            for (size_t x = 0; x < cols; x++) {
                
//                idct(idct1[x]);
                
                std::vector<int> data(idct1[x].size());
                for(int i=0;i<data.size();i++)
                {
                    data[i]=idct1[x][i];
                }
                double out[64];
                IDCT(icontext, data, out);
                for(int i=0;i<data.size();i++)
                {
                    idct1[x][i]=out[i];
                }
                
                
                for (size_t y = 0; y < colsPos[x].size(); y++) {
                    originalChannel(colsPos[x][y], x) = idct1[x][y];
                }
            }
            
            channel = originalChannel;
        }
        
        const Mat_64FC3 originalYCrCbImg = merge<3>(YCrCbChannels);
        return originalYCrCbImg;
    }

    
    
    //mask encode
    std::vector<uint8_t> encode_mask(const Mat_bool &mask)
    {
        const size_t rows = mask.rows();
        const size_t cols = mask.cols();
        
        /*
         * +-----------------------+    +-----------------------+
         * | first: true  -> false |    | first: true  -> false |
         * +-----------------------+ -> +-----------------------+ -> ...
         * | second: false -> true |    | second: false -> true |
         * +-----------------------+    +-----------------------+
         *
         * NOTICE:
         *   For the final level, the second mat is unused.
         */
        std::list<std::pair<Mat_bool, Mat_bool>> levels(1);
        
        levels.back().first = mask;
        
        for (size_t current_rows = rows, current_cols = cols; current_rows > 1 && current_cols > 1; current_rows = (current_rows + 1) >> 1, current_cols = (current_cols + 1) >> 1) {
            
            const size_t next_rows = (current_rows + 1) >> 1;
            const size_t next_cols = (current_cols + 1) >> 1;
            
            if (next_rows > 0 && next_cols > 0) {
                // need down-sampling
                levels.back().second = Mat_bool::Constant(current_rows, current_cols, false);
                
                std::pair<Mat_bool, Mat_bool> next_level;
                
                next_level.first = Mat_bool::Constant(next_rows, next_cols, false);
                
                for (size_t x = 0; x < next_cols; x++) {
                    for (size_t y = 0; y < next_rows; y++) {
                        
                        const size_t min_x = x * 2;
                        const size_t min_y = y * 2;
                        const size_t max_x = std::min(current_cols - 1, x * 2 + 1);
                        const size_t max_y = std::min(current_rows - 1, y * 2 + 1);
                        
                        uint8_t true_cnt = 0;
                        
                        for (size_t curr_x = min_x; curr_x <= max_x; curr_x++) {
                            for (size_t curr_y = min_y; curr_y <= max_y; curr_y++) {
                                true_cnt += levels.back().first(curr_y, curr_x) ? 1 : 0;
                            }
                        }
                        
                        if (true_cnt >= 2) {
                            
                            next_level.first(y, x) = true;
                            
                            for (size_t curr_x = min_x; curr_x <= max_x; curr_x++) {
                                for (size_t curr_y = min_y; curr_y <= max_y; curr_y++) {
                                    levels.back().second(curr_y, curr_x) = !levels.back().first(curr_y, curr_x);
                                    levels.back().first(curr_y, curr_x) = false;
                                }
                            }
                        }
                    }
                }
                
                levels.push_back(std::move(next_level));
            }
        }
        
        /*
         * NOTICE:
         *   After `reverse()`, the second mat of the first level is unused.
         */
        levels.reverse();
        
        std::vector<bool> mask_level_mat_vec;
        
        for (const auto &level : levels) {
            
            for (size_t x = 0; x < level.first.cols(); x++) {
                for (size_t y = 0; y < level.first.rows(); y++) {
                    mask_level_mat_vec.push_back(level.first(y, x));
                }
            }
            
            for (size_t x = 0; x < level.second.cols(); x++) {
                for (size_t y = 0; y < level.second.rows(); y++) {
                    mask_level_mat_vec.push_back(level.second(y, x));
                }
            }
        }
        
        std::vector<uint8_t> encoded_data;
        const size_t mask_level_mat_vec_len = mask_level_mat_vec.size();
        
        entropyCodec::encode(mask_level_mat_vec, encoded_data);
        
        return encoded_data;
    }
    
    Mat_bool decode_mask(const size_t rows, const size_t cols, const std::vector<uint8_t> &encoded_data)
    {
        /*
         * each pair in `levels_size`:
         *   .first  <- rows
         *   .second <- cols
         */
        std::vector<std::pair<size_t, size_t>> levels_size;
        size_t mask_level_mat_vec_len = rows * cols;
        
        levels_size.push_back(std::make_pair(rows, cols));
        
        for (size_t current_rows = rows, current_cols = cols; current_rows > 1 && current_cols > 1; current_rows = (current_rows + 1) >> 1, current_cols = (current_cols + 1) >> 1) {
            
            const size_t next_rows = (current_rows + 1) >> 1;
            const size_t next_cols = (current_cols + 1) >> 1;
            
            if (next_rows > 0 && next_cols > 0) {
                // need down-sampling
                levels_size.push_back(std::make_pair(next_rows, next_cols));
                mask_level_mat_vec_len += current_rows * current_cols + next_rows * next_cols;
            }
        }
        
        std::vector<bool> decoded_data;
        
        entropyCodec::decode(encoded_data, mask_level_mat_vec_len, decoded_data);
        
        Mat_bool current_level_mat(levels_size.back().first, levels_size.back().second);
        size_t cnt = 0;
        
        for (size_t x = 0; x < current_level_mat.cols(); x++) {
            for (size_t y = 0; y < current_level_mat.rows(); y++) {
                current_level_mat(y, x) = decoded_data[cnt];
                cnt++;
            }
        }
        
        for (auto level_size = std::next(levels_size.crbegin()); level_size != levels_size.crend(); level_size++) {
            
            Mat_bool next_edge1(level_size->first, level_size->second);
            Mat_bool next_edge2(level_size->first, level_size->second);
            
            for (size_t x = 0; x < next_edge1.cols(); x++) {
                for (size_t y = 0; y < next_edge1.rows(); y++) {
                    next_edge1(y, x) = decoded_data[cnt];
                    cnt++;
                }
            }
            
            for (size_t x = 0; x < next_edge2.cols(); x++) {
                for (size_t y = 0; y < next_edge2.rows(); y++) {
                    next_edge2(y, x) = decoded_data[cnt];
                    cnt++;
                }
            }
            
            for (size_t x = 0; x < current_level_mat.cols(); x++) {
                for (size_t y = 0; y < current_level_mat.rows(); y++) {
                    if (current_level_mat(y, x) == 1) {
                        
                        const size_t min_next_x = x * 2;
                        const size_t min_next_y = y * 2;
                        const size_t max_next_x = std::min(size_t(next_edge1.cols()) - 1, x * 2 + 1);
                        const size_t max_next_y = std::min(size_t(next_edge1.rows()) - 1, y * 2 + 1);
                        
                        for (size_t next_x = min_next_x; next_x <= max_next_x; next_x++) {
                            for (size_t next_y = min_next_y; next_y <= max_next_y; next_y++) {
                                next_edge1(next_y, next_x) = !next_edge2(next_y, next_x);
                            }
                        }
                    }
                }
            }
            
            current_level_mat = std::move(next_edge1);
        }
        
        return current_level_mat;
    }
    
    
}
