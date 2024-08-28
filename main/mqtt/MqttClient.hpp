#pragma once

#include "esp_event.h"
#include "esp_log.h"
#include "mqtt_client.h"

#include "units/si/time.hpp"
#include "wrappers/StreamBuffer.hpp"
#include "wrappers/Task.hpp"

#include <array>
#include <string>
#include <string_view>

class MqttClient : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    static constexpr auto PrintTag = "[MqttClient]";
    static constexpr auto RetryDelay = 5.0_s;
    static constexpr auto BrokerUri = "mqtt://10.0.0.25:1883";

    static constexpr std::string_view TopicWildcard = "nixie-vfd-clock/#";
    static constexpr std::string_view TopicMainString = "nixie-vfd-clock/";

    MqttClient(util::wrappers::StreamBuffer &txStreamBuffer)
        : TaskWithMemberFunctionBase("mqttClientTask", 1024, osPriorityNormal3), //
          txStreamBuffer(txStreamBuffer)                                         //
    {};

    static void eventHandlerCallback(void *handlerArgs, esp_event_base_t base, int32_t eventId,
                                     void *eventData);

protected:
    void taskMain(void *) override;

private:
// supress initialization warning
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    esp_mqtt_client_config_t mqttConfig = {.broker = {.address = {.uri = BrokerUri}}};
#pragma GCC diagnostic pop

    esp_mqtt_client_handle_t client;

    util::wrappers::StreamBuffer &txStreamBuffer;

    void init();
    void subscribeToTopic(const std::string_view &topic);
    void createSubscribers();
    static void logErrorIfNotZero(const char *message, int errorCode);
    void dataCallback(void *eventData);

    static constexpr auto BufferSize = 256;
    char buffer[BufferSize];
};