#ifndef BIQUAD_H
#define BIQUAD_H

#include <stdint.h>

// 一个二阶 IIR 滤波器状态结构
typedef struct {
    // 系数
    float b0, b1, b2;
    float a1, a2;
    // 延迟线
    float x1, x2; // 上一次、上上次输入
    float y1, y2; // 上一次、上上次输出
} biquad_t;

/**
 * @brief  初始化一个陷波器
 * @param  f0   陷波中心频率 (Hz)
 * @param  fs   采样率 (Hz)
 * @param  Q    品质因数 (推荐 5~20，Q 越大带宽越窄)
 * @param  filter 指向 biquad_t 实例
 */
void biquad_notch_init(biquad_t *filter, float f0, float fs, float Q);

/**
 * @brief  对一个样本做滤波
 * @param  filter 已初始化的 biquad_t
 * @param  x      当前输入
 * @return        滤波后的输出
 */
float biquad_process(biquad_t *filter, float x);

#endif // BIQUAD_H
