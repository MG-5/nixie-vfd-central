#include "PacketProcessor.hpp"
#include "protocol.hpp"
#include "sync.hpp"

#include <algorithm>
#include <cstring>
#include <string>

void PacketProcessor::taskMain(void *)
{
    while (true)
    {
        if (!extractPacketFromReceiveStream())
            continue;

        processPacket();
    }
}

//-----------------------------------------------------------------------------
void PacketProcessor::processPacket()
{
    std::string topicString{reinterpret_cast<char *>(topic), header.topicLength};
    std::string payloadString = {reinterpret_cast<char *>(payload), header.payloadSize};

    ESP_LOGI(PrintTag, "Processing packet");
    ESP_LOGI(PrintTag, "Topic %s", topicString.c_str());
    ESP_LOGI(PrintTag, "Payload %s", payloadString.c_str());

    if (topicString == "clock" && payloadString == "request")
    {
        ESP_LOGI(PrintTag, "Time request received.");
        timeSource.sendTimePerUart();
    }
}

//-----------------------------------------------------------------------------
bool PacketProcessor::extractPacketFromReceiveStream()
{
    topic = nullptr;
    payload = nullptr;

    if (bufferStartPosition == bufferLastPosition)
    {
        bufferStartPosition = 0;
        bufferLastPosition = 0;
    }

    const auto NumberOfBytes =
        rxStream.receive(rxBuffer + bufferLastPosition, RxBufferSize - bufferLastPosition, portMAX_DELAY);

    if (NumberOfBytes == 0)
        return false; // no bytes received

    bufferLastPosition += NumberOfBytes;

    if (bufferLastPosition >= RxBufferSize && bufferStartPosition > 0)
    {
        // shift buffer to the beginning to make space for new data
        std::memmove(rxBuffer, rxBuffer + bufferStartPosition, bufferLastPosition - bufferStartPosition);

        bufferStartPosition = 0;
        bufferLastPosition -= bufferStartPosition;
    }

    while (true)
    {
        if (bufferLastPosition - bufferStartPosition < sizeof(PacketHeader))
        {
            // no header to read in available in buffer
            // wait for new bytes from UART
            return false;
        }

        std::memcpy(&header, rxBuffer + bufferStartPosition, sizeof(PacketHeader));

        if (header.magic == ProtocolMagic)
            break; // header found

        // no valid packet header found in buffer
        // get rid the first byte in buffer and try it again
        bufferStartPosition++;
    }

    if (header.topicLength + header.payloadSize > (bufferLastPosition - bufferStartPosition) - sizeof(PacketHeader))
    {
        // not enough data for the expected topic and payload
        // wait for new bytes from UART
        return false;
    }

    topic = rxBuffer + bufferStartPosition + sizeof(PacketHeader);

    if (header.payloadSize > 0)
        payload = topic + header.topicLength;

    bufferStartPosition += sizeof(PacketHeader) + header.topicLength + header.payloadSize;
    return true;
}
