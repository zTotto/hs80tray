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

    std::cout << "--- STEP 2.8: SYNC TOTALE (STABILE + DINAMICO) ---" << std::endl;

    unsigned char read_buf[65];
    bool mic_muted = false;
    auto last_led_time = std::chrono::steady_clock::now();
    auto last_handshake_time = std::chrono::steady_clock::now();

    while (true) {
        auto now = std::chrono::steady_clock::now();

        // 1. Handshake ogni 10 secondi
        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_handshake_time).count() >= 10) {
            send_cmd(h, {0x01, 0x03, 0x00, 0x02});
            send_cmd(h, {0x0D, 0x00, 0x01});
            last_handshake_time = now;
        }

        // 2. Leggi stato microfono (Reattivo 100ms)
        int res = hid_read_timeout(h, read_buf, 65, 100);
        if (res > 0 && read_buf[0] == 0x03 && read_buf[3] == 0xA6) {
            mic_muted = (read_buf[5] == 1);
            std::cout << "\rSTATO RILEVATO: " << (mic_muted ? "MUTO" : "ATTIVO") << "    " << std::flush;
        }

        // 3. Sincronizza LED ogni secondo
        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_led_time).count() >= 1) {
            unsigned char led_pkt[65] = {0};
            led_pkt[0] = 0x02; led_pkt[1] = 0x09; led_pkt[2] = 0x06; led_pkt[3] = 0x00;
            led_pkt[4] = 0x09; 
            
            int mr = mic_muted ? 255 : 0;
            led_pkt[10] = mr;  // Mic (I2)
            led_pkt[12] = 255; // Power Verde (I1)
            
            hid_write(h, led_pkt, 65);
            last_led_time = now;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    hid_exit();
    return 0;
}
