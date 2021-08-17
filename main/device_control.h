#include <driver/dac.h>

#define GPIO_OUTPUT_MAINLED 2
#define GPIO_INPUT_SWITCH 27
#define GPIO_OUTPUT_BUZZER 12
#define DAC_OUTPUT_RGBLED_R DAC_CHANNEL_2
#define DAC_OUTPUT_RGBLED_G DAC_CHANNEL_1
#define GPIO_OUTPUT_RGBLED_B 32
#define ADC_TEMPERATURE_SENSOR 13

#define TEMPERATURE_EVENT_MS_RATE 3000
#define BUZZER_SOUND_DURATION 3000
#define RGB_BOILING_ADJUSTMENT_DURATION 20
#define BUTTON_DEBOUNCE_TIME_MS 20
#define BLINK_DURATION 300

// init rgb values -- green 
#define RGB_LED_R 0
#define RGB_LED_G 255
#define RGB_LED_B 0

enum switch_onoff_state {
    SWITCH_OFF = 1,
    SWITCH_ON = 0,
};

enum thermostat_onoff_state {
    THERMOSTAT_OFF = 0,
    THERMOSTAT_ON = 1,
};

enum buzzer_onoff_state {
    BUZZER_OFF = 0,
    BUZZER_ON = 1,
};

enum led_gpio_state {
    LED_GPIO_ON = 1,
    LED_GPIO_OFF = 0,
    LED_DAC_MAX = 255
};

void change_switch_state(int switch_state);
void change_thermostat_state(int *thermostat_state);
void iot_gpio_init(void);
void blink_led(double heating_setpoint, int led_state);
double temperature_event(double temp);
void change_rgb_state(int pin, int value);
void beep();
int get_button_event();
void change_rgb_led_boiling(double heating_setpoint, double current_temperature);
void change_rgb_led_state(int red, int green, int blue);
void temperature_events_task(void *arg);