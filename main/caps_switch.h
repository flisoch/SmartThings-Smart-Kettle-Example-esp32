#include "st_dev.h"
#include "caps/iot_caps_helper_switch.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct caps_switch_data
{
    IOT_CAP_HANDLE* handle;
    void *usr_data;
    void *cmd_data;

    char *switch_value;

    const char *(*get_switch_value)(struct caps_switch_data *caps_data);
    void (*set_switch_value)(struct caps_switch_data *caps_data, const char *value);
    int (*attr_switch_str2idx)(const char *value);
    void (*attr_switch_send)(struct caps_switch_data *caps_data);

    void (*init_usr_cb)(struct caps_switch_data *caps_data);

    void (*cmd_on_usr_cb)(struct caps_switch_data *caps_data);
    void (*cmd_off_usr_cb)(struct caps_switch_data *caps_data);
} caps_switch_data_t;

caps_switch_data_t *caps_switch_initialize(IOT_CTX *ctx, const char *component, void *init_usr_cb, void *usr_data);
#ifdef __cplusplus
}
#endif
