#ifndef _S_DCT_
#define _S_DCT_

#define SIZE sizeof(double)

#include <vector>

typedef struct {
    std::vector<float> DctContext;
     int start[64];
} DCTcontext;

void InitDctContext(char* src, DCTcontext * context);  //input table and generate a context
void DCT(DCTcontext *context, std::vector<double> data, std::vector<double> &output); // input context
void destroyDCTcontext(DCTcontext *context);

#endif // !_S_DCT_
