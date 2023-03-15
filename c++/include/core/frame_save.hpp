#pragma once
#include <thread>
#include <opencv2/core.hpp> // Basic OpenCV structures (cv::Mat)
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include "core/capture.hpp"
// 保存灰度图片的接口类，方便调试
class ImageSaver {
public:
    std::string save_path_;
    std::mutex _mutex;
    std::condition_variable cond_;
    bool last_frame_ = false;
    Mat cached_frame_;
    std::shared_ptr<std::thread> _thread;

    ImageSaver(std::string save_path = "./"): save_path_(save_path) {
        start();
    }

    ~ImageSaver() {}
    void start() {
        _thread = std::make_unique<std::thread>([this]() {
            int counter = 0;
            while(1) {
                Mat frame;
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    while(!last_frame_) {
                        cond_.wait(lock);
                    }
                    frame = cached_frame_;
                    last_frame_ = false;
                }
                Mat imageGray;
                // cv::cvtColor(frame, imageGray, COLOR_BGR2GRAY);
                auto startTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
                cv::imwrite(save_path_ + std::to_string(counter) + ".jpg", frame);
                auto curTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
                cout  << "imwrite cost:" << curTime - startTime << endl;
                ++counter;
            }
        });
    }
    // 外部调用接口，在需要保存图片的地方调用这个接口即可。
    // 图片保存功能实现在一个单独的线程中
    void PutFrame(Mat frame) {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            cached_frame_ = frame.clone();
            last_frame_ = true;
            cond_.notify_all();
        }
    }

};