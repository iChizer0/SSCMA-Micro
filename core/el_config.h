/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Hongtai Liu (Seeed Technology Inc.)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef _EL_CONFIG_H_
#define _EL_CONFIG_H_

/* thirdparty */
#define CONFIG_EL_LIB_FLASHDB
#define CONFIG_EL_LIB_JPEGENC

/* os related config */
#define CONFIG_EL_HAS_FREERTOS_SUPPORT

/* device related config */
// #define CONFIG_EL_TARGET_ESPPRESSIF
#ifdef CONFIG_EL_TARGET_ESPPRESSIF
    // #define CONFIG_EL_BOARD_ESPRESSIF_ESP32S3_EYE
    // #define CONFIG_EL_BOARD_SEEED_XIAO_ESP32S3
#endif
// #define CONFIG_EL_TARGET_HIMAX
#ifdef CONFIG_EL_TARGET_HIMAX
// #define CONFIG_EL_BOARD_WE_1
#endif

/* engine related config */
// #define CONFIG_EL_TFLITE
#ifdef CONFIG_EL_TFLITE
    // #define CONFIG_EL_TFLITE_OP_ABS
    // #define CONFIG_EL_TFLITE_OP_ADD
    // #define CONFIG_EL_TFLITE_OP_ADDN
    // #define CONFIG_EL_TFLITE_OP_ARGMAX
    // #define CONFIG_EL_TFLITE_OP_ARGMIN
    // #define CONFIG_EL_TFLITE_OP_ASSIGN_VARIABLE
    // #define CONFIG_EL_TFLITE_OP_AVERAGE_POOL_2D
    // #define CONFIG_EL_TFLITE_OP_BATCH_TO_SPACE_ND
    // #define CONFIG_EL_TFLITE_OP_BROADCAST_ARGS
    // #define CONFIG_EL_TFLITE_OP_BROADCAST_TO
    // #define CONFIG_EL_TFLITE_OP_CALL_ONCE
    // #define CONFIG_EL_TFLITE_OP_CAST
    // #define CONFIG_EL_TFLITE_OP_CEIL
    // #define CONFIG_EL_TFLITE_OP_CIRULAR_BUFFER
    // #define CONFIG_EL_TFLITE_OP_CONCATENATION
    // #define CONFIG_EL_TFLITE_OP_CONV_2D
    // #define CONFIG_EL_TFLITE_OP_COS
    // #define CONFIG_EL_TFLITE_OP_CUM_SUM
    // #define CONFIG_EL_TFLITE_OP_DEPTH_TO_SPACE
    // #define CONFIG_EL_TFLITE_OP_DEPTHWISE_CONV_2D
    // #define CONFIG_EL_TFLITE_OP_DEQUANTIZE
    // #define CONFIG_EL_TFLITE_OP_DETECTION_POSTPROCESS
    // #define CONFIG_EL_TFLITE_OP_DIV
    // #define CONFIG_EL_TFLITE_OP_ELU
    // #define CONFIG_EL_TFLITE_OP_EQUAL
    // #define CONFIG_EL_TFLITE_ETHOS_U
    // #define CONFIG_EL_TFLITE_OP_EXP
    // #define CONFIG_EL_TFLITE_OP_EXPAND_DIMS
    // #define CONFIG_EL_TFLITE_OP_FILL
    // #define CONFIG_EL_TFLITE_OP_FLOOR
    // #define CONFIG_EL_TFLITE_OP_FLOOR_DIV
    // #define CONFIG_EL_TFLITE_OP_FLOOR_MOD
    // #define CONFIG_EL_TFLITE_OP_FULLY_CONNECTED
    // #define CONFIG_EL_TFLITE_OP_GATHER
    // #define CONFIG_EL_TFLITE_OP_GATHER_ND
    // #define CONFIG_EL_TFLITE_OP_GREATER
    // #define CONFIG_EL_TFLITE_OP_GREATER_EQUAL
    // #define CONFIG_EL_TFLITE_OP_HARD_SWISH
    // #define CONFIG_EL_TFLITE_OP_L2_NORMALIZATION
    // #define CONFIG_EL_TFLITE_OP_L2_POOL_2D
    // #define CONFIG_EL_TFLITE_OP_LEAKY_RELU
    // #define CONFIG_EL_TFLITE_OP_LESS
    // #define CONFIG_EL_TFLITE_OP_LESS_EQUAL
    // #define CONFIG_EL_TFLITE_OP_LOG
    // #define CONFIG_EL_TFLITE_OP_LOGICAL_AND
    // #define CONFIG_EL_TFLITE_OP_LOGICAL_NOT
    // #define CONFIG_EL_TFLITE_OP_LOGICAL_OR
    // #define CONFIG_EL_TFLITE_OP_LOGISTIC
    // #define CONFIG_EL_TFLITE_OP_LOG_SOFTMAX
    // #define CONFIG_EL_TFLITE_OP_MAX_POOL_2D
    // #define CONFIG_EL_TFLITE_OP_MAXIMUM
    // #define CONFIG_EL_TFLITE_OP_MEAN
    // #define CONFIG_EL_TFLITE_OP_MINIMUM
    // #define CONFIG_EL_TFLITE_OP_MIRROR_PAD
    // #define CONFIG_EL_TFLITE_OP_MUL
    // #define CONFIG_EL_TFLITE_OP_NEG
    // #define CONFIG_EL_TFLITE_OP_NOT_EQUAL
    // #define CONFIG_EL_TFLITE_OP_PACK
    // #define CONFIG_EL_TFLITE_OP_PAD
    // #define CONFIG_EL_TFLITE_OP_PADV2
    // #define CONFIG_EL_TFLITE_OP_PRELU
    // #define CONFIG_EL_TFLITE_OP_QUANTIZE
    // #define CONFIG_EL_TFLITE_OP_READ_VARIABLE
    // #define CONFIG_EL_TFLITE_OP_REDUCE_ANY
    // #define CONFIG_EL_TFLITE_OP_RELU
    // #define CONFIG_EL_TFLITE_OP_RELU6
    // #define CONFIG_EL_TFLITE_OP_RESHAPE
    // #define CONFIG_EL_TFLITE_OP_RESIZE_BILINEAR
    // #define CONFIG_EL_TFLITE_OP_RESIZE_NEAREST_NEIGHBOR
    // #define CONFIG_EL_TFLITE_OP_ROUND
    // #define CONFIG_EL_TFLITE_OP_RSQRT
    // #define CONFIG_EL_TFLITE_OP_SELECT_V2
    // #define CONFIG_EL_TFLITE_OP_SHAPE
    // #define CONFIG_EL_TFLITE_OP_SIN
    // #define CONFIG_EL_TFLITE_OP_SLICE
    // #define CONFIG_EL_TFLITE_OP_SOFTMAX
    // #define CONFIG_EL_TFLITE_OP_SPACE_TO_BATCH_ND
    // #define CONFIG_EL_TFLITE_OP_SPACE_TO_DEPTH
    // #define CONFIG_EL_TFLITE_OP_SPLIT
    // #define CONFIG_EL_TFLITE_OP_SPLIT_V
    // #define CONFIG_EL_TFLITE_OP_SQRT
    // #define CONFIG_EL_TFLITE_OP_SQUARE
    // #define CONFIG_EL_TFLITE_OP_SQUARED_DIFFERENCEs
    // #define CONFIG_EL_TFLITE_OP_SQUEEZE
    // #define CONFIG_EL_TFLITE_OP_STRIDED_SLICE
    // #define CONFIG_EL_TFLITE_OP_SUB
    // #define CONFIG_EL_TFLITE_OP_SUM
    // #define CONFIG_EL_TFLITE_SVDF
    // #define CONFIG_EL_TFLITE_OP_TANH
    // #define CONFIG_EL_TFLITE_OP_TRANSPOSE
    // #define CONFIG_EL_TFLITE_OP_TRANSPOSE_CONV
    // #define CONFIG_EL_TFLITE_OP_UNIDIRECTIONAL_SEQUENCE_LSTM
    // #define CONFIG_EL_TFLITE_OP_UNPACK
    // #define CONFIG_EL_TFLITE_OP_VARHANDLE
    // #define CONFIG_EL_TFLITE_OP_WHILE
    // #define CONFIG_EL_TFLITE_OP_ZEROS_LIKE
#endif

#endif
