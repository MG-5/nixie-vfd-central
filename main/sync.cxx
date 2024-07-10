#include "sync.hpp"

namespace
{
EventGroupHandle_t syncEventGroup = xEventGroupCreate();
} // namespace

namespace util::wrappers::sync
{
void waitForOne(EventBits_t events)
{
    (void)xEventGroupWaitBits(syncEventGroup, events, pdFALSE, pdFALSE, portMAX_DELAY);
}

void waitForOne(EventBits_t events, TickType_t ticksToWait)
{
    (void)xEventGroupWaitBits(syncEventGroup, events, pdFALSE, pdFALSE, ticksToWait);
}

void waitForAll(EventBits_t events)
{
    (void)xEventGroupWaitBits(syncEventGroup, events, pdFALSE, pdTRUE, portMAX_DELAY);
}

void waitForAll(EventBits_t events, TickType_t ticksToWait)
{
    (void)xEventGroupWaitBits(syncEventGroup, events, pdFALSE, pdTRUE, ticksToWait);
}

void clearEvents(EventBits_t events)
{
    (void)xEventGroupClearBits(syncEventGroup, events);
}

void signal(EventBits_t events)
{
    (void)xEventGroupSetBits(syncEventGroup, events);
}
} // namespace util::wrappers::sync
