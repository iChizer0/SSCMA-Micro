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

namespace edgelab {

class Network {
public:
    Network() : _is_present(false) {}
    virtual ~Network() = default;

    /* Initialize TCP/IP protocol stack */
    virtual el_err_code_t init();
    virtual el_err_code_t deinit();
    /* Connect to network interface */
    virtual el_err_code_t connect(const char* ssid, const char *pwd);
    virtual el_err_code_t close();

    /* TODO: BSD Socket-like API */
    // virtual void socket();
    // virtual bool connect(char* host, unsigned short port);
    // virtual bool close(bool only_visible = true);
    // virtual size_t read(char* data, size_t size);
    // virtual size_t send(char* data, size_t size);

    operator bool() const { return _is_present; }

protected:
    bool _is_present;
};

}  // namespace edgelab

#endif