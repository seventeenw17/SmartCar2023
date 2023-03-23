#pragma once
#include <libserial/SerialPort.h>
#include <cstdlib>
#include <fstream>
#include <gpiod.h>
#include <iostream>
#include <memory>
#include <unistd.h>

using namespace LibSerial;
class RS485 {
    private:
    std::shared_ptr<SerialPort> _serial_port;
    std::string _port_name; //端口名
    BaudRate _bps;  // 波特率
    std::string _path_gpio_chip; /*gpiochip设备文件的路径。*/

    int _en_line;

    struct gpiod_chip *_gpio_chip;
    struct gpiod_line *_gpio_line;

    public:
    //构造函数初始化列表
    RS485(const std::string &port_name, BaudRate bps, std::string path_chip,int en_line)
        : _port_name(port_name), _bps(bps), _path_gpio_chip(path_chip),_en_line(en_line){};
    //析构函数
    ~RS485(){};

    int start() {
        int ret = 0;
        _serial_port = std::make_shared<SerialPort>();
        try {  //检测可能出现异常代码段
                //打开端口
            _serial_port->Open(_port_name);
        } catch (const OpenFailed &) {  //catch捕获并处理 try 检测到的异常。
            std::cerr << "Serial port: " << _port_name << "open failed ..."
                        << std::endl;
            return -1;
        } catch (const AlreadyOpen &) { //catch捕获并处理 try 检测到的异常。
            std::cerr << "Serial port: " << _port_name << "open failed ..."
                        << std::endl;
            return -2;
        }

        _serial_port->SetBaudRate(_bps);    //设置波特率
        _serial_port->SetCharacterSize(CharacterSize::CHAR_SIZE_8); //设置数据位
        _serial_port->SetFlowControl( FlowControl::FLOW_CONTROL_NONE); //设置流控
        _serial_port->SetParity(Parity::PARITY_NONE);   //无校验
        _serial_port->SetStopBits(StopBits::STOP_BITS_1);   //1位停止位
        _gpio_chip = gpiod_chip_open(_path_gpio_chip.c_str());
        if (!_gpio_chip) {
            printf("Serial port: Fail to open %s!\n", _path_gpio_chip.c_str());
            return -1;
        }
        _gpio_line = gpiod_chip_get_line(_gpio_chip, _en_line);
        if (!_gpio_line) {
            printf("Serial port: Fail to get gpio_line %d!\n", _en_line);
            return -1;
        }

        /* 设置GPIO为输出模式 */
        ret = gpiod_line_request_output(_gpio_line, "rs485-en",1);
        if (ret) {
            printf("Serial port: Fail to config output!\n");
            return -1;
        }
        return 0;
    };

    int send(unsigned char charbuffer) {
        /*读取单个GPIO线的当前值*/
        int value = gpiod_line_get_value(_gpio_line);
        if (value != 1) {
            /*设置单个GPIO的值。*/
            gpiod_line_set_value(_gpio_line, 1);
            value = gpiod_line_get_value(_gpio_line);
            if (value != 1) {
                std::cerr << "Serial port: Set GPIO High Failed." << std::endl;
                return -1;
            }
        }
        try {   //检测可能出现异常代码段
            _serial_port->WriteByte(charbuffer);//写数据到串口
        } catch (const std::runtime_error &) {  //catch捕获并处理 try 检测到的异常。
            std::cerr << "The Write() runtime_error." << std::endl;
            return -1;
        } catch (const NotOpen &) {    //catch捕获并处理 try 检测到的异常。
            std::cerr << "Port Not Open ..." << std::endl;
            return -1;
        }
        _serial_port->DrainWriteBuffer();//刷新缓冲区
        return 0;
    };

    int recv(unsigned char &charBuffer, size_t msTimeout = 0) {
        /*读取单个GPIO线的当前值。*/
        int value = gpiod_line_get_value(_gpio_line);
        if (value != 0) {
            /*设置单个GPIO的值。*/
            gpiod_line_set_value(_gpio_line, 0);
            value = gpiod_line_get_value(_gpio_line);
            if (value != 0) {
                std::cerr << "Serial port: Set GPIO Low Failed." << std::endl;
                return -1;
            }
        }
        try {
            /*从串口读取一个数据,指定msTimeout时长内,没有收到数据，抛出异常。
            如果msTimeout为0，则该方法将阻塞，直到数据可用为止。*/
            _serial_port->ReadByte(charBuffer,msTimeout);

        } catch (const ReadTimeout &) {
            std::cerr << "The ReadByte() call has timed out." << std::endl;
            return -2;
        } catch (const NotOpen &) {
            std::cerr << "Port Not Open ..." << std::endl;
            return -1;
        }
        return 0;
    };

    int stop() {
        _serial_port->Close();
        if (_gpio_chip != nullptr) {
            gpiod_chip_close(_gpio_chip);
        }
        return 0;
    }
};