#include <iostream>
#include <opencv2/core.hpp> // Basic OpenCV structures (cv::Mat)
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <stdint.h>
#include <fstream>

int main(int argc, char const *argv[]) {

    if (argc < 1) {
        std::cout << "Please Input ./rgb_mat2gray_array  xxxx.jpg. ";
        return -1;
    }
    std::string img_path = argv[1];

    //读取一张图片
    cv::Mat im_rgb = cv::imread(img_path);
    if (im_rgb.empty()) {
        std::cout << "Error, imread img:" << img_path << " failed." << std::endl;
        return -1;
    }

    //转化为灰度图片
    cv::Mat im_gray;
    cv::cvtColor(im_rgb, im_gray, cv::COLOR_RGB2GRAY);

    std::cout << "rgb size: " << im_rgb.size() << " --> " << im_rgb.cols << " x "
              << im_rgb.rows << " x " << im_rgb.elemSize()
              << " channels: " << im_rgb.channels() << " depth:" << im_rgb.depth()
              << std::endl;
    std::cout << "gray size: " << im_gray.size() << " --> " << im_gray.cols
              << " x " << im_gray.rows << " x " << im_gray.elemSize()
              << " channels: " << im_gray.channels()
              << " depth:" << im_gray.depth() << std::endl;

    cv::Mat frame = im_gray.isContinuous() ? im_gray : im_gray.clone();
    if (!frame.isContinuous()) {
        std::cout << "Error frame is not continuous ..." << std::endl;
        return -1;
    }
    cv::imwrite("gray.jpg", frame);

#if 1
    std::ofstream outFile("gray-array", std::ios::out | std::ios::binary);
    outFile.write((char *)frame.data, frame.cols * frame.rows);
    outFile.close();
#endif
    return 0;
}