#pragma once

#include <cstdint>
#include <cstring>
#include <functional>
#include <string>

#include "core/ma_core.h"
#include "porting/ma_porting.h"
#include "resource.hpp"

static int32_t                _mqtt_status = 0;
static ma_mqtt_config_t       _mqtt_server_config{};
static ma_mqtt_topic_config_t _mqtt_topic_config{};

namespace ma::server::callback {






}