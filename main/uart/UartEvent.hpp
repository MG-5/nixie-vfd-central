#pragma once

#include "driver/uart.h"

#include "wrappers/StreamBuffer.hpp"
#include "wrappers/Task.hpp"

#include <array>

class UartEvent : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    explicit UartEvent(uart_port_t uartNumber)
        : TaskWithMemberFunctionBase("uartEventTask", 1024, osPriorityNormal2), //
          UartNumber(uartNumber) {};

protected:
    void taskMain(void *) override;

private:
    const uart_port_t UartNumber{};
    static constexpr auto PrintTag = "[UartEvent]";

    void init();

    static constexpr auto BufferSize = 1024;
    std::array<uint8_t, BufferSize> rawDataBuffer{};

    QueueHandle_t eventQueue = nullptr;
};
