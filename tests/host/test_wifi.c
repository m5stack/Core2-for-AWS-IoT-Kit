/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../../lib/wifi/core2foraws_wifi.c"

esp_event_base_t const WIFI_EVENT = "WIFI_EVENT";
esp_event_base_t const IP_EVENT = "IP_EVENT";

/* ── FreeRTOS ────────────────────────────────────────────────────────── */

static int lock_depth;
SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t *storage) { return storage; }
BaseType_t xSemaphoreTake(SemaphoreHandle_t mutex, TickType_t timeout)
{
    (void)mutex; (void)timeout;
    assert(lock_depth == 0);
    lock_depth++;
    return pdTRUE;
}
BaseType_t xSemaphoreGive(SemaphoreHandle_t mutex) { (void)mutex; assert(lock_depth == 1); lock_depth--; return pdTRUE; }
void vTaskDelay(TickType_t ticks) { (void)ticks; }

EventGroupHandle_t xEventGroupCreateStatic(StaticEventGroup_t *storage) { storage->bits = 0; return storage; }
EventBits_t xEventGroupSetBits(EventGroupHandle_t group, EventBits_t bits) { return group->bits |= bits; }
EventBits_t xEventGroupClearBits(EventGroupHandle_t group, EventBits_t bits)
{
    EventBits_t previous = group->bits;
    group->bits &= ~bits;
    return previous;
}
EventBits_t xEventGroupGetBits(EventGroupHandle_t group) { return group->bits; }
/* Events are delivered synchronously, so a wait never blocks: it reports what is already set. */
EventBits_t xEventGroupWaitBits(EventGroupHandle_t group, EventBits_t bits, BaseType_t clear,
                                BaseType_t all, TickType_t timeout)
{
    (void)all; (void)timeout;
    EventBits_t current = group->bits;
    if (clear && (current & bits)) group->bits &= ~bits;
    return current;
}

/* ── Events ──────────────────────────────────────────────────────────── */

typedef struct { esp_event_base_t base; int32_t id; esp_event_handler_t handler; } handler_slot_t;
static handler_slot_t slots[8];
static int slot_count;

esp_err_t esp_event_loop_create_default(void) { return ESP_OK; }
esp_err_t esp_event_handler_register(esp_event_base_t base, int32_t id, esp_event_handler_t handler, void *arg)
{
    (void)arg;
    assert(slot_count < 8);
    slots[slot_count++] = (handler_slot_t){ base, id, handler };
    return ESP_OK;
}
esp_err_t esp_event_handler_unregister(esp_event_base_t base, int32_t id, esp_event_handler_t handler)
{
    for (int i = 0; i < slot_count; i++)
    {
        if (slots[i].base == base && slots[i].id == id && slots[i].handler == handler)
        {
            slots[i] = slots[--slot_count];
            return ESP_OK;
        }
    }
    return ESP_ERR_INVALID_STATE;
}
static void fire(esp_event_base_t base, int32_t id, void *data)
{
    for (int i = 0; i < slot_count; i++)
        if (slots[i].base == base && slots[i].id == id) slots[i].handler(NULL, base, id, data);
}

/* ── Driver ──────────────────────────────────────────────────────────── */

typedef enum { REPLY_GOT_IP, REPLY_DISCONNECT } reply_kind_t;
static struct { reply_kind_t kind; uint8_t reason; } replies[8];
static int reply_count, reply_next;
static wifi_config_t stored;
static bool driver_ready, radio_on, linked;
static int starts, connects, disconnects, set_configs, scans;
static uint64_t timer_delay_us;
static bool timer_armed;

static void link_lost(uint8_t reason)
{
    linked = false;
    wifi_event_sta_disconnected_t event = { reason };
    fire(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &event);
}
static void got_ip(void)
{
    linked = true;
    ip_event_got_ip_t event = { { { 0x0100a8c0 } } };
    fire(IP_EVENT, IP_EVENT_STA_GOT_IP, &event);
}
static void script(reply_kind_t kind, uint8_t reason)
{
    assert(reply_count < 8);
    replies[reply_count].kind = kind;
    replies[reply_count++].reason = reason;
}

esp_err_t nvs_flash_init(void) { return ESP_OK; }
esp_err_t esp_netif_init(void) { return ESP_OK; }
static int netif_storage;
esp_netif_t *esp_netif_create_default_wifi_sta(void) { return (esp_netif_t *)&netif_storage; }
void esp_netif_destroy(esp_netif_t *netif) { (void)netif; }
esp_err_t esp_wifi_clear_default_wifi_driver_and_handlers(void *netif) { (void)netif; return ESP_OK; }
esp_err_t esp_wifi_init(const wifi_init_config_t *config) { (void)config; driver_ready = true; return ESP_OK; }
esp_err_t esp_wifi_deinit(void) { driver_ready = false; return ESP_OK; }
esp_err_t esp_wifi_set_mode(wifi_mode_t mode) { assert(driver_ready && mode == WIFI_MODE_STA); return ESP_OK; }
esp_err_t esp_wifi_start(void)
{
    starts++;
    radio_on = true;
    fire(WIFI_EVENT, WIFI_EVENT_STA_START, NULL);
    return ESP_OK;
}
esp_err_t esp_wifi_stop(void)
{
    if (!radio_on) return ESP_ERR_WIFI_NOT_STARTED;
    radio_on = linked = false;
    fire(WIFI_EVENT, WIFI_EVENT_STA_STOP, NULL);
    return ESP_OK;
}
esp_err_t esp_wifi_connect(void)
{
    assert(radio_on);
    connects++;
    if (reply_next < reply_count)
    {
        int reply = reply_next++;
        if (replies[reply].kind == REPLY_GOT_IP) got_ip();
        else link_lost(replies[reply].reason);
    }
    return ESP_OK;
}
esp_err_t esp_wifi_disconnect(void)
{
    disconnects++;
    if (linked) link_lost(8);
    return ESP_OK;
}
esp_err_t esp_wifi_set_config(wifi_interface_t interface, wifi_config_t *config)
{
    assert(interface == WIFI_IF_STA);
    set_configs++;
    stored = *config;
    return ESP_OK;
}
esp_err_t esp_wifi_get_config(wifi_interface_t interface, wifi_config_t *config)
{
    assert(interface == WIFI_IF_STA);
    *config = stored;
    return ESP_OK;
}
esp_err_t esp_wifi_scan_start(const void *config, bool block) { (void)config; assert(block && radio_on); scans++; return ESP_OK; }
esp_err_t esp_wifi_scan_get_ap_records(uint16_t *count, wifi_ap_record_t *records)
{
    *count = 1;
    strcpy((char *)records[0].ssid, "home");
    return ESP_OK;
}
esp_err_t esp_wifi_clear_ap_list(void) { return ESP_OK; }

static int timer_storage;
esp_err_t esp_timer_create(const esp_timer_create_args_t *args, esp_timer_handle_t *handle)
{
    (void)args;
    *handle = (esp_timer_handle_t)&timer_storage;
    return ESP_OK;
}
esp_err_t esp_timer_start_once(esp_timer_handle_t timer, uint64_t timeout_us)
{
    (void)timer;
    timer_armed = true;
    timer_delay_us = timeout_us;
    return ESP_OK;
}
esp_err_t esp_timer_stop(esp_timer_handle_t timer) { (void)timer; timer_armed = false; return ESP_OK; }
esp_err_t esp_timer_delete(esp_timer_handle_t timer) { (void)timer; return ESP_OK; }

/* ── Provisioning hooks ──────────────────────────────────────────────── */

static core2foraws_wifi_prov_state_t prov_phase;
static int prov_stops;
esp_err_t core2foraws_wifi_priv_prov_init(void) { return ESP_OK; }
void core2foraws_wifi_priv_prov_deinit(void) {}
void core2foraws_wifi_priv_prov_stop(void) { prov_stops++; prov_phase = CORE2FORAWS_WIFI_PROV_OFF; }
core2foraws_wifi_prov_state_t core2foraws_wifi_priv_prov_state(void) { return prov_phase; }
void core2foraws_wifi_priv_prov_on_connected(void) {}

/* ── Helpers ─────────────────────────────────────────────────────────── */

static void reset_script(void) { reply_count = reply_next = 0; }

static void save_network(const char *ssid, const char *password)
{
    memset(&stored, 0, sizeof(stored));
    memcpy(stored.sta.ssid, ssid, strlen(ssid));
    memcpy(stored.sta.password, password, strlen(password));
}

static void expect_saved(const char *ssid, const char *password)
{
    assert(strncmp((const char *)stored.sta.ssid, ssid, sizeof(stored.sta.ssid)) == 0);
    assert(strncmp((const char *)stored.sta.password, password, sizeof(stored.sta.password)) == 0);
}

static void expect_state(core2foraws_wifi_state_t state)
{
    EventBits_t link = xEventGroupGetBits(wifi_event_group) & WIFI_LINK_BITS;
    EventBits_t expected = state == CORE2FORAWS_WIFI_STATE_CONNECTED  ? WIFI_CONNECTED_BIT :
                           state == CORE2FORAWS_WIFI_STATE_CONNECTING ? WIFI_CONNECTING_BIT :
                                                                        WIFI_DISCONNECTED_BIT;
    core2foraws_wifi_state_t actual;
    assert(core2foraws_wifi_state_get(&actual) == ESP_OK && actual == state);
    assert(link == expected);
    assert(lock_depth == 0);
}

/* ── Tests ───────────────────────────────────────────────────────────── */

static void test_init_and_arguments(void)
{
    core2foraws_wifi_state_t state = CORE2FORAWS_WIFI_STATE_CONNECTED;
    assert(core2foraws_wifi_state_get(NULL) == ESP_ERR_INVALID_ARG);
    assert(core2foraws_wifi_state_get(&state) == ESP_OK && state == CORE2FORAWS_WIFI_STATE_IDLE);
    assert(core2foraws_wifi_connect("home", "pw", 100) == ESP_ERR_INVALID_STATE);
    assert(core2foraws_wifi_init() == ESP_OK);
    EventGroupHandle_t group = wifi_event_group;
    assert(core2foraws_wifi_init() == ESP_OK && wifi_event_group == group && slot_count == 4);
    expect_state(CORE2FORAWS_WIFI_STATE_IDLE);
    assert(!radio_on);

    char too_long[MAX_PASSPHRASE_LEN + 2];
    memset(too_long, 'a', sizeof(too_long) - 1);
    too_long[sizeof(too_long) - 1] = '\0';
    assert(core2foraws_wifi_connect(NULL, "pw", 100) == ESP_ERR_INVALID_ARG);
    assert(core2foraws_wifi_connect("", "pw", 100) == ESP_ERR_INVALID_ARG);
    assert(core2foraws_wifi_connect(too_long, "pw", 100) == ESP_ERR_INVALID_ARG);
    assert(core2foraws_wifi_connect("home", too_long, 100) == ESP_ERR_INVALID_ARG);

    char ssid[MAX_SSID_LEN + 1];
    assert(core2foraws_wifi_saved_ssid_get(ssid) == ESP_ERR_NOT_FOUND && ssid[0] == '\0');
    assert(core2foraws_wifi_reconnect(100) == ESP_ERR_NOT_FOUND);
}

static void test_scan_starts_radio_once(void)
{
    wifi_ap_record_t records[2];
    uint16_t count = 2;
    assert(core2foraws_wifi_scan(records, &count) == ESP_OK && count == 1 && starts == 1);
    expect_state(CORE2FORAWS_WIFI_STATE_DISCONNECTED);
    count = 2;
    assert(core2foraws_wifi_scan(records, &count) == ESP_OK && starts == 1 && scans == 2);
    assert(core2foraws_wifi_scan(records, NULL) == ESP_ERR_INVALID_ARG);
}

static void test_connect_saves_on_success(void)
{
    reset_script();
    script(REPLY_GOT_IP, 0);
    assert(core2foraws_wifi_connect("home", "secret", 100) == ESP_OK);
    expect_state(CORE2FORAWS_WIFI_STATE_CONNECTED);
    expect_saved("home", "secret");
    assert(core2foraws_wifi_last_error_get() == ESP_OK);

    char ssid[MAX_SSID_LEN + 1];
    assert(core2foraws_wifi_saved_ssid_get(ssid) == ESP_OK && strcmp(ssid, "home") == 0);
}

static void test_disconnect_is_sticky(void)
{
    assert(core2foraws_wifi_disconnect() == ESP_OK);
    expect_state(CORE2FORAWS_WIFI_STATE_DISCONNECTED);
    int before = connects;
    link_lost(WIFI_REASON_BEACON_TIMEOUT);
    assert(connects == before && !timer_armed);
    expect_state(CORE2FORAWS_WIFI_STATE_DISCONNECTED);
}

static void test_wrong_password_restores_saved_network(void)
{
    reset_script();
    script(REPLY_DISCONNECT, WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT);
    int before = connects;
    assert(core2foraws_wifi_connect("cafe", "wrong", 100) == ESP_ERR_WIFI_PASSWORD);
    assert(connects == before + 1);
    expect_state(CORE2FORAWS_WIFI_STATE_DISCONNECTED);
    expect_saved("home", "secret");
    assert(core2foraws_wifi_last_error_get() == ESP_ERR_WIFI_PASSWORD);
}

static void test_missing_network_retries_then_restores(void)
{
    reset_script();
    for (int i = 0; i < 3; i++) script(REPLY_DISCONNECT, WIFI_REASON_NO_AP_FOUND);
    int before = connects;
    assert(core2foraws_wifi_connect("away", "pw", 100) == ESP_ERR_NOT_FOUND);
    assert(connects == before + (int)WIFI_CONNECT_ATTEMPTS);
    expect_saved("home", "secret");
    expect_state(CORE2FORAWS_WIFI_STATE_DISCONNECTED);
}

static void test_timeout_cancels_and_restores(void)
{
    reset_script();
    assert(core2foraws_wifi_connect("slow", "pw", 100) == ESP_ERR_TIMEOUT);
    expect_saved("home", "secret");
    expect_state(CORE2FORAWS_WIFI_STATE_DISCONNECTED);
    assert(core2foraws_wifi_last_error_get() == ESP_ERR_TIMEOUT);
    assert(!core2foraws_wifi_priv_attempt_active());
}

static void test_async_connect_completes_from_events(void)
{
    reset_script();
    assert(core2foraws_wifi_connect("cafe", "right", 0) == ESP_OK);
    expect_state(CORE2FORAWS_WIFI_STATE_CONNECTING);
    assert(core2foraws_wifi_priv_attempt_active());
    wifi_ap_record_t records[1];
    uint16_t count = 1;
    assert(core2foraws_wifi_scan(records, &count) == ESP_ERR_INVALID_STATE);
    assert(core2foraws_wifi_connect("other", "pw", 0) == ESP_ERR_INVALID_STATE);
    got_ip();
    expect_state(CORE2FORAWS_WIFI_STATE_CONNECTED);
    expect_saved("cafe", "right");
    assert(!core2foraws_wifi_priv_attempt_active());
}

static void test_reconnect_backs_off_and_recovers(void)
{
    assert(core2foraws_wifi_disconnect() == ESP_OK);
    reset_script();
    script(REPLY_GOT_IP, 0);
    assert(core2foraws_wifi_reconnect(100) == ESP_OK);
    expect_state(CORE2FORAWS_WIFI_STATE_CONNECTED);

    int before = connects;
    link_lost(WIFI_REASON_BEACON_TIMEOUT);
    expect_state(CORE2FORAWS_WIFI_STATE_CONNECTING);
    assert(connects == before + 1 && !timer_armed);
    link_lost(WIFI_REASON_BEACON_TIMEOUT);
    assert(timer_armed && timer_delay_us == 1000000U);
    link_lost(WIFI_REASON_BEACON_TIMEOUT);
    assert(timer_delay_us == 2000000U);
    assert(core2foraws_wifi_last_error_get() == ESP_FAIL);
    got_ip();
    expect_state(CORE2FORAWS_WIFI_STATE_CONNECTED);
    assert(core2foraws_wifi_last_error_get() == ESP_OK);
}

static void test_reconnect_stops_after_repeated_auth_failures(void)
{
    for (unsigned int i = 0; i < WIFI_CONNECT_ATTEMPTS - 1; i++)
    {
        link_lost(WIFI_REASON_AUTH_FAIL);
        expect_state(CORE2FORAWS_WIFI_STATE_CONNECTING);
    }
    link_lost(WIFI_REASON_AUTH_FAIL);
    expect_state(CORE2FORAWS_WIFI_STATE_DISCONNECTED);
    assert(core2foraws_wifi_last_error_get() == ESP_ERR_WIFI_PASSWORD && !timer_armed);
}

static void test_phone_takes_priority(void)
{
    reset_script();
    script(REPLY_GOT_IP, 0);
    assert(core2foraws_wifi_reconnect(100) == ESP_OK);

    wifi_ap_record_t records[1];
    uint16_t count = 1;
    for (int phase = CORE2FORAWS_WIFI_PROV_PHONE_CONNECTED; phase <= CORE2FORAWS_WIFI_PROV_APPLYING; phase++)
    {
        prov_phase = (core2foraws_wifi_prov_state_t)phase;
        assert(core2foraws_wifi_connect("home", "secret", 100) == ESP_ERR_INVALID_STATE);
        assert(core2foraws_wifi_reconnect(100) == ESP_ERR_INVALID_STATE);
        assert(core2foraws_wifi_scan(records, &count) == ESP_ERR_INVALID_STATE);
        assert(core2foraws_wifi_forget() == ESP_ERR_INVALID_STATE);
    }

    /* The manager owns the station while applying phone credentials */
    int before = connects;
    link_lost(WIFI_REASON_BEACON_TIMEOUT);
    assert(connects == before && !timer_armed);

    prov_phase = CORE2FORAWS_WIFI_PROV_WAITING;
    reset_script();
    script(REPLY_GOT_IP, 0);
    assert(core2foraws_wifi_connect("home", "secret", 100) == ESP_OK);
    assert(prov_stops == 1 && prov_phase == CORE2FORAWS_WIFI_PROV_OFF);
    expect_state(CORE2FORAWS_WIFI_STATE_CONNECTED);
}

static void test_forget_erases_saved_network(void)
{
    assert(core2foraws_wifi_forget() == ESP_OK);
    expect_state(CORE2FORAWS_WIFI_STATE_DISCONNECTED);
    char ssid[MAX_SSID_LEN + 1];
    assert(core2foraws_wifi_saved_ssid_get(ssid) == ESP_ERR_NOT_FOUND);
    assert(core2foraws_wifi_reconnect(100) == ESP_ERR_NOT_FOUND);
}

static void test_deinit_keeps_event_group(void)
{
    save_network("home", "secret");
    EventGroupHandle_t group = wifi_event_group;
    assert(core2foraws_wifi_deinit() == ESP_OK);
    assert(wifi_event_group == group && slot_count == 0 && !radio_on && !driver_ready);
    expect_state(CORE2FORAWS_WIFI_STATE_IDLE);
    assert(core2foraws_wifi_disconnect() == ESP_ERR_INVALID_STATE);
    assert(core2foraws_wifi_init() == ESP_OK && wifi_event_group == group);
    char ssid[MAX_SSID_LEN + 1];
    assert(core2foraws_wifi_saved_ssid_get(ssid) == ESP_OK && strcmp(ssid, "home") == 0);
}

int main(void)
{
    test_init_and_arguments();
    test_scan_starts_radio_once();
    test_connect_saves_on_success();
    test_disconnect_is_sticky();
    test_wrong_password_restores_saved_network();
    test_missing_network_retries_then_restores();
    test_timeout_cancels_and_restores();
    test_async_connect_completes_from_events();
    test_reconnect_backs_off_and_recovers();
    test_reconnect_stops_after_repeated_auth_failures();
    test_phone_takes_priority();
    test_forget_erases_saved_network();
    test_deinit_keeps_event_group();
    puts("Wi-Fi state machine, credential restore, reconnect backoff and provisioning priority tests passed");
    return 0;
}
