#pragma once

#include "freertos/event_groups.h"

namespace sync_events
{
constexpr EventBits_t ConnectedToWifi = 1 << 1;
constexpr EventBits_t ConnectionFailed = 1 << 2;
constexpr EventBits_t TimeIsSynchronized = 1 << 3;

} // namespace sync_events
