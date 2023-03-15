#include "util/stop_watch.hpp"
#include <iostream>
#include <libserial/SerialPort.h>
#include <memory>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <thread>

using namespace LibSerial;

class Driver {

#define PKT_HEAD 0x42
#define PKT_TAIL 0x10

enum CmdType { SPEED_ANGLE = 0x01, SPEED_ANGLE_PWM = 0x0C, TEST = 0xBB };
//1字节对齐
#pragma pack(1)
    union Payload {
        uint8_t data[8];
        float speed_angle[2];
    };
//小车上位机与下位机通讯的协议结构体
    struct Pkt {
        uint8_t head;
        uint8_t cmd;
        Payload payload;
        uint8_t check_sum;
        uint8_t tail;
    };
#pragma pack()

#define MAX_SPEED 10
#define MIN_SPEED -10

#define MAX_ANGLE 38
#define MIN_ANGLE -38

#define MAX_ANGLE_PWM 1700
#define MIN_ANGLE_PWM 1200

public:
    Driver(const std::string &port_name, BaudRate bps, bool debug_mode = false,
           bool log_en = false)
            : _port_name(port_name), _bps(bps), _debug_mode(debug_mode),
              _log_en(log_en){};
    ~Driver() { close(); };
    int open() {
        if (_debug_mode) {
            /*debug 模式 : 在没有设备时 调试协议使用*/
            return 0;
        }
        _serial_port = std::make_shared<SerialPort>();
        if (_serial_port == nullptr) {
            std::cerr << "Serial Create Failed ." << std::endl;
            return -1;
        }
        try {
            _serial_port->Open(_port_name);//打开串口
            _serial_port->SetBaudRate(_bps);//设置波特率
            _serial_port->SetCharacterSize(CharacterSize::CHAR_SIZE_8);//8位数据位
            _serial_port->SetFlowControl(FlowControl::FLOW_CONTROL_NONE);//设置流控
            _serial_port->SetParity(Parity::PARITY_NONE);//无校验
            _serial_port->SetStopBits(StopBits::STOP_BITS_1);//1个停止位
        } catch (const OpenFailed &) {
            std::cerr << "Serial port: " << _port_name << "open failed ..."
                      << std::endl;
            return -2;
        } catch (const AlreadyOpen &) {
            std::cerr << "Serial port: " << _port_name << "open failed ..."
                      << std::endl;
            return -3;
        } catch (...) {
            std::cerr << "Serial port: " << _port_name << " recv exception ..."
                      << std::endl;
            return -4;
        }
        return 0;
    };
    int move_steer(float speed, float angle) {
        check_speed_and_angle(speed, angle);
        _speed = speed;
        _angle = angle;
        Payload data;     //send函数传参要是Payload
        data.speed_angle[0] = _speed;
        data.speed_angle[1] = _angle;
        return send(SPEED_ANGLE, data);
    }

    int move(float speed) { return move_steer(speed, _angle); }
    int steer(float angle) { return move_steer(_speed, angle); }
    int recvdata(unsigned char &charBuffer, size_t msTimeout){return recv(charBuffer,msTimeout);}

    int stop() { return move_steer(0, _angle); };

    int move_steer_pwm(float speed, uint32_t angle) {
        check_speed_and_angle_pwm(speed, angle);
        _speed = speed;
        _angle = angle;
        Payload data;
        data.speed_angle[0] = _speed;
        data.speed_angle[1] = _angle;
        return send(SPEED_ANGLE_PWM, data);
    }

    int reverse(float speed) { return move_steer(-speed, _angle); };
    void close() {
        if (_debug_mode) {
            /*debug 模式 : 在没有设备时 调试协议使用*/
            return;
        }
        if (_serial_port != nullptr) {
            _serial_port->Close();
            _serial_port = nullptr;
        }
    };
    int move_steer_sync(float speed, float angle, bool log_en = false) {
        check_speed_and_angle(speed, angle);
        _speed = speed;
        _angle = angle;
        Payload data;
        data.speed_angle[0] = _speed;
        data.speed_angle[1] = _angle;
        return sendSync(CmdType::SPEED_ANGLE, data, log_en);
    }
    int test_sync(float speed, float angle, bool log_en = false) {
        check_speed_and_angle(speed, angle);
        _speed = speed;
        _angle = angle;
        Payload data;
        data.speed_angle[0] = _speed;
        data.speed_angle[1] = _angle;
        return sendSync(CmdType::TEST, data, log_en);
    }

private:
    uint8_t check_sum(Pkt *pkt) //校验和计算
    {
        uint8_t check_sum = 0;
        check_sum += pkt->head;
        check_sum += pkt->cmd;
        for (size_t i = 0; i < sizeof(pkt->payload.data); i++) {
            check_sum += pkt->payload.data[i];
        }
        return check_sum;
    }

    int send(CmdType cmd, Payload &payload) {

        DataBuffer pkt_buf(sizeof(Pkt));
        Pkt *pkt = (Pkt *)&pkt_buf[0];

        pkt->head = PKT_HEAD;//赋值
        pkt->cmd = (uint8_t)cmd;
        //将联合体中的数据复制到要发送的数据结构体中
        memcpy(pkt->payload.data, payload.data, sizeof(payload.data));
        pkt->check_sum = check_sum(pkt);
        pkt->tail = PKT_TAIL;
        //发送打印
        if (_log_en) {
            printf("[Driver Car] Send Serial Cmd:[");
            for (size_t i = 0; i < pkt_buf.size(); i++) {
                printf("%02x ", pkt_buf[i]);
            }
            printf("]\n");
        }
        if (_debug_mode) {
            /*debug 模式 : 在没有设备时 调试协议使用*/
            return 0;
        }
        try {
            _serial_port->Write(pkt_buf);//写数据到串口
        } catch (const std::runtime_error &) {
            std::cerr << "The Write() runtime_error." << std::endl;
            return -1;
        } catch (const NotOpen &) {
            std::cerr << "Port Not Open ..." << std::endl;
            return -1;
        }
        _serial_port->DrainWriteBuffer();//等待，直到写缓冲区耗尽，然后返回。
        return 0;
    }
    //调试数据结构体
    void debugPkt(std::vector<uint8_t> pkt, std::string prefix) {
        printf("%s", prefix.c_str());
        for (size_t i = 0; i < pkt.size(); i++) {
            printf("%02X ", pkt[i]);
        }
        printf("\n");
    }
    //检查返回的数据
    int checkValidPkt(std::vector<uint8_t> pkt_vector) {
        int head_index = -1;
        if (pkt_vector.size() < sizeof(Pkt)) {
            return -1;
        }
        for (size_t i = 0; i < pkt_vector.size(); i++) {
            if (pkt_vector[i] == PKT_HEAD) {
                head_index = i;
                break;
            } else {
                // TODO: clear
            }
        }
        if (head_index < 0) {
            return -1;
        }
        Pkt *p_pkt = (Pkt *)&pkt_vector[head_index];
        if (head_index + sizeof(Pkt) > pkt_vector.size()) {
            return -1;
        }
        if (p_pkt->tail != PKT_TAIL) {
            printf("Alarm Recv Error: Tail is error . Recv Tail:%02X \n",
                   p_pkt->tail);
            return -2;
        }
        uint8_t checkSum = check_sum(p_pkt);
        if (p_pkt->check_sum != checkSum) {
            printf("Alarm Recv Error: CheckSum is error .\n");
            return -3;
        }
        return 0;
    }

    int sendSync(CmdType cmd, Payload &payload, bool log_en = false) {
        int ret = 0;
        StopWatch stop_watch;
        stop_watch.tic();

        ret = send(cmd, payload);
        if (ret != 0) {
            return ret;
        }

        std::cout << "Send times:" << stop_watch.toc() << std::endl;

        size_t timeout_ms = 2000;
        std::vector<uint8_t> recv_pkt(0);

        while (1) {
            uint8_t recv_data;
            ret = recv(recv_data, timeout_ms);
            if (ret != 0) {
                if (ret == -2) {
                    printf("Alarm Driver Recv Feedback TimeOut:%lu .\n", timeout_ms);
                }
                debugPkt(recv_pkt, "Recv Error:");
                break;
            }
            recv_pkt.push_back(recv_data);
            ret = checkValidPkt(recv_pkt);
            if (ret == 0) {
                /*Recv Complete PKT*/
                // printf("Alarm Recv FeadBack Ok !!!\n");
                break;
            } else if ((ret == -2) || (ret == -3)) {
                break;
            }
            if (recv_pkt.size() > sizeof(Pkt)) {
                ret = -3;
                printf("Alarm Recv too more data, so break. %lu\n", recv_pkt.size());
                debugPkt(recv_pkt, "Recv Error:");
                break;
            }
            timeout_ms = 2000; /*First time long ...*/
        }
        if ((ret == 0) && (log_en)) {
            debugPkt(recv_pkt, "Recv Pkt:");
        }
        return ret;
    }

    int recv(unsigned char &charBuffer, size_t msTimeout = 0) {
        try {
            // Read a single byte of data from the serial ports.
            //从串口读取一个数据
            _serial_port->ReadByte(charBuffer, msTimeout);

        } catch (const ReadTimeout &) {
            std::cerr << "The ReadByte() call has timed out." << std::endl;
            return -2;
        } catch (const NotOpen &) {
            std::cerr << "Port Not Open ..." << std::endl;
            return -1;
        }
        return 0;
    };

    void check_speed_and_angle(float &speed, float &angle) {
        if (speed > MAX_SPEED) {
            std::cout << "Warning Speed: " << speed
                      << " too big, set MaxSpeed: " << MAX_SPEED << std::endl;
            speed = MAX_SPEED;
        }
        if (speed < MIN_SPEED) {
            std::cout << "Warning Speed: " << speed
                      << " too small, set MaxSpeed: " << MIN_SPEED << std::endl;
            speed = MAX_SPEED;
        }
        if (angle > MAX_ANGLE) {
            std::cout << "Warning angle: " << angle
                      << " too big, set Maxangle: " << MAX_ANGLE << std::endl;
            angle = MAX_ANGLE;
        }
        if (angle < MIN_ANGLE) {
            std::cout << "Warning angle: " << angle
                      << " too small, set MIN_ANGLE: " << MIN_ANGLE << std::endl;
            angle = MIN_ANGLE;
        }
    }

    void check_speed_and_angle_pwm(float &speed, uint32_t &angle) {
        if (speed > MAX_SPEED) {
            std::cout << "Warning Speed: " << speed
                      << " too big, set MaxSpeed: " << MAX_SPEED << std::endl;
            speed = MAX_SPEED;
        }
        if (speed < MIN_SPEED) {
            std::cout << "Warning Speed: " << speed
                      << " too small, set MaxSpeed: " << MIN_SPEED << std::endl;
            speed = MAX_SPEED;
        }
        if (angle > MAX_ANGLE_PWM) {
            std::cout << "Warning angle: " << angle
                      << " too big, set Maxangle: " << MAX_ANGLE_PWM << std::endl;
            angle = MAX_ANGLE_PWM;
        }
        if (angle < MIN_ANGLE_PWM) {
            std::cout << "Warning angle: " << angle
                      << " too small, set MIN_ANGLE_PWM: " << MIN_ANGLE_PWM << std::endl;
            angle = MIN_ANGLE_PWM;
        }
    }

private:
    std::shared_ptr<SerialPort> _serial_port = nullptr;
    std::string _port_name;
    BaudRate _bps;

    float _speed = 0.0;
    float _angle;

    bool _debug_mode;
    bool _log_en;//打印使能标志
};