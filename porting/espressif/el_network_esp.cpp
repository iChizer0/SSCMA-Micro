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

#include "el_network_esp.h"

#include "core/el_debug.h"

namespace edgelab {

el_err_code_t NetworkEsp::init() {
    /* esp_netif_init */
    static sys_sem_t api_sync_sem = NULL;
    static sys_sem_t api_lock_sem = NULL;
    if (!sys_thread_tcpip(LWIP_CORE_IS_TCPIP_INITIALIZED)) {
        sys_sem_t init_sem;
        if (sys_sem_new(&init_sem, 0) != EL_OK) {
            EL_ELOG("esp netif cannot create tcpip_init semaphore");
            return EL_EBUSY;
        }
        tcpip_init(tcpip_init_done, &init_sem);
        sys_sem_wait(&init_sem);
        sys_sem_free(&init_sem);
        EL_ELOG("LwIP stack has been initialized");
    }

    if (!api_sync_sem) {
        if (EL_OK != sys_sem_new(&api_sync_sem, 0)) {
            EL_ELOG("esp netif api sync sem init fail");
            return EL_EBUSY;
        }
    }
    if (!api_lock_sem) {
        if (EL_OK != sys_sem_new(&api_lock_sem, 1)) {
            EL_ELOG("esp netif api lock sem init fail");
            return EL_EBUSY;
        }
    }
    EL_ELOG("esp-netif has been successfully initialized");

    this->_is_present = true;
    return EL_OK;
}

el_err_code_t NetworkEsp::deinit() {
    this->_is_present = false;
    return EL_OK;
}

el_err_code_t NetworkEsp::connect(const char* ssid, const char *pwd) {
    /* Initialize Wi-Fi and start STA mode */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_default_wifi_sta_handlers();
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();

    /* Create a netif for WIFI station */
    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    esp_netif_config.if_desc = "edgelab";
    esp_netif_config.route_prio = 128;
    esp_netif = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);

    /* Set ssid and password for connection */
    uint8_t wifi_ssid[32];
    uint8_t wifi_password[64];
    if (ssid != NULL && strlen(ssid) < 32) {
        memcpy(wifi_ssid, ssid, strlen(ssid) + 1);
    }
    if (pwd != NULL && strlen(pwd) < 64) {
        memcpy(wifi_password, pwd, strlen(pwd) + 1);
    }
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = wifi_ssid,
            .password = wifi_password,
            .scan_method = WIFI_FAST_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold.rssi = -127,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

    /* Start DHCP client */
    esp_netif_dhcp_status_t status = ESP_NETIF_DHCP_INIT;
    esp_netif_dhcpc_get_status(esp_netif, &status);
    esp_netif_dhcpc_start(esp_netif);
    esp_wifi_connect();

    /* Add netif to lwIP */
    netif_add(esp_netif->lwip_netif,
            (struct ip4_addr*)&esp_netif->ip_info->ip,
            (struct ip4_addr*)&esp_netif->ip_info->netmask,
            (struct ip4_addr*)&esp_netif->ip_info->gw,
            esp_netif, esp_netif->lwip_init_fn, tcpip_input);

    return EL_OK;
}



} // namespace edgelab
