

#include "ma_transport_console.h"

#include <cctype>
#include <cstdint>
#include <cstring>
#include <new>

extern "C" {
#include <console_io.h>
#include <hx_drv_uart.h>
}

#include "core/utils/ma_ringbuffer.hpp"
#include "porting/ma_osal.h"

namespace ma {

static SPSCRingBuffer<char>* _rb_rx   = nullptr;
static SPSCRingBuffer<char>* _rb_tx   = nullptr;
static char*                 _rx_buf  = nullptr;
static char*                 _tx_buf  = nullptr;
static volatile bool         _tx_busy = false;
static Mutex                 _tx_mutex;
static DEV_UART*             _uart      = nullptr;
static volatile bool         _is_opened = false;

static void _uart_dma_recv(void*) {
    if (!_is_opened) {
        return;
    }
    _rb_rx->push(_rx_buf, 1);
    _uart->uart_read_udma(_rx_buf, 1, (void*)_uart_dma_recv);
}

static void _uart_dma_send(void*) {
    if (!_is_opened) {
        _tx_busy = false;
        return;
    }
    size_t remain = _rb_tx->size() < 4095 ? _rb_tx->size() : 4095;
    _tx_busy      = remain != 0;
    if (remain != 0) {
        remain = _rb_tx->pop(_tx_buf, remain);
        SCB_CleanDCache_by_Addr((volatile void*)_tx_buf, remain);
        _uart->uart_write_udma(_tx_buf, remain, (void*)_uart_dma_send);
    }
}

Console::Console() : Transport(MA_TRANSPORT_CONSOLE) { _is_opened = false; }

Console::~Console() {}

ma_err_t Console::open() {
    if (_is_opened) {
        return MA_OK;
    }

    _uart = hx_drv_uart_get_dev(USE_DW_UART_0);
    if (_uart == nullptr) {
        return MA_EIO;
    }

    int ret = _uart->uart_open(UART_BAUDRATE_921600);
    if (ret != 0) {
        return MA_EIO;
    }

    if (_rx_buf == nullptr) {
        _rx_buf = new (std::align_val_t{32}) char[32];
    }

    if (_tx_buf == nullptr) {
        _tx_buf = new (std::align_val_t{32}) char[4096];
    }

    if (_rb_rx == nullptr) {
        _rb_rx = new SPSCRingBuffer<char>(8192);
    }

    if (_rb_tx == nullptr) {
        _rb_tx = new SPSCRingBuffer<char>(48 * 1024);
    }

    if (!_rx_buf || !_tx_buf || !_rb_rx || !_rb_tx) {
        return MA_ENOMEM;
    }

    std::memset(_rx_buf, 0, 32);
    std::memset(_tx_buf, 0, 4096);

    _is_opened = true;

    _uart->uart_read_udma(_rx_buf, 1, (void*)_uart_dma_recv);

    return MA_OK;
}

ma_err_t Console::close() {
    if (!_is_opened) {
        return MA_OK;
    }

    _is_opened = false;
    while (_tx_busy) {
        ma::Thread::yield();
    }

    if (_uart) {
        int ret = _uart->uart_close();
        if (ret != 0) {
            return MA_EIO;
        }

        ret = hx_drv_uart_deinit(USE_DW_UART_0);
        if (ret != 0) {
            return MA_EIO;
        }
    }

    if (_rb_rx) {
        delete _rb_rx;
        _rb_rx = nullptr;
    }

    if (_rb_tx) {
        delete _rb_tx;
        _rb_tx = nullptr;
    }

    if (_rx_buf) {
        delete[] _rx_buf;
        _rx_buf = nullptr;
    }

    if (_tx_buf) {
        delete[] _tx_buf;
        _tx_buf = nullptr;
    }

    _is_opened = false;

    return MA_OK;
}
size_t Console::available() const { return _rb_rx->size(); }

size_t Console::send(const char* data, size_t length, int timeout) {
    if (!_is_opened || length == 0) {
        return 0;
    }

    Guard guard(_tx_mutex);

    ma_tick_t time_start    = ma::Tick::current();
    size_t    bytes_to_send = 0;
    size_t    sent          = 0;

    while (length) {
        bytes_to_send = _rb_tx->push(data + sent, length);
        length -= bytes_to_send;
        sent += bytes_to_send;

        if (!_tx_busy) {
            _tx_busy      = true;
            bytes_to_send = _rb_tx->size() < 4095 ? _rb_tx->size() : 4095;
            _rb_tx->pop(_tx_buf, bytes_to_send);
            SCB_CleanDCache_by_Addr((volatile void*)_tx_buf, bytes_to_send);
            _uart->uart_write_udma(_tx_buf, bytes_to_send, (void*)_uart_dma_send);
        }

        if (ma::Tick::current() - time_start > static_cast<ma_tick_t>(timeout)) {
            return sent;
        }
    }

    return sent;
}

size_t Console::receive(char* data, size_t length, int timeout) {
    if (!_is_opened || length == 0) {
        return 0;
    }

    ma_tick_t time_start = ma::Tick::current();
    while (_rb_rx->size() == 0) {
        if (ma::Tick::current() - time_start > static_cast<ma_tick_t>(timeout)) {
            return 0;
        }
        ma::Thread::yield();
    }

    return _rb_rx->pop(data, length);
}

size_t Console::receiveUtil(char* data, size_t length, char delimiter, int timeout) {
    if (!_is_opened || length == 0) {
        return 0;
    }

    ma_tick_t time_start = ma::Tick::current();
    size_t    read       = 0;
    while (read <= _rb_rx->size() && read < length &&
           ma::Tick::current() - time_start <= static_cast<ma_tick_t>(timeout)) {
        read += _rb_rx->popIf(data + read, length - read, delimiter);
        ma::Thread::yield();
    }
    return read;
}

Console::operator bool() const { return _is_opened; }

}  // namespace ma