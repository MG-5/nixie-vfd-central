#pragma once

#include "driver/ledc.h"
#include "sync.hpp"
#include "wrappers/Task.hpp"

using util::wrappers::TaskWithMemberFunctionBase;

class StatusLed : public TaskWithMemberFunctionBase
{
public:
    static constexpr auto LedPin = gpio_num_t::GPIO_NUM_8;
    static constexpr auto LedChannel = LEDC_CHANNEL_0;

    static constexpr auto PwmMode = LEDC_LOW_SPEED_MODE;
    static constexpr auto PwmResolution = LEDC_TIMER_8_BIT;
    static constexpr auto MaximumDuty = (1 << LEDC_TIMER_8_BIT) - 1;

    StatusLed()
        : TaskWithMemberFunctionBase("statusLedTask", 256, osPriorityBelowNormal3) //
    {};

protected:
    void taskMain(void *) override
    {
        initLedPwm();

        bool toggleLed = false;

        while (true)
        {
            if ((util::wrappers::Task::syncEventGroup.waitBits(sync_events::ConnectedToWifi, true, true, 0) &
                 sync_events::ConnectedToWifi) != 0)
            {
                // connected to wifi

                setLedDuty(25);
                util::wrappers::Task::syncEventGroup.waitBits(sync_events::ConnectionFailed, true, true, portMAX_DELAY);
            }
            else
            {
                // not connected to wifi

                setLedDuty(toggleLed ? 100 : 0);
                toggleLed = !toggleLed;
                vTaskDelay(toOsTicks(500.0_ms));
            }
        }
    }

private:
    void initLedPwm()
    {
        // timer configuration
        ledc_timer_config_t ledcTimer = {.speed_mode = PwmMode,
                                         .duty_resolution = PwmResolution,
                                         .timer_num = LEDC_TIMER_0,
                                         .freq_hz = 5000, // frequency in Hz
                                         .clk_cfg = LEDC_AUTO_CLK,
                                         .deconfigure = false};
        ESP_ERROR_CHECK(ledc_timer_config(&ledcTimer));

        // channel configuration
        ledc_channel_config_t ledcChannel = {.gpio_num = LedPin,
                                             .speed_mode = PwmMode,
                                             .channel = LedChannel,
                                             .intr_type = LEDC_INTR_DISABLE,
                                             .timer_sel = LEDC_TIMER_0,
                                             .duty = 0, // Set duty to 0%
                                             .hpoint = 0,
                                             .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
                                             .flags = {0}};
        ESP_ERROR_CHECK(ledc_channel_config(&ledcChannel));
    }

    void setLedDuty(uint8_t percentage)
    {
        const uint32_t Duty = uint32_t(percentage) * uint32_t(MaximumDuty) / 100;

        ESP_ERROR_CHECK(ledc_set_duty(PwmMode, LedChannel, Duty));
        ESP_ERROR_CHECK(ledc_update_duty(PwmMode, LedChannel));
    }
};