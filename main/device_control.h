#include <driver/dac.h>

#define GPIO_OUTPUT_MAINLED 2
#define GPIO_INPUT_SWITCH 27
#define GPIO_OUTPUT_BUZZER 12
#define DAC_OUTPUT_RGBLED_R DAC_CHANNEL_2
#define DAC_OUTPUT_RGBLED_G DAC_CHANNEL_1
#define GPIO_OUTPUT_RGBLED_B 32
#define GPIO_TEMPERATURE_SENSOR 13

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_HS_MODE LEDC_HIGH_SPEED_MODE
#define LEDC_RED_GPIO (26)
#define LEDC_RED_CHANNEL LEDC_CHANNEL_0
#define LEDC_GREEN_GPIO (25)
#define LEDC_GREEN_CHANNEL LEDC_CHANNEL_1
#define LEDC_BLUE_GPIO (32)
#define LEDC_BLUE_CHANNEL LEDC_CHANNEL_2
#define LEDC_MIN_DUTY (0)
#define LEDC_MAX_DUTY (4000)
#define LEDC_FADE_TIME (1000)
#define LEDC_CH_NUM (3)

#define TEMPERATURE_EVENT_MS_RATE 1000
#define BUZZER_SOUND_DURATION 3000
#define RGB_BOILING_ADJUSTMENT_DURATION 20
#define BUTTON_DEBOUNCE_TIME_MS 20
#define HEATING_SETPOINT_RGB_DURATION 1500

// init rgb values -- green
#define RGB_LED_R 0
#define RGB_LED_G 255
#define RGB_LED_B 0

enum switch_onoff_state
{
    SWITCH_OFF = 1,
    SWITCH_ON = 0,
};

enum thermostat_onoff_state
{
    THERMOSTAT_OFF = 0,
    THERMOSTAT_ON = 1,
};

enum buzzer_onoff_state
{
    BUZZER_OFF = 0,
    BUZZER_ON = 1,
};

enum led_gpio_state
{
    LED_ON = 1,
    LED_OFF = 0,
    LED_DAC_MAX = 255
};

typedef struct temperature_state
{
    double current_temperature;
    double prev_temperature;
    double heating_setpoint;
    void (*update_temperature)(struct temperature_state *temperature_state, double value);
} temperature_state_t;

void recalculate_duty_new_setpoint(double current_temperature, double prev_heating_setpoint, double heating_setpoint);
void change_switch_state(int switch_state);
void change_thermostat_state(int *thermostat_state);
void iot_gpio_init(void);
void setpoint_rgb_indication(double heating_setpoint);
double temperature_event(double temp);
void change_rgb_state(int pin, int value);
void beep();
int get_button_event();
void change_rgb_led_heating(double heating_setpoint, double prev_temperature, double current_temperature);
void change_rgb_led_state(int red, int green, int blue);
void temperature_events_task(void *arg);