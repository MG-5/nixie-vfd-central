#pragma once

#include "esp_event.h"
#include "esp_netif_ip_addr.h"
#include "esp_wifi_types.h"

#include "helpers/freertos.hpp"
#include "wrappers/Task.hpp"

using util::wrappers::TaskWithMemberFunctionBase;

class Wireless : public TaskWithMemberFunctionBase
{
public:
    static constexpr auto RetryDelay = 3.0_s;
    static constexpr auto ReconnectionCounterThreshould = 5;

    Wireless()
        : TaskWithMemberFunctionBase("wirelessTask", 2048, osPriorityAboveNormal3) //
    {};

    static void eventHandler(void *arg, esp_event_base_t eventBase, int32_t eventId,
                             void *eventData);

    inline static wifi_event_sta_connected_t staInfos;
    inline static esp_ip4_addr_t ipAdress;
    static std::string_view getAuthModeAsString(wifi_auth_mode_t mode);

protected:
    void taskMain(void *) override;

private:
    inline static uint8_t reconnectionCounter = 0;

    void init();
    void configureStation();
    void startWifi();
};