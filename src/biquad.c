#include "biquad.h"
#include <math.h>

void biquad_notch_init(biquad_t *f, float f0, float fs, float Q) {
    // 计算标准形式陷波器系数
    float w0 = 2.0f * M_PI * f0 / fs;
    float alpha = sinf(w0) / (2.0f * Q);

    float cosw0 = cosf(w0);
    float b0 =  1.0f;
    float b1 = -2.0f * cosw0;
    float b2 =  1.0f;
    float a0 =  1.0f + alpha;
    float a1 = -2.0f * cosw0;
    float a2 =  1.0f - alpha;

    // 归一化
    f->b0 = b0 / a0;
    f->b1 = b1 / a0;
    f->b2 = b2 / a0;
    f->a1 = a1 / a0;
    f->a2 = a2 / a0;

    // 清空状态
    f->x1 = f->x2 = f->y1 = f->y2 = 0.0f;
}

float biquad_process(biquad_t *f, float x) {
    // y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2]
    //        - a1*y[n-1] - a2*y[n-2]
    float y = f->b0*x + f->b1*f->x1 + f->b2*f->x2
                    - f->a1*f->y1 - f->a2*f->y2;
    // 更新延迟线
    f->x2 = f->x1;
    f->x1 = x;
    f->y2 = f->y1;
    f->y1 = y;
    return y;
}
