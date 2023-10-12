/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 (Seeed Technology Inc.)
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

#include <cstdarg>
#include <cstdint>
#include <cstdlib>

#include "el_board_config.h"
#include "el_common.h"

EL_ATTR_WEAK void el_sleep(uint32_t ms) { board_delay_ms(ms); }

EL_ATTR_WEAK uint64_t el_get_time_ms(void) {
#ifdef FREERTOS
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
#else
    uint32_t tick = 0;
    uint32_t time = 0;
    SystemGetTick(&tick, &time);
    return (uint64_t)time;
#endif
}

EL_ATTR_WEAK uint64_t el_get_time_us(void) {
#ifdef FREERTOS
    return xTaskGetTickCount() * portTICK_PERIOD_MS * 1000;
#else;
    return el_get_time_ms() * 1000;
#endif
}

EL_ATTR_WEAK int el_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    xvprintf(fmt, args);
    va_end(args);
    return 0;
}

EL_ATTR_WEAK int el_putchar(char c) { return 0; }

EL_ATTR_WEAK void* el_malloc(size_t size) {
#ifdef FREERTOS
    return pvPortMalloc(size);
#else
    return malloc(size);
#endif
}

EL_ATTR_WEAK void* el_calloc(size_t nmemb, size_t size) {
#ifdef FREERTOS
    return pvPortMalloc(nmemb * size);
#else
    return calloc(nmemb, size);
#endif
}

EL_ATTR_WEAK void el_free(void* ptr) {
#ifdef FREERTOS
    vPortFree(ptr);
#else
    free(ptr);
#endif
}

EL_ATTR_WEAK void el_reset(void) { exit(0); }

EL_ATTR_WEAK void el_status_led(bool on) { xprintf("TEST LED STAT: %s", on ? "on" : "off"); }

#ifdef FREERTOS
EL_ATTR_WEAK void* operator new[](size_t size) { return pvPortMalloc(size); }

EL_ATTR_WEAK void operator delete[](void* p) { vPortFree(p); }

EL_ATTR_WEAK void* operator new(size_t size) { return pvPortMalloc(size); }

EL_ATTR_WEAK void operator delete(void* p) { vPortFree(p); }
#endif
