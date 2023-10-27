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

lwRingBuffer at_rbuf(AT_RX_MAX_LEN);
static uint8_t dma_rx[4] = {0};
static esp_at_t at;
static TaskHandle_t at_rx_task = NULL;
// Indicates how many rows of data are in at_rbuf
static SemaphoreHandle_t sem_line_cnt = NULL;
// Indicates how the network status should change
static SemaphoreHandle_t sem_network_flag = NULL;

static el_err_code_t at_send(esp_at_t *at, uint32_t timeout) {
    uint32_t t = 0;
    el_printf(TAG_TX " %s\n" TAG_RST, at->tbuf);
    at->state = AT_STATE_PROCESS;
    at->port->uart_write(at->tbuf, strlen(at->tbuf));

    while (at->state == AT_STATE_PROCESS) {
        if (t >= timeout) {
            el_printf(TAG_SYS "AT TIMEOUT\n" TAG_RST);
            return EL_ETIMOUT;
        }
        el_sleep(10);
        t += 10;
    }

    if (at->state != AT_STATE_OK) {
        el_printf(TAG_SYS "AT STATE ERROR: %d\n" TAG_RST, at->state);
        return EL_FAILED;
    }
    return EL_OK;
}

static void newline_parse() {
    // 命令响应消息
    // char line[len + 1] = {0};
    // memcpy(line, str, len);
    // line[len] = '\0';
    // el_printf(TAG_RX "%s\n" TAG_RST, line);
    char str[512] = {0};
    uint32_t len = at_rbuf.extract('\n', str, sizeof(str));
    if (len < 3) return;

    if (strncmp(str, AT_STR_RESP_OK, strlen(AT_STR_RESP_OK)) == 0) {
        // el_printf(TAG_RX "OK\n" TAG_RST);
        at.state = AT_STATE_OK;
        return;
    } else if (strncmp(str, AT_STR_RESP_ERROR, strlen(AT_STR_RESP_ERROR)) == 0) {
        // el_printf(TAG_RX "ERROR\n" TAG_RST);
        at.state = AT_STATE_ERROR;
        return;
    } 

    // 状态变化消息 或 MQTT数据接收
    if (len < 6) return;
    if (strncmp(str, AT_STR_RESP_READY, strlen(AT_STR_RESP_READY)) == 0) {
        // el_printf(TAG_RX "READY\n" TAG_RST);
        at.state = AT_STATE_READY;
        return;
    } else if (strncmp(str, AT_STR_RESP_WIFI_H, strlen(AT_STR_RESP_WIFI_H)) == 0) {
        if (str[strlen(AT_STR_RESP_WIFI_H)] == 'C') { // connected
            // el_printf(TAG_RX "WIFI CONNECTED\n" TAG_RST);
            if (uxSemaphoreGetCount(sem_network_flag) == 0) {
                xSemaphoreGive(sem_network_flag);
            }
            return;
        } else if (str[strlen(AT_STR_RESP_WIFI_H)] == 'D') { // disconnected
            // el_printf(TAG_RX "WIFI DISCONNECTED\n" TAG_RST);
            while (uxSemaphoreGetCount(sem_network_flag) > 0) {
                xSemaphoreTake(sem_network_flag, 0);
            }
            return;
        }
    } else if (strncmp(str, AT_STR_RESP_MQTT_H, strlen(AT_STR_RESP_MQTT_H)) == 0) {
        if (str[strlen(AT_STR_RESP_MQTT_H)] == 'C') { // connected
            // el_printf(TAG_RX "MQTT CONNECTED\n" TAG_RST);
            while (uxSemaphoreGetCount(sem_network_flag) != 2) {
                xSemaphoreGive(sem_network_flag);
            }
            return;
        } else if (str[strlen(AT_STR_RESP_MQTT_H)] == 'D') { // disconnected
            // el_printf(TAG_RX "MQTT DISCONNECTED\n" TAG_RST);
            if (uxSemaphoreGetCount(sem_network_flag) == 2) {
                xSemaphoreTake(sem_network_flag, 0);
            }
            return;
        } else if (str[strlen(AT_STR_RESP_MQTT_H)] == 'S') { // subscribe received
            // el_printf(TAG_RX "MQTT SUBRECV\n" TAG_RST); 
            // EXAMPLE: +MQTTSUBRECV:0,"topic",4,test
            int topic_len = 0, msg_len = 0, str_len = 0;

            char *topic_pos = strchr(str, '"');
            if (topic_pos == NULL) {
                // el_printf(TAG_SYS "MQTT SUBRECV TOPIC ERROR\n" TAG_RST);
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
                // el_printf(TAG_SYS "MQTT SUBRECV MSG ERROR\n" TAG_RST);
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

            at.cb(topic_pos, topic_len, msg_pos, msg_len);
            return;
        }
    }
}

static void at_recv(void *arg) {
    // esp_at_t *at = (esp_at_t *)arg;
    el_net_sta_t *status = (el_net_sta_t *)arg;
    uint8_t status_flag = 0;
    while (1) {
        if (*status != NETWORK_LOST) {
            status_flag = uxSemaphoreGetCount(sem_network_flag);
            *status = status_flag == 2 ? NETWORK_CONNECTED 
                    : status_flag == 1 ? NETWORK_JOINED : NETWORK_IDLE;
        }
        while (uxSemaphoreGetCount(sem_line_cnt) > 0) {
            newline_parse();
            xSemaphoreTake(sem_line_cnt, 0);
        }
        el_sleep(500);
    }
    return;
}

static void dma_rx_cb(void *arg) {
    at.port->uart_read_udma(dma_rx, 1, (void *)dma_rx_cb);
    if (at.state == AT_STATE_LOST) {
        return;
    }

    at_rbuf << dma_rx[0];
    if (dma_rx[0] == '\n') {
        xSemaphoreGive(sem_line_cnt);
    }
    return;
}

namespace edgelab {


// TODO: 错误退出时释放资源
void NetworkWE2::init() {
    el_err_code_t err;
    _at = &at;
    at.state = AT_STATE_LOST;
    at.rbuf = &at_rbuf;

    // 创建串口、初始化发送缓冲区和接收缓冲区
    hx_drv_scu_set_PB9_pinmux(SCU_PB9_PINMUX_UART2_RX);
    hx_drv_scu_set_PB10_pinmux(SCU_PB10_PINMUX_UART2_TX);
    hx_drv_uart_init(USE_DW_UART_2, HX_UART2_BASE);
    at.port = hx_drv_uart_get_dev(USE_DW_UART_2);
    if (at.port == NULL) {
        el_printf(TAG_SYS "uart init error\n" TAG_RST);
        return;
    }
    at.port->uart_open(UART_BAUDRATE_115200);
    memset((void *)at.tbuf, 0, sizeof(at.tbuf));
    at.port->uart_read_udma(dma_rx, 1, (void *)dma_rx_cb);

    // 创建线程，用于接收数据、解析数据并触发事件
    if (xTaskCreate(at_recv, "at_recv", 4096, &network_status, 1, &at_rx_task) != pdPASS ) {
        el_printf(TAG_SYS "at_recv create error\n" TAG_RST);
        return;
    }
    sem_line_cnt = xSemaphoreCreateCounting(AT_MAX_LINES_CNT, 0);
    sem_network_flag = xSemaphoreCreateCounting(2, 0);
    if (sem_line_cnt == NULL || sem_network_flag == NULL) {
        el_printf(TAG_SYS "sem create error\n" TAG_RST);
        vTaskDelete(at_rx_task);
        return;
    }
    at.state = AT_STATE_IDLE;

    sprintf(at.tbuf, AT_STR_HEADER AT_STR_RST AT_STR_CRLF);
    uint32_t t = 0;
    at.state = AT_STATE_PROCESS;
    at.port->uart_write(at.tbuf, strlen(at.tbuf));
    el_printf(TAG_TX " %s\n" TAG_RST, at.tbuf);
    while (at.state != AT_STATE_READY) {
        if (t >= AT_RETRY_TIME_MS) {
            el_printf(TAG_SYS "AT RST TIMEOUT\n" TAG_RST);
            return;
        }
        el_sleep(10);
        t += 10;
    }

    sprintf(at.tbuf, AT_STR_ECHO AT_STR_CRLF);
    err = at_send(&at, AT_SHORT_TIME_MS);
    if (err != EL_OK) {
        el_printf(TAG_SYS "AT ECHO ERROR : %d\n" TAG_RST, err);
        return;
    }
    sprintf(at.tbuf, AT_STR_HEADER AT_STR_CWMODE "=1" AT_STR_CRLF);
    err = at_send(&at, AT_SHORT_TIME_MS);
    if (err != EL_OK) {
        el_printf(TAG_SYS "AT CWMODE ERROR : %d\n" TAG_RST, err);
        return;
    }
    network_status = NETWORK_IDLE;
}

void NetworkWE2::deinit() {
    at.port->uart_close();
    at.state = AT_STATE_LOST;
}

el_net_sta_t NetworkWE2::status() {
    return network_status;
}

el_err_code_t NetworkWE2::join(const char* ssid, const char *pwd) {
    el_err_code_t err;
    el_printf(TAG_SYS "join %s %s\n" TAG_RST, ssid, pwd);
    if (network_status == NETWORK_JOINED || network_status == NETWORK_CONNECTED) {
        return EL_OK;
    } else if (network_status == NETWORK_LOST) {
        return EL_EPERM;
    }
    sprintf(at.tbuf, AT_STR_HEADER AT_STR_CWJAP "=\"%s\",\"%s\"" AT_STR_CRLF, ssid, pwd);
    err = at_send(&at, AT_RETRY_TIME_MS*2);
    if (err != EL_OK) {
        el_printf(TAG_SYS "AT CWJAP ERROR : %d\n" TAG_RST, err);
        return err;
    }
    return EL_OK;
}

el_err_code_t NetworkWE2::quit() {
    // do nothing
    return EL_OK;
}

el_err_code_t NetworkWE2::connect(const char* server, const char *user, const char *pass, topic_cb_t cb) 
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
        el_printf(TAG_SYS "AT MQTTUSERCFG ERROR : %d\n" TAG_RST, err);
        return err;
    }

    sprintf(at.tbuf, AT_STR_HEADER AT_STR_MQTTCONN "=0,\"%s\",1883,1" AT_STR_CRLF, 
            server);
    err = at_send(&at, AT_RETRY_TIME_MS*20);
    if (err != EL_OK) {
        el_printf(TAG_SYS "AT MQTTCONN ERROR : %d\n" TAG_RST, err);
        return err;
    }

    return EL_OK;
}

el_err_code_t NetworkWE2::subscribe(const char* topic, mqtt_qos_t qos)
{
    el_err_code_t err;
    if (network_status != NETWORK_CONNECTED) {
        return EL_EPERM;
    }

    sprintf(at.tbuf, AT_STR_HEADER AT_STR_MQTTSUB "=0,\"%s\",%d" AT_STR_CRLF, topic, qos);
    err = at_send(&at, AT_SHORT_TIME_MS);
    if (err != EL_OK) {
        el_printf(TAG_SYS "AT MQTTSUB ERROR : %d\n" TAG_RST, err);
        return err;
    }
    return EL_OK;
}

el_err_code_t NetworkWE2::unsubscribe(const char* topic)
{
    el_err_code_t err;
    if (network_status != NETWORK_CONNECTED) {
        return EL_EPERM;
    }

    sprintf(at.tbuf, AT_STR_HEADER AT_STR_MQTTUNSUB "=0,\"%s\"" AT_STR_CRLF, topic);
    err = at_send(&at, AT_SHORT_TIME_MS);
    if (err != EL_OK) {
        el_printf(TAG_SYS "AT MQTTUNSUB ERROR : %d\n" TAG_RST, err);
        return err;
    }
    return EL_OK;
}

el_err_code_t NetworkWE2::publish(const char* topic, const char* dat, uint32_t len, mqtt_qos_t qos)
{
    el_err_code_t err;
    if (network_status != NETWORK_CONNECTED) {
        return EL_EPERM;
    }

    sprintf(at.tbuf, AT_STR_HEADER AT_STR_MQTTPUB "=0,\"%s\",\"%s\",%d,0" AT_STR_CRLF, 
            topic, dat, qos);
    err = at_send(&at, AT_LONG_TIME_MS);
    if (err != EL_OK) {
        el_printf(TAG_SYS "AT MQTTPUB ERROR : %d\n" TAG_RST, err);
        return err;
    }
    return EL_OK;
}


} // namespace edgelab
