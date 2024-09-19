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

    static constexpr auto Uart1Number = UART_NUM_1;
    static constexpr auto Uart2Number = UART_NUM_2;
    static constexpr auto BufferSize = 512;

    util::wrappers::StreamBuffer uart1TxStream{BufferSize, 1};
    util::wrappers::StreamBuffer uart1RxStream{BufferSize, 1};
    util::wrappers::StreamBuffer uart2TxStream{BufferSize, 1};
    util::wrappers::StreamBuffer uart2RxStream{BufferSize, 1};

    UartEvent uart1Event{Uart1Number, uart1RxStream};
    UartTx uart1Tx{Uart1Number, uart1TxStream};
    UartEvent uart2Event{Uart2Number, uart2RxStream};
    UartTx uart2Tx{Uart2Number, uart2TxStream};

    static void syncTimeHandler(timeval *tv)
    {
        getApplicationInstance().timeSource.onTimeSync();
    }

    TimeSource timeSource{syncTimeHandler, uart1TxStream, uart2TxStream};
    PacketProcessor packetProcessorUart1{uart1RxStream, timeSource};
    PacketProcessor packetProcessorUart2{uart2RxStream, timeSource};
    MqttClient mqttClient{uart1TxStream, uart2TxStream};
};