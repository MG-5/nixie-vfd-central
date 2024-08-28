#pragma once

#include "esp_log.h"
#include "esp_sntp.h"

#include "sync.hpp"
#include "uart/protocol.hpp"

#include "util/gpio.hpp"
#include "wrappers/Task.hpp"
#include <chrono>

class TimeSource : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    TimeSource(sntp_sync_time_cb_t syncTimeHandler, util::wrappers::StreamBuffer &txStreamBuffer)
        : TaskWithMemberFunctionBase("timeSourceTask", 1024, osPriorityNormal1), //
          syncTimeHandler(syncTimeHandler),                                      //
          txStreamBuffer(txStreamBuffer)
    {
        assert(syncTimeHandler);
    };

    static constexpr auto PrintTag = "[TimeSource]";
    static constexpr auto TimezoneBerlin =
        "CET-1CEST,M3.5.0,M10.5.0/3"; // incl. automatic summer/winter time
    using Timestamp = std::chrono::time_point<std::chrono::system_clock>;

    //--------------------------------------------------------------------------------------------------
    void onTimeSync()
    {
        ESP_LOGI(PrintTag, "Time synchronization event arrived.");
        printLocaltime();
        syncEventGroup.setBits(sync_events::TimeIsSynchronized);

        // send time per UART
        auto localTime = getLocaltime();

        std::string topic = "clock";

        constexpr auto BufferSize = 64;
        char buffer[BufferSize];
        uint16_t numberOfCharacters =
            std::snprintf(buffer, BufferSize, "%02d:%02d:%02d", localTime->tm_hour,
                          localTime->tm_min, localTime->tm_sec);

        PacketHeader header{.topicLength = (uint16_t)topic.size(),
                            .payloadSize = numberOfCharacters};

        txStreamBuffer.send(reinterpret_cast<uint8_t *>(&header), sizeof(header));
        txStreamBuffer.send(reinterpret_cast<uint8_t *>(topic.data()), topic.size());
        txStreamBuffer.send(reinterpret_cast<uint8_t *>(buffer), numberOfCharacters);
    }

    //--------------------------------------------------------------------------------------------------
    [[nodiscard]] static Timestamp getCurrentUTC()
    {
        return std::chrono::system_clock::now();
    }

    //--------------------------------------------------------------------------------------------------
    [[nodiscard]] static std::tm *getLocaltime(Timestamp now = getCurrentUTC())
    {
        const auto CurrentTime = std::chrono::system_clock::to_time_t(now);
        return std::localtime(&CurrentTime);
    }

protected:
    void taskMain(void *) override
    {
        timeSync.init(GPIO_MODE_OUTPUT);
        syncEventGroup.waitBits(sync_events::ConnectedToWifi, pdFALSE, pdTRUE, portMAX_DELAY);

        while (true)
        {
            initTimeSychronization();
            syncEventGroup.waitBits(sync_events::TimeIsSynchronized, pdFALSE, pdTRUE,
                                    toOsTicks(10.0_s));

            // check if time synchronization was successful
            if (syncEventGroup.getBits() & sync_events::TimeIsSynchronized)
                break;
        }

        auto lastWakeTime = xTaskGetTickCount();

        while (true)
        {
            vTaskDelayUntil(&lastWakeTime, toOsTicks(1.0_s));
            timeSync.write(true);
            vTaskDelay(toOsTicks(500.0_ms));
            timeSync.write(false);
        }
    }

private:
    sntp_sync_time_cb_t syncTimeHandler;
    util::wrappers::StreamBuffer &txStreamBuffer;

    util::Gpio timeSync{GPIO_NUM_13};

    //--------------------------------------------------------------------------------------------------
    void initTimeSychronization()
    {
        esp_sntp_stop();
        ESP_LOGI(PrintTag, "Initializing SNTP");
        esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
        esp_sntp_setservername(0, "pool.ntp.org");
        sntp_set_time_sync_notification_cb(syncTimeHandler);
        esp_sntp_init();
        ESP_LOGI(PrintTag, "Update system time every %d minutes.",
                 CONFIG_LWIP_SNTP_UPDATE_DELAY / 1000 / 60);

        // set timezone to Berlin
        setenv("TZ", TimezoneBerlin, 1);
        tzset();
    }

    //--------------------------------------------------------------------------------------------------
    void printLocaltime()
    {
        std::tm *localTime = getLocaltime(getCurrentUTC());
        ESP_LOGI(PrintTag, "local time in Berlin: %02d:%02d:%02d", localTime->tm_hour,
                 localTime->tm_min, localTime->tm_sec);
    }
};