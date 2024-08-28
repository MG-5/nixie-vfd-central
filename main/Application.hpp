#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "helpers/freertos.hpp"

#include "mqtt/MqttClient.hpp"
#include "time/TimeSource.hpp"
#include "uart/UartEvent.hpp"
#include "uart/UartTx.hpp"
#include "wifi/Wireless.hpp"

class Application
{
public:
    static constexpr auto PrintTag = "[Application]";

    Application() = default;

    void run();

    static Application &getApplicationInstance();

private:
    Wireless wifi{};

    static constexpr auto UartNumber = UART_NUM_1;
    util::wrappers::StreamBuffer txStreamBuffer{1024, 1};
    UartEvent uartEvent{UartNumber};
    UartTx uartTx{UartNumber, txStreamBuffer};

    static void syncTimeHandler(timeval *tv)
    {
        getApplicationInstance().timeSource.onTimeSync();
    }

    TimeSource timeSource{syncTimeHandler, txStreamBuffer};

    MqttClient mqttClient{txStreamBuffer};
};