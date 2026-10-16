#include <stdbool.h>

#include <kernel/serial.h>

int serial_get(void) {
    if (!serial_received()) return 0;
    return (int) read_serial();
}

void serial_put(int value) {
    write_serial((char) value);
}
