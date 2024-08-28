#include "Application.hpp"
#include "sync.hpp"
#include "wrappers/Task.hpp"

#include "esp_log.h"
#include <memory>

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

    vTaskDelay(toOsTicks(100.0_ms));

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