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

#include "el_network_we2.h"

#include "core/el_debug.h"

static el_err_code_t at_send(esp_at_t *at, uint32_t timeout) {
    uint32_t t = 0;
    printf(TAG_TX " %s\n" LOG_RESET, at->tbuf);
    at->state = AT_STATE_PROCESS;
    at->port->uart_write(at->tbuf, strlen(at->tbuf));

    while (at->state == AT_STATE_PROCESS) {
        if (t >= timeout) {
            printf(TAG_SYS "AT TIMEOUT\n" LOG_RESET);
            return EL_ETIMOUT;
        }
        delay(1);
        t++;
    }

    if (at->state != AT_STATE_OK) {
        printf(TAG_SYS "AT STATE ERROR: %d\n" LOG_RESET, at->state);
        return EL_FAILED;
    }
    return EL_OK;
}

static void newline_parse(esp_at_t *at, char *str, int len) {
    // 命令响应消息
    if (len < 4) return;
    if (strncmp(str, AT_STR_RESP_OK, strlen(AT_STR_RESP_OK)) == 0) {
        printf(TAG_RX "OK\n" LOG_RESET);
        at->state = AT_STATE_OK;
        return;
    } else if (strncmp(str, AT_STR_RESP_ERROR, strlen(AT_STR_RESP_ERROR)) == 0) {
        printf(TAG_RX "ERROR\n" LOG_RESET);
        at->state = AT_STATE_ERROR;
        return;
    } 

    // 状态变化消息 或 MQTT数据接收
    if (len < 7) return;
    if (strncmp(str, AT_STR_RESP_READY, strlen(AT_STR_RESP_READY)) == 0) {
        printf(TAG_RX "READY\n" LOG_RESET);
        at->state = AT_STATE_READY;
        return;
    } else if (strncmp(str, AT_STR_RESP_WIFI_H, strlen(AT_STR_RESP_WIFI_H)) == 0) {
        if (str[strlen(AT_STR_RESP_WIFI_H)] == 'C') { // connected
            printf(TAG_RX "WIFI CONNECTED\n" LOG_RESET);
            *at->ns = NETWORK_JOINED;
            return;
        } else if (str[strlen(AT_STR_RESP_WIFI_H)] == 'D') { // disconnected
            printf(TAG_RX "WIFI DISCONNECTED\n" LOG_RESET);
            *at->ns = NETWORK_IDLE;
            return;
        }
    } else if (strncmp(str, AT_STR_RESP_MQTT_H, strlen(AT_STR_RESP_MQTT_H)) == 0) {
        if (str[strlen(AT_STR_RESP_MQTT_H)] == 'C') { // connected
            printf(TAG_RX "MQTT CONNECTED\n" LOG_RESET);
            *at->ns = NETWORK_CONNECTED;
            return;
        } else if (str[strlen(AT_STR_RESP_MQTT_H)] == 'D') { // disconnected
            printf(TAG_RX "MQTT DISCONNECTED\n" LOG_RESET);
            if (*at->ns == NETWORK_CONNECTED) *at->ns = NETWORK_JOINED;
            return;
        } else if (str[strlen(AT_STR_RESP_MQTT_H)] == 'S') { // subscribe received
            printf(TAG_RX "MQTT SUBRECV\n" LOG_RESET); 
            // EXAMPLE: +MQTTSUBRECV:0,"topic",4,test
            int topic_len = 0, msg_len = 0, str_len = 0;

            char *topic_pos = strchr(str, '"');
            if (topic_pos == NULL) {
                printf(TAG_SYS "MQTT SUBRECV TOPIC ERROR\n" LOG_RESET);
                return;
            }
            topic_pos++; // Skip start character
            while (topic_pos[str_len] != '"') {
                str_len++;
            }
            topic_len = str_len;

            str_len = 0;
            char *msg_pos = &topic_pos[topic_len+1];
            if (msg_pos[0] != ',') {
                printf(TAG_SYS "MQTT SUBRECV MSG ERROR\n" LOG_RESET);
                return;
            }
            msg_pos++; // Skip start character
            while (msg_pos[str_len] != ',') {
                str_len++;
            }
            for (int i = 0; i < str_len; i++) {
                msg_len = msg_len * 10 + (msg_pos[i] - '0');
            }
            msg_pos += str_len + 1;

            at->cb(topic_pos, topic_len, msg_pos, msg_len);
            return;
        }
    }
}

static void *recv_process(void *arg) {
    esp_at_t *at = (esp_at_t *)arg;
    int cnt = 0 , len = 0;
    while (1) {
        // 未初始化
        if (at->state == AT_STATE_LOST) {
            delay(1);
            continue;
        }
        // 接收串口数据
        cnt = at->port.receive(at->rbuf, sizeof(at->rbuf));
        for (int i = 0; i < cnt; i++) {
            len++;
            if (at->rbuf[i] == '\n') {
                newline_parse(at, at->rbuf + i - len + 1, len);
                len = 0;
            }
        }
        delay(1);
    }
    return NULL;
}

namespace edgelab {

void Network::init() {
    el_err_code_t err;
    at.cur_cmd = AT_CMD_NONE;
    at.state = AT_STATE_LOST;
    at.ns = &network_status;

    // 创建串口、初始化发送缓冲区和接收缓冲区
    hx_drv_scu_set_PB9_pinmux(SCU_PB9_PINMUX_UART2_RX);
    hx_drv_scu_set_PB10_pinmux(SCU_PB10_PINMUX_UART2_TX);
    hx_drv_uart_init((USE_DW_UART_E)DW_UART_2_ID, HX_UART2_BASE);
    port = hx_drv_uart_get_port((USE_DW_UART_E)DW_UART_2_ID);
    port->uart_open(UART_BAUDRATE_115200);
    memset((void *)at.tbuf, 0, sizeof(at.tbuf));
    memset((void *)at.rbuf, 0, sizeof(at.rbuf));

    // 创建线程，用于接收数据、解析数据并触发事件
    HANDLE h;
    DWORD id;
    h = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)recv_process, &this->at, 0, &id);
    if (h == NULL) {
        return;
    }
    at.state = AT_STATE_IDLE;

    sprintf(at.tbuf, AT_STR_HEADER AT_STR_RST AT_STR_CRLF);
    uint32_t t = 0;
    at.state = AT_STATE_PROCESS;
    at.port->uart_write(at.tbuf, strlen(at.tbuf));
    printf(TAG_TX " %s\n" LOG_RESET, at.tbuf);
    while (at.state != AT_STATE_READY) {
        if (t++ >= AT_LONG_TIME_MS) {
            printf(TAG_SYS "AT RST TIMEOUT\n" LOG_RESET);
            return;
        }
        delay(1);
    }

    sprintf(at.tbuf, AT_STR_ECHO AT_STR_CRLF);
    err = at_send(&at, AT_SHORT_TIME_MS);
    if (err != EL_OK) {
        printf(TAG_SYS "AT ECHO ERROR : %d\n" LOG_RESET, err);
        return;
    }
    sprintf(at.tbuf, AT_STR_HEADER AT_STR_CWMODE "=1" AT_STR_CRLF);
    err = at_send(&at, AT_SHORT_TIME_MS);
    if (err != EL_OK) {
        printf(TAG_SYS "AT CWMODE ERROR : %d\n" LOG_RESET, err);
        return;
    }
    network_status = NETWORK_IDLE;
}

void Network::deinit() {
    at.port.close();
    at.cur_cmd = AT_CMD_NONE;
    at.state = AT_STATE_LOST;
}

el_net_sta_t Network::status() {
    return network_status;
}

el_err_code_t Network::join(const char* ssid, const char *pwd) {
    el_err_code_t err;
    if (network_status == NETWORK_JOINED || network_status == NETWORK_CONNECTED) {
        return EL_OK;
    } else if (network_status == NETWORK_LOST) {
        return EL_EPERM;
    }
    sprintf(at.tbuf, AT_STR_HEADER AT_STR_CWJAP "=\"%s\",\"%s\"" AT_STR_CRLF, ssid, pwd);
    err = at_send(&at, AT_LONG_TIME_MS);
    if (err != EL_OK) {
        printf(TAG_SYS "AT CWJAP ERROR : %d\n" LOG_RESET, err);
        return err;
    }
    return EL_OK;
}

el_err_code_t Network::quit() {
    // do nothing
    return EL_OK;
}

el_err_code_t Network::connect(const char* server, const char *user, const char *pass, topic_cb_t cb) 
{
    el_err_code_t err;
    if (network_status == NETWORK_CONNECTED) {
        return EL_OK;
    } else if (network_status != NETWORK_JOINED) {
        return EL_EPERM;
    }

    if (cb == NULL) {
        return EL_EINVAL;
    }
    at.cb = cb;

    sprintf(at.tbuf, AT_STR_HEADER AT_STR_MQTTUSERCFG "=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"" AT_STR_CRLF, 
            "HIMAX_WE2", user, pass);
    err = at_send(&at, AT_SHORT_TIME_MS);
    if (err != EL_OK) {
        printf(TAG_SYS "AT MQTTUSERCFG ERROR : %d\n" LOG_RESET, err);
        return err;
    }

    sprintf(at.tbuf, AT_STR_HEADER AT_STR_MQTTCONN "=0,\"%s\",1883,1" AT_STR_CRLF, 
            server);
    err = at_send(&at, AT_LONG_TIME_MS);
    if (err != EL_OK) {
        printf(TAG_SYS "AT MQTTCONN ERROR : %d\n" LOG_RESET, err);
        return err;
    }

    return EL_OK;
}

el_err_code_t Network::subscribe(const char* topic, mqtt_qos_t qos)
{
    el_err_code_t err;
    if (network_status != NETWORK_CONNECTED) {
        return EL_EPERM;
    }

    sprintf(at.tbuf, AT_STR_HEADER AT_STR_MQTTSUB "=0,\"%s\",%d" AT_STR_CRLF, topic, qos);
    err = at_send(&at, AT_SHORT_TIME_MS);
    if (err != EL_OK) {
        printf(TAG_SYS "AT MQTTSUB ERROR : %d\n" LOG_RESET, err);
        return err;
    }
    return EL_OK;
}

el_err_code_t Network::unsubscribe(const char* topic)
{
    el_err_code_t err;
    if (network_status != NETWORK_CONNECTED) {
        return EL_EPERM;
    }

    sprintf(at.tbuf, AT_STR_HEADER AT_STR_MQTTUNSUB "=0,\"%s\"" AT_STR_CRLF, topic);
    err = at_send(&at, AT_SHORT_TIME_MS);
    if (err != EL_OK) {
        printf(TAG_SYS "AT MQTTUNSUB ERROR : %d\n" LOG_RESET, err);
        return err;
    }
    return EL_OK;
}

el_err_code_t Network::publish(const char* topic, const char* dat, uint32_t len, mqtt_qos_t qos)
{
    el_err_code_t err;
    if (network_status != NETWORK_CONNECTED) {
        return EL_EPERM;
    }

    sprintf(at.tbuf, AT_STR_HEADER AT_STR_MQTTPUB "=0,\"%s\",\"%s\",%d,0" AT_STR_CRLF, 
            topic, dat, qos);
    err = at_send(&at, AT_LONG_TIME_MS);
    if (err != EL_OK) {
        printf(TAG_SYS "AT MQTTPUB ERROR : %d\n" LOG_RESET, err);
        return err;
    }
    return EL_OK;
}


} // namespace edgelab
