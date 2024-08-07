#ifndef _MA_MODEL_IMCLS_H
#define _MA_MODEL_IMCLS_H

#include "ma_model_detector.h"

namespace ma::model {

class ImCls : public Detector {
   private:
    ma_tensor_t output_;

   protected:
    ma_err_t postprocess() override;

    ma_err_t postProcessI8();
    ma_err_t postProcessF32();

   public:
    ImCls(Engine* engine);
    ~ImCls();

    static bool isValid(Engine* engine);

    static const char* getTag();
};

}  // namespace ma::model

#endif  // _MA_MODEL_YOLO_H