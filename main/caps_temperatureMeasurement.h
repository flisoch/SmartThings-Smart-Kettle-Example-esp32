#include "caps/iot_caps_helper_temperatureMeasurement.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct caps_temperature_data {
    IOT_CAP_HANDLE* handle;
    void *usr_data;
    void *cmd_data;

    double temperature_value;
    char *temperature_unit;

    double (*get_temperature_value)(struct caps_temperature_data *caps_data);
    void (*set_temperature_value)(struct caps_temperature_data *caps_data, double value);
    const char *(*get_temperature_unit)(struct caps_temperature_data *caps_data);
    void (*set_temperature_unit)(struct caps_temperature_data *caps_data, const char *unit);
    void (*attr_temperature_send)(struct caps_temperature_data *caps_data);

    void (*init_usr_cb)(struct caps_temperature_data *caps_data);
} caps_temperature_data_t;

caps_temperature_data_t *caps_temperatureMeasurement_initialize(IOT_CTX *ctx, const char *component, void *init_usr_cb, void *usr_data);
#ifdef __cplusplus
}
#endif

