#pragma once

#include "driver/uart.h"
#include "esp_log.h"

#include "wrappers/StreamBuffer.hpp"
#include "wrappers/Task.hpp"

#include <array>

class UartTx : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    UartTx(uart_port_t uartNumber, util::wrappers::StreamBuffer &txStream)
        : TaskWithMemberFunctionBase("uartTxTask", 1024, osPriorityNormal1), //
          UartNumber(uartNumber),                                            //
          txStream(txStream)                                                 //
    {};

protected:
    void taskMain(void *) override
    {
        while (true)
        {
            const auto PacketSize = txStream.receive(rawDataBuffer.data(), BufferSize, portMAX_DELAY);

            uart_write_bytes(UartNumber, rawDataBuffer.data(), PacketSize);
            ESP_LOGI(PrintTag, "Sent %d bytes over uart%d", PacketSize, UartNumber);
        }
    }

private:
    const uart_port_t UartNumber{};
    util::wrappers::StreamBuffer &txStream;
    static constexpr auto PrintTag = "[UartTx]";

    static constexpr auto BufferSize = 256;
    std::array<uint8_t, BufferSize> rawDataBuffer{};
};
