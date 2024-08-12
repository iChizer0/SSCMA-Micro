#ifndef _MA_ENGINE_BASE_H_
#define _MA_ENGINE_BASE_H_

#include <cstdint>
#include <vector>
#include <forward_list>

#include "core/ma_common.h"

#if MA_USE_FILESYSTEM
#include <fstream>
#include <iostream>
#endif

namespace ma::engine {

class Engine {
public:
    Engine()          = default;
    virtual ~Engine() = default;

    virtual ma_err_t init()                        = 0;
    virtual ma_err_t init(size_t size)             = 0;
    virtual ma_err_t init(void* pool, size_t size) = 0;

    virtual ma_err_t run() = 0;

    virtual ma_err_t loadModel(const void* model_data, size_t model_size) = 0;
#if MA_USE_FILESYSTEM
    virtual ma_err_t loadModel(const char* model_path) = 0;
    virtual ma_err_t loadModel(const std::string& model_path) = 0;
#endif

    virtual int32_t getInputSize()  = 0;
    virtual int32_t getOutputSize() = 0;

    virtual ma_tensor_t getInput(int32_t index)                 = 0;
    virtual ma_tensor_t getOutput(int32_t index)                = 0;
    virtual ma_shape_t getInputShape(int32_t index)             = 0;
    virtual ma_shape_t getOutputShape(int32_t index)            = 0;
    virtual ma_quant_param_t getInputQuantParam(int32_t index)  = 0;
    virtual ma_quant_param_t getOutputQuantParam(int32_t index) = 0;

#if MA_USE_ENGINE_TENSOR_NAME
    virtual int32_t getInputNum(const char* name)  = 0;
    virtual int32_t getOutputNum(const char* name) = 0;
    // virtual ma_tensor_t      getInput(const char* name)              = 0;
    // virtual ma_tensor_t      getOutput(const char* name)             = 0;
    // virtual ma_shape_t       getInputShape(const char* name)        = 0;
    // virtual ma_shape_t       getOutputShape(const char* name)       = 0;
    // virtual ma_quant_param_t getInputQuantParam(const char* name)  = 0;
    // virtual ma_quant_param_t getOutputQuantParam(const char* name) = 0;
#endif

    static std::vector<ma_model_t>& getModels() {
        return m_models;
    }

    static std::vector<ma_model_t>& findModels(const char* address, size_t size = 0);
    static std::vector<ma_model_t> m_models;
};

}  // namespace ma::engine

#endif