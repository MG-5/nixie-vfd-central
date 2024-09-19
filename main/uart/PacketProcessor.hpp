#pragma once

#include "wrappers/EventGroup.hpp"
#include "wrappers/StreamBuffer.hpp"
#include "wrappers/Task.hpp"

#include "UartTx.hpp"
#include "protocol.hpp"
#include "time/TimeSource.hpp"

class PacketProcessor : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    PacketProcessor(util::wrappers::StreamBuffer &rxStream, TimeSource &timeSource)
        : TaskWithMemberFunctionBase("packetProcessor", 1024, osPriorityAboveNormal), //
          rxStream(rxStream),                                                         //
          timeSource(timeSource) {};

    static constexpr auto PrintTag = "[PacketProcessor]";

protected:
    [[noreturn]] void taskMain(void *) override;

private:
    util::wrappers::StreamBuffer &rxStream;
    TimeSource &timeSource;

    size_t bufferStartPosition = 0;
    size_t bufferLastPosition = 0;

    uint8_t *topic = nullptr;
    uint8_t *payload = nullptr;
    PacketHeader header{};

    static constexpr auto RxBufferSize = 256;
    uint8_t rxBuffer[RxBufferSize];

    bool extractPacketFromReceiveStream();
    void processPacket();
};