#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

namespace util::wrappers::sync
{
constexpr EventBits_t ConfigurationLoaded = 1 << 0;
constexpr EventBits_t ConnectedToWifi = 1 << 1;
constexpr EventBits_t ConnectionFailed = 1 << 2;
constexpr EventBits_t TimeIsSynchronized = 1 << 3;
constexpr EventBits_t LedDriverStarted = 1 << 4;
constexpr EventBits_t NvmInitialized = 1 << 5;

void waitForOne(EventBits_t events);
void waitForOne(EventBits_t events, TickType_t ticksToWait);
void waitForAll(EventBits_t events);
void waitForAll(EventBits_t events, TickType_t ticksToWait);
void clearEvents(EventBits_t events);

void signal(EventBits_t events);
} // namespace util::wrappers::sync
