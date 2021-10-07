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
#include <stdbool.h>

// static temperature_state_t *temperature_state;

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
		.timer_num = LEDC_TIMER,
		.clk_cfg = 0,
	};
	ledc_timer_config(&ledc_timer);
	ledc_channel_config_t ledc_channel = {
		.channel = LEDC_RED_CHANNEL,
		.duty = 0,
		.gpio_num = LEDC_RED_GPIO,
		.speed_mode = LEDC_HS_MODE,
		.hpoint = 0,
		.timer_sel = LEDC_TIMER};
	ledc_channel_config(&ledc_channel);
	ledc_channel.gpio_num = LEDC_GREEN_GPIO;
	ledc_channel.channel = LEDC_GREEN_CHANNEL;
	ledc_channel_config(&ledc_channel);
	ledc_channel.gpio_num = LEDC_BLUE_GPIO;
	ledc_channel.channel = LEDC_BLUE_CHANNEL;
	ledc_channel_config(&ledc_channel);
	ledc_fade_func_install(0);
	gpio_set_level(GPIO_OUTPUT_MAINLED, LED_ON);
	gpio_set_level(GPIO_OUTPUT_BUZZER, BUZZER_OFF);

}

void setpoint_rgb_indication(double heating_setpoint)
{
	// any signalling of chosen temperature/heating mode
	if (heating_setpoint <= 30)
	{
		//green
		change_rgb_led_state(LEDC_MIN_DUTY, LEDC_MAX_DUTY, LEDC_MIN_DUTY);
	}
	else if (heating_setpoint <= 50)
	{
		// yellow
		change_rgb_led_state(LEDC_MAX_DUTY, LEDC_MAX_DUTY, LEDC_MIN_DUTY);
	}
	else if (heating_setpoint <= 75)
	{
		//orange
		change_rgb_led_state(LEDC_MAX_DUTY, (int)round(LEDC_MAX_DUTY / 2), LEDC_MIN_DUTY);
	}
	else if (heating_setpoint <= 100)
	{
		//red
		change_rgb_led_state(LEDC_MAX_DUTY, LEDC_MIN_DUTY, LEDC_MIN_DUTY);
	}
	else
	{
		printf("heating setpoint is more than 100 or not set!\nPlease, set correct number");
	}

	vTaskDelay(pdMS_TO_TICKS(HEATING_SETPOINT_RGB_DURATION));
	change_rgb_led_state(LEDC_MIN_DUTY, LEDC_MIN_DUTY, LEDC_MAX_DUTY);
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
	double progress = (current_temperature - min) / (max - min);
	double prev_progress = (prev_temperature - min)/ (max - min);
	double progress_middle = 0.5;
	double total_duty_resource = LEDC_MAX_DUTY * 2;

	printf("heating progress: %f \n", progress);
	printf("previous progress: %f \n", prev_progress);
	printf("Boiling RGB before update: %d, %d, %d\n",
		   ledc_get_duty(LEDC_HS_MODE, LEDC_RED_CHANNEL),
		   ledc_get_duty(LEDC_HS_MODE, LEDC_GREEN_CHANNEL),
		   ledc_get_duty(LEDC_HS_MODE, LEDC_BLUE_CHANNEL));

	double update_progress;
	int updated_duty;
	bool just_crossed_middle_up = progress > progress_middle && prev_progress < progress_middle;
	bool just_crossed_middle_down = progress < progress_middle && prev_progress > progress_middle;

	if (progress < progress_middle) {
		// Green max
		ledc_set_duty(LEDC_HS_MODE, LEDC_GREEN_CHANNEL, LEDC_MAX_DUTY);
		ledc_update_duty(LEDC_HS_MODE, LEDC_GREEN_CHANNEL);
		if (just_crossed_middle_down) {
			update_progress = progress_middle - progress;
			updated_duty = LEDC_MAX_DUTY - update_progress * total_duty_resource;
		}
		else {
			update_progress = progress - prev_progress;
			updated_duty = ledc_get_duty(LEDC_HS_MODE, LEDC_RED_CHANNEL) + update_progress * total_duty_resource;
		}
		ledc_set_duty(LEDC_HS_MODE, LEDC_RED_CHANNEL, updated_duty);
		ledc_update_duty(LEDC_HS_MODE, LEDC_RED_CHANNEL);
	}
	else {
		// Red max
		ledc_set_duty(LEDC_HS_MODE, LEDC_RED_CHANNEL, LEDC_MAX_DUTY);
		ledc_update_duty(LEDC_HS_MODE, LEDC_RED_CHANNEL);
		if (just_crossed_middle_up) {
			update_progress = progress - progress_middle;
			updated_duty = LEDC_MAX_DUTY - update_progress * total_duty_resource;
		}
		else {
			update_progress = progress - prev_progress;
			updated_duty = ledc_get_duty(LEDC_HS_MODE, LEDC_GREEN_CHANNEL) - update_progress * total_duty_resource;
		}
		ledc_set_duty(LEDC_HS_MODE, LEDC_GREEN_CHANNEL, updated_duty);
		ledc_update_duty(LEDC_HS_MODE, LEDC_GREEN_CHANNEL);
	}
	printf("heating progress: %f \n", progress);
	printf("previous progress: %f \n", prev_progress);
	printf("Boiling RGB after update: %d, %d, %d\n",
		   ledc_get_duty(LEDC_HS_MODE, LEDC_RED_CHANNEL),
		   ledc_get_duty(LEDC_HS_MODE, LEDC_GREEN_CHANNEL),
		   ledc_get_duty(LEDC_HS_MODE, LEDC_BLUE_CHANNEL));
	vTaskDelay(pdMS_TO_TICKS(RGB_BOILING_ADJUSTMENT_DURATION));
}

void change_rgb_led_state(int red, int green, int blue)
{
	ledc_set_duty(LEDC_HS_MODE, LEDC_RED_CHANNEL, red);
	ledc_update_duty(LEDC_HS_MODE, LEDC_RED_CHANNEL);
	ledc_set_duty(LEDC_HS_MODE, LEDC_GREEN_CHANNEL, green);
	ledc_update_duty(LEDC_HS_MODE, LEDC_GREEN_CHANNEL);
	ledc_set_duty(LEDC_HS_MODE, LEDC_BLUE_CHANNEL, blue);
	ledc_update_duty(LEDC_HS_MODE, LEDC_BLUE_CHANNEL);
}
