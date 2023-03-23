#include <memory>
#include <iostream>
#include <unistd.h>
#include <gpiod.h>

int main(int argc, char *argv[]) {

    int ret = 0;
    std::string line1;
    int line;

    struct gpiod_chip *_gpio_chip;
    struct gpiod_line *_gpio_line;

    if (argc > 1) {
        line1 = argv[1];
        std::cout << "The passed argument is" << line1 << std::endl;
        line = atoi(line1.c_str());
    } else {
        std::cout << "Parameter error,example:./gpioDemo xxx" << std::endl;
        return -1;
    }

    _gpio_chip = gpiod_chip_open("/dev/gpiochip0");
    if (!_gpio_chip) {
        printf("Serial port: Fail to open %s!\n", "/dev/gpiochip0");
        return -1;
    }
    _gpio_line = gpiod_chip_get_line(_gpio_chip, line);
    if (!_gpio_line) {
        printf("Serial port: Fail to get gpio_line %d!\n", line);
        return -1;
    }
    /* 设置GPIO为输出模式 */
    ret = gpiod_line_request_output(_gpio_line, "GPIO-en",1);
    if (ret) {
        printf("Serial port: Fail to config output!\n");
        return -1;
    }

    while (1) {
        gpiod_line_set_value(_gpio_line, 1);
        sleep(1);
        gpiod_line_set_value(_gpio_line, 0);
        sleep(1);
    }
    gpiod_chip_close(_gpio_chip);

    return 0;
}