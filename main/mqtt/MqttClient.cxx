#include "MqttClient.hpp"
#include "sync.hpp"
#include "uart/protocol.hpp"

#include "helpers/freertos.hpp"
#include <string>

using namespace util::wrappers;

void MqttClient::taskMain(void *)
{
    syncEventGroup.waitBits(sync_events::ConnectedToWifi, pdFALSE, pdTRUE, portMAX_DELAY);
    init();

    while (true)
    {
        vTaskDelay(portMAX_DELAY);
    }
}

//--------------------------------------------------------------------------------------------------
void MqttClient::dataCallback(void *eventData)
{
    auto event = reinterpret_cast<esp_mqtt_event_handle_t>(eventData);
    if (event->topic_len <= 0 || event->data_len <= 0)
        return;

    uint16_t dataLength = event->data_len;

    std::string topic(event->topic, event->topic_len);
    std::string data(event->data, dataLength);

    topic = topic.substr(TopicMainString.size(), topic.size());

    ESP_LOGI(PrintTag, "topic: %s", topic.c_str());
    ESP_LOGI(PrintTag, "data: %s", data.c_str());

    PacketHeader header = {.topicLength = (uint16_t)topic.size(), .payloadSize = dataLength};

    txStream.send(reinterpret_cast<uint8_t *>(&header), sizeof(header));
    txStream.send(reinterpret_cast<uint8_t *>(topic.data()), topic.size());
    txStream.send(reinterpret_cast<uint8_t *>(event->data), dataLength);
}

void MqttClient::init()
{
    // also intialize the mqtt configuration with default values
    client = esp_mqtt_client_init(&mqttConfig);
    if (client == nullptr)
        ESP_LOGE(PrintTag, "Creating mqtt client was not successful!");

    auto returnValue =
        esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, eventHandlerCallback, this);
    if (returnValue != ESP_OK)
        ESP_LOGE(PrintTag, "Cannot register mqtt client callback! error: %s",
                 esp_err_to_name(returnValue));

    returnValue = esp_mqtt_client_start(client);
    if (returnValue != ESP_OK)
        ESP_LOGE(PrintTag, "Cannot start mqtt client! error: %s", esp_err_to_name(returnValue));
}

void MqttClient::createSubscribers()
{
    auto messageId = esp_mqtt_client_subscribe(client, TopicWildcard.data(), 0);

    if (messageId < 0)
        ESP_LOGE(PrintTag, "Cannot subscribe to topic %s! error: %d", TopicWildcard.data(),
                 messageId);
}

//--------------------------------------------------------------------------------------------------
void MqttClient::eventHandlerCallback(void *handlerArgs, esp_event_base_t base, int32_t eventId,
                                      void *eventData)
{
    auto mqttClientContext = reinterpret_cast<MqttClient *>(handlerArgs);

    ESP_LOGD(PrintTag, "Event dispatched from event loop base=%s, eventId=%ld", base, eventId);
    esp_mqtt_event_handle_t event = reinterpret_cast<esp_mqtt_event_handle_t>(eventData);

    switch ((esp_mqtt_event_id_t)eventId)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(PrintTag, "MQTT_EVENT_CONNECTED");
        mqttClientContext->createSubscribers();
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(PrintTag, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGD(PrintTag, "MQTT_EVENT_SUBSCRIBED, msgId=%d", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGD(PrintTag, "MQTT_EVENT_UNSUBSCRIBED, msgId=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGD(PrintTag, "MQTT_EVENT_PUBLISHED, msgId=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGD(PrintTag, "MQTT_EVENT_DATA");
        mqttClientContext->dataCallback(eventData);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(PrintTag, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            logErrorIfNotZero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            logErrorIfNotZero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            logErrorIfNotZero("captured as transport's socket errno",
                              event->error_handle->esp_transport_sock_errno);
            ESP_LOGE(PrintTag, "Last errno string (%s)",
                     strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;

    default:
        ESP_LOGD(PrintTag, "Other event id:%d", event->event_id);
        break;
    }
}

//--------------------------------------------------------------------------------------------------
void MqttClient::logErrorIfNotZero(const char *message, int errorCode)
{
    if (errorCode != 0)
        ESP_LOGE(PrintTag, "Last error %s: 0x%x", message, errorCode);
}