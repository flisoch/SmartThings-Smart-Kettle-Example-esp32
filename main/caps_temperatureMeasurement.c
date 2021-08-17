#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "st_dev.h"
#include "caps_temperatureMeasurement.h"

static double caps_temperature_get_temperature_value(caps_temperature_data_t *caps_data)
{
    if (!caps_data) {
        printf("caps_data is NULL\n");
        return -1;
    }
    return caps_data->temperature_value;
}

static void caps_temperature_set_temperature_value(caps_temperature_data_t *caps_data, double value)
{
    if (!caps_data) {
        printf("caps_data is NULL\n");
        return;
    }
    caps_data->temperature_value = value;
}

static const char *caps_temperature_get_temperature_unit(caps_temperature_data_t *caps_data)
{
    if (!caps_data) {
        printf("caps_data is NULL\n");
        return NULL;
    }
    return caps_data->temperature_unit;
}

static void caps_temperature_set_temperature_unit(caps_temperature_data_t *caps_data, const char *unit)
{
    if (!caps_data) {
        printf("caps_data is NULL\n");
        return;
    }
    caps_data->temperature_unit = (char *)unit;
}

static void caps_temperatureMeasurement_attr_temperatureValue_send(caps_temperature_data_t *caps_data)
{
    int sequence_no = -1;

    if (!caps_data || !caps_data->handle) {
        printf("fail to get handle\n");
        return;
    }

    ST_CAP_SEND_ATTR_NUMBER(caps_data->handle,
            (char *)caps_helper_temperatureMeasurement.attr_temperature.name,
            caps_data->temperature_value,
            caps_data->temperature_unit,
            NULL,
            sequence_no);

    if (sequence_no < 0)
        printf("fail to send temperature value\n");
    else
        printf("Sequence number return : %d\n", sequence_no);
}

static void caps_temperatureMeasurement_init_cb(IOT_CAP_HANDLE *handle, void *usr_data)
{
    caps_temperature_data_t *caps_data = usr_data;
    if (caps_data && caps_data->init_usr_cb)
        caps_data->init_usr_cb(caps_data);
    caps_temperatureMeasurement_attr_temperatureValue_send(caps_data);
}

caps_temperature_data_t *caps_temperatureMeasurement_initialize(IOT_CTX *ctx, const char *component, void *init_usr_cb, void *usr_data)
{
    caps_temperature_data_t *caps_data = NULL;
    caps_data = malloc(sizeof(caps_temperature_data_t));
    if (!caps_data) {
        printf("fail to malloc for caps_temperature_data_t\n");
        return NULL;
    }

    memset(caps_data, 0, sizeof(caps_temperature_data_t));

    caps_data->init_usr_cb = init_usr_cb;
    caps_data->usr_data = usr_data;

    caps_data->get_temperature_value = caps_temperature_get_temperature_value;
    caps_data->set_temperature_value = caps_temperature_set_temperature_value;
    caps_data->get_temperature_unit = caps_temperature_get_temperature_unit;
    caps_data->set_temperature_unit = caps_temperature_set_temperature_unit;
    caps_data->attr_temperature_send = caps_temperatureMeasurement_attr_temperatureValue_send;
    caps_data->temperature_value = 0;
    if (ctx) {
        caps_data->handle = st_cap_handle_init(ctx, component, caps_helper_temperatureMeasurement.id, caps_temperatureMeasurement_init_cb, caps_data);
    }
    if (!caps_data->handle) {
        printf("fail to init temperatureSensor handle\n");
    }

    return caps_data;
}