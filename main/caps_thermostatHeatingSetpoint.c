#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "caps_thermostatHeatingSetpoint.h"

static const char *caps_get_unit(caps_thermostatHeatingSetpoint_data_t *caps_data)
{
    if (!caps_data) {
        printf("caps_data is NULL\n");
        return NULL;
    }
    return caps_data->unit;
}

static void caps_set_unit(caps_thermostatHeatingSetpoint_data_t *caps_data, const char *unit)
{
    if (!caps_data) {
        printf("caps_data is NULL\n");
        return;
    }
    caps_data->unit = (char *)unit;
}

static double caps_get_value(caps_thermostatHeatingSetpoint_data_t *caps_data)
{
    if (!caps_data) {
        printf("caps_data is NULL\n");
        return 0;
    }
    return caps_data->value;
}

static void caps_set_value(caps_thermostatHeatingSetpoint_data_t *caps_data, double value)

{
    if (!caps_data) {
        printf("caps_data is NULL\n");
        return;
    }
    caps_data->value = value;
}

static double caps_get_min(caps_thermostatHeatingSetpoint_data_t *caps_data)
{
    if (!caps_data) {
        printf("caps_data is NULL\n");
        return 0;
    }
    return caps_data->min;
}

static void caps_set_min(caps_thermostatHeatingSetpoint_data_t *caps_data, double min)

{
    if (!caps_data) {
        printf("caps_data is NULL\n");
        return;
    }
    caps_data->min = min;
}

static double caps_get_max(caps_thermostatHeatingSetpoint_data_t *caps_data)
{
    if (!caps_data) {
        printf("caps_data is NULL\n");
        return 0;
    }
    return caps_data->max;
}

static void caps_set_max(caps_thermostatHeatingSetpoint_data_t *caps_data, double max)

{
    if (!caps_data) {
        printf("caps_data is NULL\n");
        return;
    }
    caps_data->max = max;
}

static void caps_attr_setpointValue_send(caps_thermostatHeatingSetpoint_data_t *caps_data)
{
    int sequence_no = -1;

    if (!caps_data || !caps_data->handle) {
        printf("fail to get handle\n");
        return;
    }
    ST_CAP_SEND_ATTR_NUMBER(caps_data->handle,
            (char *)caps_helper_thermostatHeatingSetpoint.attr_heatingSetpoint.name,
            caps_data->value,
            (char *)caps_data->unit,
            NULL,
            sequence_no);

    if (sequence_no < 0)
        printf("fail to send temperature value\n");
    else
        printf("Sequence number return : %d\n", sequence_no);
}

static void caps_cmd_setHeatingSetpoint_cb(IOT_CAP_HANDLE *handle, iot_cap_cmd_data_t *cmd_data, void *usr_data)
{
    caps_thermostatHeatingSetpoint_data_t *caps_data = (caps_thermostatHeatingSetpoint_data_t *)usr_data;
    double value = cmd_data->cmd_data[0].integer;

    printf("called [%s] func with num_args:%u\n", __func__, cmd_data->num_args);

    caps_set_value(caps_data, value);
    if (caps_data && caps_data->cmd_setHeatingSetpoint_usr_cb) {
        caps_data->cmd_setHeatingSetpoint_usr_cb(caps_data);
    }
    caps_attr_setpointValue_send(caps_data);
}

static void caps_heatingSetpoint_init_cb(IOT_CAP_HANDLE *handle, void *usr_data)
{
    caps_thermostatHeatingSetpoint_data_t *caps_data = usr_data;
    if (caps_data && caps_data->init_usr_cb)
        caps_data->init_usr_cb(caps_data);
    caps_attr_setpointValue_send(caps_data);
}

caps_thermostatHeatingSetpoint_data_t *caps_thermostatHeatingSetpoint_initialize(IOT_CTX *ctx, const char *component, void *init_usr_cb, void *usr_data) 
{
    caps_thermostatHeatingSetpoint_data_t *caps_data = NULL;
    int err; 

    caps_data = malloc(sizeof(caps_thermostatHeatingSetpoint_data_t));
    if (!caps_data) {
        printf("fail to malloc for caps_thermostatHeatingSetpoint_data_t\n");
        return NULL;
    }

    memset(caps_data, 0, sizeof(caps_thermostatHeatingSetpoint_data_t));

    caps_data->init_usr_cb = init_usr_cb;
    caps_data->usr_data = usr_data;

    caps_data->get_value = caps_get_value;
    caps_data->set_value = caps_set_value;
    caps_data->get_unit = caps_get_unit;
    caps_data->set_unit = caps_set_unit;
    caps_data->get_max = caps_get_max;
    caps_data->set_max = caps_set_max;
    caps_data->get_min = caps_get_min;
    caps_data->set_min = caps_set_min;
    caps_data->attr_setpointValue_send = caps_attr_setpointValue_send;

    caps_data->set_unit(caps_data, caps_helper_thermostatHeatingSetpoint.attr_heatingSetpoint.unit_C);
    caps_data->set_value(caps_data, 0.0);

    if (ctx) {
        caps_data->handle = st_cap_handle_init(ctx, component, caps_helper_thermostatHeatingSetpoint.id, caps_heatingSetpoint_init_cb, caps_data);
    }
    if(caps_data->handle) {
        err = st_cap_cmd_set_cb(caps_data->handle, caps_helper_thermostatHeatingSetpoint.cmd_setHeatingSetpoint.name, caps_cmd_setHeatingSetpoint_cb, caps_data);
        if (err) {
            printf("fail to set cmd_cb for on of switch\n");
        }
    }
    else {
        printf("fail to init thermostatHeatingSetpoint handle\n");
    }

    return caps_data;
}