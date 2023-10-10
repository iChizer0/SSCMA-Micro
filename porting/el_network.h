/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Seeed Technology Co.,Ltd
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

#ifndef _EL_NETWORK_H_
#define _EL_NETWORK_H_

#include "core/el_types.h"

typedef enum {
    WL_IDLE_STATUS      = 0,
    WL_NO_SSID_AVAIL    = 1,
    WL_SCAN_COMPLETED   = 2,
    WL_CONNECTED        = 3,
    WL_CONNECT_FAILED   = 4,
    WL_CONNECTION_LOST  = 5,
    WL_DISCONNECTED     = 6
} el_wl_sta_t;

namespace edgelab {

// WIFI-STA for MQTT
class Network {
public:
    Network() : _is_present(false) {}
    virtual ~Network() = default;

    /* WIFI station */
    virtual el_wl_sta_t open(const char* ssid, const char *pwd) = 0;
    virtual el_wl_sta_t close() = 0;
    virtual el_wl_sta_t status() = 0;

    /* MQTT client */
    // virtual el_err_code_t setup(const char *id, const char *user, const char *pass);
    // virtual el_err_code_t connect(const char* server, uint16_t port);
    // virtual el_err_code_t subscribe(const char* topic);
    // virtual el_err_code_t publish(const char* topic, const char* payload);

    /* HTTP client */
    // virtual el_err_code_t request(const char* url, const char* method, const char* payload);
    // virtual el_err_code_t get(const char* url, const char* payload);
    // virtual el_err_code_t post(const char* url, const char* payload);

    operator bool() const { return _is_present; }

protected:
    bool _is_present;
};

}  // namespace edgelab

#endif