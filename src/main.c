#include <zephyr.h>
#include "drivers/can.h"
#include "drivers/gpio.h"


struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});


struct zcan_frame frame1 = {
    .id_type = CAN_STANDARD_IDENTIFIER,
    .rtr = CAN_DATAFRAME,
    .id = 0x123,
    .dlc = 8,
    .data = {1,2,3,4,5,6,7,8}
};

struct zcan_frame frame2 = {
    .id_type = CAN_STANDARD_IDENTIFIER,
    .rtr = CAN_DATAFRAME,
    .id = 0x456,
    .dlc = 8,
    .data = {1,2,3,4,5,6,7,8}
};

const struct zcan_filter filter = {
    .id_type = CAN_STANDARD_IDENTIFIER,
    .rtr = CAN_DATAFRAME,
    .id = 0x123,
    .rtr_mask = 0,
    .id_mask = 0xFFF
};

const struct device *can_dev;
int ret;
int next_message = 1;

void send_message() {
    if (next_message == 1){
        ret = can_send(can_dev, &frame1, K_MSEC(100), NULL, NULL);
        next_message = 2;
    } else {
        ret = can_send(can_dev, &frame2, K_MSEC(100), NULL, NULL);
        next_message = 1;
    }
    
}

void rcv_message(struct zcan_frame *msg, void *arg) {
    printk("received a thing\n");
    gpio_pin_toggle(led.port, led.pin);
}

void main(void) {
    can_dev = device_get_binding("CAN_1");
    can_set_mode(can_dev, CAN_LOOPBACK_MODE);
    can_attach_isr(can_dev, &rcv_message, NULL, &filter);
    while (true) {
        send_message();
        k_sleep(K_MSEC(500));
    }
}
