#pragma once

#include "core/capture.hpp"
#include "core/predictor.hpp"
#include "core/preprocess.hpp"
#include "util/mat_util.hpp"
#include "util/stop_watch.hpp"
#include <condition_variable>
#include <mutex>
#include <thread>
#include <exception>
#include "core/frame_save.hpp"

struct DetectionResult {
    cv::Mat det_render_frame; // 画了框的图
    cv::Mat rgb_frame;        // 原图
    std::vector<PredictResult> predictor_results; // 预测结果
};
// 对检测模型的封装类，通过一个单独线程完成从摄像头读图，到模型推理
class Detection {

public:
    Detection(bool logEn = false) : _log_en(logEn) {}
    ~Detection() {}
    // 初始化, 传入相机结点和模型路径
    int init(std::string file_path, std::string model_config_path) {
        _is_file = true;
        _file_path = file_path;
        _capture = std::make_shared<Capture>();
        if (_capture == nullptr) {
            std::cout << "Capture create failed." << std::endl;
            return -1;
        }
        int ret = _capture->open(file_path);
        if (ret != 0) {
            std::cout << "Capture open failed." << std::endl;
            return -1;
        }
        return _init(model_config_path);
    };

    int init(int camera_index, std::string model_config_path) {
        _capture = std::make_shared<Capture>();
        if (_capture == nullptr) {
            std::cout << "Capture create failed." << std::endl;
            return -1;
        }
        int ret = _capture->open(camera_index);
        if (ret != 0) {
            std::cout << "Capture open failed." << std::endl;
            return -1;
        }
        return _init(model_config_path);
    };
    void start() {
        _thread = std::make_unique<std::thread>([this]() {
            while (1) {
                std::shared_ptr<DetectionResult> result =
                        std::make_shared<DetectionResult>();

                StopWatch stop_watch_capture;
                stop_watch_capture.tic();
                result->rgb_frame = _capture->read();
                //reopen file
                if (result->rgb_frame.empty() && _is_file) {
                    _capture->close();
                    int ret = _capture->open(_file_path);
                    if (ret != 0) {
                        std::cout << "Capture open failed." << std::endl;
                        return -1;
                    }
                    continue;
                }

                double capture_times = stop_watch_capture.toc();

                if (result->rgb_frame.empty()) {
                    std::cout << "Error: Capture Get Empty Error Frame." << std::endl;
                    exit(-1);
                }
                result->predictor_results = _predictor->run(result->rgb_frame);
                result->det_render_frame = result->rgb_frame.clone();
                _predictor->render(result->det_render_frame, result->predictor_results);
                if (_log_en) {
                    _predictor->printer(result->rgb_frame, result->predictor_results);
                }

                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    _lastResult = result;
                    cond_.notify_all();
                }

            }
        });
    }
    int stop() { return 0; }
    int deinit() { return 0; }
    // 外部获取推理结果的接口函数
    std::shared_ptr<DetectionResult> getLastFrame() {
        std::shared_ptr<DetectionResult> ret = nullptr;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            // 临界区内，等待预测线程的结果，等待条件变量
            while (_lastResult == nullptr) {
                cond_.wait(lock);
            }
            // 拿到当前最新结果帧，返回
            ret = _lastResult;
            _lastResult = nullptr;
        }
        return ret;
    }

    std::string getLabel(int type) { return _predictor->getLabel(type); }

public:
    // 单例模式，避免被复制，用户可自行选择是否使用单例
    // 用户可以用此API调用模型推理，传入的是模型目录，具体目录的配置方式，详见ReadMe文档
    static std::shared_ptr<Detection> DetectionInstance(std::string file_path, std::string model_path) {
        static std::shared_ptr<Detection> detectioner = nullptr;
        if (detectioner == nullptr) {
            detectioner = std::make_shared<Detection>();

            int ret = detectioner->init(file_path, model_path);
            if (ret != 0) {
                std::cout << "Detection init error :" << model_path << std::endl;
                exit(-1);
            }
            // Detection类实例化过程中, 启动预测器，完成模型加载并开始异步推理
            // 用户通过调用getLastFrame获取最新一帧的推理结果
            // 如果用户可以设计一个轻巧的模型，使得一帧推理的时间小于相机取图周期，那么异步的方式可以有效的避免神经网络处理带来的延迟
            // 进而方便提升小车速度
            detectioner->start();
        }
        return detectioner;
    }

    // 同上一个API, 默认模型路径是"../../res/model/mobilenet-ssd/"
    // 不建议使用这个API
    static std::shared_ptr<Detection> DetectionInstance() {
        static std::shared_ptr<Detection> detectioner = nullptr;
        if (detectioner == nullptr) {
            detectioner = std::make_shared<Detection>();
            std::string model_path = "../../res/model/mobilenet-ssd/";
            int ret = detectioner->init(0, model_path);
            if (ret != 0) {
                std::cout << "Detection init error :" << model_path << std::endl;
                exit(-1);
            }
            detectioner->start();
        }
        return detectioner;
    }

private:
    int _init(std::string model_config_path) {
        _predictor = std::make_shared<Predictor>(model_config_path);
        if (_predictor == nullptr) {
            std::cout << "Predictor Create failed." << std::endl;
            return -1;
        }
        int ret = _predictor->init();
        if (ret != 0) {
            std::cout << "Predictor init failed." << std::endl;
            exit(-1);
        }

        std::shared_ptr<DetectionResult> result =
                std::make_shared<DetectionResult>();

        result->rgb_frame = _capture->read();

        if (result->rgb_frame.empty()) {
            std::cout << "Error: Capture Get Empty Error Frame." << std::endl;
            exit(-1);
        }

        try {
            _predictor->run(result->rgb_frame);
        }
        catch(exception& e) {
            std::cout << "Predictor optimize failed with " << e.what() << std::endl;
            exit(-1);
        }

        return 0;
    }

private:
    bool _is_file = false;
    std::string _file_path;
    bool _log_en;
    std::shared_ptr<DetectionResult> _lastResult;

    std::mutex _mutex;
    std::condition_variable cond_;

    std::unique_ptr<std::thread> _thread;

    std::shared_ptr<Capture> _capture;
    std::shared_ptr<Predictor> _predictor;
};