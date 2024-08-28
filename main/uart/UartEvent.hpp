#pragma once

#include "driver/uart.h"

#include "wrappers/StreamBuffer.hpp"
#include "wrappers/Task.hpp"

#include <array>

class UartEvent : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    explicit UartEvent(uart_port_t uartNumber, util::wrappers::StreamBuffer &rxStream)
        : TaskWithMemberFunctionBase("uartEventTask", 1024, osPriorityNormal2), //
          UartNumber(uartNumber),                                               //
          rxStream(rxStream) {};

protected:
    void taskMain(void *) override;

private:
    const uart_port_t UartNumber{};
    static constexpr auto PrintTag = "[UartEvent]";

    void init();

    static constexpr auto RxBufferSize = 256;
    std::array<uint8_t, RxBufferSize> rawDataBuffer{};
    util::wrappers::StreamBuffer &rxStream;

    QueueHandle_t eventQueue = nullptr;
};
