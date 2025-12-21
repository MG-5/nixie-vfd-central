#include "Application.hpp"
#include "sync.hpp"
#include "wrappers/Task.hpp"

#include "esp_log.h"
#include "wifi_manager.h"
#include <memory>

// -------------------------------------------------------------------------------------------------
void connectionCallback(void *pvParameter)
{
    ip_event_got_ip_t *param = (ip_event_got_ip_t *)pvParameter;

    char str_ip[16];
    esp_ip4addr_ntoa(&param->ip_info.ip, str_ip, IP4ADDR_STRLEN_MAX);

    ESP_LOGI("Wifi", "Connection to Wifi establisched with IP: %s!", str_ip);

    util::wrappers::Task::syncEventGroup.clearBits(sync_events::ConnectionFailed);
    util::wrappers::Task::syncEventGroup.setBits(sync_events::ConnectedToWifi);
}

// -------------------------------------------------------------------------------------------------
void disconnectionCallback(void *pvParameter)
{
    ESP_LOGI("Wifi", "Disconnected from Wifi!");

    util::wrappers::Task::syncEventGroup.clearBits(sync_events::ConnectedToWifi);
    util::wrappers::Task::syncEventGroup.setBits(sync_events::ConnectionFailed);
}

// called by ESP-IDF
extern "C" void app_main(void) // NOLINT
{
    auto previousHeapFreeSpace = esp_get_free_heap_size();
    auto &app = Application::getApplicationInstance();
    auto currentHeapFreeSpace = esp_get_free_heap_size();

    ESP_LOGI(Application::PrintTag, "Moin");
    ESP_LOGI(Application::PrintTag, "Free memory: %lu bytes", currentHeapFreeSpace);
    ESP_LOGI(Application::PrintTag, "Application consumes %lu bytes on heap",
             (previousHeapFreeSpace - currentHeapFreeSpace));

    vTaskDelay(toOsTicks(2.0_s));

    wifi_manager_start();
    wifi_manager_set_callback(WM_EVENT_STA_GOT_IP, &connectionCallback);
    wifi_manager_set_callback(WM_EVENT_STA_DISCONNECTED, &disconnectionCallback);
    app.run();
}

//--------------------------------------------------------------------------------------------------
void Application::run()
{
    util::wrappers::Task::applicationIsReadyStartAllTasks();

    vTaskSuspend(nullptr);
}

//--------------------------------------------------------------------------------------------------
Application &Application::getApplicationInstance()
{
    static auto app = std::make_unique<Application>();
    return *app;
}