#pragma once

#include "esp_log.h"
#include "esp_sntp.h"

#include "sync.hpp"
#include "uart/protocol.hpp"

#include "helpers/freertos.hpp"
#include "util/gpio.hpp"
#include "wrappers/StreamBuffer.hpp"
#include "wrappers/Task.hpp"
#include <chrono>

class TimeSource : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    TimeSource(sntp_sync_time_cb_t syncTimeHandler, util::wrappers::StreamBuffer &txStream0,
               util::wrappers::StreamBuffer &txStream1)
        : TaskWithMemberFunctionBase("timeSourceTask", 1024, osPriorityNormal1), //
          syncTimeHandler(syncTimeHandler),                                      //
          txStream0(txStream0),                                                  //
          txStream1(txStream1)
    {
        assert(syncTimeHandler);
    };

    static constexpr auto PrintTag = "[TimeSource]";
    static constexpr auto TimezoneBerlin = "CET-1CEST,M3.5.0,M10.5.0/3"; // incl. automatic summer/winter time
    using Timestamp = std::chrono::time_point<std::chrono::system_clock>;

    //--------------------------------------------------------------------------------------------------
    void onTimeSync()
    {
        ESP_LOGI(PrintTag, "Time synchronization event arrived.");
        printLocaltime();
        syncEventGroup.setBits(sync_events::TimeIsSynchronized);
        queueTimeSend();
    }

    //--------------------------------------------------------------------------------------------------
    void sendTimePerUart()
    {
        auto localTime = getLocaltime();

        std::string topic = "clock";

        constexpr auto BufferSize = 64;
        char buffer[BufferSize];
        uint16_t numberOfCharacters = std::snprintf(buffer, BufferSize, "%02d:%02d:%02d", localTime->tm_hour,
                                                    localTime->tm_min, localTime->tm_sec);

        PacketHeader header{.topicLength = (uint16_t)topic.size(), .payloadSize = numberOfCharacters};

        txStream0.send(reinterpret_cast<uint8_t *>(&header), sizeof(header));
        txStream0.send(reinterpret_cast<uint8_t *>(topic.data()), topic.length());
        txStream0.send(reinterpret_cast<uint8_t *>(buffer), numberOfCharacters);

        txStream1.send(reinterpret_cast<uint8_t *>(&header), sizeof(header));
        txStream1.send(reinterpret_cast<uint8_t *>(topic.data()), topic.length());
        txStream1.send(reinterpret_cast<uint8_t *>(buffer), numberOfCharacters);
    }

    //--------------------------------------------------------------------------------------------------

    void sendTimeSync()
    {
        std::string topic = "sync";
        PacketHeader header{.topicLength = (uint16_t)topic.length(), .payloadSize = 0};

        txStream0.send(reinterpret_cast<uint8_t *>(&header), sizeof(header));
        txStream0.send(reinterpret_cast<uint8_t *>(topic.data()), topic.length());

        txStream1.send(reinterpret_cast<uint8_t *>(&header), sizeof(header));
        txStream1.send(reinterpret_cast<uint8_t *>(topic.data()), topic.length());
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

    void queueTimeSend()
    {
        shouldSendTime = true;
    }

    void alignTimeSyncToCountdown()
    {
        prepareForCountdown = true;
        notify(0, util::wrappers::NotifyAction::SetBits);
    }

protected:
    void taskMain(void *) override
    {
        syncEventGroup.waitBits(sync_events::ConnectedToWifi, pdFALSE, pdTRUE, portMAX_DELAY);

        while (true)
        {
            initTimeSychronization();
            syncEventGroup.waitBits(sync_events::TimeIsSynchronized, pdFALSE, pdTRUE, toOsTicks(10.0_s));

            // check if time synchronization was successful
            if (syncEventGroup.getBits() & sync_events::TimeIsSynchronized)
                break;
        }

        while (true)
        {
            delayUntilEventOrTimeout(1.0_s);

            if (prepareForCountdown)
            {
                // align time sync to countdown start/resume
                // wait until next second
                prepareForCountdown = false;
                continue;
            }

            if (shouldSendTime)
            {
                sendTimePerUart();
                shouldSendTime = false;
            }
            else
                sendTimeSync();
        }
    }

private:
    sntp_sync_time_cb_t syncTimeHandler;
    util::wrappers::StreamBuffer &txStream0;
    util::wrappers::StreamBuffer &txStream1;

    bool shouldSendTime = false;
    bool prepareForCountdown = false;

    //--------------------------------------------------------------------------------------------------
    void initTimeSychronization()
    {
        esp_sntp_stop();
        ESP_LOGI(PrintTag, "Initializing SNTP");
        esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
        esp_sntp_setservername(0, "pool.ntp.org");
        sntp_set_time_sync_notification_cb(syncTimeHandler);
        esp_sntp_init();
        ESP_LOGI(PrintTag, "Update system time every %d minutes.", CONFIG_LWIP_SNTP_UPDATE_DELAY / 1000 / 60);

        // set timezone to Berlin
        setenv("TZ", TimezoneBerlin, 1);
        tzset();
    }

    //--------------------------------------------------------------------------------------------------
    void printLocaltime()
    {
        std::tm *localTime = getLocaltime(getCurrentUTC());
        ESP_LOGI(PrintTag, "local time in Berlin: %02d:%02d:%02d", localTime->tm_hour, localTime->tm_min,
                 localTime->tm_sec);
    }

    //--------------------------------------------------------------------------------------------------
    /// block task for specified time but can be unblocked by external event e.g. time sync shift
    /// @return true if timeout is occurred
    bool delayUntilEventOrTimeout(units::si::Time blockTime, bool blockIndefinitely = false)
    {
        return notifyWait(ULONG_MAX, ULONG_MAX, (uint32_t *)0,
                          blockIndefinitely ? portMAX_DELAY : toOsTicks(blockTime)) == 0;
    }
};