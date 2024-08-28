#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "helpers/freertos.hpp"

#include "mqtt/MqttClient.hpp"
#include "time/TimeSource.hpp"
#include "uart/PacketProcessor.hpp"
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
    static constexpr auto BufferSize = 512;
    util::wrappers::StreamBuffer txStream{BufferSize, 1};
    util::wrappers::StreamBuffer rxStream{BufferSize, 1};
    UartEvent uartEvent{UartNumber, rxStream};
    UartTx uartTx{UartNumber, txStream};

    static void syncTimeHandler(timeval *tv)
    {
        getApplicationInstance().timeSource.onTimeSync();
    }

    TimeSource timeSource{syncTimeHandler, txStream};
    PacketProcessor packetProcessor{rxStream, timeSource};
    MqttClient mqttClient{txStream};
};