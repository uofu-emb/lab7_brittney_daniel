#include <zephyr.h>
#include <kernel.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/can.h>
#include <drivers/gpio.h>
#include <sys/byteorder.h>


struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});
struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {0});


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

const struct zcan_filter filter123 = {
    .id_type = CAN_STANDARD_IDENTIFIER,
    .rtr = CAN_DATAFRAME,
    .id = 0x123,
    .rtr_mask = 0,
    .id_mask = 0xFFF
};

const struct zcan_filter filter456 = {
    .id_type = CAN_STANDARD_IDENTIFIER,
    .rtr = CAN_DATAFRAME,
    .id = 0x456,
    .rtr_mask = 0,
    .id_mask = 0xFFF
};

const struct device *can_dev;
int ret;
int next_message = 1;

void send_message_alternate() {
    if (next_message == 1){
        ret = can_send(can_dev, &frame1, K_MSEC(100), NULL, NULL);
        next_message = 2;
    } else {
        ret = can_send(can_dev, &frame2, K_MSEC(100), NULL, NULL);
        next_message = 1;
    }
    
}

void rcv_message1(struct zcan_frame *msg, void *arg) {
    printk("received a thing\n");
    gpio_pin_toggle(led0.port, led0.pin);
}

void rcv_message2(struct zcan_frame *msg, void *arg) {
    printk("received a thing\n");
    gpio_pin_toggle(led1.port, led1.pin);
}

void send_message_high_prio() {
    ret = can_send(can_dev, &frame1, K_MSEC(100), NULL, NULL);
}

void send_loop_high_prio() {
    while (true) {
        send_message_high_prio();
        k_sleep(K_MSEC(400));
    }
}

void send_message_low_prio() {
    ret = can_send(can_dev, &frame2, K_MSEC(100), NULL, NULL);
}

void send_loop_low_prio() {
    while (true) {
        send_message_low_prio();
        k_sleep(K_MSEC(500));
    }
}

void main(void) {
    can_dev = device_get_binding("CAN_1");
    // can_set_mode(can_dev, CAN_LOOPBACK_MODE);
    can_set_mode(can_dev, CAN_NORMAL_MODE);
    can_attach_isr(can_dev, &rcv_message1, NULL, &filter123);
    can_attach_isr(can_dev, &rcv_message2, NULL, &filter456);

    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_HIGH);
    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_HIGH);

    //Activity 3 (first board high priority)
    send_loop_high_prio();
    //Activity 3 (second board low priority)
    //send_loop_low_prio();

    //Activity 2
    // while (true) {
        // send_message_alternate();
        // k_sleep(K_MSEC(500));
    // }
}
