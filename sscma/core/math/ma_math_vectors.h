#ifndef _MA_MATH_VECTORS_H
#define _MA_MATH_VECTORS_H

#include <cstdint>
#include <cstddef>

namespace ma::math {

void softmax(float* data, size_t size);

void fastSoftmax(float* data, size_t size);

}  // namespace ma::math

#endif