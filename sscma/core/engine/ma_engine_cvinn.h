// #ifndef _MA_ENGINE_CVINN_H_
// #define _MA_ENGINE_CVINN_H_

// #include <cstddef>
// #include <cstdint>

// #include <cviruntime.h>

// #include "core/ma_types.h"
// #include "ma_engine_base.h"

// namespace ma::engine {

// class EngineCVINNNN final : public base::BaseEngine {
// public:
//     EngineCVINN();
//     ~EngineCVINN();

//     ma_err_t init() override;
//     ma_err_t init(size_t size) override;
//     ma_err_t init(void* pool, size_t size) override;

//     ma_err_t run() override;

// #if MA_USE_FILESYSTEM
//     ma_err_t load_model(const char* model_path) override;
// #endif

//     ma_err_t           load_model(const void* model_data, size_t model_size) override;

//     ma_err_t           set_input(size_t index, const void* input_data, size_t input_size) override;
//     void*              get_input(size_t index) override;
//     void*              get_output(size_t index) override;

//     virtual ma_shape_t get_input_shape(size_t index) const override;
//     virtual ma_shape_t get_output_shape(size_t index) const override;
//     virtual ma_quant_param_t get_input_quant_param(size_t index) const override;
//     virtual ma_quant_param_t get_output_quant_param(size_t index) const override;


// private:
//     const CVI_MODEL_HANDLE* model;
// };

// }  // namespace ma::engine

// #endif  // _MA_ENGINE_CVINN_H_