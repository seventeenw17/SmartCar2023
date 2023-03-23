#include "core/detection.hpp"
#include "util/json.hpp"
#include <iostream>
// 设置opencv显示窗口
std::string rgb_window_name = "RGB";
std::string gray_window_name = "Gray";

int display_window_init() {
    cv::namedWindow(rgb_window_name, WINDOW_NORMAL);
    cv::resizeWindow(rgb_window_name, 640, 480);
    cv::moveWindow(rgb_window_name, 0, 0);

    cv::namedWindow(gray_window_name, WINDOW_NORMAL);
    cv::moveWindow(gray_window_name, 800, 0);
    cv::resizeWindow(gray_window_name, 640, 480);
}

int main(int argc, char const *argv[]) {
    // 命令行传入模型目录
    std::string model_path;
    if (argc < 2) {
        std::cout << "Please Input ./palator_car  model_path. " << std::endl;
        return -1;
    } else {
        model_path = argv[1];
        std::cout << "Model Path :" << model_path << std::endl;
    }
    // 实例化检测器，并初始化
    // 对于这种非单例方式的Detection启动，需要三步：1. 初始化类；2.init；3.start线程
    std::shared_ptr<Detection> detection = std::make_shared<Detection>(true);
    if (detection == nullptr) {
        std::cout << "Error : create detection failed." << std::endl;
        return -1;
    }
    int ret = detection->init(0, model_path);
    if (ret != 0) {
        std::cout << "Detection init failed. " << std::endl;
        return -1;
    }
    detection->start();
    // 单例模式启动只要一个API调用即可
    // std::shared_ptr<Detection> detection = Detection::DetectionInstance(0, model_path);

    display_window_init();

    while (1) {
        std::shared_ptr<DetectionResult> result = detection->getLastFrame();

        cv::imshow(rgb_window_name, result->rgb_frame);
        cv::imshow(gray_window_name, result->det_render_frame);
        cv::waitKeyEx(1);
    }

    return 0;
}
