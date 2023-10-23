#include <zephyr.h>
#include "drivers/can.h"

struct zcan_frame frame = {
    .id_type = CAN_STANDARD_IDENTIFIER,
    .rtr = CAN_DATAFRAME,
    .id = 0x123,
    .dlc = 8,
    .data = {1,2,3,4,5,6,7,8}
};

const struct zcan_filter filter = {
    .id_type = CAN_STANDARD_IDENTIFIER,
    .rtr = CAN_DATAFRAME,
    .id = 0x123,
    .rtr_mask = 0,
    .id_mask = 0
};

const struct device *can_dev;
int ret;

void send_message() {
    ret = can_send(can_dev, &frame, K_MSEC(100), NULL, NULL);
}

void rcv_message(struct zcan_frame *msg, void *arg) {
    printk("received a thing\n");
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
