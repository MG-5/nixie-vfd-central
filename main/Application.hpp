#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "helpers/freertos.hpp"

#include "mqtt/MqttClient.hpp"
#include "uart/UartEvent.hpp"
#include "uart/UartTx.hpp"
#include "wifi/Wireless.hpp"

class Application
{
public:
    static constexpr auto PrintTag = "[Application]";

    Application()
    {
        timeoutTimer = xTimerCreate("timeoutTimer", toOsTicks(10.0_s), pdFALSE, nullptr, onTimeout);
    };

    void run();

    static Application &getApplicationInstance();

    static void onTimeout(TimerHandle_t);

private:
    bool isConnected = false;

    Wireless wifi{isConnected};

    static constexpr auto UartNumber = UART_NUM_1;
    util::wrappers::StreamBuffer txStreamBuffer{1024, 1};
    UartEvent uartEvent{UartNumber};
    UartTx uartTx{UartNumber, txStreamBuffer};

    MqttClient mqttClient{txStreamBuffer};

    inline static TimerHandle_t timeoutTimer = nullptr;

    static void stopTimer()
    {
        xTimerStop(timeoutTimer, 0);
    }

    static void resetTimer()
    {
        xTimerReset(timeoutTimer, 0);
    }
};