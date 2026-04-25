#include <hidapi/hidapi.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

void send_cmd(hid_device* handle, const std::vector<unsigned char>& data) {
    unsigned char buf[65] = {0};
    buf[0] = 0x02; buf[1] = 0x09;
    for (size_t i = 0; i < data.size() && (i + 2) < 65; ++i) {
        buf[i + 2] = data[i];
    }
    hid_write(handle, buf, 65);
}

int main() {
    if (hid_init()) return 1;

    struct hid_device_info *devs, *cur_dev;
    devs = hid_enumerate(0x1B1C, 0x0A6B);
    cur_dev = devs;
    hid_device* h = nullptr;
    while (cur_dev) {
        if (cur_dev->interface_number == 3) {
            h = hid_open_path(cur_dev->path);
            if (h) break;
        }
        cur_dev = cur_dev->next;
    }
    hid_free_enumeration(devs);

    if (!h) return 1;

    std::cout << "BASELINE STABILE: Power Verde, Logo Off, Mic Off." << std::endl;

    int count = 0;
    while (true) {
        if (count % 5 == 0) {
            send_cmd(h, {0x01, 0x03, 0x00, 0x02});
            send_cmd(h, {0x0D, 0x00, 0x01});
        }

        unsigned char led_pkt[65] = {0};
        led_pkt[0] = 0x02; led_pkt[1] = 0x09; led_pkt[2] = 0x06; led_pkt[3] = 0x00;
        led_pkt[4] = 0x09; 
        
        led_pkt[8] = 0;   led_pkt[9] = 0;   led_pkt[10] = 0;   // Rossi
        led_pkt[11] = 0;  led_pkt[12] = 255; led_pkt[13] = 0;   // Verdi (Power G:255)
        led_pkt[14] = 0;  led_pkt[15] = 0;   led_pkt[16] = 0;   // Blu

        hid_write(h, led_pkt, 65);
        std::this_thread::sleep_for(std::chrono::seconds(2));
        count++;
    }

    hid_exit();
    return 0;
}
