#ifndef _S_IDCT_
#define _S_IDCT_

#include <vector>

typedef struct {
    std::vector<float> IDctContext;
    int indexMe[64];
    int num1[64];
    int numNe1[64];
    int start[64];

} IDCTcontext;

void InitIDctContext(char* src, IDCTcontext * context);
void IDCT(IDCTcontext *context, std::vector<int> data, double *output);
void destroyIDCTcontext(IDCTcontext *context);



#endif // !_S_IDCT_

