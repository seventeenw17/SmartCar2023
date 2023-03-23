#include "util/stop_watch.hpp"
#include <iostream>
#include <libserial/SerialPort.h>
#include <string.h>

using namespace LibSerial;

class Driver{

private:
    std::shared_ptr<SerialPort> _serial_port = nullptr;
    std::string _port_name;//端口名字
    BaudRate _bps;//波特率

private:

    int recv(unsigned char &charBuffer, size_t msTimeout = 0) {
        /*try检测语句块有没有异常。如果没有发生异常,就检测不到。
        如果发生异常，則交给 catch 处理，执行 catch 中的语句* */
        try {
        /*从串口读取一个数据,指定msTimeout时长内,没有收到数据，抛出异常。
        如果msTimeout为0，则该方法将阻塞，直到数据可用为止。*/
        _serial_port->ReadByte(charBuffer, msTimeout);//可能出现异常的代码段
        } catch (const ReadTimeout &){ //catch捕获并处理 try 检测到的异常。
            std::cerr << "The ReadByte() call has timed out." << std::endl;
            return -2;
        } catch (const NotOpen &){ //catch()中指明了当前 catch 可以处理的异常类型
            std::cerr << "Port Not Open ..." << std::endl;
            return -1;
        }
        return 0;
    };

    int send(unsigned char charbuffer){

        //try检测语句块有没有异常
        try {
        _serial_port->WriteByte(charbuffer);//写数据到串口
        } catch (const std::runtime_error &){ //catch捕获并处理 try 检测到的异常。
        std::cerr << "The Write() runtime_error." << std::endl;
        return -2;
        } catch (const NotOpen &){ //catch捕获并处理 try 检测到的异常。
        std::cerr << "Port Not Open ..." << std::endl;
        return -1;
        }
        _serial_port->DrainWriteBuffer();//等待，直到写缓冲区耗尽，然后返回。
        return 0;
    }


public:
    //定义构造函数
    Driver(const std::string &port_name, BaudRate bps): _port_name(port_name), _bps(bps){};
    //定义析构函数
    ~Driver() { close(); };

public:
    int open(){
        _serial_port = std::make_shared<SerialPort>();
        if (_serial_port == nullptr) {
        std::cerr << "Serial Create Failed ." << std::endl;
        return -1;
        } 
        //try检测语句块有没有异常
        try {
        _serial_port->Open(_port_name);//打开串口
        _serial_port->SetBaudRate(_bps);//设置波特率
        _serial_port->SetCharacterSize(CharacterSize::CHAR_SIZE_8);//8位数据位
        _serial_port->SetFlowControl(FlowControl::FLOW_CONTROL_NONE);//设置流控
        _serial_port->SetParity(Parity::PARITY_NONE);//无校验
        _serial_port->SetStopBits(StopBits::STOP_BITS_1);//1个停止位
        } catch (const OpenFailed &){ //catch捕获并处理 try 检测到的异常。
        std::cerr << "Serial port: " << _port_name << "open failed ..."
                    << std::endl;
        return -2;
        } catch (const AlreadyOpen &){ //catch捕获并处理 try 检测到的异常。
        std::cerr << "Serial port: " << _port_name << "open failed ..."
                    << std::endl;
        return -3;
        } catch (...){ //catch捕获并处理 try 检测到的异常。
        std::cerr << "Serial port: " << _port_name << " recv exception ..."
                    << std::endl;
        return -4;
        }
        return 0;
    };


    int senddata(unsigned char charbuffer){return send(charbuffer);}
    int recvdata(unsigned char &charBuffer, size_t msTimeout){return recv(charBuffer,msTimeout);}

        
    void close() {
        if (_serial_port != nullptr) {
        /*关闭串口。串口的所有设置将会丢失，并且不能在串口上执行更多的I/O操作。*/
        _serial_port->Close();
        _serial_port = nullptr;
        }
    };





};

