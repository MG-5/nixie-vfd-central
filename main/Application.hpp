#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "helpers/freertos.hpp"

class Application
{
public:
    static constexpr auto PrintTag = "[Application]";

    Application()
    {
        timeoutTimer = xTimerCreate("timeoutTimer", toOsTicks(10.0_s), pdFALSE, nullptr, onTimeout);
    };

    void run();

    static Application &getApplicationInstance();

    static void onTimeout(TimerHandle_t);

private:
    bool isConnected = false;

    inline static TimerHandle_t timeoutTimer = nullptr;

    static void stopTimer()
    {
        xTimerStop(timeoutTimer, 0);
    }

    static void resetTimer()
    {
        xTimerReset(timeoutTimer, 0);
    }
};