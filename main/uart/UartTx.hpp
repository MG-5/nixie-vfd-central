#pragma once

#include "driver/uart.h"
#include "esp_log.h"

#include "wrappers/StreamBuffer.hpp"
#include "wrappers/Task.hpp"

#include <array>

class UartTx : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    UartTx(uart_port_t uartNumber, util::wrappers::StreamBuffer &txStreamBuffer)
        : TaskWithMemberFunctionBase("uartTxTask", 1024, osPriorityNormal1), //
          UartNumber(uartNumber),                                            //
          txStreamBuffer(txStreamBuffer)                                     //
    {};

protected:
    void taskMain(void *) override
    {
        // esp_log_level_set(PrintTag, ESP_LOG_DEBUG);

        while (true)
        {
            const auto PacketSize =
                txStreamBuffer.receive(rawDataBuffer.data(), BufferSize, portMAX_DELAY);

            uart_write_bytes(UartNumber, rawDataBuffer.data(), PacketSize);
            ESP_LOGI(PrintTag, "Sent %d bytes over uart%d", PacketSize, UartNumber);
            printData(PacketSize);
        }
    }

private:
    const uart_port_t UartNumber{};
    util::wrappers::StreamBuffer &txStreamBuffer;
    static constexpr auto PrintTag = "[UartTx]";

    static constexpr auto BufferSize = 1024;
    std::array<uint8_t, BufferSize> rawDataBuffer{};

    void printData(size_t packetSize)
    {
        if (esp_log_level_get(PrintTag) != esp_log_level_t::ESP_LOG_DEBUG &&
            esp_log_level_get(PrintTag) != esp_log_level_t::ESP_LOG_VERBOSE)
            return;

        std::string foo;

        for (int i = 0; i < packetSize; i++)
        {
            foo += std::to_string(rawDataBuffer[i]);
            foo += " ";
        }

        ESP_LOGD(PrintTag, "%s", foo.c_str());
    }
};
