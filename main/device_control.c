#include "device_control.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/ledc.h"

#include "owb.h"
#include "owb_rmt.h"
#include "ds18b20.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include <math.h>

ledc_channel_config_t ledc_channel[LEDC_CH_NUM] = {
	{.channel = LEDC_HS_CH0_CHANNEL,
	 .duty = 0,
	 .gpio_num = LEDC_HS_CH0_GPIO,
	 .speed_mode = LEDC_HS_MODE,
	 .hpoint = 0,
	 .timer_sel = LEDC_HS_TIMER},
	{.channel = LEDC_HS_CH1_CHANNEL,
	 .duty = 0,
	 .gpio_num = LEDC_HS_CH1_GPIO,
	 .speed_mode = LEDC_HS_MODE,
	 .hpoint = 0,
	 .timer_sel = LEDC_HS_TIMER},
	{.channel = LEDC_HS_CH2_CHANNEL,
	 .duty = 0,
	 .gpio_num = LEDC_HS_CH2_GPIO,
	 .speed_mode = LEDC_HS_MODE,
	 .hpoint = 0,
	 .timer_sel = LEDC_HS_TIMER},
};

void change_switch_state(int switch_state)
{
	if (switch_state == SWITCH_OFF)
	{
		gpio_set_level(GPIO_OUTPUT_MAINLED, LED_OFF);
	}
	else
	{
		gpio_set_level(GPIO_OUTPUT_MAINLED, LED_ON);
	}
}

void change_thermostat_state(int *thermostat_state)
{
	if (thermostat_state == THERMOSTAT_OFF)
	{
		*thermostat_state = THERMOSTAT_ON;
	}
	else
	{
		*thermostat_state = THERMOSTAT_OFF;
	}
}
int get_button_event()
{
	static uint32_t button_last_state = SWITCH_OFF;
	uint32_t gpio_level = SWITCH_OFF;

	gpio_level = gpio_get_level(GPIO_INPUT_SWITCH);
	if (button_last_state != gpio_level)
	{
		printf("BTN LAST STATE(%d) != gpio level(%d)\n", button_last_state, gpio_level);
		/* wait debounce time to ignore small ripple of currunt */
		vTaskDelay(pdMS_TO_TICKS(BUTTON_DEBOUNCE_TIME_MS));
		gpio_level = gpio_get_level(GPIO_INPUT_SWITCH);
		if (button_last_state != gpio_level)
		{
			printf("Button event, val: %d, \n", gpio_level);
			button_last_state = gpio_level;
			if (gpio_level == SWITCH_ON)
			{
				return 1;
			}
		}
	}

	return 0;
}

void iot_gpio_init(void)
{
	// esp sdk specific
	gpio_config_t io_conf;

	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = 1 << GPIO_OUTPUT_MAINLED;
	io_conf.pull_down_en = 1;
	io_conf.pull_up_en = 0;
	gpio_config(&io_conf);
	io_conf.pin_bit_mask = 1 << GPIO_OUTPUT_BUZZER;
	gpio_config(&io_conf);

	io_conf.pin_bit_mask = 1ULL << GPIO_OUTPUT_RGBLED_B;
	gpio_config(&io_conf);
	io_conf.intr_type = GPIO_INTR_ANYEDGE;
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pin_bit_mask = 1 << GPIO_INPUT_SWITCH;
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 1;
	gpio_config(&io_conf);

	gpio_install_isr_service(0);

	ledc_timer_config_t ledc_timer = {
		.duty_resolution = LEDC_TIMER_13_BIT,
		.freq_hz = 5000,
		.speed_mode = LEDC_HS_MODE,
		.timer_num = LEDC_HS_TIMER,
		.clk_cfg = 0,
	};
	ledc_timer_config(&ledc_timer);
	int ch;
	for (ch = 0; ch < LEDC_CH_NUM; ch++)
	{
		ledc_channel_config(&ledc_channel[ch]);
	}
	ledc_fade_func_install(0);

	gpio_set_level(GPIO_OUTPUT_MAINLED, LED_ON);
	gpio_set_level(GPIO_OUTPUT_BUZZER, BUZZER_OFF);

	// //ledc_set_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel, LEDC_MAX_DUTY);
	// //ledc_update_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel);

	// ledc_set_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel, LEDC_MAX_DUTY);
	// ledc_update_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel);
	// //ledc_set_duty(ledc_channel[2].speed_mode, ledc_channel[2].channel, LEDC_MAX_DUTY);
	// //ledc_update_duty(ledc_channel[2].speed_mode, ledc_channel[2].channel);
	// vTaskDelay(pdMS_TO_TICKS(1000));

	// ledc_set_fade_with_time(ledc_channel[0].speed_mode,
	// ledc_channel[0].channel, LEDC_MAX_DUTY, LEDC_FADE_TIME*2);
	// ledc_fade_start(ledc_channel[0].speed_mode,
	// 		ledc_channel[0].channel, LEDC_FADE_NO_WAIT);
	// vTaskDelay(pdMS_TO_TICKS(4000));

	// ledc_set_fade_with_time(ledc_channel[1].speed_mode,
	// 		ledc_channel[1].channel, 0, LEDC_FADE_TIME*2);
	// ledc_fade_start(ledc_channel[1].speed_mode,
	// 		ledc_channel[1].channel, LEDC_FADE_NO_WAIT);
	// vTaskDelay(pdMS_TO_TICKS(4000));
}

void change_rgb_state(int pin, int value)
{
	if (pin == DAC_OUTPUT_RGBLED_R)
	{
		dac_output_voltage(DAC_OUTPUT_RGBLED_R, value);
	}
	else if (pin == DAC_OUTPUT_RGBLED_G)
	{
		dac_output_voltage(DAC_OUTPUT_RGBLED_G, value);
	}
	if (pin == GPIO_OUTPUT_RGBLED_B)
	{
		gpio_set_level(GPIO_OUTPUT_RGBLED_B, value);
	}
}

void setpoint_rgb_indication(double heating_setpoint)
{
	// any signalling of chosen temperature/heating mode
	if (heating_setpoint <= 30)
	{
		//green
		change_rgb_state(GPIO_OUTPUT_RGBLED_B, LED_OFF);
		change_rgb_led_state(0, 255, 0);
	}
	else if (heating_setpoint <= 50)
	{
		// yellow
		change_rgb_state(GPIO_OUTPUT_RGBLED_B, LED_OFF);
		change_rgb_led_state(255, 255, 0);
	}
	else if (heating_setpoint <= 75)
	{
		//orange
		change_rgb_state(GPIO_OUTPUT_RGBLED_B, LED_OFF);
		change_rgb_led_state(255, 195, 0);
	}
	else if (heating_setpoint <= 100)
	{
		//red
		change_rgb_state(GPIO_OUTPUT_RGBLED_B, LED_OFF);
		change_rgb_led_state(255, 0, 0);
	}
	else
	{
		printf("heating setpoint is more than 100 or not set!\nPlease, set correct number");
	}

	vTaskDelay(pdMS_TO_TICKS(HEATING_SETPOINT_RGB_DURATION));
	change_rgb_state(DAC_OUTPUT_RGBLED_G, LED_OFF);
	change_rgb_state(DAC_OUTPUT_RGBLED_R, LED_OFF);
	change_rgb_state(GPIO_OUTPUT_RGBLED_B, LED_ON);
}

void temperature_events_task(void *arg)
{
	QueueHandle_t queue = (QueueHandle_t)arg;
	// Create a 1-Wire bus, using the RMT timeslot driver
	owb_rmt_driver_info rmt_driver_info;
	OneWireBus *owb;
	owb = owb_rmt_initialize(&rmt_driver_info, GPIO_TEMPERATURE_SENSOR, RMT_CHANNEL_1, RMT_CHANNEL_0);
	owb_use_crc(owb, true); // enable CRC check for ROM code
	DS18B20_Info *ds18b20_info;
	ds18b20_info = ds18b20_malloc();
	ds18b20_init_solo(ds18b20_info, owb);
	ds18b20_use_crc(ds18b20_info, true);
	ds18b20_set_resolution(ds18b20_info, DS18B20_RESOLUTION_12_BIT);

	float temperature_value;
	double send_value;
	for (;;)
	{
		ds18b20_convert_all(owb);
		ds18b20_wait_for_conversion(ds18b20_info);
		ds18b20_read_temp(ds18b20_info, &temperature_value);
		printf("TEMP READ: %f\n", temperature_value);
		send_value = (double)temperature_value;
		xQueueSendToBack(queue, &send_value, 0);
		vTaskDelay(pdMS_TO_TICKS(TEMPERATURE_EVENT_MS_RATE));
	}
}

void change_buzzer_state(int buzzer_state)
{
	if (buzzer_state == BUZZER_OFF)
	{
		gpio_set_level(GPIO_OUTPUT_BUZZER, BUZZER_OFF);
	}
	else
	{
		gpio_set_level(GPIO_OUTPUT_BUZZER, BUZZER_ON);
	}
}
void beep()
{
	printf("BEEP CALLED\n");
	change_buzzer_state(BUZZER_ON);
	vTaskDelay(pdMS_TO_TICKS(BUZZER_SOUND_DURATION));
	change_buzzer_state(BUZZER_OFF);
}

void change_rgb_led_heating(double heating_setpoint, double prev_temperature, double current_temperature)
{

	double min = 0;
	double max = heating_setpoint;

	double progress_f = (current_temperature - min) / (max - min) * 100;
	double progress_delta_f = ((prev_temperature - min) / (max - min) * 100) - progress_f;
	int progress = (int)round(progress_f);
	int progress_delta = (int)round(progress_delta_f);

	printf("heating progress: %d %% \n", progress);
	printf("delta progress: %d %% \n", progress_delta);

	printf("Boiling RGB before update: %d, %d, %d\n",
		   ledc_get_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel),
		   ledc_get_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel),
		   ledc_get_duty(ledc_channel[2].speed_mode, ledc_channel[2].channel));


	int duty_step = LEDC_MAX_DUTY / 100;
	int updated_duty;
	if (progress < 25)
	{
		ledc_set_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel, LEDC_MAX_DUTY);
		ledc_update_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel);
		updated_duty = ledc_get_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel) + duty_step * progress_delta;
		printf("updated duty: %d", updated_duty);
		ledc_set_fade_with_time(ledc_channel[0].speed_mode, ledc_channel[0].channel, updated_duty, TEMPERATURE_EVENT_MS_RATE);
		ledc_fade_start(ledc_channel[0].speed_mode,
						ledc_channel[0].channel, LEDC_FADE_NO_WAIT);
		// red += 2.5 * progress;
	}
	else if (progress < 50)
	{

		ledc_set_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel, LEDC_MAX_DUTY);
		ledc_update_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel);
		updated_duty = ledc_get_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel) + duty_step * progress_delta;
		printf("updated duty: %d", updated_duty);

		ledc_set_fade_with_time(ledc_channel[0].speed_mode, ledc_channel[0].channel, updated_duty, TEMPERATURE_EVENT_MS_RATE);
		ledc_fade_start(ledc_channel[0].speed_mode,
						ledc_channel[0].channel, LEDC_FADE_NO_WAIT);
		// red += 5 * progress;
	}
	else if (progress >= 50)
	{

		ledc_set_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel, LEDC_MAX_DUTY);
		ledc_update_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel);
		updated_duty = ledc_get_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel) - duty_step * progress_delta;
		ledc_set_fade_with_time(ledc_channel[1].speed_mode, ledc_channel[1].channel, updated_duty, TEMPERATURE_EVENT_MS_RATE);
		ledc_fade_start(ledc_channel[1].speed_mode,
						ledc_channel[1].channel, LEDC_FADE_NO_WAIT);
		// red = 255;
		// green -= 2.5 * (progress - 50);
	}
	else if (progress >= 75)
	{
		ledc_set_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel, LEDC_MAX_DUTY);
		ledc_update_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel);
		updated_duty = ledc_get_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel) - duty_step * progress_delta;
		ledc_set_fade_with_time(ledc_channel[1].speed_mode, ledc_channel[1].channel, updated_duty, TEMPERATURE_EVENT_MS_RATE);
		ledc_fade_start(ledc_channel[1].speed_mode,
						ledc_channel[1].channel, LEDC_FADE_NO_WAIT);
		// red = 255;
		// green -= 5 * (progress - 50);
	}

	vTaskDelay(pdMS_TO_TICKS(RGB_BOILING_ADJUSTMENT_DURATION));
}

void change_rgb_led_state(int red, int green, int blue)
{
	dac_output_voltage(DAC_OUTPUT_RGBLED_G, green);
	dac_output_voltage(DAC_OUTPUT_RGBLED_R, red);
	gpio_set_level(GPIO_OUTPUT_RGBLED_B, blue);
}