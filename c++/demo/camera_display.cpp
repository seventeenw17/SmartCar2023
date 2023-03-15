#include <fstream>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char *argv[]) {
    /*注意：
      使用 0 和 /dev/video0 的分辨率不同：
        0           : opencv 内部的采集，可能是基于 V4L2, 分辨率：1280 * 960
        /dev/video0 : 基于Gstreamer ， 分辨率：640 * 480
    */
    VideoCapture capture(0);
    if (!capture.isOpened()) {
        std::cout << "can not open video device "  << std::endl;
        return 1;
    }

    double rate = capture.get(CAP_PROP_FPS);
    double width = capture.get(CAP_PROP_FRAME_WIDTH);
    double height = capture.get(CAP_PROP_FRAME_HEIGHT);
    std::cout << "Camera Param: frame rate = " << rate << " width = " << width
              << " height = " << height << std::endl;

    std::string window_name = "usbcamera";
    namedWindow(window_name, WINDOW_NORMAL);


    int width_ = 640;
    int height_ = 480;

    Mat frame;
    while (1) {
        if (!capture.read(frame)) {
            std::cout << "no video frame" << std::endl;
            continue;
        }


        imshow(window_name, frame);
        waitKey(1);
    }
    capture.release();
}