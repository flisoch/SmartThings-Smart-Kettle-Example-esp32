#include "caps/iot_caps_helper_heatingSetpoint.h"
#include "st_dev.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct caps_thermostatHeatingSetpoint_data
{
    IOT_CAP_HANDLE *handle;
    void *usr_data;
    void *cmd_data;

    double min;
    double max;
    double value;
    char *unit;

    void (*init_usr_cb)(struct caps_thermostatHeatingSetpoint_data *caps_data);

    double (*get_value)(struct caps_thermostatHeatingSetpoint_data *caps_data);
    void (*set_value)(struct caps_thermostatHeatingSetpoint_data *caps_data, double value);
    double (*get_min)(struct caps_thermostatHeatingSetpoint_data *caps_data);
    void (*set_min)(struct caps_thermostatHeatingSetpoint_data *caps_data, double value);
    double (*get_max)(struct caps_thermostatHeatingSetpoint_data *caps_data);
    void (*set_max)(struct caps_thermostatHeatingSetpoint_data *caps_data, double value);
    const char *(*get_unit)(struct caps_thermostatHeatingSetpoint_data *caps_data);
    void (*set_unit)(struct caps_thermostatHeatingSetpoint_data *caps_data, const char *unit);
    
    void (*attr_setpointValue_send)(struct caps_thermostatHeatingSetpoint_data *caps_data);
    void (*cmd_setHeatingSetpoint_usr_cb)(struct caps_thermostatHeatingSetpoint_data *caps_data);
} caps_thermostatHeatingSetpoint_data_t;

caps_thermostatHeatingSetpoint_data_t *caps_thermostatHeatingSetpoint_initialize(IOT_CTX *ctx, const char *component, void *init_usr_cb, void *usr_data);
#ifdef __cplusplus
}
#endif