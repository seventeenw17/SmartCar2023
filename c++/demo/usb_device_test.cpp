#include "core/uart.hpp"
#include <fstream>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;

int main(int argc, char *argv[]) {

    std::shared_ptr<Driver> driver =
            std::make_shared<Driver>("/dev/ttyUSB0", BaudRate::BAUD_115200,
                                     false, true);
    if (driver == nullptr) {
        std::cout << "Create Driver Error , tell 15201197364 ask hcy." << std::endl;
        return -1;
    }

    int ret = driver->open();
    if (ret != 0) {
        std::cout << "Driver Open failed , tell 15201197364 ask hcy." << std::endl;
        return -1;
    }

    VideoCapture capture(0);
    if (!capture.isOpened()) {
        std::cout << "can not open video device " << std::endl;
        return 1;
    }

    double rate = capture.get(CAP_PROP_FPS);
    double width = capture.get(CAP_PROP_FRAME_WIDTH);
    double height = capture.get(CAP_PROP_FRAME_HEIGHT);
    std::cout << "Camera Param: frame rate = " << rate << " width = " << width
              << " height = " << height << std::endl;

    std::string window_name = "usbcamera";
    namedWindow(window_name, WINDOW_NORMAL);

    if ((height != 480) && (width != 640) && (rate != 30)) {
        std::cout << "Camera Param is Error, tell 15201197364 ask hcy."
                  << std::endl;
        return -1;
    }
    int width_ = 640;
    int height_ = 480;

    Mat frame;
    double steer_ = -3;

    while (1) {
        if (!capture.read(frame)) {
            std::cout << "Read Camera Error, tell 15201197364 ask hcy. " << std::endl;
            return -1;
        }
        Mat displayMat;
        resize(frame, displayMat, Size(width_, height_));
        imshow(window_name, displayMat);
        waitKey(1);
        driver->steer(steer_);
        steer_ = -steer_;
    }
    capture.release();
}