#pragma once
#include <memory>
#include <opencv2/opencv.hpp>
// 相机功能的封装类
class Capture {
private:
    std::shared_ptr<cv::VideoCapture> _capture;
    std::string path;
    int dev_index;
    bool _isOpend = false;

public:
    // 打开相机
    int open(int dev_index) {
        _capture = std::make_shared<cv::VideoCapture>(dev_index);
        return _open();
    }
    int open(std::string path) {
        _capture = std::make_shared<cv::VideoCapture>(path);
        return _open();
    };
    // 相机状态查询
    bool is_open() { return _isOpend; }
    // 读相机结果
    cv::Mat read() {
        cv::Mat frame;
        if (is_open()) {
            _capture->read(frame);
        }
        return frame;
    };
    // 关闭相机
    void close() {
        _isOpend = false;
        _capture->release();
    }

    Capture(){};
    ~Capture(){};

private:
    int _open() {
        if ((_capture == nullptr) || (!_capture->isOpened())) {
            std::cout << "Video Capture create failed. " << std::endl;
            return -1;
        }
        _isOpend = true;
#if 1
        double rate = _capture->get(cv::CAP_PROP_FPS);
        double width = _capture->get(cv::CAP_PROP_FRAME_WIDTH);
        double height = _capture->get(cv::CAP_PROP_FRAME_HEIGHT);
        std::cout << "Camera Param: frame rate = " << rate << " width = " << width
                  << " height = " << height << std::endl;
#endif
        return 0;
    }
};